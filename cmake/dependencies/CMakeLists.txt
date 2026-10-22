cmake_minimum_required(VERSION 3.21)
project(MazeTestDependencies LANGUAGES CXX)
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_EXTRAS ON CACHE BOOL "" FORCE)
# This disposable project builds only Catch2. Its upstream installation rules
# are enabled only when PROJECT_NAME is initially undefined. Keep this override
# confined to the dependency bootstrap; the application uses normal FetchContent.
unset(PROJECT_NAME)
include("${CMAKE_CURRENT_LIST_DIR}/../TestDependencies.cmake")

