#include "kalmanFilter.h"

KalmanFilter::KalmanFilter(cv::Point2i &startPoint){
	KF.transitionMatrix = *(cv::Mat_<float>(4, 4) << 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1);
	
	measurement(0) = startPoint.x;
	measurement(1) = startPoint.y;
	estimated(0) = startPoint.x;
	estimated(1) = startPoint.y;

	KF.statePre.at<float>(0) = 0;
	KF.statePre.at<float>(1) = 0;
	KF.statePre.at<float>(2) = 0;
	KF.statePre.at<float>(3) = 0;

	KF.statePost.at<float>(0) = startPoint.x;
	KF.statePost.at<float>(1) = startPoint.y;
	KF.statePost.at<float>(2) = startPoint.x;
	KF.statePost.at<float>(3) = startPoint.y;


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
	estimated = KF.correct(measurement);
	cv::Point statePt(estimated.at<float>(0), estimated.at<float>(1));
	
	return statePt;
}

cv::Point2i KalmanFilter::getPrediction(){
	cv::Mat prediction = KF.predict();
	cv::Point predictPt(prediction.at<float>(0), prediction.at<float>(1));
	return predictPt;
}
