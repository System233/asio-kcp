// Copyright (c) 2022 github.com/System233
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ikcp.hpp"
#include <protocol.hpp>
#include <iostream>
class kcp_protocol : public protocol<kcp_protocol>
{
    boost::asio::io_context &m_io_context;
    ikcp *m_kcp;
    output_func m_output;
    boost::array<char, 0x10000> m_buffer;
    boost::asio::steady_timer m_update_timer;
    bool m_stopped;
    static int kcp_output(const char *buf, int len, ikcpcb *kcp, void *user)
    {
        kcp_protocol *protocol = static_cast<kcp_protocol *>(user);
        return (int)protocol->m_output(boost::asio::const_buffer(buf, len));
    }
    void update(boost::system::error_code const &error){
        if(error==boost::system::errc::operation_canceled){
            return;
        }
        if(!error){
            m_kcp->update(now());
        }
        if(!m_stopped){
            start_update();
        }
    }
    void start_update(){
        auto time=now();
        auto delay=m_kcp->check(time)-time;
        m_update_timer.expires_from_now(std::chrono::milliseconds(delay));
        m_update_timer.async_wait(boost::bind(&kcp_protocol::update,shared_from_this(),boost::asio::placeholders::error));
    }
    static uint32_t now(){
        // return boost::posix_time::microsec_clock::universal_time().time_of_day().total_milliseconds();
        return (uint32_t)std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    }
public:
    kcp_protocol(boost::asio::io_context &io, intptr_t id, output_func func)
        : protocol(io, id, func),
          m_io_context(io),
          m_kcp(ikcp::create((uint32_t)id, this)),
          m_output(func),
          m_update_timer(io),
          m_stopped(false)
    {
        m_kcp->setoutput(kcp_output);
        m_kcp->nodelay(0, 40, 2, 1);
        m_kcp->wndsize(512,512);
        // m_kcp->rx_minrto=50;
    };
    void start(){
        m_stopped=false;
        start_update();
    }
    void stop(){
        m_stopped=true;
        m_update_timer.cancel();
    }
    ~kcp_protocol()
    {
        m_kcp->release();
    };
    virtual intptr_t id() const
    {
        return m_kcp->getconv();
    };
    virtual size_t handle(boost::asio::const_buffer buffer,std::function<void(boost::asio::const_buffer buffer)>const&handler){
        m_kcp->input(buffer.data(), buffer.size());
        while(auto len = m_kcp->recv(m_buffer.data(), m_buffer.size())){
            if(len<=0){
                break;
            }
            handler(boost::asio::const_buffer(m_buffer.data(), len));
        };
        return buffer.size();
    };


    virtual size_t write(boost::asio::const_buffer buffer) const
    {
        return m_kcp->send(buffer.data(), buffer.size());
    };
    static intptr_t id(boost::asio::const_buffer buffer)
    {
        return ikcp::getconv(buffer.data());
    }

    ikcp*raw(){
        return m_kcp;
    }
    template<class T>
    void config(T const&){

    };
    template<class T>
    T const& config()const{

    };
};