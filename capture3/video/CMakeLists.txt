cmake_minimum_required(VERSION 3.12)
project(cube_detector)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(cube_detect ./cube_detector.cpp)

target_link_libraries(cube_detect ${OpenCV_LIBS})