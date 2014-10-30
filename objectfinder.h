#pragma  once
#include "types.h"
#include <opencv2/opencv.hpp>


class ObjectFinder {
protected:
	virtual cv::Point2i LocateOnScreen(HSVColorRangeMap &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target);
	ObjectPosition ConvertPixelToRealWorld(const cv::Point2i &point, const cv::Point2i &frame_size);
	bool validateBall(HSVColorRangeMap &HSVRanges, cv::Point2d point, cv::Mat &frameHSV, cv::Mat &frameBGR);
private:
	void WriteInfoOnScreen(const ObjectPosition &info);
	//Vars
	double Hfov = 35.21;
	double Vfov = 21.65; //half of cameras vertical field of view (degrees)
	double CamHeight = 345; //cameras height from ground (mm)
	double CamAngleDev = 26; //deviation from 90* between ground
	
public:
	ObjectFinder();
	virtual bool Locate(HSVColorRangeMap &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target, ObjectPosition &targetPos);
	virtual void IsolateField(HSVColorRangeMap &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR);
    virtual ~ObjectFinder(){ }

};
