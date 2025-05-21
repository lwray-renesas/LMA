cmake_minimum_required(VERSION 3.28)

# Create the Project
project(LMA-sim-windows VERSION 1.0 LANGUAGES C)

# Setup source and header files
set (SOURCES
    "src/main.c"
    "src/simulation/simulation.c"
    "src/args_parser/args_parser.c"
    "../../src/LMA_Core.c"
    "../../port/windows/LMA_Port.c"
)
set (HEADERS
    "src/simulation/simulation.h"
    "src/args_parser/args_parser.h"
    "../../src/LMA_Core.h"
    "../../src/LMA_Utils/LMA_Types.h"
    "../../port/windows/LMA_Port.h"
)
# setup directories
set (DIRECTORIES
    "src"
    "src/simulation"
    "src/args_parser"
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
# Add a test executable
add_executable(LMA-sim-windows ${SOURCES} ${HEADERS})

# Source Grouping For cleaner output
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Local source grouping (within the current source dir)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "" FILES
    "src/main.c"
    "src/simulation/simulation.c"
    "src/args_parser/args_parser.c"
    "src/simulation/simulation.h"
    "src/args_parser/args_parser.h"
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
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF)

