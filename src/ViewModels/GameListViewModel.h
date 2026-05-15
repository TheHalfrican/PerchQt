#ifndef PERCHQT_GAMELISTVIEWMODEL_H
#define PERCHQT_GAMELISTVIEWMODEL_H

#include <QObject>
#include <QVector>
#include <QString>
#include "Models/Game.h"

class GameListViewModel : public QObject {
    Q_OBJECT

public:
    explicit GameListViewModel(QObject* parent = nullptr);
    ~GameListViewModel() override;

    // Load games from the database and emit gamesChanged.
    Q_INVOKABLE void loadGames();

    // Add a new game to the database.
    Q_INVOKABLE void addGame(const QString& title,
                             const QString& filePath,
                             const QString& coverPath);

    // Remove a game from the database.
    Q_INVOKABLE void removeGame(int gameId);

    // Launch the game executable by ID. Bumps play_count and last_played on success.
    Q_INVOKABLE void launchGame(int gameId);

    // Show the game file in the system file browser.
    Q_INVOKABLE void showGameFile(int gameId);

    // Set a custom cover image for the game.
    Q_INVOKABLE void setCoverImage(int gameId, const QString& coverPath);

    // Remove the cover image for the game.
    Q_INVOKABLE void removeCoverImage(int gameId);

    // Scan a directory (and its subdirectories) for executables and add them.
    Q_INVOKABLE void scanFolder(const QString& folderPath);

signals:
    void gameLaunched(const Game& game);
    void gamesChanged(const QVector<Game>& games);
    void statusMessage(const QString& message);

private:
    // Insert a single row without reloading. Returns true if a row was inserted.
    bool insertGame(const QString& title,
                    const QString& filePath,
                    const QString& coverPath);
};

#endif // PERCHQT_GAMELISTVIEWMODEL_H
