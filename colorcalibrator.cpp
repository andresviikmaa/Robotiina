#include "colorcalibrator.h"

ColorCalibrator::ColorCalibrator()
{

    cvNamedWindow("ColorCalibrator", CV_WINDOW_AUTOSIZE); //create a window called "Control"
    cvCreateTrackbar("LowH", "ColorCalibrator", &range.hue.low, range.hue.high); //Hue (0 - 179)
    cvCreateTrackbar("HighH", "ColorCalibrator", &range.hue.high, range.hue.high);


    cvCreateTrackbar("LowS", "ColorCalibrator", &range.sat.low, range.sat.high); //Saturation (0 - 255)
    cvCreateTrackbar("HighS", "ColorCalibrator", &range.sat.high, range.sat.high);


    cvCreateTrackbar("LowV", "ColorCalibrator", &range.val.low, range.val.high); //Value (0 - 255)
    cvCreateTrackbar("HighV", "ColorCalibrator", &range.val.high, range.val.high);


};

void ColorCalibrator::LoadImage(const cv::Mat &image, int numberOfObjects)
{
    this->image = image;
};
HSVColorRange ColorCalibrator::GetObjectThresholds (int index)
{

    cv::Mat imgThresholded, imgHSV;
    cvtColor(image, imgHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV



    while (true)
    {
        //inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image
        cv::inRange(imgHSV, cv::Scalar(range.hue.low, range.sat.low, range.val.low), cv::Scalar(range.hue.high, range.sat.high, range.val.high), imgThresholded); //Threshold the image
        //cv::Mat dst(imgThresholded.rows, imgThresholded.cols, CV_8U, cv::Scalar::all(0));

        cv::imshow("Thresholded Image", imgThresholded); //show the thresholded image

        if (cv::waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            std::cout << "esc key is pressed by user" << std::endl;
            return range;
        }
    }

};

ColorCalibrator::~ColorCalibrator(){
    cvDestroyWindow("ColorCalibrator");
    cvDestroyWindow("Thresholded Image");
}