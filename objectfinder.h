#include "types.h"
#include <opencv2/opencv.hpp>



class ObjectFinder {
public:
    ObjectFinder(){ };
    CvPoint Locate(const HSVColorRange &HSVRange);
    ~ObjectFinder(){ }

};
