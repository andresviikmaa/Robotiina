#pragma once
#include <opencv2/opencv.hpp>
#ifdef WIN32
#define _WIN32_WINNT 0x0501 // win xp
#endif

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
    STATE_END_OF_GAME
};

class ICamera
{
public:
    virtual const cv::Mat & Capture() = 0;
};

extern std::map<OBJECT, std::string> OBJECT_LABELS;

