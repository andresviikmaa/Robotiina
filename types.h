#pragma once

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


enum OBJECTS{
    BALL = 0, FIELD, GATE, BORDER1, BORDER2, NUMBER_OF_OBJECTS
};

