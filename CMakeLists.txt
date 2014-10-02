cmake_minimum_required(VERSION 2.8.4)
project(robotiina)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
set(OpenCV_DIR "e:/programs/opencv/build/x86/mingw")
set(BOOST_DIR "e:/programs/boost")
ENDIF()

# SET(CMAKE_PREFIX_PATH Libs)

#boost
SET(Boost_ADDITIONAL_VERSIONS "1.56" "1.56.0" "1.55" "1.55.0")

list( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs")

find_package( OpenCV REQUIRED )

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.55.0 COMPONENTS propertytree program_options)

include_directories(${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})


set(SOURCE_FILES Robotiina.cpp camera.cpp stillcamera.cpp robot.cpp objectfinder.cpp colorcalibrator.cpp autocalibrator.cpp wheelcontroller.cpp calibrationconfreader.cpp dialog.cpp)

add_executable(robotiina ${SOURCE_FILES})
target_link_libraries( robotiina ${OpenCV_LIBS} ${Boost_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARY} )
