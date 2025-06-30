#ifndef PERCHQT_MAINWINDOW_H
#define PERCHQT_MAINWINDOW_H

#include <QMainWindow>
#include "ui_MainWindow.h"
#include <QVector>
#include "Models/Game.h"

// Forward declarations
class GameListViewModel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    // Constructor
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    // ViewModel driving this window
    GameListViewModel* m_viewModel{nullptr};
    Ui::MainWindow*    ui{nullptr};

private slots:
    void onAddGameClicked();
    void onGamesLoaded(const QVector<Game>& games);
    void onRemoveGame(int gameId);
};

#endif // PERCHQT_MAINWINDOW_H