#include "Models/GameListModel.h"
#include <QVariant>

GameListModel::GameListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int GameListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_games.size();
}

QVariant GameListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_games.size())
        return QVariant();

    const Game& game = m_games.at(index.row());
    switch (role) {
    case IdRole:
        return game.id;
    case TitleRole:
        return game.title;
    case FilePathRole:
        return game.filePath;
    case CoverPathRole:
        return game.coverPath;
    case LastPlayedRole:
        return game.lastPlayed;
    case PlayCountRole:
        return game.playCount;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> GameListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[FilePathRole] = "filePath";
    roles[CoverPathRole] = "coverPath";
    roles[LastPlayedRole] = "lastPlayed";
    roles[PlayCountRole] = "playCount";
    return roles;
}

void GameListModel::setGames(const QVector<Game>& games)
{
    beginResetModel();
    m_games = games;
    endResetModel();
}

Game GameListModel::gameAt(int row) const
{
    if (row < 0 || row >= m_games.size())
        return Game();
    return m_games.at(row);
}
