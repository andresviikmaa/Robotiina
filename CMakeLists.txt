cmake_minimum_required(VERSION 2.8.4)
project(robotiina)

list( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs")

find_package( OpenCV REQUIRED )

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.56.0 COMPONENTS PropertyTree Program_options) 

include_directories(${Boost_INCLUDE_DIRS}) 

set(SOURCE_FILES Robotiina.cpp camera.cpp robot.cpp objectfinder.cpp colorcalibrator.cpp stillcamera.cpp wheelcontroller.cpp)

add_executable(robotiina ${SOURCE_FILES})
target_link_libraries( robotiina ${OpenCV_LIBS} ${Boost_LIBRARIES} )
