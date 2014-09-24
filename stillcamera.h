#pragma  once
#include "types.h"

class StillCamera: public ICamera, protected cv::VideoCapture
{
private:
    cv::Mat frame;
public:
    StillCamera(const std::string &fileName);
    const cv::Mat & Capture();
    virtual ~StillCamera(){ }

};
