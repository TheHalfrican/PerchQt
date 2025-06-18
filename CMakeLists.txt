cmake_minimum_required(VERSION 3.16)
project(PerchQt VERSION 0.1 LANGUAGES CXX)

# Enable Qt auto-moc, auto-uic, and auto-rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
# Tell AUTOUIC where to find .ui files under src/UI
set(CMAKE_AUTOUIC_SEARCH_PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UI"
)
set(CMAKE_AUTORCC ON)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)

# Define the executable and its sources
add_executable(PerchQt
    src/main.cpp
    src/Models/Game.cpp
    src/Models/GameListModel.cpp
    src/ViewModels/GameListViewModel.cpp
    src/Views/MainWindow.cpp
    src/Views/GameWidgetView.cpp
    src/UI/MainWindow.ui
    src/Assets/resources.qrc
)

# Include header directories
target_include_directories(PerchQt PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Link against Qt6 libraries
target_link_libraries(PerchQt PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)