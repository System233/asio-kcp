// Copyright (c) 2022 github.com/System233
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef ASIO_UDP_CHANNEL_H
#define ASIO_UDP_CHANNEL_H
#include <boost/asio.hpp>
#include <thread>
#include <memory>
#include <string>
#include <iostream>

namespace iudp
{
    using boost::asio::ip::udp;
    class udp_channel
    {
        void start_recvice()
        {
            m_socket.async_receive_from(
                boost::asio::buffer(m_recv_buffer),
                m_remote_endpoint,
                boost::bind(&udp_channel::handle_recvice, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        }
        void handle_recvice(boost::system::error_code const &error, size_t bytes_transferred)
        {
            if (!error)
            {
                udp_handle(boost::asio::const_buffer(m_recv_buffer.data(), bytes_transferred), m_remote_endpoint);
            }
            if (!stopped)
            {
                start_recvice();
            }
        }

        void udp_handle(boost::asio::const_buffer buffer, udp::endpoint const &endpoint)
        {
            m_handler(buffer, endpoint);
        }
        void start_thread(){
            if(!m_worker_thread.joinable()){
                m_worker_thread=std::thread(
                [this]()
                {
                    start_recvice();
                    stopped = false;
                    m_io_context.run();
                });
            }
        }
        void stop_thread(){
            if(!m_io_context.stopped()){
                m_io_context.stop();
                if (m_worker_thread.joinable())
                {
                    m_worker_thread.join();
                    stopped = true;
                }
            }
        }
    public:
        using handler_t = std::function<void(boost::asio::const_buffer buffer, udp::endpoint endpoint)>;
        udp_channel(udp::endpoint const &endpoint, handler_t handler)
            : m_io_context(),
              m_socket(m_io_context, endpoint),
              m_handler(handler)
        {
        }
        ~udp_channel(){
            stop();
        }
        udp::endpoint local_endpoint() const
        {
            return m_socket.local_endpoint();
        }
        auto udp_send(boost::asio::const_buffer const &buffer, udp::endpoint const &endpoint)
        {
            return m_socket.async_send_to(buffer, endpoint, boost::asio::use_future);
        }
        void start()
        {
            start_thread();
        }
        void stop()
        {
            stop_thread();
        }

    private:
        boost::asio::io_context m_io_context;
        udp::socket m_socket;
         std::thread m_worker_thread;
        udp::endpoint m_remote_endpoint;
        boost::array<char, 0x10000> m_recv_buffer;
        handler_t m_handler;
        bool stopped = true;
    };
}

#endif // ASIO_UDP_CHANNEL_H