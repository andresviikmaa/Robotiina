#pragma once
#include "objectfinder.h"
class MouseFinder :
	public ObjectFinder
{
protected:
	virtual cv::Point2f LocateOnScreen(const HSVColorRange &HSVRange, cv::Mat &frameHSV, cv::Mat &frameBGR, bool gate, const HSVColorRange &inner, const HSVColorRange &outer);
public:
	MouseFinder();
	~MouseFinder();
private:
	cv::Point2i mouseLocation;
};

