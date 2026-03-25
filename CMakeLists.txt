cmake_minimum_required(VERSION 3.13)
project(chessbox)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2 packages installed via apt
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2       REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_TTF   REQUIRED SDL2_ttf)

# Include paths
include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/tiny-process-library
)

# Common source files shared between targets
set(COMMON_SOURCES
    common/thc.cpp
    common/generalexception.cpp
    common/socketinstance.cpp
    common/PGNUtils.cpp
)

# Tiny process library (Linux version)
set(PROCESS_SOURCES
    tiny-process-library/process.cpp
    tiny-process-library/process_unix.cpp
)

# Controller GUI sources
set(CONTROLLER_SOURCES
    ${COMMON_SOURCES}
    ${PROCESS_SOURCES}
    src/main.cpp
    src/Sprite.cpp
    src/Button.cpp
    src/Board.cpp
    src/Component.cpp
    src/UIGroup.cpp
    src/Label.cpp
    src/MovesPanel.cpp
    src/LevelPopup.cpp
    src/DepthPopup.cpp
    src/TimePopup.cpp
    src/PGNLoader.cpp
    src/Connector.cpp
    src/Dialog.cpp
    src/TextField.cpp
    src/FontManager.cpp
    src/Window.cpp
    src/ControllerGUIjson.cpp
    src/Prefs.cpp
)

# Sim sources
set(SIM_SOURCES
    ${COMMON_SOURCES}
    src/sim.cpp
    src/telnetserver.cpp
    src/Sprite.cpp
    src/Button.cpp
    src/Component.cpp
    src/UIGroup.cpp
    src/Label.cpp
    src/FontManager.cpp
    src/Window.cpp
)

# Build controller_gui
add_executable(controller_gui ${CONTROLLER_SOURCES})
target_compile_options(controller_gui PRIVATE
    ${SDL2_CFLAGS_OTHER}
    -DSDL_MAIN_HANDLED
)
target_link_libraries(controller_gui
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    pthread
)

# Build sim
add_executable(sim ${SIM_SOURCES})
target_compile_options(sim PRIVATE
    ${SDL2_CFLAGS_OTHER}
    -DSDL_MAIN_HANDLED
)
target_link_libraries(sim
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    pthread
)
