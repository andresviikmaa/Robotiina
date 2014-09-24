cmake_minimum_required(VERSION 2.8.4)
project(robotiina)

find_package( OpenCV REQUIRED )

set(SOURCE_FILES main.cpp camera.cpp robot.cpp objectfinder.cpp colorcalibrator.cpp stillcamera.cpp)

add_executable(robotiina ${SOURCE_FILES})
target_link_libraries( robotiina ${OpenCV_LIBS} )