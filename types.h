#pragma once
#include <opencv2/opencv.hpp>
#include <math.h> 
#ifdef WIN32
#define _WIN32_WINNT 0x0600 // vista
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


enum OBJECT
{
//    BALL = 0, GATE, FIELD, INNER_BORDER, OUTER_BORDER, NUMBER_OF_OBJECTS
    BALL = 0, GATE, NUMBER_OF_OBJECTS, FIELD, INNER_BORDER, OUTER_BORDER,
};

enum STATE
{
    STATE_NONE = 0,
    STATE_AUTOCALIBRATE,
    STATE_CALIBRATE,
    STATE_CONFIGURE_USB,
    STATE_LAUNCH,
    STATE_LOCATE_BALL,
    STATE_BALL_LOCATED,
    STATE_LOCATE_GATE,
    STATE_GATE_LOCATED,
    STATE_REMOTE_CONTROL,
    STATE_END_OF_GAME,
	STATE_CRASH,
	STATE_MANUAL_CONTROL
};

class ICamera
{
public:
    virtual const cv::Mat & Capture() = 0;
};

extern std::map<OBJECT, std::string> OBJECT_LABELS;

