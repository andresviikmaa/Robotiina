#pragma  once
#include "types.h"
#include <opencv2/opencv.hpp>
#include "kalmanFilter.h"


class ObjectFinder {
protected:
	virtual cv::Point2i LocateGateOnScreen(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target);
	virtual cv::Point2i LocateBallOnScreen(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target, cv::Point2d &lastPosition);
	ObjectPosition ConvertPixelToRealWorld(const cv::Point2i &point, const cv::Point2i &frame_size);
	bool validateBall(ThresholdedImages &HSVRanges, cv::Point2d point, cv::Mat &frameHSV, cv::Mat &frameBGR);
private:
	void WriteInfoOnScreen(const ObjectPosition &info);
	KalmanFilter* filter = new KalmanFilter(cv::Point2i (40, 40));
	//Vars
	double Hfov = 35.21;
	double Vfov = 21.65; //half of cameras vertical field of view (degrees)
	double CamHeight = 345; //cameras height from ground (mm)
	double CamAngleDev = 26; //deviation from 90* between ground
	cv::Point2d &lastPosition = cv::Point2d(-1, -1);
public:
	ObjectFinder();
	virtual bool Locate(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target, ObjectPosition &targetPos);
	virtual void IsolateField(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR);
    virtual ~ObjectFinder(){ }

};
