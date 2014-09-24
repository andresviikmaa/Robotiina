#include "types.h"
#include "objectfinder.h"

class Robot {
protected:
    ICamera *camera;
    ObjectFinder objectFinder;
    std::map<OBJECTS, HSVColorRange> objectThresholds;
    void CalibrateObjects();
public:
    Robot();
    int run();
    ~Robot();
};
