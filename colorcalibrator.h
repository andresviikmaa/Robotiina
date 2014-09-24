#include "types.h"
#include <opencv2/opencv.hpp>

class ColorCalibrator {
public:
    ColorCalibrator(){ };
    void ClusterImage(const cv::Mat &image, int numberOfClusters){};
    HSVColorRange GetObjectThresholds (int index) {};
    ~ColorCalibrator(){ }

};