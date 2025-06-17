#ifndef PERCHQT_GAMEWIDGETVIEW_H
#define PERCHQT_GAMEWIDGETVIEW_H

#include <QWidget>

// Forward declarations
class QLabel;
class QVBoxLayout;

class GameWidgetView : public QWidget {
    Q_OBJECT

public:
    explicit GameWidgetView(QWidget* parent = nullptr);

    // Set the displayed game title
    void setTitle(const QString& title);
    // Set the displayed cover art image
    void setCoverArt(const QString& coverArtPath);

private:
    QLabel*      m_coverArtLabel{nullptr};
    QLabel*      m_titleLabel{nullptr};
    QVBoxLayout* m_layout{nullptr};
};

#endif // PERCHQT_GAMEWIDGETVIEW_H
