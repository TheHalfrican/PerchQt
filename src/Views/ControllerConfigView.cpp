#include "Views/ControllerConfigView.h"
#include "ui_ControllerConfigView.h"
#include <SDL.h>
#include <QString>

ControllerConfigView::ControllerConfigView(QWidget* parent)
  : QDialog(parent),
    ui(new Ui::ControllerConfigView)
{
    ui->setupUi(this);

    // Initialize SDL game controller subsystem
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
        ui->statusLabel->setText(tr("SDL_Init failed"));
    } else {
        updateStatus();
    }
}

ControllerConfigView::~ControllerConfigView() {
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    delete ui;
}

void ControllerConfigView::updateStatus() {
    int count = SDL_NumJoysticks();
    if (count <= 0) {
        ui->statusLabel->setText(tr("No Controllers Connected"));
        ui->countLabel->setText(tr("Controllers Connected: 0"));
        ui->nameLabel->setText(tr("Controller Name(s): —"));
        return;
    }
    // Build a list of names for all connected controllers
    QStringList names;
    int controllerCount = 0;
    for (int i = 0; i < count; ++i) {
        if (SDL_IsGameController(i)) {
            SDL_GameController* controller = SDL_GameControllerOpen(i);
            if (controller) {
                QString name = QString::fromUtf8(SDL_GameControllerName(controller));
                names << name;
                SDL_GameControllerClose(controller);
                ++controllerCount;
            }
        }
    }
    if (controllerCount == 0) {
        ui->statusLabel->setText(tr("No Controllers Connected"));
        ui->countLabel->setText(tr("Controllers Connected: 0"));
        ui->nameLabel->setText(tr("Controller Name(s): —"));
    } else {
        ui->statusLabel->setText(tr("Controller(s) Connected"));
        ui->countLabel->setText(tr("Controllers Connected: %1").arg(controllerCount));
        ui->nameLabel->setText(tr("Controller Name(s): %1").arg(names.join(", ")));
    }
}