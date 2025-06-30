#include "Views/GameWidgetView.h"
#include "ui_GameWidgetView.h"
#include <QPixmap>
#include <QMenu>
#include <QContextMenuEvent>
#include <QAction>
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

    QAction* launchAction = menu.addAction("Launch Game");
    connect(launchAction, &QAction::triggered, this, [this]() {
        emit launchRequested(m_game.id);
    });

    QAction* showAction = menu.addAction("Show File in Browser");
    connect(showAction, &QAction::triggered, this, [this]() {
        emit showFileRequested(m_game.id);
    });

    QAction* setCoverAction = menu.addAction("Set Cover Image...");
    connect(setCoverAction, &QAction::triggered, this, [this]() {
        emit setCoverRequested(m_game.id);
    });

    QAction* removeCoverAction = menu.addAction("Remove Cover Image");
    connect(removeCoverAction, &QAction::triggered, this, [this]() {
        emit removeCoverRequested(m_game.id);
    });

    QAction* removeAction = menu.addAction("Remove Game");
    connect(removeAction, &QAction::triggered, this, [this]() {
        emit removeRequested(m_game.id);
    });
    menu.exec(event->globalPos());
}