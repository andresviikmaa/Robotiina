#include "types.h"
#include <opencv2/opencv.hpp>

class ColorCalibrator {
protected:
    HSVColorRange range =  {{0,179},{0,255},{0,255}};
    cv::Mat image;
public:
    ColorCalibrator();
    void LoadImage(const cv::Mat &image, int numberOfObjects);
    HSVColorRange GetObjectThresholds (int index);
    ~ColorCalibrator();

};