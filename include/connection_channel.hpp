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
#include <udp_channel.hpp>

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

    public:
        using connection_channel_t = connection_channel<protocol>;
        using connection_manager_t = connection_manager<protocol>;
        using connection_t = connection<protocol>;
        using handler_t = std::function<bool(connection_t *conn, channel_event event, boost::asio::const_buffer const &buffer)>;

        connection_channel(boost::asio::io_context &io_context,
                           udp::endpoint const &endpoint,
                           handler_t handler) :
                           m_channel_context(),
                           m_io_context(io_context),
                                                m_udp_channel(m_channel_context,endpoint, [this](auto buffer, auto endpoint)
                                                              { udp_handle(buffer, endpoint); }),
                                                m_handler(handler),
                                                m_connection_manager(this)
        {
        }

        std::future<size_t> send(boost::asio::const_buffer const &buffer, udp::endpoint const &endpoint)
        {
            return udp_send(buffer, endpoint);
        }
        auto udp_send(boost::asio::const_buffer const &buffer, udp::endpoint const &endpoint)
        {
            return m_udp_channel.udp_send(buffer, endpoint);
        }
        void start()
        {
            stopped = false;
            m_udp_channel.start();
        }
        void stop()
        {
            m_udp_channel.stop();
            stopped = true;
        }
        boost::asio::io_context &io_context() const
        {
            return m_io_context;
        }

        boost::asio::io_context & channel_context(){
            return m_channel_context;
        };

        void dispatch(connection_t *conn, channel_event event, boost::asio::const_buffer const &buffer)
        {
            auto result = m_handler(conn, event, buffer);
            if (result)
            {
                return;
            }
            switch (event)
            {
            case channel_event::disconnect:
                disconnect(conn);
                break;
            case channel_event::timeout:
                dispatch(conn, channel_event::disconnect);
                break;
            default:
                break;
            }
        }
        void dispatch(connection_t *conn, channel_event event)
        {
            boost::asio::const_buffer buffer(nullptr, 0);
            dispatch(conn, event, buffer);
        }
        template <class F>
        void post(F func)
        {
            io_context().post(func);
        }

        void connect(udp::endpoint endpoint, intptr_t id = 0)
        {
            io_context().post(
                boost::bind(
                    &connection_manager_t::create_connection,
                    &m_connection_manager,
                    endpoint,
                    id));
        }
        void disconnect(udp::endpoint endpoint, intptr_t id = 0)
        {
            io_context().post(
                boost::bind(
                    &connection_manager_t::remove_connection,
                    &m_connection_manager,
                    endpoint,
                    id));
        }
        void disconnect(connection_t *conn)
        {
            io_context().post(
                boost::bind(
                    &connection_manager_t::remove_connection,
                    &m_connection_manager,
                    conn));
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
        connection_manager_t const &connection_manager() const
        {
            return m_connection_manager;
        }

    private:
        void data_handle(std::string data, connection_t*conn)
        {
            boost::asio::const_buffer buffer(data.data(), data.size());
            dispatch(conn, channel_event::data, buffer);
//            io_context().post(boost::bind(&connection_channel_t::udp_handle,
//                                          this,
//                                          std::string(static_cast<const char *>(buffer.data()), buffer.size()),
//                                          endpoint));
        }

        void udp_handle(boost::asio::const_buffer buffer, udp::endpoint endpoint)
        {
//            boost::asio::const_buffer buffer(data.data(), data.size());
            auto key = protocol::id(buffer);
            auto conn = m_connection_manager.get_connection(endpoint, key);
            if (!conn)
            {
                conn = m_connection_manager.create_connection(endpoint, key);
            }
            if (conn)
            {
                size_t current = 0;
                while (current < buffer.size())
                {

                    current += conn->handle(boost::asio::const_buffer(((char const*)buffer.data())+current,buffer.size()-current),
                                            [&](auto buffer)
                                            {
                                                io_context().post(boost::bind(&connection_channel_t::data_handle,
                                                                              this,
                                                                              std::string(static_cast<const char *>(buffer.data()), buffer.size()),
                                                                              conn));
                                            });
                }
            }
        }
        boost::asio::io_context m_channel_context;
        connection_manager_t m_connection_manager;
        boost::asio::io_context &m_io_context;
        udp_channel m_udp_channel;
        // udp::socket m_socket;
        // udp::endpoint m_remote_endpoint;
        // boost::array<char, 0x10000> m_recv_buffer;
        handler_t m_handler;
        size_t m_timeout = 10000;
        bool stopped = false;
    };

}

#endif // ASIO_CONNECTION_CHANNEL_H