#ifndef PERCHQT_GAMEWIDGETVIEW_H
#define PERCHQT_GAMEWIDGETVIEW_H

#include <QWidget>
#include "ui_GameWidgetView.h"

class GameWidgetView : public QWidget {
    Q_OBJECT

public:
    explicit GameWidgetView(QWidget* parent = nullptr);
    ~GameWidgetView() override;

    void setTitle(const QString& title);
    void setCoverArt(const QString& coverArtPath);

private:
    Ui::GameWidgetView* ui{nullptr};
};

#endif // PERCHQT_GAMEWIDGETVIEW_H
