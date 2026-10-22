# Prefer a declared package (including a future vcpkg toolchain). The bootstrap
# fallback is test-only and pinned to an immutable archive with a verified hash.
find_package(Catch2 3.7.1 EXACT QUIET)
if(NOT Catch2_FOUND)
    option(MAZE_AMALGAMATED_TEST_DEPS "Build the official Catch2 single-translation-unit distribution" ON)
    include(FetchContent)
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()
    FetchContent_Declare(Catch2
        URL https://codeload.github.com/catchorg/Catch2/tar.gz/9827c148c397289df17d3967692619a198032a24
        URL_HASH SHA256=9f37d318e6bdfdcedee9260cfd5e769bdc0757d1f8a9aa978461e6bc6454e104
    )
    FetchContent_MakeAvailable(Catch2)
    if(MAZE_AMALGAMATED_TEST_DEPS)
        set_property(TARGET Catch2 PROPERTY SOURCES "${catch2_SOURCE_DIR}/extras/catch_amalgamated.cpp")
        target_compile_definitions(Catch2 PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN)
    endif()
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
endif()
