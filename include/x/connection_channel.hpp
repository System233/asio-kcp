// Copyright (c) 2022 github.com/System233
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// Copyright (c) 2022 github.com/System233
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef ASIO_KCP_UDP_H
#define ASIO_KCP_UDP_H

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
namespace asiokcp{
    using boost::asio::ip::udp;
    class connection;
    class channel
    {
    protected:

        boost::asio::io_context &m_io_context;
        udp::socket m_socket;
        udp::endpoint m_remote_endpoint;
        boost::array<char, 0x10000> m_recv_buffer;
        size_t m_timeout=10000;
        bool stopped=false;
        void start_recvice()
        {
            m_socket.async_receive_from(
                boost::asio::buffer(m_recv_buffer),
                m_remote_endpoint,
                boost::bind(&channel::handle_recvice, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        }
        void handle_recvice(boost::system::error_code const &error, size_t bytes_transferred)
        {
            if (!error)
            {
                udp_handle(boost::asio::const_buffer(m_recv_buffer.data(), bytes_transferred), m_remote_endpoint);
            }
            if(!stopped){
                start_recvice();
            }
        }
        
        void udp_handle(boost::asio::const_buffer buffer,udp::endpoint endpoint){

        }

    public:
        
        channel(boost::asio::io_context &io_context, udp::endpoint const &endpoint) : m_io_context(io_context), m_socket(io_context, endpoint)
        {
        }
        virtual ~channel(){};
        udp::endpoint local_endpoint() const { return m_socket.local_endpoint(); }
        auto udp_send(boost::asio::const_buffer const &buffer, udp::endpoint const &endpoint)
        {
            return m_socket.async_send_to(buffer, endpoint, boost::asio::use_future);
        }
        std::future<size_t> send(boost::asio::const_buffer const &buffer, udp::endpoint const &endpoint)
        {
            return udp_send(buffer, endpoint);
        }

        void start()
        {
            stopped=false;
            start_recvice();
        }
        void stop(){
            stopped=true;
        }
        boost::asio::io_context&io_context()const{return m_io_context;}
        size_t timeout()const{return m_timeout;}
        void timeout(size_t time){m_timeout=time;}
    };
}

#endif