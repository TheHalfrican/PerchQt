

#include "MarqueeLabel.h"
#include <QPainter>
#include <QFontMetrics>

// Constructor with parent only
MarqueeLabel::MarqueeLabel(QWidget* parent)
    : QLabel(parent)
    , m_timer(new QTimer(this))
    , m_offset(0)
    , m_step(1)
    , m_pause(0)
{
    // Start the scroll timer
    connect(m_timer, &QTimer::timeout, this, &MarqueeLabel::scrollText);
    m_timer->start(50); // adjust interval as needed
}

// Constructor with initial text
MarqueeLabel::MarqueeLabel(const QString& text, QWidget* parent)
    : MarqueeLabel(parent)
{
    setText(text);
}

MarqueeLabel::~MarqueeLabel()
{
}

// Paint event: draw scrolling or static text
void MarqueeLabel::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setPen(palette().windowText().color());
    painter.setFont(font());

    const QString txt = text();
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(txt);
    int labelWidth = width();
    int labelHeight = height();
    int baseline = (labelHeight + fm.ascent() - fm.descent()) / 2;

    if (textWidth <= labelWidth) {
        // No need to scroll; draw centered or aligned text
        painter.drawText(rect(), alignment(), txt);
        return;
    }

    // Draw two copies for seamless scrolling
    int spacing = 20; // pixels between repeats
    int x = -m_offset;
    painter.drawText(x, baseline, txt);
    painter.drawText(x + textWidth + spacing, baseline, txt);
}

// Slot to advance scroll offset
void MarqueeLabel::scrollText()
{
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(text());
    int spacing = 20;
    int cycleLength = textWidth + spacing;

    // If no scrolling needed, skip
    if (textWidth <= width()) {
        m_offset = 0;
        return;
    }

    // Pause at end of cycle
    if (m_pause > 0) {
        --m_pause;
        return;
    }

    m_offset += m_step;
    if (m_offset > cycleLength) {
        m_offset = 0;
        m_pause = 20; // pause 20 intervals at reset
    }
    update();
}