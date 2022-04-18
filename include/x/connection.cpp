// Copyright (c) 2022 github.com/System233
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <channel.hpp>
#include <connection.hpp>

namespace asiokcp
{

    connection::connection(channel &channel, udp::endpoint endpoint, uint32_t conv_id) : m_channel(channel),
                                                                                         m_endpoint(endpoint),
                                                                                         m_timeout_timer(channel.io_context()),
                                                                                         m_update_timer(channel.io_context()),
                                                                                         m_kcp(ikcp::create(conv_id, this))
    {
        m_kcp->setoutput(kcp_output);
        start_update();
        update_timeout();
    }

    void connection::update_timeout()
    {
        m_timeout_timer.expires_from_now(std::chrono::milliseconds(channel().timeout()));
        m_timeout_timer.async_wait(boost::bind(&connection::timeout, this, boost::asio::placeholders::error));
    }
    void connection::timeout(boost::system::error_code const &error)
    {
        if (!error)
        {
        }
    }
    void connection::start_update()
    {
        auto delay = m_kcp->check(now()) - now();
        m_update_timer.expires_from_now(std::chrono::milliseconds(delay));
        m_update_timer.async_wait(boost::bind(&connection::update, this, boost::asio::placeholders::error));
    }
    void connection::update(boost::system::error_code const &error)
    {
        if (!error)
        {
            m_kcp->update(now());
        }
        start_update();
    }
    udp::endpoint const &connection::endpoint() const
    {
        return m_endpoint;
    }

    size_t connection::time() const
    {
        return m_last_time;
    }

    virtual void connection::send(boost::asio::const_buffer buffer) const
    {
        m_kcp->send(buffer.data(), buffer.size());
    }
    virtual void connection::input(boost::asio::const_buffer buffer)
    {
        m_kcp->input(buffer.data(), buffer.size());
        update_timeout();
    }

    static void connection::kcp_output(const char *buf, int len, ikcpcb *kcp, void *user)
    {
        connection *conn = dynamic_cast<connection *>(user);
        conn->output(boost::asio::const_buffer(buf, len));
    }

    void connection::output(boost::asio::const_buffer buffer)
    {
        m_channel.send(buffer, endpoint());
    }

    udp_base &connection::sender()
    {
        return m_ctx;
    }

    boost::asio::steady_timer &connection::loop()
    {
        return m_loop;
    }

    static uint32_t connection::now()
    {
        return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    }
}