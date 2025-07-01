#ifndef PERCHQT_GAMELISTVIEWMODEL_H
#define PERCHQT_GAMELISTVIEWMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QModelIndex>
#include <QVector>
#include "Models/Game.h"
#include <QString>

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

    // Add a new game to the database and model
    Q_INVOKABLE void addGame(const QString& title,
                             const QString& filePath,
                             const QString& coverPath);

    // Remove a game from the database and update the model
    Q_INVOKABLE void removeGame(int gameId);

    // Launch the game executable by ID
    Q_INVOKABLE void launchGame(int gameId);

    // Show the game file in the system file browser
    Q_INVOKABLE void showGameFile(int gameId);

    // Set a custom cover image for the game
    Q_INVOKABLE void setCoverImage(int gameId, const QString& coverPath);

    // Remove the cover image for the game
    Q_INVOKABLE void removeCoverImage(int gameId);
    // Scan a directory (and its subdirectories) for executables and add them
    Q_INVOKABLE void scanFolder(const QString& folderPath);

signals:
    // Emitted when a game should be launched
    void gameLaunched(const Game& game);

    // Emitted when the list of games has been updated
    void gamesChanged(const QVector<Game>& games);

private:
    GameListModel* m_model{nullptr};
};

#endif // PERCHQT_GAMELISTVIEWMODEL_H
