#ifndef PERCHQT_GAME_H
#define PERCHQT_GAME_H

#include <QString>

// Represents a single game entry
struct Game {
    int     id;
    QString title;
    QString filePath;
    QString coverPath;
    QString lastPlayed;
    int     playCount;

    // Constructor matching Python PHGameModel fields
    Game(int id = 0,
         const QString& title = {},
         const QString& filePath = {},
         const QString& coverPath = {},
         const QString& lastPlayed = {},
         int playCount = 0)
        : id(id)
        , title(title)
        , filePath(filePath)
        , coverPath(coverPath)
        , lastPlayed(lastPlayed)
        , playCount(playCount)
    {}
};

#endif // PERCHQT_GAME_H