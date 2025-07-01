#ifndef PERCHQT_MAINWINDOW_H
#define PERCHQT_MAINWINDOW_H

#include <QMainWindow>
#include "ui_MainWindow.h"
#include <QVector>
#include "Models/Game.h"

// Forward declarations
class GameListViewModel;
class GameWidgetView;

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
    // Currently selected game widget for single selection
    GameWidgetView* m_selectedView{nullptr};

private slots:
    void onAddGameClicked();
    void onGamesLoaded(const QVector<Game>& games);
    void onRemoveGame(int gameId);
    void onLaunchGame(int gameId);
    void onShowFile(int gameId);
    void onSetCoverImage(int gameId);
    void onRemoveCoverImage(int gameId);
    void onSettingsClicked();
};

#endif // PERCHQT_MAINWINDOW_H