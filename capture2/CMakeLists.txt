cmake_minimum_required(VERSION 3.12)
project(apple_detector)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(apple_detector ./apple_detector.cpp)

target_link_libraries(apple_detector ${OpenCV_LIBS})