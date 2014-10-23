#include "ComPortScanner.h"
#include "simpleserial.h"
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/foreach.hpp>

ComPortScanner::ComPortScanner()
{
}

bool ComPortScanner::Verify(boost::asio::io_service &io_service, const std::string &conf_file) 
{
	bool ok = true;
	boost::property_tree::ptree ports;
	try {
		read_ini(conf_file, ports);
	}
	catch (...) {
		std::cout << "Error reading old port configuration: " << std::endl;
		return false;
	}
	//BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, ports) {
	for (int i = ID_WHEEL_LEFT; i < ID_OBJECT_COUNT; i++) {
		// v.first is the name of the child.
		// v.second is the child tree.
		std::stringstream portNum;
		//portNum << prefix << 
		std::string _id = std::to_string(i); // v.first;
		try {
			portNum << ports.get<std::string>(std::to_string(i));//(v.second).data();
		}
		catch (...)
		{
			std::cout << "ID: " << _id << " not found in conf file" << std::endl;
			ok = false;
			continue;
		}
		try {
			SimpleSerial port = SimpleSerial(io_service, portNum.str(), 115200);
			port.writeString("?\n");
			std::string id = port.readLineAsync(500);
			if (id.empty()) throw std::runtime_error(("No ID received from port " + portNum.str()).c_str());
			if (id == "<id:0>") throw std::runtime_error(("ID not set in port " + portNum.str()).c_str());
			if (id.substr(0, 4) != "<id:") throw std::runtime_error(("Invalid ID " + id + " received from port " + portNum.str()).c_str());

			id = id.substr(4, 1);
			if (id != _id) throw std::runtime_error((std::string("ID mismatch (old)'") + _id + "' != '" + id + "' (new) for port " + portNum.str()).c_str());
			std::cout << "Found port " << portNum.str() << ", id: " << id << std::endl;
			//portMap[atoi(id.c_str())] = portNum.str();
			ports.put(id, portNum.str());
		}
		catch (std::runtime_error const&e){
			std::cout << "Port not accessible: " << portNum.str() << ", error: " << e.what() << std::endl;
			ok = false;
		}

	}
	return ok;
}

bool ComPortScanner::Scan(boost::asio::io_service &io_service)
{
//	std::map<short, std::string> portMap;
	boost::property_tree::ptree ports;


	bool ok = true;
	for (int i = 0; i < 20; i++) {
		std::stringstream portNum;
		portNum << prefix << i;
		try {
			SimpleSerial port = SimpleSerial(io_service, portNum.str(), 115200);
			port.writeString("?\n");
			std::string id = port.readLineAsync(500);
			if (id.empty()) throw std::runtime_error(("No ID received from port " + portNum.str()).c_str());
			if (id == "<id:0>") throw std::runtime_error(("ID not set in port " + portNum.str()).c_str());
			if (id.substr(0, 4) != "<id:") throw std::runtime_error(("Invalid ID " + id + " received from port " + portNum.str()).c_str());
			
			id = id.substr(4, 1);
			std::cout << "Found port " << portNum.str() << ", id: " << id << std::endl;
			//portMap[atoi(id.c_str())] = portNum.str();
			ports.put(id, portNum.str());
		}
		catch (std::runtime_error const&e){
			std::cout << "Port not accessible: " << portNum.str() << ", error: " << e.what() << std::endl;
			ok = false;
		}
	}
	if (true){
		write_ini("conf/ports_new.ini", ports);
	}
	if (Verify(io_service, "conf/ports_new.ini")) {
		unlink("conf/ports.ini");
		rename("conf/ports_new.ini", "conf/ports.ini");
		return true;
	}
	return false;



}

ComPortScanner::~ComPortScanner()
{
}
