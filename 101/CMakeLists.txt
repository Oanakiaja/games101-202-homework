cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME Rasterizer)
project(${PROJECT_NAME})

set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")

set(OpenCV_DIR /opt/homebrew/Cellar/opencv/4.10.0_2/lib/cmake/opencv4)
find_package(OpenCV REQUIRED)

set(CMAKE_CXX_STANDARD 17)

include_directories(/usr/local/include)

add_executable(${PROJECT_NAME} main.cpp rasterizer.hpp rasterizer.cpp triangle.hpp triangle.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBRARIES})
