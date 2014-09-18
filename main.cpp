#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

std::string color = "green";

int main( int argc, char** argv )
{
    VideoCapture cap(0); //capture the video from web cam

    if ( !cap.isOpened() )  // if not success, exit program
    {
        cout << "Cannot open the web cam" << endl;
        return -1;
    }

    namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
    int iLowH = 0;
    int iHighH = 179;


    int iLowS = 0;
    int iHighS = 255;


    int iLowV = 0;
    int iHighV = 255;


    if (color == "orange"){
         iLowH = 0;
         iHighH = 38;

         iLowS = 143;
         iHighS = 205;

         iLowV = 148;
         iHighV = 230;
    }
    if (color == "green"){
        iLowH =77;
        iHighH = 107;

        iLowS = 149;
        iHighS = 217;

        iLowV = 47;
        iHighV = 153;
    }


    //Create trackbars in "Control" window
    cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
    cvCreateTrackbar("HighH", "Control", &iHighH, 179);


    cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
    cvCreateTrackbar("HighS", "Control", &iHighS, 255);


    cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
    cvCreateTrackbar("HighV", "Control", &iHighV, 255);




    while (true)
    {
        Mat imgOriginal;

        bool bSuccess = cap.read(imgOriginal); // read a new frame from video

        if (!bSuccess) //if not success, break loop
        {
            cout << "Cannot read a frame from video stream" << endl;
            break;
        }

        Mat imgHSV;

        cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

        Mat imgThresholded;

        inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image
        Mat dst(imgThresholded.rows,imgThresholded.cols, CV_8U,Scalar::all(0));



        //biggest area
        vector<vector<Point> > contours; // Vector for storing contour
        vector<Vec4i> hierarchy;
        int largest_area=0;
        int largest_contour_index=0;
        Rect bounding_rect;
        findContours( imgThresholded, contours, hierarchy,CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE ); // Find the contours in the image
        for( int i = 0; i< contours.size(); i++ ) // iterate through each contour.
        {
            double a=contourArea( contours[i],false);  //  Find the area of contour
            if(a>largest_area){
                largest_area=a;
                largest_contour_index=i;                //Store the index of largest contour
                bounding_rect=boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
            }
        }
        Scalar color( 255,255,255);


        drawContours( dst, contours,largest_contour_index, color, CV_FILLED, 8, hierarchy ); // Draw the largest contour using previously stored index.

        //find center
        Scalar colorCircle(133,33,55);
        Point2f center;
        if (contours.size() > largest_contour_index){
            Moments M = moments(contours[largest_contour_index]);
            center = Point2f( M.m10/M.m00 , M.m01/M.m00 );
            cout << center.x << ", " << center.y << endl;
        }


        //Draw circle
        circle(dst, center, 10, colorCircle, 3);

        imshow("Thresholded Image", dst); //show the thresholded image

        if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed by user" << endl;
            break;
        }
    }

    return 0;

}
