#pragma  once
#include "types.h"
#include <opencv2/opencv.hpp>

class ObjectFinder {
private:
	cv::Point2f LocateOnScreen(const HSVColorRange &HSVRange, const cv::Mat &frame);
	std::pair<int, double> ObjectFinder::ConvertPixelToRealWorld(const cv::Point2f &point, const cv::Point2i &frame_size);

	//Vars
	float Vfov = 21.65; //half of cameras vertical field of view (degrees)
	float CamHeight = 345; //cameras height from ground (mm)
	float CamAngleDev = 26; //deviation from 90* between ground

public:
    ObjectFinder();
	std::pair<int, double> Locate(const HSVColorRange &HSVRange, const cv::Mat &frame);
    ~ObjectFinder(){ }

};
