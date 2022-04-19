// Copyright (c) 2022 github.com/System233
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <connection_channel.hpp>
#include <connection_manager.hpp>
#include <connection.hpp>
#include <iostream>
#include <string>
#include "kcp_protocol.hpp"
#include "raw_protocol.hpp"
using boost::asio::ip::udp;
using namespace iudp;

#include "utils.h"
char test[1000];
using use_protocol=kcp_protocol;
int main(int argc, char const *argv[])
{
    const char*addr="127.0.0.1";
    int port=10001;
    if(argc>1){
        addr=argv[1];
    }
    try
    {
        boost::asio::io_context io_context;
        udp::resolver resolver(io_context);
        tick_counter count_timer(io_context);
        auto handlder = [&](connection<use_protocol> *conn, channel_event event, boost::asio::const_buffer buffer)
        {
            switch (event)
            {
            case channel_event::initialize:
                std::cout << "initialize:" << conn->endpoint() << std::endl;
 
                // conn->send(boost::asio::const_buffer("hello!",7));
                
                for(auto i=0;i<10000;++i)
                conn->send(boost::asio::const_buffer(test,sizeof(test)));
                break;

            case channel_event::connect:
                std::cout << "connect:" << conn->endpoint() << std::endl;
                break;
            case channel_event::disconnect:
                std::cout << "disconnect:" << conn->endpoint() << std::endl;
                break;
            case channel_event::timeout:
                std::cout << "timeout:" << conn->endpoint() << std::endl;
                break;
            case channel_event::data:
                count_timer.add(buffer.size());
                conn->send(buffer);
                break;
            default:
                break;
            }
            return false;
        };
        connection_channel<use_protocol> channel(io_context, udp::endpoint(), handlder);
        channel.connect(udp::endpoint(boost::asio::ip::make_address(addr), port));
        channel.start();
        io_context.run();
    }
    catch (std::exception &err)
    {
        std::cerr << "error:" << err.what() << std::endl;
    }
    return 0;
}
