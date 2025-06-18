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

    // 1) Open the SQLite database
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("perch.db");  // adjust path if necessary
    if (!db.open()) {
        qWarning() << "Failed to open database:" << db.lastError().text();
        return;
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

    // 4) Update the model
    m_model->setGames(games);
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
        INSERT INTO games(title, file_path, cover_path, last_played, play_count)
             VALUES(:title, :filePath, :coverPath, :lastPlayed, :playCount)
    )");
    insert.bindValue(":title", title);
    insert.bindValue(":filePath", filePath);
    insert.bindValue(":coverPath", coverPath);
    insert.bindValue(":lastPlayed", QString());
    insert.bindValue(":playCount", 0);
    if (!insert.exec()) {
        qWarning() << "Failed to insert game:" << insert.lastError().text();
        return;
    }

    // Reload model to include the new entry
    loadGames();
}