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

// using use_protocol=raw_protocol;
using use_protocol=kcp_protocol;
#include "utils.h"
int main(int argc, char const *argv[])
{
    try
    {

        int port = 10001;
        boost::asio::io_context io_context;
        tick_counter count_timer(io_context);
        auto handlder = [&](connection<use_protocol> *conn, channel_event event, boost::asio::const_buffer buffer)
        {
            switch (event)
            {
            case channel_event::initialize:
                std::cout << "initialize:" << conn->endpoint() << std::endl;

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
                // conn->send(buffer);
                // for(auto i=0;i<100;++i)
                conn->send(buffer);
                // std::cout << "data:" << std::string((char const *)buffer.data(), buffer.size()) << std::endl;

                break;
            default:
                break;
            }
            return false;
        };
        connection_channel<use_protocol> channel(io_context, udp::endpoint(udp::v4(), port), handlder);
        channel.start();
        io_context.run();
    }
    catch (std::exception &err)
    {
        std::cerr << "error:" << err.what() << std::endl;
    }
    return 0;
}
