#ifndef PERCHQT_CONTROLLERMANAGER_H
#define PERCHQT_CONTROLLERMANAGER_H

#include <QObject>
#include <QStringList>

class QTimer;

// Polls SDL game controller events and emits Qt signals for navigation.
// Lifetime: a single instance owned by MainWindow (or the app); SDL is initialized
// in the ctor and torn down in the dtor.
class ControllerManager : public QObject {
    Q_OBJECT

public:
    explicit ControllerManager(QObject* parent = nullptr);
    ~ControllerManager() override;

    // True if SDL initialized and at least one controller is open.
    bool hasController() const;

    // Names of currently open controllers.
    QStringList controllerNames() const;

signals:
    // Discrete navigation events fired on press (auto-repeats at a slow cadence).
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();

    // Action buttons.
    void confirmPressed();   // A
    void backPressed();      // B
    void menuPressed();      // Start

    // Hot-plug.
    void controllersChanged();

private slots:
    void poll();

private:
    void openAllControllers();
    void closeAllControllers();

    QTimer* m_timer{nullptr};
    bool m_sdlOk{false};
};

#endif // PERCHQT_CONTROLLERMANAGER_H
