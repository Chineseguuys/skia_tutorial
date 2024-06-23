cmake_minimum_required(VERSION 3.16.3)

project(skia_test)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_VERBOSE_MAKEFILE ON)

find_package(spdlog REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        ~/Android/Skia/)

target_link_directories(${PROJECT_NAME}
    PUBLIC
        ~/Android/Skia/lib/Shared/
)

target_link_libraries(${PROJECT_NAME}
    skia
    spdlog::spdlog
)
