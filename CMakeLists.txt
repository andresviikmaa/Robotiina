cmake_minimum_required(VERSION 2.8.4)
project(robotiina)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(OpenCV_DIR "e:/programs/opencv/build/x86/mingw")
    set(BOOST_ROOT "E:/programs/boost_1_55_0")
    set(BOOST_INCLUDEDIR "E:/programs/boost_1_5_0")
    set(BOOST_LIBRARYDIR "E:/programs/boost_1_55_0/stage/lib")
    add_definitions( -DWIN32 )
    set(Boost_USE_STATIC_LIBS ON)

    find_library(WINSOCK_LIB libws2_32.a HINTS "E:/programs/mingw-w64/i686-w64-mingw32/lib")
    find_library(WINSOCK2_LIB libwsock32.a HINTS "E:/programs/mingw-w64/i686-w64-mingw32/lib")
##    set(LIBS ${LIBS} ${WINSOCK_LIB})

ELSE()
    set(Boost_USE_STATIC_LIBS OFF)
    set(WINSOCK_LIB "")
    set(WINSOCK2_LIB "")
ENDIF()
 set(Boost_DEBUG 1)
# SET(CMAKE_PREFIX_PATH Libs)

#boost
SET(Boost_ADDITIONAL_VERSIONS "1.56" "1.56.0" "1.55" "1.55.0")

#list( APPEND CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
find_package( OpenCV REQUIRED )

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.54.0 COMPONENTS program_options system thread chrono date_time filesystem)

#if(Boost_FOUND)
include_directories(${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

#endif()

set(SOURCE_FILES Robotiina.cpp camera.cpp stillcamera.cpp robot.cpp objectfinder.cpp colorcalibrator.cpp autocalibrator.cpp wheelcontroller.cpp calibrationconfreader.cpp dialog.cpp wheel.cpp remotecontrol.cpp MouseFinder.cpp ComPortScanner.cpp coilBoard.cpp AutoPilot.cpp RobotTracker.cpp ThreadedClass.cpp kalmanFilter.cpp VideoRecorder.cpp Arduino.cpp NewAutoPilot.cpp GateFinder.cpp BallFinder.cpp)
add_executable(robotiina ${SOURCE_FILES})
target_link_libraries( robotiina  ${OpenCV_LIBS} ${Boost_LIBRARIES} ${WINSOCK_LIB} ${WINSOCK2_LIB})

set(WHEELEMULATOR_FILES wheelemulator.cpp)
add_executable(wheelemulator ${WHEELEMULATOR_FILES})
target_link_libraries( wheelemulator ${OpenCV_LIBS} ${Boost_LIBRARIES} ${WINSOCK_LIB})

