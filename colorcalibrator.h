#pragma once
#include "types.h"
#include <opencv2/opencv.hpp>

class ColorCalibrator {
protected:
    HSVColorRange range/* =  {{0,179},{0,255},{0,255}}*/;
    cv::Mat image;
	void LoadConf(const std::string &name);
	void SaveConf(const std::string &name);
public:
    ColorCalibrator();
    void LoadImage(const cv::Mat &image, int numberOfObjects);
	HSVColorRange GetObjectThresholds(int index, const std::string &name);
    ~ColorCalibrator();

};