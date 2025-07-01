#pragma once
#include <QDialog>
namespace Ui { class SettingsDialog; }
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;
private:
    Ui::SettingsDialog* ui;
};