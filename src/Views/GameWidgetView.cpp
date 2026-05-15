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

GameWidgetView::~GameWidgetView() = default;

void GameWidgetView::setGame(const Game& game)
{
    m_game = game;
    ui->titleLabel->setText(game.title);

    m_originalCover = QPixmap();
    if (!game.coverPath.isEmpty())
        m_originalCover.load(game.coverPath);

    qreal dpr = 1.0;
    if (auto* screen = QGuiApplication::primaryScreen())
        dpr = screen->devicePixelRatio();

    const int w = ui->coverLabel->width();
    ui->coverLabel->setFixedHeight((w * 3) / 2);
    if (!m_originalCover.isNull()) {
        QPixmap scaled = m_originalCover.scaledToWidth(int(w * dpr), Qt::SmoothTransformation);
        scaled.setDevicePixelRatio(dpr);
        ui->coverLabel->setPixmap(scaled);
    } else {
        ui->coverLabel->setPixmap(PlaceholderImage::generate(w, dpr));
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
        const QColor c = palette().color(QPalette::Highlight);
        setStyleSheet(QStringLiteral("GameWidgetView { border: 2px solid %1; }").arg(c.name()));
    } else {
        setStyleSheet(QString());
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
        QPixmap scaled = m_originalCover.scaled(physSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        scaled.setDevicePixelRatio(dpr);
        ui->coverLabel->setPixmap(scaled);
    } else {
        QPixmap placeholder = PlaceholderImage::generate(logicalWidth, dpr);
        ui->coverLabel->setPixmap(placeholder);
    }
}

// Handle double-click to launch the game
void GameWidgetView::mouseDoubleClickEvent(QMouseEvent* event)
{
    emit launchRequested(m_game.id);
    QWidget::mouseDoubleClickEvent(event);
}