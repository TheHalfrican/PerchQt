#include "Views/GameWidgetView.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>

GameWidgetView::GameWidgetView(QWidget* parent)
    : QWidget(parent)
    , m_coverArtLabel(new QLabel(this))
    , m_titleLabel(new QLabel(this))
    , m_layout(new QVBoxLayout(this))
{
    // Configure labels
    m_coverArtLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setAlignment(Qt::AlignCenter);

    // Assemble layout
    m_layout->addWidget(m_coverArtLabel);
    m_layout->addWidget(m_titleLabel);
    setLayout(m_layout);
}

void GameWidgetView::setTitle(const QString& title)
{
    m_titleLabel->setText(title);
}

void GameWidgetView::setCoverArt(const QString& coverArtPath)
{
    QPixmap pixmap(coverArtPath);
    m_coverArtLabel->setPixmap(pixmap);
}