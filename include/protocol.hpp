// Copyright (c) 2022 github.com/System233
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef ASIO_PROTOCOL_H
#define ASIO_PROTOCOL_H
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <functional>
#include <memory>
#include <functional>
template<class P>
class protocol:public boost::enable_shared_from_this<P>{
public:
    using output_func=std::function<size_t(boost::asio::const_buffer buffer)>;
    protocol(boost::asio::io_context&io,intptr_t id, output_func func){};
    virtual ~protocol(){};
    virtual intptr_t id()const=0;
    virtual void start(){};
    virtual void stop(){};

    template<class T>
    void config(T const&);
    template<class T>
    T const& config()const;

    virtual size_t handle(boost::asio::const_buffer buffer,std::function<void(boost::asio::const_buffer buffer)>const&handler)=0;
    virtual size_t write(boost::asio::const_buffer buffer)const=0;
    static intptr_t id(boost::asio::const_buffer buffer);
};

#endif //ASIO_PROTOCOL_H