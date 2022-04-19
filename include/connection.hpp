// Copyright (c) 2022 github.com/System233
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef ASIO_CONNECTION_H
#define ASIO_CONNECTION_H
#include "connection_channel.hpp"
#include <iostream>
namespace iudp
{
    template <class protocol>
    class connection_channel;

    
    template <class protocol>
    class connection
    {
        enum status
        {
            unknown,
            initialize,
            connect,
            disconnect,
            timeout
        };

        void update_timeout()
        {
            m_timeout_timer.expires_from_now(std::chrono::milliseconds(channel()->timeout()));
            m_timeout_timer.async_wait(boost::bind(&connection_t::on_timeout, this, boost::asio::placeholders::error));
        }
        void on_timeout(boost::system::error_code const &error)
        {
            if (!error)
            {
                update_status(status::timeout);
            }
        }
        void update_status(status st)
        {
            if (m_status != st)
            {
                m_status = st;
                channel_event event = channel_event::unknown;
                switch (st)
                {
                case initialize:
                    event = channel_event::initialize;
                    break;
                case connect:
                    event = channel_event::connect;
                    break;
                case disconnect:
                    event = channel_event::disconnect;
                    break;
                case timeout:
                    event = channel_event::timeout;
                    break;
                default:
                    break;
                }
                channel()->post([this, event]()
                                { channel()->dispatch(this, event); });
            }
        }

    public:
        using connection_channel_t=connection_channel<protocol>;
        using connection_t=connection<protocol>;
        connection(connection_channel_t *ch, udp::endpoint endpoint, intptr_t id) 
        : m_protocol(ch->io_context(), id, boost::bind(&connection_t::output, this,boost::placeholders::_1)),
            m_channel(ch),
            m_endpoint(endpoint),
            m_timeout_timer(ch->io_context())
        {
            update_status(status::initialize);
        };

        udp::endpoint const &endpoint() const{
            return m_endpoint;
        };

        intptr_t id() const {
            return m_protocol.id();
        }

        size_t send(boost::asio::const_buffer buffer) const
        {
            return m_protocol.write(buffer);
        };

        size_t output(boost::asio::const_buffer buffer)
        {
            channel()->send(buffer, endpoint());//TODO: std::future???
            return buffer.size();
        };

        template<class T>
        void config(T const&conf){
            m_protocol.config(conf);
        }
        template<class T>
        T const& config()const{
            return m_protocol.config();
        }

        protocol*get(){
            return &m_protocol;
        }

        connection_channel_t*channel() const { return m_channel; };
        boost::asio::const_buffer io(boost::asio::const_buffer buffer)
        {
            update_status(status::connect);
            update_timeout();
            return m_protocol.io(buffer);
        };
        private:
        
        connection_channel_t *m_channel;
        protocol m_protocol;
        udp::endpoint m_endpoint;
        boost::asio::steady_timer m_timeout_timer;
        status m_status=status::unknown;
    };
}

#endif // ASIO_CONNECTION_H