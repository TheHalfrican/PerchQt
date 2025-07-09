#include "ViewModels/GameListViewModel.h"
#include "Models/GameListModel.h"
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
#include <algorithm>

GameListViewModel::GameListViewModel(QObject* parent)
    : QObject(parent)
    , m_model(new GameListModel(this))
{
}

GameListViewModel::~GameListViewModel() = default;

QAbstractListModel* GameListViewModel::gameListModel() const
{
    return m_model;
}

void GameListViewModel::loadGames()
{
    QVector<Game> games;

    // 1) Use the existing default database connection
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        qWarning() << "Database connection is invalid or not open";
        return;
    }

    // Debug: list all tables in the default connection
    qDebug() << "Listing tables in default connection";
    QSqlQuery tbls(db);
    if (!tbls.exec("SELECT name FROM sqlite_master WHERE type='table'")) {
        qWarning() << "Can't list tables:" << tbls.lastError().text();
    } else {
        while (tbls.next())
            qDebug() << "  Table:" << tbls.value(0).toString();
    }

    // 2) Execute the query
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

    // 4) Update the model
    m_model->setGames(games);
    emit gamesChanged(games);
}

void GameListViewModel::onGameSelected(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    // Retrieve the selected game
    Game game = m_model->gameAt(index.row());

    // Launch the game executable
    QString executable = game.filePath;
    if (!QProcess::startDetached(executable)) {
        qWarning() << "Failed to launch game:" << executable;
        return;
    }

    // Update lastPlayed and playCount
    QDateTime now = QDateTime::currentDateTime();
    game.lastPlayed = now.toString(Qt::ISODate);
    game.playCount += 1;

    // Write update back to the database
    QSqlQuery updateQuery;
    updateQuery.prepare(R"(
        UPDATE games
           SET last_played = :lastPlayed,
               play_count   = :playCount
         WHERE id           = :id
    )");
    updateQuery.bindValue(":lastPlayed", game.lastPlayed);
    updateQuery.bindValue(":playCount", game.playCount);
    updateQuery.bindValue(":id", game.id);
    if (!updateQuery.exec()) {
        qWarning() << "Failed to update game record:" << updateQuery.lastError().text();
    }

    // Refresh the model to show updated values
    loadGames();

    // Emit the signal with the updated game
    emit gameLaunched(game);
}

void GameListViewModel::addGame(const QString& title,
                                const QString& filePath,
                                const QString& coverPath)
{
    QSqlQuery insert;
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
        return;
    }

    // Reload model to include the new entry
    loadGames();
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
    QSqlQuery query(QSqlDatabase::database());
    query.prepare("SELECT file_path FROM games WHERE id = ?");
    query.addBindValue(gameId);
    if (!query.exec() || !query.next()) {
        qWarning() << "Failed to retrieve file path for game ID" << gameId
                   << ":" << query.lastError().text();
        return;
    }
    QString path = query.value(0).toString();
    if (!QProcess::startDetached(path)) {
        qWarning() << "Failed to launch game executable:" << path;
    }
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
    QDirIterator it(folderPath,
                    QDir::Files | QDir::NoSymLinks,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fi(filePath);
        // Only consider Xenia_Canary supported image files
        const QStringList allowedExt = { "iso", "xex", "stfs" };
        QString ext = fi.suffix().toLower();
        if (!allowedExt.contains(ext))
            continue;
        QString title = fi.baseName();
        // Use empty coverPath for now
        addGame(title, filePath, QString());
    }
}