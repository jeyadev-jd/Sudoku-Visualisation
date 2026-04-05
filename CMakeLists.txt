cmake_minimum_required(VERSION 3.14)
project(SudokuVisualizer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
)
FetchContent_MakeAvailable(SFML)

add_executable(SudokuVisualizer
    src/main.cpp
    src/SudokuGrid.cpp
    src/Solver.cpp
    src/Application.cpp
    src/EventLogger.cpp
)

target_include_directories(SudokuVisualizer PRIVATE include)

target_link_libraries(SudokuVisualizer PRIVATE sfml-graphics sfml-window sfml-system)

# Ensure required DLLs (like openal, freetype) are copied to the executable directory on Windows
if(WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET SudokuVisualizer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:SudokuVisualizer> $<TARGET_FILE_DIR:SudokuVisualizer>
        COMMAND_EXPAND_LISTS
    )
endif()
