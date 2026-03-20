# -- NOTE: File generated with use of AI
cmake_minimum_required(VERSION 3.15)

project(ConstellaVS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    src/ConfigurationParser.cpp
    src/Main.cpp
    src/Utility.cpp
    src/Vector.cpp
)

add_executable(constella ${SOURCES})

target_include_directories(constella PRIVATE src)
