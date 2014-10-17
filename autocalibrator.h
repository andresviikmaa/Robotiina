#pragma once
#include "colorcalibrator.h"

class AutoCalibrator: public ColorCalibrator {
public:
    AutoCalibrator();
    void LoadImage(const cv::Mat &image);
    HSVColorRange GetObjectThresholds(int index, const std::string &name);

    ~AutoCalibrator();
protected:
    cv::Mat bestLabels, clustered, centers;
    void DetectThresholds(int number_of_objects);
	void mouseClicked(int x, int y, int flags);
private:
    bool done;
	std::string name;


};