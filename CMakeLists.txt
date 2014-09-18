cmake_minimum_required(VERSION 2.8.4)
project(robotext)

find_package( OpenCV REQUIRED )


set(SOURCE_FILES main.cpp)
add_executable(robotext ${SOURCE_FILES})
target_link_libraries( robotext ${OpenCV_LIBS} )