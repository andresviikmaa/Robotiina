#pragma  once
#include "types.h"

class Camera: public ICamera
{
private:
    cv::Mat frame, buffer;
	cv::VideoCapture *cap;
public:
    Camera(const std::string &device);
	Camera(int device);
    const cv::Mat & Capture();
    const cv::Mat & CaptureHSV();
    virtual ~Camera(){ 
		cap->release();
		delete cap;
	}

};