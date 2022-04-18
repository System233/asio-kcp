// Copyright (c) 2022 github.com/System233
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#include <cstdint>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
#include <iomanip>
class tick_counter{
    size_t step=200;
    boost::asio::steady_timer timer;
    size_t count=0,avg=0,max=0,total=0,start=0;
    static uint32_t now(){
        return (uint32_t)std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    }
    void show(boost::system::error_code const&error){
        if(!error){
            auto cur=count*1000/step;
            auto time=now()-start;
            auto base=1024.0*1024;
            max=std::max<size_t>(cur,max);
            total+=count;
            avg=total*1000/time;
            std::cout<<"\r["<<time<<"ms]"<<std::setprecision(1)<<std::fixed
            <<"  "<<cur/base<<"MBps/CUR"
            <<"  "<<avg/base<<"MBps/AVG"
            <<"  "<<max/base<<"MBps/MAX"
            <<"  "<<total/base<<"MB/TOTAL"
            <<" ";
            count=0;
            start_timer();
        }
    }
    void start_timer(){
        timer.expires_from_now(std::chrono::milliseconds(step));
        timer.async_wait(boost::bind(&tick_counter::show,this,boost::asio::placeholders::error));
    }
public:
    tick_counter(boost::asio::io_context&io):timer(io){
        start_timer();
        start=now();
    }
    size_t add(size_t byte){
        count+=byte;
        return count;
    }
};
