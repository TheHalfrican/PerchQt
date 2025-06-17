

#include "ViewModels/GameListViewModel.h"
#include "Models/GameListModel.h"
#include "Models/Game.h"

#include <QVector>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QVariant>
#include <QDebug>

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

    // Retrieve the selected game and emit the launch signal
    Game game = m_model->gameAt(index.row());
    emit gameLaunched(game);
}