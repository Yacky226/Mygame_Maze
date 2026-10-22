# Prefer a declared package (including a future vcpkg toolchain). The bootstrap
# fallback is test-only and pinned to an immutable archive with a verified hash.
find_package(Catch2 3.7.1 EXACT QUIET)
if(NOT Catch2_FOUND)
    include(FetchContent)
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()
    FetchContent_Declare(Catch2
        URL https://codeload.github.com/catchorg/Catch2/tar.gz/9827c148c397289df17d3967692619a198032a24
        URL_HASH SHA256=9f37d318e6bdfdcedee9260cfd5e769bdc0757d1f8a9aa978461e6bc6454e104
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
endif()
