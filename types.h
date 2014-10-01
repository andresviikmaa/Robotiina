#pragma once
#include <opencv2/opencv.hpp>

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
    BALL = 0, GATE/*, FIELD, BORDER1, BORDER2*/, NUMBER_OF_OBJECTS
};

enum STATE
{
    STATE_NONE = 0,
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

