#pragma once
 
#include <iostream>
#include "inter_thread_pubsub.hpp"
#include "oop_observer.hpp"
#include "thread_pool.hpp"

class Module {
    public:
        Module() {

        }
        ~Module() {

        }

        virtual void task() = 0;
        
        //======================Create New Thread Version=================================//
        /* create a new thread and run the module in that thread */
        void run() {
            mthread = boost::shared_ptr<boost::thread>(
                new boost::thread(boost::bind(&Module::task, this))
            );
        }
        /* don't use this method if the threadpool version of Module::run() was used */
        void idle() {
            mthread->yield(); 
        }

        /* don't use this method if the threadpool version of Module::run() was used */
        void join() {
            mthread->join();
        }
        //================================================================================//




        //============================Thread Pool Version=================================//
        /* run the module as a task to be queued for a thread pool*/
        void run(ThreadPool& thread_pool) {
            thread_pool.execute(boost::bind(&Module::task, this));
        }
        //================================================================================//

    private:
        boost::shared_ptr<boost::thread> mthread;

};
