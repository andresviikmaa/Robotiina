#pragma  once
#include "types.h"
#include <opencv2/opencv.hpp>


class ObjectFinder {
protected:
	virtual cv::Point2f LocateOnScreen(const HSVColorRange &HSVRange, cv::Mat &frameHSV, cv::Mat &frameBGR, bool gate);
	cv::Point3d ConvertPixelToRealWorld(const cv::Point2f &point, const cv::Point2i &frame_size);
private:
	void WriteInfoOnScreen(const cv::Point3d &info);
	//Vars
	float Hfov = 35.21;
	float Vfov = 21.65; //half of cameras vertical field of view (degrees)
	float CamHeight = 345; //cameras height from ground (mm)
	float CamAngleDev = 26; //deviation from 90* between ground

public:
    ObjectFinder();
	virtual cv::Point3d Locate(const HSVColorRange &HSVRange, cv::Mat &frameHSV, cv::Mat &frameBGR, bool gate);
	virtual void IsolateField(const HSVColorRange &inner, const HSVColorRange &outer, cv::Mat &frameHSV, cv::Mat &frameBGR);
    virtual ~ObjectFinder(){ }

};
