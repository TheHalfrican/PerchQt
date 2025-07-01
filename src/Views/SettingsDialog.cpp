#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include <QDialogButtonBox>
SettingsDialog::SettingsDialog(QWidget* p)
  : QDialog(p), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    // Connect standard dialog buttons
    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);
}
SettingsDialog::~SettingsDialog() { delete ui; }