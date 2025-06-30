#include "Views/GameWidgetView.h"
#include "ui_GameWidgetView.h"
#include <QPixmap>

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

void GameWidgetView::setTitle(const QString& title)
{
    ui->title_label->setText(title);
}

void GameWidgetView::setCoverArt(const QString& coverArtPath)
{
    QPixmap pixmap(coverArtPath);
    ui->cover_label->setPixmap(pixmap);
}