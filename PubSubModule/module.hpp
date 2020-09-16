#pragma once
 
#include <iostream>
#include "inter_thread_pubsub.hpp"
#include "oop_observer.hpp"

class Module {
    public:
        Module() {

        }
        ~Module() {

        }

        virtual void task() = 0;
        
        void run() {
            mthread = boost::shared_ptr<boost::thread>(
                new boost::thread(boost::bind(&Module::task, this))
            );
        }

        void idle() {
            mthread->yield(); 
        }

        void join() {
            mthread->join();
        }
    private:
        boost::shared_ptr<boost::thread> mthread;

};
