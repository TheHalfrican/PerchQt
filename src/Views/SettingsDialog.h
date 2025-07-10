#pragma once
#include <QDialog>
#include <QStringList>
#include <QString>
#include <QSettings>
namespace Ui { class SettingsDialog; }
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;
    // Return the list of scan folders entered by the user
    QStringList scanFolders() const;
    QString emulatorPath() const;
    QString selectedTheme() const;
private slots:
    void onThemeComboChanged(const QString& theme);
    void onAccepted();
    void onEditCustomTheme();
private:
    Ui::SettingsDialog* ui;
};