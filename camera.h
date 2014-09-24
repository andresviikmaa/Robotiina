#pragma  once
#include "types.h"

class Camera: public ICamera, protected cv::VideoCapture
{
private:
    cv::Mat frame, buffer;
public:
    Camera();
    const cv::Mat & Capture();
    const cv::Mat & CaptureHSV();
    virtual ~Camera(){ }

};