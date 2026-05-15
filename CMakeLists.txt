cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(PerchQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/src/UI")
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)
find_package(SDL2 REQUIRED)

add_executable(PerchQt
    src/main.cpp
    src/ViewModels/GameListViewModel.cpp
    src/Views/MainWindow.cpp
    src/Views/SettingsDialog.cpp
    src/Views/GameWidgetView.cpp
    src/Views/ControllerConfigView.cpp
    src/Views/GameListView.cpp
    src/Utils/PlaceholderImage.cpp
    src/Utils/Themes.cpp
    src/Widgets/MarqueeLabel.cpp
    src/Assets/resources.qrc
    src/UI/MainWindow.ui
    src/UI/GameListView.ui
    src/UI/GameWidgetView.ui
    src/UI/SettingsDialog.ui
    src/UI/ControllerConfigView.ui
)

target_include_directories(PerchQt PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(PerchQt PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    SDL2::SDL2
)

# Treat WinMain as already provided by Qt; SDL_MAIN_HANDLED keeps SDL out of main().
target_compile_definitions(PerchQt PRIVATE SDL_MAIN_HANDLED)

# --- Deployment -------------------------------------------------------------
# Run windeployqt after build so the exe is runnable from the build dir.
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt
        HINTS ${Qt6_DIR}/../../../bin
    )
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET PerchQt POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    --no-translations
                    --no-system-d3d-compiler
                    --no-opengl-sw
                    "$<TARGET_FILE:PerchQt>"
            COMMENT "Running windeployqt"
            VERBATIM
        )
    endif()

    # Find SDL2.dll next to the import lib and copy it.
    get_target_property(_sdl2_loc SDL2::SDL2 IMPORTED_LOCATION)
    if(_sdl2_loc)
        get_filename_component(_sdl2_dir "${_sdl2_loc}" DIRECTORY)
        if(EXISTS "${_sdl2_dir}/SDL2.dll")
            set(_sdl2_dll "${_sdl2_dir}/SDL2.dll")
        elseif(EXISTS "${_sdl2_dir}/../bin/SDL2.dll")
            set(_sdl2_dll "${_sdl2_dir}/../bin/SDL2.dll")
        endif()
        if(_sdl2_dll)
            add_custom_command(TARGET PerchQt POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${_sdl2_dll}"
                        "$<TARGET_FILE_DIR:PerchQt>/SDL2.dll"
                VERBATIM
            )
        endif()
    endif()
endif()
