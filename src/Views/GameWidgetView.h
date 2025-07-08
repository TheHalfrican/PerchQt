#ifndef PERCHQT_GAMEWIDGETVIEW_H
#define PERCHQT_GAMEWIDGETVIEW_H

#include <QWidget>
#include "ui_GameWidgetView.h"
#include "Models/Game.h"
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPixmap>
#include <QLabel>

class GameWidgetView : public QWidget {
    Q_OBJECT

public:
    explicit GameWidgetView(QWidget* parent = nullptr);
    ~GameWidgetView() override;

    // Populate the view from a Game model
    void setGame(const Game& game);

    // Highlight this widget when selected
    void setSelected(bool selected);

public slots:
    // Show or hide the game title footer
    void setTitleVisible(bool visible);

signals:
    void launchRequested(int gameId);
    void showFileRequested(int gameId);
    void setCoverRequested(int gameId);
    void removeCoverRequested(int gameId);
    void removeRequested(int gameId);
    void clicked(int gameId);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    Ui::GameWidgetView* ui{nullptr};
    Game m_game;
    bool m_selected{false};
    // Store the original cover image for dynamic resizing
    QPixmap m_originalCover;
};

#endif // PERCHQT_GAMEWIDGETVIEW_H
