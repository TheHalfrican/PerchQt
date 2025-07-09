#include <QLineEdit>
#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include <QDialogButtonBox>
#include <QStringList>
#include <QFileDialog>
#include <QListWidget>
#include <QSettings>
SettingsDialog::SettingsDialog(QWidget* p)
  : QDialog(p), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    // Save settings and accept
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        QSettings settings("PerchOrg", "PerchQt");
        // Persist emulator path
        settings.setValue("emulatorPath", ui->emu_edit->text());
        // Persist scan folders
        settings.setValue("scanFolders", scanFolders());
        accept();
    });
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