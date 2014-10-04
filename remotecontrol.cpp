#include "types.h"
#include <boost/atomic.hpp>
#include <boost/asio.hpp>
#include "robot.h"
#include "remotecontrol.h"
#include <sstream>

using boost::asio::ip::udp;
RemoteControl::RemoteControl( boost::asio::io_service &io, Robot * robot):io(io),robot(robot) {

}

void RemoteControl::Start(){
    stop = false;

    threads.create_thread(boost::bind(&RemoteControl::loop, this));

}

void RemoteControl::Stop(){
    stop = true;
    threads.join_all();
}

std::string RemoteControl::respond(const std::string &query){
    std::stringstream response;
    STATE s = (STATE)robot->GetState();
    if(query == "s") response << s;
    else if(query == "r") robot->SetState(STATE_REMOTE_CONTROL);
    else if(query == "c") robot->SetState(STATE_LOCATE_BALL);
    else if(query == "b") robot->SetState(STATE_NONE);
    else if(query == "z") robot->SetState(STATE_END_OF_GAME);
    else if (STATE_REMOTE_CONTROL == s) robot->remote_command = query[0];
    return response.str();
}

void RemoteControl::loop(){

    udp::socket socket(io, udp::endpoint(udp::v4(), 10004));

    while (!stop)
    {
        boost::array<char, 1> recv_buf;
        udp::endpoint remote_endpoint;
        boost::system::error_code error;
        socket.receive_from(boost::asio::buffer(recv_buf),
                remote_endpoint, 0, error);

        if (error && error != boost::asio::error::message_size)
            throw boost::system::system_error(error);

        std::string message = respond(std::string(recv_buf.begin(), recv_buf.end()));

        boost::system::error_code ignored_error;
        socket.send_to(boost::asio::buffer(message),
                remote_endpoint, 0, ignored_error);
    }

}


