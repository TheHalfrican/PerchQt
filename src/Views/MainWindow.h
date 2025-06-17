

#ifndef PERCHQT_MAINWINDOW_H
#define PERCHQT_MAINWINDOW_H

#include <QMainWindow>

// Forward declarations
class GameListViewModel;
class QListView;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    // Constructor
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    // UI setup helpers
    void setupUi();
    void setupConnections();

    // ViewModel driving this window
    GameListViewModel* m_viewModel{nullptr};
    // The main list view for displaying games
    QListView*      m_listView{nullptr};
};

#endif // PERCHQT_MAINWINDOW_H