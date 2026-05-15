#include "ViewModels/GameListViewModel.h"
#include "Models/Game.h"

#include <QVector>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QVariant>
#include <QDebug>
#include <QtCore/QDateTime>
#include <QtCore/QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QFileInfo>
#include <QDirIterator>
#include <QSettings>
#include <algorithm>

GameListViewModel::GameListViewModel(QObject* parent)
    : QObject(parent)
{
}

GameListViewModel::~GameListViewModel() = default;

void GameListViewModel::loadGames()
{
    QVector<Game> games;

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

    while (query.next()) {
        Game g(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString(),
            query.value(4).toString(),
            query.value(5).toInt()
        );
        games.append(g);
    }

    std::sort(games.begin(), games.end(),
              [](const Game& a, const Game& b) {
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
    QSqlQuery query(QSqlDatabase::database());
    query.prepare("DELETE FROM games WHERE id = ?");
    query.addBindValue(gameId);
    if (!query.exec()) {
        qWarning() << "Failed to remove game with ID" << gameId
                   << ":" << query.lastError().text();
        return;
    }
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
    static const QStringList allowedExt = { "iso", "xex", "stfs" };

    QSqlDatabase db = QSqlDatabase::database();
    const bool inTransaction = db.transaction();
    if (!inTransaction)
        qWarning() << "scanFolder: failed to begin transaction:" << db.lastError().text();

    int inserted = 0;
    QDirIterator it(folderPath,
                    QDir::Files | QDir::NoSymLinks,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fi(filePath);
        if (!allowedExt.contains(fi.suffix().toLower()))
            continue;
        if (insertGame(fi.baseName(), filePath, QString()))
            ++inserted;
    }

    if (inTransaction && !db.commit()) {
        qWarning() << "scanFolder: commit failed:" << db.lastError().text();
        db.rollback();
        emit statusMessage(tr("Scan of %1 failed").arg(folderPath));
        return;
    }

    emit statusMessage(inserted > 0
        ? tr("Added %1 game(s) from %2").arg(inserted).arg(folderPath)
        : tr("No new games found in %1").arg(folderPath));

    if (inserted > 0)
        loadGames();
}
