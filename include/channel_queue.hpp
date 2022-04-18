// Copyright (c) 2022 github.com/System233
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <condition_variable>
#include <mutex>
#include <queue>
template<class T>
class channel_queue{
    std::mutex mutex;
    std::queue<T>queue;
    std::condition_variable cv;
    size_t max_count;
    bool stopped;
public:
    channel_queue(size_t max_count=1024):max_count(max_count),stopped(false){}
    bool push(T data){
        {
            std::unique_lock<std::mutex> lock(mutex);
            if(max_count==queue.size()){
                return false;
            }
            queue.push(data);
        }
        cv.notify_one();
        return true;
    }
    operator bool(){
        return !stopped&&queue.size();
    }
    bool pop(T&data){
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock,[&](){return stopped||queue.size();});
        if(stopped){
            return false;
        }
        data=std::move(queue.front());
        queue.pop();
        return true;
    }
    size_t size()const{return queue.size();}
    void close(){
        std::unique_lock<std::mutex> lock(mutex);
        stopped= true;
        cv.notify_all();
    }
};