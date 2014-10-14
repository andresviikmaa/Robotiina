#include "camera.h"
#include <opencv2/opencv.hpp>

Camera::Camera(const std::string &device)
{

	cap = new cv::VideoCapture(device.c_str());
	if (!cap->isOpened())  // if not success, exit program
    {
       throw std::string("Camera not found");
    }

}
Camera::Camera(int device)
{

	cap = new cv::VideoCapture(device);
	if (!cap->isOpened())  // if not success, exit program
	{
		throw std::string("Camera is missing");
	}

}

const cv::Mat &Camera::Capture()
{
	if (cap->isOpened())
		*cap >> frame;
	/*	cv::flip(frame, frame, 0);
		cv::Point2f src_center(frame.cols / 2.0F, frame.rows / 2.0F);
		cv::Mat rot_matrix = getRotationMatrix2D(src_center, 180.0, 1.0);
		cv::Mat rotated_img(cv::Size(frame.size().height, frame.size().width), frame.type());
		warpAffine(frame, frame, rot_matrix, frame.size());*/
    return frame;
}
const cv::Mat &Camera::CaptureHSV() {
    cvtColor(frame, buffer, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
    return buffer;
}
