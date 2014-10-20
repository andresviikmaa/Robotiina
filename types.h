#pragma once
#include <opencv2/opencv.hpp>
#include <math.h> 
#ifdef WIN32
	#define _WIN32_WINNT 0x0600 // vista for socket.cancel()
	#ifndef _WIN32_WINNT_WS08
		#define _WIN32_WINNT_WS08 // GetTickCount64 is missing in mingw, so emulate Windows Server 2008
	#endif
#endif
#define PI 3.14159265

struct ColorRange
{
    int low;
    int high;
};

struct HSVColorRange
{
    ColorRange hue;
    ColorRange sat;
    ColorRange val;
};

enum OBJECT_ID {
	ID_WHEEL_LEFT = 1,
	ID_WHEEL_RIGHT = 2,
	ID_WHEEL_BACK = 3,
	//ID_COILGUN = 4,
	//ID_TRIBBLER = 5
	ID_OBJECT_COUNT
};
enum OBJECT
{
    BALL = 0, GATE1, GATE2, FIELD, INNER_BORDER, OUTER_BORDER, NUMBER_OF_OBJECTS
};

enum STATE
{
    STATE_NONE = 0,
    STATE_AUTOCALIBRATE,
    STATE_CALIBRATE,
    STATE_LAUNCH,
	STATE_SELECT_GATE,
	STATE_LOCATE_BALL,
    STATE_LOCATE_GATE,
	STATE_REMOTE_CONTROL,
	STATE_CRASH,
	STATE_MANUAL_CONTROL,
	STATE_DANCE,
	STATE_END_OF_GAME /* leave this last*/
};

class ICamera
{
public:
    virtual const cv::Mat & Capture() = 0;
};

extern std::map<OBJECT, std::string> OBJECT_LABELS;

