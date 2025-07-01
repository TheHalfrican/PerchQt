#pragma once
#include <QDialog>
#include <QStringList>
namespace Ui { class SettingsDialog; }
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;
    // Return the list of scan folders entered by the user
    QStringList scanFolders() const;
private:
    Ui::SettingsDialog* ui;
};