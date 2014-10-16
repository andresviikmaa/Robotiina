#pragma once
#include "types.h"
#include <boost/asio.hpp>

class ComPortScanner
{
private:
#ifdef WIN32
	const std::string prefix = "COM";
#else
	const std::string prefix = "/dev/ttyACM";
#endif

public:
	ComPortScanner();
	bool Verify(boost::asio::io_service &io_service);
	void Scan(boost::asio::io_service &io_service);
	~ComPortScanner();
};
