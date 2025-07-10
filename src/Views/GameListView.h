#ifndef PERCHQT_GAMELISTVIEW_H
#define PERCHQT_GAMELISTVIEW_H

#include <QWidget>
#include <QVector>
#include <QPoint>
#include "Models/Game.h"

namespace Ui {
class GameListView;
}

class GameListView : public QWidget {
    Q_OBJECT

public:
    explicit GameListView(QWidget* parent = nullptr);
    ~GameListView() override;

    // Populate the table with a list of games
    void setGames(const QVector<Game>& games);

public slots:
    /// Refreshes a single row’s icon when its coverPath changes
    void updateGameCover(int gameId);

signals:
    void removeRequested(int gameId);
    void launchRequested(int gameId);
    void showFileRequested(int gameId);
    void setCoverRequested(int gameId);
    void removeCoverRequested(int gameId);

private slots:
    void onCustomContextMenuRequested(const QPoint& pos);
    void onCellActivated(int row, int column);

private:
    Ui::GameListView* ui;
    QVector<Game> m_games;
};

#endif // PERCHQT_GAMELISTVIEW_H