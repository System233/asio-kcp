// Copyright (c) 2022 github.com/System233
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef ASIO_KCP_CONNECTION_H
#define ASIO_KCP_CONNECTION_H
#include <unordered_map>
#include <boost/asio.hpp>
namespace asiokcp{
    using boost::asio::ip::udp;
    class channel;
    class connection;

    class connection_manager{
        channel&m_channel;
        std::unordered_map<std::pair<uint32_t,udp::endpoint>,std::unique_ptr<connection>> m_endpoints;
    public:
        connection_manager(channel&ch);
        connection* create_connection(uint32_t conv_id,udp::endpoint const&endpoint);
        bool remove_connection(connection const*conn);
        connection* get_connection(uint32_t conv_id,udp::endpoint const&endpoint);
        void handle(boost::asio::const_buffer buffer);
    };
}

#endif
