#include "Views/GameWidgetView.h"
#include "ui_GameWidgetView.h"
#include <QPixmap>
#include <QMenu>
#include <QContextMenuEvent>
#include <QAction>
#include "Models/Game.h"
#include <QMouseEvent>
#include <Qt>
#include <QRect>
#include <QGuiApplication>
#include <QScreen>
#include "Utils/PlaceholderImage.h"

GameWidgetView::GameWidgetView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::GameWidgetView)
{
    ui->setupUi(this);
    ui->coverLabel->setScaledContents(false);
    ui->coverLabel->setAlignment(Qt::AlignCenter);
}

GameWidgetView::~GameWidgetView()
{
    qDebug() << "GameWidgetView destroyed:" << this;
}

void GameWidgetView::setGame(const Game& game)
{
    m_game = game;

    // Set the title text
    ui->titleLabel->setText(game.title);

    // Store and display cover image, preserving aspect ratio on resize
    m_originalCover = QPixmap();
    if (m_originalCover.load(game.coverPath)) {
        // Scale to current label width
        int w = ui->coverLabel->width();
        QPixmap scaled = m_originalCover.scaledToWidth(w, Qt::SmoothTransformation);
        ui->coverLabel->setPixmap(scaled);
        ui->coverLabel->setFixedHeight((w * 3) / 2);
    } else {
        // Draw placeholder using helper
        qreal dpr = 1.0;
        if (auto screen = QGuiApplication::primaryScreen())
            dpr = screen->devicePixelRatio();
        int width = ui->coverLabel->width();
        QPixmap placeholder = PlaceholderImage::generate(width, dpr);
        ui->coverLabel->setPixmap(placeholder);
        ui->coverLabel->setFixedHeight((width * 3) / 2);
    }
}

void GameWidgetView::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu;

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

void GameWidgetView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_game.id);
        setSelected(true);
    }
    QWidget::mousePressEvent(event);
}


void GameWidgetView::setSelected(bool selected)
{
    m_selected = selected;
    if (selected) {
        this->setStyleSheet("border: 2px solid blue;");
    } else {
        this->setStyleSheet("");
    }
}

void GameWidgetView::setTitleVisible(bool visible)
{
    ui->titleLabel->setVisible(visible);
}


void GameWidgetView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    // Enforce 2:3 aspect ratio on every resize
    int w = ui->coverLabel->width();
    ui->coverLabel->setFixedHeight((w * 3) / 2);

    // Determine device pixel ratio for high-DPI scaling
    qreal dpr = 1.0;
    if (auto screen = QGuiApplication::primaryScreen())
        dpr = screen->devicePixelRatio();

    int logicalWidth = ui->coverLabel->width();
    int logicalHeight = ui->coverLabel->height();
    QSize physSize(logicalWidth * dpr, logicalHeight * dpr);

    if (!m_originalCover.isNull()) {
        // Scale original cover pixmap to physical size, preserving aspect
        QPixmap scaled = m_originalCover.scaled(
            physSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        // Inform Qt this pixmap is high-DPI
        scaled.setDevicePixelRatio(dpr);
        ui->coverLabel->setPixmap(scaled);
    } else {
        // Draw placeholder using helper
        qreal dpr = 1.0;
        if (auto screen = QGuiApplication::primaryScreen())
            dpr = screen->devicePixelRatio();
        int width = ui->coverLabel->width();
        QPixmap placeholder = PlaceholderImage::generate(width, dpr);
        ui->coverLabel->setPixmap(placeholder);
        ui->coverLabel->setFixedHeight((width * 3) / 2);
    }
}

// Handle double-click to launch the game
void GameWidgetView::mouseDoubleClickEvent(QMouseEvent* event)
{
    emit launchRequested(m_game.id);
    QWidget::mouseDoubleClickEvent(event);
}