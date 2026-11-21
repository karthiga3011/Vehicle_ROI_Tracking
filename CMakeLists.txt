cmake_minimum_required(VERSION 3.16)
project(VehicleROI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(VehicleROI
    src/main.cpp
    src/RoiManager.cpp
    src/Detector.cpp
    src/Tracker.cpp
    src/RoiTimer.cpp
)

target_include_directories(VehicleROI PRIVATE src)
target_link_libraries(VehicleROI PRIVATE ${OpenCV_LIBS})

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
