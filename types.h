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


enum OBJECTS
{
    BALL = 0, FIELD, GATE, BORDER1, BORDER2, NUMBER_OF_OBJECTS
};


class ICamera
{
public:
    virtual const cv::Mat & Capture() = 0;
};