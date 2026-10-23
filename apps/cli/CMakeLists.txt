add_executable(maze_cli main.cpp scenario_text.cpp)
target_link_libraries(maze_cli PRIVATE maze::core)
target_compile_definitions(maze_cli PRIVATE MAZE_ENGINE_VERSION="${PROJECT_VERSION}")
install(TARGETS maze_cli RUNTIME DESTINATION .)

add_executable(maze_generate generate.cpp)
target_link_libraries(maze_generate PRIVATE maze::core)
install(TARGETS maze_generate RUNTIME DESTINATION .)

if(BUILD_TESTING)
    add_executable(maze_cli_tests tests/scenario_text_tests.cpp scenario_text.cpp)
    target_include_directories(maze_cli_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(maze_cli_tests PRIVATE maze::core Catch2::Catch2WithMain)
    include(Catch)
    catch_discover_tests(maze_cli_tests)
    add_test(NAME cli.weighted-detour
        COMMAND maze_cli "${PROJECT_SOURCE_DIR}/scenarios/weighted-detour.maze.txt" --algorithm astar)
    set_tests_properties(cli.weighted-detour PROPERTIES PASS_REGULAR_EXPRESSION "\"cost\":\"6000\"")
    add_test(NAME cli.contracts COMMAND "${CMAKE_COMMAND}"
        "-DCLI=$<TARGET_FILE:maze_cli>" "-DSCENARIOS=${PROJECT_SOURCE_DIR}/scenarios"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_contracts.cmake")
endif()
