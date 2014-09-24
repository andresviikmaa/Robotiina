#include <opencv2/opencv.hpp>

typedef std::pair<CvPoint3D, CvPoint3D> ColorRange;

class ObjectFinder {
public:
    ObjectFinder(){ };
    CvPoint Locate(const ColorRange &Hue, const ColorRange &Sat, const ColorRange &Val);
    ~ObjectFinder(){ }

};
