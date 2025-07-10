#pragma once

#include <QDialog>
#include <QLabel>
#include <SDL.h>

namespace Ui { class ControllerConfigView; }

class ControllerConfigView : public QDialog {
    Q_OBJECT

public:
    explicit ControllerConfigView(QWidget* parent = nullptr);
    ~ControllerConfigView() override;

private:
    Ui::ControllerConfigView* ui;
    void updateStatus();  // polls SDL2 for controller status

private slots:
    void onBluetoothButtonClicked();
};