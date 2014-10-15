#pragma once
#include "objectfinder.h"
class MouseFinder :
	public ObjectFinder
{
protected:
	cv::Point2f LocateOnScreen(const HSVColorRange &r, const cv::Mat &frame);

public:
	MouseFinder();
	~MouseFinder();
private:
	cv::Point2i mouseLocation;
};

