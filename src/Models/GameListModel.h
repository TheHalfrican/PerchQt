

#ifndef PERCHQT_GAMELISTMODEL_H
#define PERCHQT_GAMELISTMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <QHash>
#include <QByteArray>
#include <QVariant>
#include <QModelIndex>

#include "Models/Game.h"

class GameListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum GameRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        FilePathRole,
        CoverPathRole,
        LastPlayedRole,
        PlayCountRole
    };

    explicit GameListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int,QByteArray> roleNames() const override;

    void setGames(const QVector<Game>& games);
    Game gameAt(int row) const;

private:
    QVector<Game> m_games;
};

#endif // PERCHQT_GAMELISTMODEL_H