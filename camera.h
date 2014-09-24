#include <opencv2/opencv.hpp>

class Camera: protected VideoCap
{
private:
    Mat frame, buffer;
public:
    Camera();
    const Mat & Capture();
    const Mat & CaptureHSV();
    ~Camera(){ }

};