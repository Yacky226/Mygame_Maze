cmake_minimum_required(VERSION 3.21)
project(MazeStudio VERSION 0.1.0 LANGUAGES CXX)

option(MAZE_BUILD_LEGACY "Build the SFML application" ON)
option(MAZE_BUILD_CLI "Build the headless command-line application" ON)
option(MAZE_BUILD_WASM "Build the Emscripten browser engine" OFF)
option(MAZE_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
include(CTest)
if(BUILD_TESTING)
    include(cmake/TestDependencies.cmake)
    include(Catch)
endif()
set(CMAKE_CXX_EXTENSIONS OFF)

if(MAZE_ENABLE_SANITIZERS AND MSVC)
    message(FATAL_ERROR "The combined ASan/UBSan configuration requires GCC or Clang on Linux")
endif()
if(MSVC)
    add_compile_options(/utf-8)
elseif(MAZE_ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_subdirectory(core)

if(MAZE_BUILD_WASM)
    if(NOT EMSCRIPTEN)
        message(FATAL_ERROR "MAZE_BUILD_WASM requires the Emscripten toolchain (emcmake)")
    endif()
    target_compile_options(maze_core PRIVATE -fexceptions)
    add_subdirectory(bindings/wasm)
endif()

if(MAZE_BUILD_CLI)
    add_subdirectory(apps/cli)
endif()

if(MAZE_BUILD_LEGACY)
    # Windows .lib files are never used to build the Linux application.
    find_package(SFML 2.6 REQUIRED COMPONENTS graphics window system)

add_executable(maze
    Mygame/main.cpp
    Mygame/MainController.cpp
    Mygame/Fenetre.cpp
    Mygame/Cell.cpp
    Mygame/Pathfinding.cpp
    Mygame/Button.cpp
    Mygame/ChecklistItem.cpp
    Mygame/DrawInterface.cpp
    Mygame/Animated.cpp
)
target_link_libraries(maze PRIVATE maze::core sfml-graphics sfml-window sfml-system)

# Keep Fonts/arial.ttf available relative to the executable's working directory.
file(COPY Mygame/Fonts DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
set_property(TARGET maze PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
install(TARGETS maze RUNTIME DESTINATION .)
install(DIRECTORY Mygame/Fonts DESTINATION .)
endif()
