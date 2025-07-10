#include <QPushButton>
#include <QLineEdit>
#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include <QDialogButtonBox>
#include <QStringList>
#include <QFileDialog>
#include <QListWidget>
#include <QSettings>
#include "Utils/Themes.h"
#include <QComboBox>
#include <QColorDialog>

SettingsDialog::SettingsDialog(QWidget* p)
  : QDialog(p), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    // Save settings and accept
    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &SettingsDialog::onAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    // Wire up Add Folder button
    connect(ui->add_folder_btn, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Folder");
        if (!dir.isEmpty() && ui->folder_list->findItems(dir, Qt::MatchExactly).isEmpty()) {
            ui->folder_list->addItem(dir);
        }
    });

    // Wire up Remove Folder button
    connect(ui->remove_folder_btn, &QPushButton::clicked, this, [this]() {
        auto items = ui->folder_list->selectedItems();
        for (auto* item : items) {
            delete ui->folder_list->takeItem(ui->folder_list->row(item));
        }
    });

    // Populate folder list from saved settings
    {
        QSettings settings("PerchOrg", "PerchQt");
        QStringList folders = settings.value("scanFolders").toStringList();
        for (const QString& folder : folders) {
            ui->folder_list->addItem(folder);
        }
    }

    // Populate emulator path from saved settings
    {
        QSettings settings("PerchOrg", "PerchQt");
        QString emulator = settings.value("emulatorPath").toString();
        ui->emu_edit->setText(emulator);
    }
    // Populate theme from saved settings
    {
        QSettings settings("PerchOrg", "PerchQt");
        QString theme = settings.value("Theme/CurrentTheme", "System Default").toString();
        int idx = ui->theme_combo->findText(theme);
        if (idx >= 0) ui->theme_combo->setCurrentIndex(idx);
        ui->edit_custom_btn->setEnabled(theme == "Custom");
    }
    // Enable Custom button only for Custom theme
    connect(ui->theme_combo, &QComboBox::currentTextChanged,
            this, &SettingsDialog::onThemeComboChanged);

    // Wire up Edit Custom Theme button
    connect(ui->edit_custom_btn, &QPushButton::clicked,
            this, &SettingsDialog::onEditCustomTheme);

    // Browse for emulator executable
    connect(ui->emu_browse, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(this, "Select Emulator Executable");
        if (!file.isEmpty()) {
            ui->emu_edit->setText(file);
        }
    });
}
QStringList SettingsDialog::scanFolders() const
{
    QStringList folders;
    int count = ui->folder_list->count();
    for (int i = 0; i < count; ++i) {
        folders << ui->folder_list->item(i)->text();
    }
    return folders;
}
SettingsDialog::~SettingsDialog() { delete ui; }

void SettingsDialog::onThemeComboChanged(const QString& theme) {
    ui->edit_custom_btn->setEnabled(theme == "Custom");
}

void SettingsDialog::onAccepted() {
    QSettings settings("PerchOrg", "PerchQt");
    settings.setValue("emulatorPath", ui->emu_edit->text());
    settings.setValue("scanFolders", scanFolders());
    settings.setValue("Theme/CurrentTheme", ui->theme_combo->currentText());
    Themes::applyTheme(ui->theme_combo->currentText());
    accept();
}

void SettingsDialog::onEditCustomTheme()
{
    QSettings settings("PerchOrg", "PerchQt");
    // 1) Background
    {
        QColorDialog dlg(this);
        dlg.setOption(QColorDialog::DontUseNativeDialog);
        dlg.setWindowTitle(tr("Select Background Color"));
        QColor init = QColor(settings.value("Theme/CustomBgColor", "#ffffff").toString());
        dlg.setCurrentColor(init);
        if (dlg.exec() != QDialog::Accepted) return;
        QColor bg = dlg.selectedColor();
        settings.setValue("Theme/CustomBgColor", bg.name());
    }
    // 2) Text
    {
        QColorDialog dlg(this);
        dlg.setOption(QColorDialog::DontUseNativeDialog);
        dlg.setWindowTitle(tr("Select Text Color"));
        QColor init = QColor(settings.value("Theme/CustomTextColor", "#000000").toString());
        dlg.setCurrentColor(init);
        if (dlg.exec() != QDialog::Accepted) return;
        QColor text = dlg.selectedColor();
        settings.setValue("Theme/CustomTextColor", text.name());
    }
    // 3) Accent
    {
        QColorDialog dlg(this);
        dlg.setOption(QColorDialog::DontUseNativeDialog);
        dlg.setWindowTitle(tr("Select Accent Color"));
        QColor init = QColor(settings.value("Theme/CustomAccentColor", "#0078d7").toString());
        dlg.setCurrentColor(init);
        if (dlg.exec() != QDialog::Accepted) return;
        QColor accent = dlg.selectedColor();
        settings.setValue("Theme/CustomAccentColor", accent.name());
    }

    // Switch combo to Custom and apply immediately
    ui->theme_combo->setCurrentText(tr("Custom"));
    Themes::applyTheme(Themes::Theme::Custom);
}