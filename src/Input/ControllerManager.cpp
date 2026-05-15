#include "Input/ControllerManager.h"

#include <QDebug>
#include <QTimer>

#include <SDL.h>

namespace {
constexpr int kPollIntervalMs = 16;          // ~60 Hz
constexpr int kAxisDeadzone = 8000;          // out of 32767
constexpr int kAxisRepeatInitialMs = 350;
constexpr int kAxisRepeatMs = 110;
}

ControllerManager::ControllerManager(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    if (SDL_WasInit(SDL_INIT_GAMECONTROLLER) == 0) {
        if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
            qWarning() << "SDL_InitSubSystem(GAMECONTROLLER) failed:" << SDL_GetError();
            return;
        }
    }
    m_sdlOk = true;
    openAllControllers();

    m_timer->setInterval(kPollIntervalMs);
    connect(m_timer, &QTimer::timeout, this, &ControllerManager::poll);
    m_timer->start();
}

ControllerManager::~ControllerManager()
{
    if (!m_sdlOk) return;
    closeAllControllers();
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

void ControllerManager::openAllControllers()
{
    const int n = SDL_NumJoysticks();
    for (int i = 0; i < n; ++i) {
        if (SDL_IsGameController(i)) {
            // Hold open for the lifetime of the manager. SDL tracks per-instance.
            (void)SDL_GameControllerOpen(i);
        }
    }
}

void ControllerManager::closeAllControllers()
{
    // SDL_QuitSubSystem closes anything still open; nothing to do explicitly.
}

bool ControllerManager::hasController() const
{
    if (!m_sdlOk) return false;
    for (int i = 0, n = SDL_NumJoysticks(); i < n; ++i)
        if (SDL_IsGameController(i)) return true;
    return false;
}

QStringList ControllerManager::controllerNames() const
{
    QStringList names;
    if (!m_sdlOk) return names;
    for (int i = 0, n = SDL_NumJoysticks(); i < n; ++i) {
        if (!SDL_IsGameController(i)) continue;
        if (const char* name = SDL_GameControllerNameForIndex(i))
            names << QString::fromUtf8(name);
    }
    return names;
}

void ControllerManager::poll()
{
    if (!m_sdlOk) return;

    // Per-direction repeat tracking. Each entry: ms-until-next-fire (<=0 fires now).
    static int leftRepeat = 0, rightRepeat = 0, upRepeat = 0, downRepeat = 0;

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_CONTROLLERDEVICEADDED:
            if (SDL_IsGameController(ev.cdevice.which))
                SDL_GameControllerOpen(ev.cdevice.which);
            emit controllersChanged();
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            emit controllersChanged();
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            switch (ev.cbutton.button) {
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  emit moveLeft();  leftRepeat  = kAxisRepeatInitialMs; break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: emit moveRight(); rightRepeat = kAxisRepeatInitialMs; break;
            case SDL_CONTROLLER_BUTTON_DPAD_UP:    emit moveUp();    upRepeat    = kAxisRepeatInitialMs; break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  emit moveDown();  downRepeat  = kAxisRepeatInitialMs; break;
            case SDL_CONTROLLER_BUTTON_A:          emit confirmPressed(); break;
            case SDL_CONTROLLER_BUTTON_B:          emit backPressed();    break;
            case SDL_CONTROLLER_BUTTON_START:      emit menuPressed();    break;
            default: break;
            }
            break;
        case SDL_CONTROLLERBUTTONUP:
            switch (ev.cbutton.button) {
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  leftRepeat  = 0; break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: rightRepeat = 0; break;
            case SDL_CONTROLLER_BUTTON_DPAD_UP:    upRepeat    = 0; break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  downRepeat  = 0; break;
            default: break;
            }
            break;
        default:
            break;
        }
    }

    // Analog stick auto-repeat: synthesize moves while held past the deadzone.
    int x = 0, y = 0;
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (!SDL_IsGameController(i)) continue;
        SDL_GameController* gc = SDL_GameControllerFromInstanceID(
            SDL_JoystickGetDeviceInstanceID(i));
        if (!gc) continue;
        x = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTX);
        y = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTY);
        if (qAbs(x) > kAxisDeadzone || qAbs(y) > kAxisDeadzone) break;
    }

    auto tickAxis = [](int& counter, bool active, auto&& fire) {
        if (!active) { counter = 0; return; }
        counter -= kPollIntervalMs;
        if (counter <= 0) { fire(); counter = kAxisRepeatMs; }
    };
    tickAxis(leftRepeat,  x < -kAxisDeadzone, [this]{ emit moveLeft();  });
    tickAxis(rightRepeat, x >  kAxisDeadzone, [this]{ emit moveRight(); });
    tickAxis(upRepeat,    y < -kAxisDeadzone, [this]{ emit moveUp();    });
    tickAxis(downRepeat,  y >  kAxisDeadzone, [this]{ emit moveDown();  });
}
