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
    LOCATE_BALL = 0,
    BALL_LOCATED,
    LOCATE_GATE,
    GATE_LOCATED,
    END_OF_GAME
};

class ICamera
{
public:
    virtual const cv::Mat & Capture() = 0;
};

extern std::map<OBJECT, std::string> OBJECT_LABELS;

