#include "types.h"
#include "camera.h"
#include "objectfinder.h"

class Robot {
protected:
    Camera camera;
    ObjectFinder objectFinder;
    std::map<OBJECTS, HSVColorRange> objectThresholds;
    void CalibrateObjects();
public:
    Robot();
    int run();
    ~Robot(){ }
};
