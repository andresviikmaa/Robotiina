#include "types.h"
#include "objectfinder.h"

#include <boost/program_options.hpp>
#include <boost/asio.hpp>

namespace po = boost::program_options;

class Robot {
private:
	po::variables_map config;

    ICamera *camera;
    std::map<OBJECT, HSVColorRange> objectThresholds;
    STATE state = STATE_NONE;
    void CalibrateObjects(bool autoCalibrate = false);
	bool ParseOptions(int argc, char* argv[]);

	void Run();
protected:
	boost::asio::io_service io;

public:
    Robot();
	bool Launch(int argc, char* argv[]);
	~Robot();
};
