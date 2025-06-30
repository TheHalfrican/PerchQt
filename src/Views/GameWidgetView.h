#ifndef PERCHQT_GAMEWIDGETVIEW_H
#define PERCHQT_GAMEWIDGETVIEW_H

#include <QWidget>
#include "ui_GameWidgetView.h"
#include "Models/Game.h"
#include <QContextMenuEvent>

class GameWidgetView : public QWidget {
    Q_OBJECT

public:
    explicit GameWidgetView(QWidget* parent = nullptr);
    ~GameWidgetView() override;

    // Populate the view from a Game model
    void setGame(const Game& game);

signals:
    void removeRequested(int gameId);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    Ui::GameWidgetView* ui{nullptr};
    Game m_game;
};

#endif // PERCHQT_GAMEWIDGETVIEW_H
