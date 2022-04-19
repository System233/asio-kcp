// Copyright (c) 2022 github.com/System233
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#include <cstdint>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
#include <iomanip>
#include <array>
class tick_counter{
    size_t step=200;
    size_t current=0,current_record=0;
    std::array<std::pair<size_t,uint32_t>,10>records;
    boost::asio::steady_timer timer;
    size_t count=0,avg=0,max=0,total=0,start=0;
    static uint32_t now(){
        return (uint32_t)std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    }
    void show(boost::system::error_code const&error){
        if(!error){
            
            auto&node=records[current];


            auto duration=now()-(node.second?node.second:start);
            auto cur=current_record*1000/duration;

            current_record-=node.first;
            current_record+=count;
            node.first=count;
            node.second=now();
            current=(current+1)%records.size();

            total+=count;
            count=0;
            // auto curx=count*1000/step;
            
            auto time=now()-start;
            auto base=1024.0*1024;
            max=std::max<size_t>(cur,max);
            avg=total*1000/time;
            std::cout<<"\r["<<time<<"ms]"<<std::setprecision(1)<<std::fixed
            <<"  "<<cur/base<<"MBps/CUR"
            <<"  "<<avg/base<<"MBps/AVG"
            <<"  "<<max/base<<"MBps/MAX"
            <<"  "<<total/base<<"MB/TOTAL"
            <<" ";
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
        return current_record;
    }
};
