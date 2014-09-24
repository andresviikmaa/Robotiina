#include "types.h"
#include <opencv2/opencv.hpp>

class ColorCalibrator {
protected:
    HSVColorRange range;
    cv::Mat image;
public:
    ColorCalibrator();
    void LoacImage(const cv::Mat &image, int numberOfObjects);
    HSVColorRange GetObjectThresholds (int index);
    ~ColorCalibrator();

};