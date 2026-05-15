#include "ViewModels/GameListViewModel.h"
#include "Models/Game.h"

#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <QUrl>
#include <QVariant>
#include <QVector>
#include <QtConcurrent/QtConcurrentRun>
#include <algorithm>

GameListViewModel::GameListViewModel(QObject* parent)
    : QObject(parent)
{
}

GameListViewModel::~GameListViewModel() = default;

void GameListViewModel::loadGames()
{
    QVector<Game> games;

    // 1) Use the existing default database connection
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        qWarning() << "Database connection is invalid or not open";
        return;
    }

    QSqlQuery query(db);
    if (!query.exec(R"(
        SELECT id, title, file_path, cover_path, last_played, play_count
          FROM games
    )")) {
        qWarning() << "Query failed:" << query.lastError().text();
        return;
    }

    // 3) Populate vector from query results
    while (query.next()) {
        Game g(
            query.value(0).toInt(),         // id
            query.value(1).toString(),      // title
            query.value(2).toString(),      // filePath
            query.value(3).toString(),      // coverPath
            query.value(4).toString(),      // lastPlayed
            query.value(5).toInt()          // playCount
        );
        games.append(g);
    }

    // Sort games alphabetically by title (case-insensitive)
    std::sort(games.begin(), games.end(),
              [](const Game &a, const Game &b) {
                  return a.title.toLower() < b.title.toLower();
              });

    emit gamesChanged(games);
}

bool GameListViewModel::insertGame(const QString& title,
                                   const QString& filePath,
                                   const QString& coverPath)
{
    QSqlQuery insert(QSqlDatabase::database());
    insert.prepare(R"(
        INSERT OR IGNORE INTO games(title, file_path, cover_path, last_played, play_count)
             VALUES(:title, :file_path, :cover_path, :last_played, :play_count)
    )");
    insert.bindValue(":title", title);
    insert.bindValue(":file_path", filePath);
    insert.bindValue(":cover_path", coverPath);
    insert.bindValue(":last_played", QString());
    insert.bindValue(":play_count", 0);
    if (!insert.exec()) {
        qWarning() << "Failed to insert game:" << insert.lastError().text();
        return false;
    }
    return insert.numRowsAffected() > 0;
}

void GameListViewModel::addGame(const QString& title,
                                const QString& filePath,
                                const QString& coverPath)
{
    if (insertGame(title, filePath, coverPath)) {
        emit statusMessage(tr("Added: %1").arg(title));
        loadGames();
    } else {
        emit statusMessage(tr("Already in library: %1").arg(title));
    }
}

void GameListViewModel::removeGame(int gameId)
{
    // Use the existing default database connection
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("DELETE FROM games WHERE id = ?");
    query.addBindValue(gameId);
    if (!query.exec()) {
        qWarning() << "Failed to remove game with ID" << gameId
                   << ":" << query.lastError().text();
        return;
    }
    // Reload model to reflect deletion
    loadGames();
}

void GameListViewModel::launchGame(int gameId)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("SELECT file_path FROM games WHERE id = ?");
    query.addBindValue(gameId);
    if (!query.exec() || !query.next()) {
        qWarning() << "Failed to retrieve file path for game ID" << gameId
                   << ":" << query.lastError().text();
        return;
    }
    QString gamePath = query.value(0).toString();

    QSettings settings;
    QString emulator = settings.value("emulatorPath").toString();
    bool ok = !emulator.isEmpty()
        ? QProcess::startDetached(emulator, QStringList{ gamePath })
        : QProcess::startDetached(gamePath);
    if (!ok) {
        qWarning() << "Failed to launch"
                   << (emulator.isEmpty() ? "game" : "emulator")
                   << (emulator.isEmpty() ? gamePath : emulator);
        return;
    }

    // Bump play_count and last_played now that the launch succeeded.
    const QString lastPlayed = QDateTime::currentDateTime().toString(Qt::ISODate);
    QSqlQuery update(db);
    update.prepare(R"(
        UPDATE games
           SET last_played = :lastPlayed,
               play_count  = play_count + 1
         WHERE id          = :id
    )");
    update.bindValue(":lastPlayed", lastPlayed);
    update.bindValue(":id", gameId);
    if (!update.exec()) {
        qWarning() << "Failed to update play stats for game ID" << gameId
                   << ":" << update.lastError().text();
        return;
    }

    loadGames();
}

void GameListViewModel::showGameFile(int gameId)
{
    QSqlQuery query(QSqlDatabase::database());
    query.prepare("SELECT file_path FROM games WHERE id = ?");
    query.addBindValue(gameId);
    if (!query.exec() || !query.next()) {
        qWarning() << "Failed to retrieve file path for game ID" << gameId
                   << ":" << query.lastError().text();
        return;
    }
    QString path = query.value(0).toString();
    QFileInfo fi(path);
    QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()));
}

void GameListViewModel::setCoverImage(int gameId, const QString& coverPath)
{
    QSqlQuery query(QSqlDatabase::database());
    query.prepare("UPDATE games SET cover_path = ? WHERE id = ?");
    query.addBindValue(coverPath);
    query.addBindValue(gameId);
    if (!query.exec()) {
        qWarning() << "Failed to set cover image for game ID" << gameId
                   << ":" << query.lastError().text();
        return;
    }
    loadGames();
}

void GameListViewModel::removeCoverImage(int gameId)
{
    QSqlQuery query(QSqlDatabase::database());
    query.prepare("UPDATE games SET cover_path = '' WHERE id = ?");
    query.addBindValue(gameId);
    if (!query.exec()) {
        qWarning() << "Failed to remove cover image for game ID" << gameId
                   << ":" << query.lastError().text();
        return;
    }
    loadGames();
}

void GameListViewModel::scanFolder(const QString& folderPath)
{
    emit statusMessage(tr("Scanning %1...").arg(folderPath));

    // Capture the default DB's filename so the worker thread can open its own connection.
    const QString dbFile = QSqlDatabase::database().databaseName();

    (void)QtConcurrent::run([this, folderPath, dbFile]() {
        static const QStringList allowedExt = { "iso", "xex", "stfs" };

        const QString connName = QStringLiteral("scan-%1")
            .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

        int inserted = 0;
        bool ok = true;
        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
            db.setDatabaseName(dbFile);
            if (!db.open()) {
                qWarning() << "scanFolder worker: failed to open" << dbFile
                           << ":" << db.lastError().text();
                ok = false;
            } else {
                db.transaction();
                QSqlQuery insert(db);
                insert.prepare(R"(
                    INSERT OR IGNORE INTO games(title, file_path, cover_path, last_played, play_count)
                         VALUES(:title, :file_path, :cover_path, '', 0)
                )");

                QDirIterator it(folderPath,
                                QDir::Files | QDir::NoSymLinks,
                                QDirIterator::Subdirectories);
                while (it.hasNext()) {
                    const QString filePath = it.next();
                    const QFileInfo fi(filePath);
                    if (!allowedExt.contains(fi.suffix().toLower())) continue;
                    insert.bindValue(":title", fi.baseName());
                    insert.bindValue(":file_path", filePath);
                    insert.bindValue(":cover_path", QString());
                    if (!insert.exec()) {
                        qWarning() << "scanFolder worker: insert failed:" << insert.lastError().text();
                        continue;
                    }
                    if (insert.numRowsAffected() > 0) ++inserted;
                }

                if (!db.commit()) {
                    qWarning() << "scanFolder worker: commit failed:" << db.lastError().text();
                    db.rollback();
                    ok = false;
                }
            }
        }
        QSqlDatabase::removeDatabase(connName);

        // Post the result back to the main thread.
        QMetaObject::invokeMethod(this, [this, folderPath, inserted, ok]() {
            if (!ok) {
                emit statusMessage(tr("Scan of %1 failed").arg(folderPath));
                return;
            }
            emit statusMessage(inserted > 0
                ? tr("Added %1 game(s) from %2").arg(inserted).arg(folderPath)
                : tr("No new games found in %1").arg(folderPath));
            if (inserted > 0) loadGames();
        }, Qt::QueuedConnection);
    });
}