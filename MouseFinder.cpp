#include "MouseFinder.h"


MouseFinder::MouseFinder()
{
	cv::namedWindow("Original");
	cv::setMouseCallback("Original", [](int event, int x, int y, int flags, void* self) {
		((MouseFinder*)self)->mouseLocation = cv::Point2i(x,y);

	}, this);
}

cv::Point2f MouseFinder::LocateOnScreen(const HSVColorRange &r, const cv::Mat &frame) {
	cv::Mat frame2;
	frame.copyTo(frame2);
	cv::Scalar colorCircle(133, 33, 55);
	cv::circle(frame2, cv::Point2i(mouseLocation), 10, colorCircle, 3);
	cv::imshow("Original", frame2);
	cv::moveWindow("Original", 0, 0);
	return cv::Point2f(mouseLocation.x, mouseLocation.y);

}
MouseFinder::~MouseFinder()
{
}
