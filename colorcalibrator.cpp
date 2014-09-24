#include "colorcalibrator.h"

ColorCalibrator::ColorCalibrator()
{
    cvNamedWindow("ColorCalibrator", CV_WINDOW_AUTOSIZE); //create a window called "Control"
    cvCreateTrackbar("LowH", "Control", &range.hue.low, 179); //Hue (0 - 179)
    cvCreateTrackbar("HighH", "Control", &range.hue.high, 179);


    cvCreateTrackbar("LowS", "Control", &range.sat.low, 255); //Saturation (0 - 255)
    cvCreateTrackbar("HighS", "Control", &range.sat.high, 255);


    cvCreateTrackbar("LowV", "Control", &range.val.low, 255); //Value (0 - 255)
    cvCreateTrackbar("HighV", "Control", &range.val.high, 255);


};

void ColorCalibrator::LoacImage(const cv::Mat &image, int numberOfObjects)
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
        cv::Mat dst(imgThresholded.rows, imgThresholded.cols, CV_8U, cv::Scalar::all(0));

        cv::imshow("Thresholded Image", dst); //show the thresholded image

        if (cv::waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            std::cout << "esc key is pressed by user" << std::endl;
            return range;
        }
    }

};

ColorCalibrator::~ColorCalibrator(){
    cvDestroyWindow("ColorCalibrator");
}