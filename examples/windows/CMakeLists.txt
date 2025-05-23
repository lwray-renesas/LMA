cmake_minimum_required(VERSION 3.28)

# Create the Project
project(LMA-sim-windows VERSION 1.0 LANGUAGES C CXX)

# Setup source and header files
set (SOURCES
    "src/main.cpp"
    "src/mainwindow.cpp"
    "src/simulation/simulation.cpp"
    "../../src/LMA_Core.c"
    "../../port/windows/LMA_Port.c"
)
set (HEADERS
    "src/mainwindow.hpp"
    "src/simulation/simulation.hpp"
    "../../src/LMA_Core.h"
    "../../src/LMA_Utils/LMA_Types.h"
    "../../port/windows/LMA_Port.h"
)
# setup directories
set (DIRECTORIES
    "src"
    "src/simulation"
    "../../src"
    "../../src/LMA_Utils"
    "../../port/windows"
)

if(CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Multi-config build! Configurations: ${CMAKE_CONFIGURATION_TYPES}")
    # Final output directory
    set(APP_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
else()
    message(STATUS "Single-config build!")
    # Final output directory
    set(APP_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

# Set output directory for everything
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${APP_OUTPUT_DIRECTORY}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${APP_OUTPUT_DIRECTORY}")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${APP_OUTPUT_DIRECTORY}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${APP_OUTPUT_DIRECTORY}")

###################################
#       APPLICATION
###################################
# Qt
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Concurrent Charts)
qt_standard_project_setup()
# Add a test executable
qt_add_executable(LMA-sim-windows ${SOURCES} ${HEADERS} "src/mainwindow.ui")

target_link_libraries(LMA-sim-windows PRIVATE Qt6::Widgets Qt6::Gui Qt6::Concurrent Qt6::Charts)

# Source Grouping For cleaner output
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Local source grouping (within the current source dir)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "" FILES
    "src/main.cpp"
    "src/mainwindow.cpp"
    "src/mainwindow.hpp"
    "src/mainwindow.ui"
    "src/simulation/simulation.cpp"
    "src/simulation/simulation.hpp"
)

# External source grouping for ../../src
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/../../src" PREFIX "LMA_Core" FILES
    "../../src/LMA_Core.c"
    "../../src/LMA_Core.h"
    "../../src/LMA_Utils/LMA_Types.h"
)

# External source grouping for ../../port
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/../../port" PREFIX "LMA_Port" FILES
    "../../port/Windows/LMA_Port.c"
    "../../port/Windows/LMA_Port.h"
)

if(WIN32)
    target_compile_definitions(LMA-sim-windows PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()


# Include directories
target_include_directories(LMA-sim-windows
    PUBLIC
    ${DIRECTORIES}
)

set_target_properties(LMA-sim-windows PROPERTIES
    CXX_STANDARD 17
    C_STANDARD 99)
