#include "Views/GameWidgetView.h"
#include "ui_GameWidgetView.h"
#include <QPixmap>
#include <QMenu>
#include <QContextMenuEvent>
#include "Models/Game.h"

GameWidgetView::GameWidgetView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::GameWidgetView)
{
    ui->setupUi(this);
}

GameWidgetView::~GameWidgetView()
{
    delete ui;
}

void GameWidgetView::setGame(const Game& game)
{
    m_game = game;

    // Set the title text
    ui->titleLabel->setText(game.title);

    // Load & scale the cover image
    QPixmap pix;
    if (pix.load(game.coverPath)) {
        ui->coverLabel->setPixmap(
            pix.scaled(
                ui->coverLabel->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            )
        );
    } else {
        ui->coverLabel->setText("(no image)");
    }
}

void GameWidgetView::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    QAction* removeAction = menu.addAction("Remove Game");
    connect(removeAction, &QAction::triggered, this, [this]() {
        emit removeRequested(m_game.id);
    });
    menu.exec(event->globalPos());
}