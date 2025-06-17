

#include "ViewModels/GameListViewModel.h"
#include "Models/GameListModel.h"
#include "Models/Game.h"

#include <QVector>

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
    // TODO: Load games from a config or data source
    QVector<Game> games;
    // Example placeholder data
    games.append(Game(
        1,
        QStringLiteral("Sample Game"),
        QStringLiteral(":/resources/sample_cover.png")
    ));

    // Populate the model
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