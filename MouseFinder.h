#pragma once
#include "objectfinder.h"
class MouseFinder :
	public ObjectFinder
{
protected:
	cv::Point2f LocateOnScreen(const HSVColorRange &HSVRange, cv::Mat &frameHSV, cv::Mat &frameBGR);

public:
	MouseFinder();
	~MouseFinder();
private:
	cv::Point2i mouseLocation;
};

