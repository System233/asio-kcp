// Copyright (c) 2022 github.com/System233
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#include <protocol.hpp>
class raw_protocol:public protocol<raw_protocol>{
    output_func m_output;
public:
    raw_protocol(boost::asio::io_context&io,intptr_t id, output_func func):protocol(io,id,func),m_output(func){};
    virtual ~raw_protocol(){};
    virtual intptr_t id()const{return 0;};

    // template<class T>
    // void config(T const&);
    // template<class T>
    // T const& config()const;
    virtual size_t handle(boost::asio::const_buffer buffer,std::function<void(boost::asio::const_buffer buffer)>const&handler){
        handler(buffer);
        return buffer.size();
    };
    virtual size_t write(boost::asio::const_buffer buffer)const{
        return m_output(buffer);
    };
    static intptr_t id(boost::asio::const_buffer buffer){return 0;};
};