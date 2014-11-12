#include "kalmanFilter.h"

KalmanFilter::KalmanFilter(){
	KF.transitionMatrix = *(cv::Mat_<float>(4, 4) << 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1);
	
	measurement.setTo(cv::Scalar(0));

	KF.statePre.at<float>(0) = 0;
	KF.statePre.at<float>(1) = 0;
	KF.statePre.at<float>(2) = 0;
	KF.statePre.at<float>(3) = 0;

	setIdentity(KF.measurementMatrix);
	setIdentity(KF.processNoiseCov, cv::Scalar::all(1e-4));
	setIdentity(KF.measurementNoiseCov, cv::Scalar::all(.1));
	setIdentity(KF.errorCovPost, cv::Scalar::all(.1));
}

cv::Point2i KalmanFilter::doFiltering(cv::Point2i &point){
	// First predict, to update the internal statePre variable
	cv::Mat prediction = KF.predict();
	cv::Point predictPt(prediction.at<float>(0), prediction.at<float>(1));

	//Get point
	measurement(0) = point.x;
	measurement(1) = point.y;

	//The update phase
	cv::Mat estimated = KF.correct(measurement);

	cv::Point statePt(estimated.at<float>(0), estimated.at<float>(1));
	
	return statePt;
}

