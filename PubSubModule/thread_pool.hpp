#pragma once

#include <iostream>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>

//-------------------------------------------------------------------------------------------------------------------//

/* reference: https://kezunlin.me/post/f241bd30/ */

class ThreadPool {
public:
    ThreadPool(unsigned int num_threads) : io_work(ios) {
        for (int i = 0; i < num_threads; i++) {
            threads.create_thread(boost::bind(&boost::asio::io_service::run, &ios));
        }
    }

    ~ThreadPool() {
        ios.stop();
        try { // suppress all exceptions 
            threads.join_all(); // wait for all threads to terminate
        }
        catch ( const std::exception& ) {}
    }

    template<class Function>
    void execute(Function func) {
        ios.post(func); // add the function to the io_service queue 
                        // to be run in the threads created in the constructor
        // non-blocking, return immediately

        /* if there aren't available threads in the pool, i.e. every
           thread in the pool is already busy executing some other 
           function, the new coming thread has to wait until one of 
           the running thread is finished. */

        /* No counter provided to track the number of free threads
           due to implementation overheads with wrapping func with 
           a counter addition when function finishes, and at the same
           time supporting boost::bind externally for passing func. 
           Refer to this post for more details: 
                https://stackoverflow.com/questions/12215395/thread-pool-using-boost-asio/12267138#12267138
        */
       num_tasks++;
    }

    /* total CUMULATIVE number of tasks ever being posted by ThreadPool::execute,
     * which also include those tasks that are already finished
     * */
    unsigned int num_posted_funcs() {
        return num_tasks;
    }

private:
    boost::thread_group threads;
    boost::asio::io_service ios;
    boost::asio::io_service::work io_work;
    unsigned int num_tasks = 0; // this includes those tasks that finished early and got dequeued,
                            // this measurement can't deduce anything about the
                            // the current available free threads

};

//-------------------------------------------------------------------------------------------------------------------//

