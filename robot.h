#include "types.h"
#include "objectfinder.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

class Robot {
private:
	po::variables_map config;

    ICamera *camera;
    std::map<OBJECT, HSVColorRange> objectThresholds;
    STATE state = LOCATE_BALL;
    void CalibrateObjects();
	bool ParseOptions(int argc, char* argv[]);

	void Run();

public:
    Robot();
	bool Launch(int argc, char* argv[]);
	~Robot();
};
