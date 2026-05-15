#include "Views/ControllerConfigView.h"
#include "ui_ControllerConfigView.h"
#include "Input/ControllerManager.h"

#include <QProcess>
#include <QStringList>

ControllerConfigView::ControllerConfigView(ControllerManager* controllers, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ControllerConfigView)
    , m_controllers(controllers)
{
    ui->setupUi(this);
    connect(ui->bluetoothButton, &QPushButton::clicked,
            this, &ControllerConfigView::onBluetoothButtonClicked);

    if (m_controllers) {
        connect(m_controllers, &ControllerManager::controllersChanged,
                this, &ControllerConfigView::refreshStatus);
    }
    refreshStatus();
}

ControllerConfigView::~ControllerConfigView() = default;

void ControllerConfigView::refreshStatus()
{
    QStringList names = m_controllers ? m_controllers->controllerNames() : QStringList{};
    if (names.isEmpty()) {
        ui->statusLabel->setText(tr("No Controllers Connected"));
        ui->countLabel->setText(tr("Controllers Connected: 0"));
        ui->nameLabel->setText(tr("Controller Name(s): \xe2\x80\x94"));
    } else {
        ui->statusLabel->setText(tr("Controller(s) Connected"));
        ui->countLabel->setText(tr("Controllers Connected: %1").arg(names.size()));
        ui->nameLabel->setText(tr("Controller Name(s): %1").arg(names.join(", ")));
    }
}

void ControllerConfigView::onBluetoothButtonClicked()
{
#ifdef Q_OS_MAC
    QProcess::startDetached("open", { "x-apple.systempreferences:com.apple.Bluetooth" });
#elif defined(Q_OS_WIN)
    QProcess::startDetached("cmd.exe", { "/c", "start", "ms-settings:bluetooth" });
#else
    QProcess::startDetached("blueman-manager", QStringList{});
#endif
}
