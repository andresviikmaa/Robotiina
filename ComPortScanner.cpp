#include "ComPortScanner.h"
#include "simpleserial.h"
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/foreach.hpp>

ComPortScanner::ComPortScanner()
{
}

bool ComPortScanner::Verify(boost::asio::io_service &io_service) 
{
	bool ok = true;
	boost::property_tree::ptree ports;
	read_ini("conf/ports.ini", ports);
	BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, ports) {
		// v.first is the name of the child.
		// v.second is the child tree.
		std::stringstream portNum;
		//portNum << prefix << 
		portNum << (v.second).data();
		std::string _id = v.first;
		try {
			SimpleSerial port = SimpleSerial(io_service, portNum.str(), 115200);
			port.writeString("?\n");
			std::string id = port.readLineAsync();
			if (id.empty()) throw std::exception(("No ID received from port " + portNum.str()).c_str());
			if (id == "<id:0>") throw std::exception(("ID not set in port " + portNum.str()).c_str());
			if (id.substr(0, 4) != "<id:") throw std::exception(("Invalid ID " + id + " received from port " + portNum.str()).c_str());

			id = id.substr(4, 1);
			if (id != _id) throw std::exception((std::string("ID mismatch (old)'") + _id + "' != '" + id + "' (new) for port " + portNum.str()).c_str());
			std::cout << "Found port " << portNum.str() << ", id: " << id << std::endl;
			//portMap[atoi(id.c_str())] = portNum.str();
			ports.put(id, portNum.str());
		}
		catch (const std::exception &e){
			std::cout << "Port not accessible: " << portNum.str() << ", error: " << e.what() << std::endl;
			ok = false;
		}

	}
	return ok;
}

void ComPortScanner::Scan(boost::asio::io_service &io_service)
{
//	std::map<short, std::string> portMap;
	boost::property_tree::ptree ports;



	for (int i = 0; i < 20; i++) {
		std::stringstream portNum;
		portNum << prefix << i;
		try {
			SimpleSerial port = SimpleSerial(io_service, portNum.str(), 115200);
			port.writeString("?\n");
			std::string id = port.readLineAsync();
			if (id.empty()) throw std::exception(("No ID received from port " + portNum.str()).c_str());
			if (id == "<id:0>") throw std::exception(("ID not set in port " + portNum.str()).c_str());
			if (id.substr(0, 4) != "<id:") throw std::exception(("Invalid ID " + id + " received from port " + portNum.str()).c_str());
			
			id = id.substr(4, 1);
			std::cout << "Found port " << portNum.str() << ", id: " << id << std::endl;
			//portMap[atoi(id.c_str())] = portNum.str();
			ports.put(id, portNum.str());
		}
		catch (const std::exception &e){
			std::cout << "Port not accessible: " << portNum.str() << ", error: " << e.what() << std::endl;
		}
	}

	write_ini("conf/ports.ini", ports);



}

ComPortScanner::~ComPortScanner()
{
}
