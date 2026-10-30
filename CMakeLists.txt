cmake_minimum_required(VERSION 3.12...3.30 FATAL_ERROR)
project(opencv_camara_demo)

set(OpenCV_DIR "/usr/local/lib/cmake/opencv4")

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(bt10101 ./video_capture.cpp)

target_link_libraries(bt10101 ${OpenCV_LIBS})