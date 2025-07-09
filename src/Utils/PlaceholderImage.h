// src/Utils/PlaceholderImage.h
#ifndef PLACEHOLDERIMAGE_H
#define PLACEHOLDERIMAGE_H

#include <QPixmap>
#include <QColor>
#include <QPainter>
#include <QFont>
#include <QRect>
#include <QString>
#include <QtGlobal> // for qMin
#include <Qt>

class PlaceholderImage {
public:
    /**
     * Generate a placeholder pixmap with indigo background and centered instruction text.
     * Aspect ratio fixed at 2:3 (width:height).
     * @param width logical width in pixels
     * @param dpr device pixel ratio (defaults to 1.0)
     * @return QPixmap ready to set on a label (devicePixelRatio set)
     */
    static QPixmap generate(int width, qreal dpr = 1.0) {
        // Compute logical size at 2:3 aspect ratio
        int w = width;
        int h = (w * 3) / 2;
        // Create pixmap at logical size
        QPixmap pixmap(w, h);
        pixmap.fill(QColor(75, 0, 130));

        QPainter painter(&pixmap);
        painter.setPen(Qt::white);

        // Padding: 5% horizontal, 10% vertical
        int paddingX = static_cast<int>(w * 0.05);
        int paddingY = static_cast<int>(h * 0.1);

        // Set font size relative to smaller dimension
        QFont font = painter.font();
        int fontSize = static_cast<int>(qMin(w, h) / 12.0);
        font.setPointSize(fontSize);
        font.setBold(true);
        painter.setFont(font);

        // Draw wrapped, centered text
        QString instrText = "(Right-click to set\nCover Art)";
        QRect textRect(paddingX, paddingY, w - 2 * paddingX, h - 2 * paddingY);
        painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, instrText);
        painter.end();

        return pixmap;
    }
};

#endif // PLACEHOLDERIMAGE_H
