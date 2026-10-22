add_executable(maze_engine engine.cpp)
target_link_libraries(maze_engine PRIVATE maze::core)
target_compile_definitions(maze_engine PRIVATE MAZE_ENGINE_VERSION="${PROJECT_VERSION}")
target_compile_options(maze_engine PRIVATE -fexceptions)
target_link_options(maze_engine PRIVATE
    --bind --no-entry -fexceptions
    -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=web,worker,node
    -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=33554432 -sMAXIMUM_MEMORY=268435456
    -sFILESYSTEM=0 -sDYNAMIC_EXECUTION=0
)
set_target_properties(maze_engine PROPERTIES SUFFIX ".mjs")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/maze_engine.mjs"
    "${CMAKE_CURRENT_BINARY_DIR}/maze_engine.wasm" DESTINATION engine)
