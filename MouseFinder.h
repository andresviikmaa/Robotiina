#pragma once
#include "objectfinder.h"
class MouseFinder :
	public ObjectFinder
{
protected:
	virtual cv::Point2i LocateOnScreen(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target);
public:
	MouseFinder();
	~MouseFinder();
private:
	cv::Point2i mouseLocation;
};

