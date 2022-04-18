// Copyright (c) 2022 github.com/System233
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <connection_manager.hpp>

namespace asiokcp{
    connection* connection_manager::create_connection(uint32_t conv_id,udp::endpoint const&endpoint);
    bool connection_manager::remove_connection(connection const*conn);
    connection* connection_manager::get_connection(uint32_t conv_id,udp::endpoint const&endpoint);
    void handle(boost::asio::const_buffer buffer);
}