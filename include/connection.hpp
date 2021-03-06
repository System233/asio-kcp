// Copyright (c) 2022 github.com/System233
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef ASIO_CONNECTION_H
#define ASIO_CONNECTION_H
#include "connection_channel.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

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
        : m_io_context(ch->channel_context()),
            m_protocol_ptr(boost::make_shared<protocol>(io_context(), id, boost::bind(&connection_t::output, this,boost::placeholders::_1))),
            m_channel(ch),
            m_endpoint(endpoint),
            m_timeout_timer(io_context())
        {
            m_protocol_ptr->start();
            update_status(status::initialize);
        };
        ~connection(){
            m_protocol_ptr->stop();
        }
        udp::endpoint const &endpoint() const{
            return m_endpoint;
        };

        intptr_t id() const {
            return m_protocol_ptr->id();
        }

        size_t send(boost::asio::const_buffer buffer) const
        {
            return m_protocol_ptr->write(buffer);
        };

        size_t output(boost::asio::const_buffer buffer)
        {
            channel()->send(buffer, endpoint());//TODO: std::future???
            return buffer.size();
        };

        template<class T>
        void config(T const&conf){
            m_protocol_ptr->config(conf);
        }
        template<class T>
        T const& config()const{
            return m_protocol_ptr->config();
        }

        protocol*get(){
            return m_protocol_ptr.get();
        }

        connection_channel_t*channel() const { return m_channel; };
        size_t handle(boost::asio::const_buffer buffer,std::function<void(boost::asio::const_buffer buffer)>const&handler)
        {
            update_status(status::connect);
            update_timeout();
            return m_protocol_ptr->handle(buffer,handler);
        };
        boost::asio::io_context& io_context(){
            return m_io_context;
        };
        private:
        boost::asio::io_context&m_io_context;
        connection_channel_t *m_channel;
        boost::shared_ptr<protocol>m_protocol_ptr;
        // protocol m_protocol_ptr;
        udp::endpoint m_endpoint;
        boost::asio::steady_timer m_timeout_timer;
        status m_status=status::unknown;
    };
}

#endif // ASIO_CONNECTION_H