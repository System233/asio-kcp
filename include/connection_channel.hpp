// Copyright (c) 2022 github.com/System233
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#ifndef ASIO_CONNECTION_CHANNEL_H
#define ASIO_CONNECTION_CHANNEL_H
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <connection_manager.hpp>
// #include <channel_queue.hpp>
namespace iudp
{
    using boost::asio::ip::udp;

    enum channel_event
    {
        unknown,
        initialize,
        connect,
        disconnect,
        timeout,
        data,
        user_envet
    };
    template <class protocol>
    class connection_channel
    {

        void start_recvice()
        {
            m_socket.async_receive_from(
                boost::asio::buffer(m_recv_buffer),
                m_remote_endpoint,
                boost::bind(&connection_channel::handle_recvice, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        }
        void handle_recvice(boost::system::error_code const &error, size_t bytes_transferred)
        {
            if (!error)
            {
                udp_handle(boost::asio::const_buffer(m_recv_buffer.data(), bytes_transferred), m_remote_endpoint);
                if (!stopped)
                {
                    start_recvice();
                }
            }
          
        }

        void udp_handle(boost::asio::const_buffer buffer, udp::endpoint endpoint)
        {
            // m_queue.push(std::make_pair(std::string((char const*)buffer.data(),buffer.size()),endpoint));
            auto key = protocol::id(buffer);
            auto conn = m_connection_manager.get_connection(endpoint, key);
            if (!conn)
            {
                conn = m_connection_manager.create_connection(endpoint, key);
            }
            if (conn)
            {
                boost::asio::const_buffer ready = conn->io(buffer);
                if (ready.size())
                {
                    dispatch(conn, channel_event::data, ready);
                }
            }
        }

    public:
        using handler_t = std::function<bool(connection<protocol> *conn, channel_event event, boost::asio::const_buffer const &buffer)>;

        connection_channel(boost::asio::io_context &io_context,
                udp::endpoint const &endpoint,
                handler_t handler) : m_io_context(io_context),
                                     m_socket(io_context, endpoint),
                                     m_handler(handler),
                                     m_connection_manager(this)
        {
        }
        udp::endpoint local_endpoint() const
        {
            return m_socket.local_endpoint();
        }
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
            stopped = false;
            start_recvice();
        }
        void stop()
        {
            stopped = true;
        }
        boost::asio::io_context &io_context() const
        {
            return m_io_context;
        }

        void dispatch(connection<protocol> *conn, channel_event event, boost::asio::const_buffer const &buffer)
        {
            auto result=m_handler(conn, event, buffer);
            if(result){
                return;
            }
            switch (event)
            {
            case channel_event::disconnect:
                m_connection_manager.remove_connection(conn);
                break;
            case channel_event::timeout:
                dispatch(conn,channel_event::disconnect);
                break;
            default:
                break;
            }
        }
        void dispatch(connection<protocol> *conn, channel_event event)
        {
            boost::asio::const_buffer buffer(nullptr, 0);
            dispatch(conn, event,buffer);
        }
        template <class F>
        void post(F func)
        {
            io_context().post(func);
        }

        void connect(udp::endpoint endpoint, intptr_t id = 0)
        {
            m_connection_manager.create_connection(endpoint, id);
        }
        void disconnect(udp::endpoint endpoint, intptr_t id = 0)
        {
            m_connection_manager.remove_connection(endpoint, id);
        }

        handler_t handler() const
        {
            return &m_handler;
        }
        void handler(handler_t h)
        {
            m_handler = h;
        }

        size_t timeout() const
        {
            return m_timeout;
        }
        void timeout(size_t time)
        {
            m_timeout = time;
        }

    private:
        connection_manager<protocol> m_connection_manager;
        // channel_queue<std::string> m_queue;
        boost::asio::io_context &m_io_context;
        udp::socket m_socket;
        udp::endpoint m_remote_endpoint;
        boost::array<char, 0x10000> m_recv_buffer;
        handler_t m_handler;
        size_t m_timeout = 10000;
        bool stopped = false;
    };

}

#endif // ASIO_CONNECTION_CHANNEL_H