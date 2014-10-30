#pragma  once
#include "types.h"
#include <opencv2/opencv.hpp>


class ObjectFinder {
protected:
	virtual cv::Point2f LocateOnScreen(HSVColorRangeMap &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target);
	ObjectPosition ConvertPixelToRealWorld(const cv::Point2f &point, const cv::Point2i &frame_size);
	bool validateBall(HSVColorRangeMap &HSVRanges, cv::Point2f point, cv::Mat &frameHSV, cv::Mat &frameBGR);
private:
	void WriteInfoOnScreen(const ObjectPosition &info);
	//Vars
	float Hfov = 35.21;
	float Vfov = 21.65; //half of cameras vertical field of view (degrees)
	float CamHeight = 345; //cameras height from ground (mm)
	float CamAngleDev = 26; //deviation from 90* between ground
	
public:
	ObjectFinder();
	virtual bool Locate(HSVColorRangeMap &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target, ObjectPosition &targetPos);
	virtual void IsolateField(HSVColorRangeMap &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR);
    virtual ~ObjectFinder(){ }

};
