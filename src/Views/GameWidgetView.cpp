#include "Views/GameWidgetView.h"
#include "ui_GameWidgetView.h"
#include <QPixmap>
#include <QMenu>
#include <QContextMenuEvent>
#include <QAction>
#include "Models/Game.h"
#include <QPainter>
#include <QColor>
#include <QFont>
#include <QMouseEvent>
#include <Qt>
#include <QRect>

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
    delete ui;
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
        // Adjust label height to maintain aspect ratio
        // ui->coverLabel->setFixedHeight(scaled.height());
    } else {
        // Draw a purple placeholder with instruction text at 2:3 aspect ratio
        int w = ui->coverLabel->width();
        int h = (w * 3) / 2;  // 2:3 width:height aspect
        QSize pSize(w, h);
        QPixmap placeholder(pSize);
        placeholder.fill(QColor(75, 0, 130));
        QPainter painter(&placeholder);
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(placeholder.rect(),
                         Qt::AlignCenter | Qt::TextWordWrap,
                         "(Right-click to set Cover Image)");
        painter.end();
        ui->coverLabel->setPixmap(placeholder);
        // ui->coverLabel->setFixedHeight(h);
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

void GameWidgetView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    int w = ui->coverLabel->width();
    if (!m_originalCover.isNull()) {
        // Rescale the actual cover image
        QPixmap scaled = m_originalCover.scaledToWidth(
            w, Qt::SmoothTransformation);
        ui->coverLabel->setPixmap(scaled);
    } else {
        // Redraw the placeholder at a 2:3 aspect ratio
        int h = (w * 3) / 2;
        QSize pSize(w, h);
        QPixmap placeholder(pSize);
        placeholder.fill(QColor(75, 0, 130));

        QPainter painter(&placeholder);
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setBold(true);
        painter.setFont(font);

        // Inset and wrap text
        int margin = 8;
        QRect textRect = placeholder.rect().adjusted(
            margin, margin, -margin, -margin);
        painter.drawText(textRect,
                         Qt::AlignCenter | Qt::TextWordWrap,
                         "(Right-click to set Cover Image)");
        painter.end();

        ui->coverLabel->setPixmap(placeholder);
    }
}