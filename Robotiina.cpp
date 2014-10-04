// Robotiina.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "robot.h"
#include "remotecontrol.h"


int main(int argc, char *argv[])
{
    boost::asio::io_service io;
	try
	{
		Robot robotiina(io);
        RemoteControl sr(io, &robotiina);
        sr.Start();

		robotiina.Launch(argc, argv);
        sr.Stop();
        return 0;
    }
	catch (std::exception &e)
	{
		std::cout << "ups, " << e.what() << std::endl;
	}
	catch (const std::string &e)
	{
		std::cout << "ups, " << e << std::endl;
	}
	catch (...)
	{
		std::cout << "ups, did not see that coming."<< std::endl;
	}

}
