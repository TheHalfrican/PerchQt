

#ifndef WIDGETS_MARQUEELABEL_H
#define WIDGETS_MARQUEELABEL_H

#include <QLabel>
#include <QTimer>
#include <QPaintEvent>

/**
 * A QLabel subclass that scrolls its text horizontally like a marquee
 * when the text width exceeds the label’s visible width.
 */
class MarqueeLabel : public QLabel {
    Q_OBJECT

public:
    explicit MarqueeLabel(QWidget* parent = nullptr);
    explicit MarqueeLabel(const QString& text, QWidget* parent = nullptr);
    ~MarqueeLabel() override;

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void scrollText();

private:
    QTimer* m_timer;    // timer driving the scroll
    int     m_offset;   // current horizontal offset
    int     m_step;     // pixels to move per timeout
    int     m_pause;    // pause counter when at ends
};

#endif // WIDGETS_MARQUEELABEL_H