#include "MouseFinder.h"


MouseFinder::MouseFinder()
{
	cv::namedWindow("MouseFinder");
	cv::setMouseCallback("MouseFinder", [](int event, int x, int y, int flags, void* self) {
		((MouseFinder*)self)->mouseLocation = cv::Point2i(x,y);

	}, this);
}

cv::Point2f MouseFinder::LocateOnScreen(HSVColorRangeMap &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target)
{
	cv::Scalar colorCircle(133, 33, 55);
	cv::circle(frameBGR, cv::Point2i(mouseLocation), 10, colorCircle, 3);
	cv::imshow("MouseFinder", frameBGR);
	return cv::Point2f(mouseLocation.x, mouseLocation.y);

}
MouseFinder::~MouseFinder()
{
}
