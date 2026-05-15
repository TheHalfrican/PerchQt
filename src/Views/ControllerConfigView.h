#pragma once

#include <QDialog>

namespace Ui { class ControllerConfigView; }
class ControllerManager;

class ControllerConfigView : public QDialog {
    Q_OBJECT

public:
    explicit ControllerConfigView(ControllerManager* controllers, QWidget* parent = nullptr);
    ~ControllerConfigView() override;

private slots:
    void onBluetoothButtonClicked();
    void refreshStatus();

private:
    Ui::ControllerConfigView* ui;
    ControllerManager* m_controllers{nullptr};
};
