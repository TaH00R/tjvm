cmake_minimum_required(VERSION 3.20)

project(tjvm
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(tjvm
    src/main.cpp
    src/class_file_reader.cpp
    src/class_file_parser.cpp
    src/constant_pool.cpp
)

target_include_directories(tjvm
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if (MSVC)
    target_compile_options(tjvm PRIVATE /W4)
else()
    target_compile_options(tjvm PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()