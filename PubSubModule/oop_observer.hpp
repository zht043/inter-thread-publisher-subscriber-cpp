/*
 * An Observer Pattern C++ implementation using boost library
 */


#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/signals2.hpp>

/* This implementation is not multithreaded-safe, 
 * for multithreaded-safe one, use the function pointer 
 * version of observer pattern in inter_thread_pubsub.hpp  
 * */




template<class Data>
class Observer {
    public:
        virtual void on_update(Data data) = 0; // pure virtual
};

template<class Data>
class Subject {
    public:
        void add_observer(Observer<Data>& obs) {
            observers.push_back(&obs);
        }

        void notify_observers(Data data) {
            for(auto& obs : observers) {
                obs->on_update(data);
            }
        }

    protected:
        std::vector< Observer<Data>* > observers;
};


