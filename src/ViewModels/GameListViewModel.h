#ifndef PERCHQT_GAMELISTVIEWMODEL_H
#define PERCHQT_GAMELISTVIEWMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QModelIndex>
#include "Models/Game.h"

// Forward declarations
class GameListModel;

class GameListViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QAbstractListModel* gameListModel READ gameListModel CONSTANT)

public:
    explicit GameListViewModel(QObject* parent = nullptr);
    ~GameListViewModel() override;

    // Expose the underlying model to the view
    QAbstractListModel* gameListModel() const;

    // Load games from config or data source
    Q_INVOKABLE void loadGames();

    // Handle selection from the view
    Q_INVOKABLE void onGameSelected(const QModelIndex& index);

signals:
    // Emitted when a game should be launched
    void gameLaunched(const Game& game);

private:
    GameListModel* m_model{nullptr};
};

#endif // PERCHQT_GAMELISTVIEWMODEL_H
