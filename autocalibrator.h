#pragma once
#include "colorcalibrator.h"

class AutoCalibrator: public ColorCalibrator {
public:
    AutoCalibrator();
    void LoadImage(const cv::Mat &image);
    HSVColorRange GetObjectThresholds(int index, const std::string &name);

    ~AutoCalibrator();
protected:
    cv::Mat bestLabels, clustered;
    cv::Mat centers;
    void DetectThresholds(int number_of_objects);
    void mouseClicked(int x, int y);
private:
    bool done;

};