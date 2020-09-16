#pragma once

/* Reference: https://gist.github.com/dpressel/de9ea7603fa3f20b55bf by Daniel Pressel
 */

#include <boost/thread/thread.hpp>
#include <queue>
#include <boost/chrono.hpp>
#include <boost/chrono/system_clocks.hpp>

template <typename data_t> 
class ConsumerProducerQueue {
    public:
        ConsumerProducerQueue(unsigned int max_size) {
            this->max_size = max_size;
        }

        void produce(data_t data) {
            mu.lock();
            while(is_full()) {
                 // freeze this thread until queue is not full
                cond_not_full.wait(mu);
            }
            cp_queue.push(data);
            
            // unlock & notify order problem: https://stackoverflow.com/questions/17101922/do-i-have-to-acquire-lock-before-calling-condition-variable-notify-one/17102100#17102100
            mu.unlock();
            
            // when a datum is enqueued, the queue must be non-empty, notify the consumer to unlock wait
            cond_not_empty.notify_all(); 
        }

        data_t consume() {
            mu.lock();
            while(is_empty()) {
                // freeze this thread until queue is not empty
                cond_not_empty.wait(mu); 
            }
            data_t rtn = cp_queue.front();
            cp_queue.pop();
            mu.unlock();

            // when a datum is dequeued, the queue must be not-full, notify the producer to unlock wait
            cond_not_full.notify_all(); 
            return rtn;
        }

        /* Timed consume: on timeout (unit: milliseconds), return dft_rtn (default return value) */
        data_t consume(unsigned int timeout_ms, data_t dft_rtn) {
            boost::system_time const timeout = boost::get_system_time()+ boost::posix_time::milliseconds(timeout_ms);
            bool fulfilled = true;
            mu.lock();
            while(is_empty()) {
                // freeze this thread until queue is not empty or timed out
                if(cond_not_empty.timed_wait(mu, timeout)) {
                    // if wait returns due to condition being fulfilled
                    fulfilled = true;
                } 
                else {
                    // if wait returns due to time out
                    fulfilled = false;
                    break;
                }
            }

            if(fulfilled) {
                data_t rtn = cp_queue.front();
                cp_queue.pop();
                mu.unlock();

                // when a datum is dequeued, the queue must be not-full, notify the producer to unlock wait
                cond_not_full.notify_all(); 
                return rtn;
            }
            else {
                mu.unlock();
                return dft_rtn;
            }
        }

        bool is_full() const {
            return cp_queue.size() >= max_size;
        }

        bool is_empty() const {
            return cp_queue.size() <= 0;
        }

        unsigned int size() const {
            return cp_queue.size();
        }

        void clear() {
            mu.lock();
            while(!is_empty()) {
                cp_queue.pop();
            }
            mu.unlock();
            cond_not_full.notify_all();
        }


    private:

        static unsigned int millis(void) {
            auto t = boost::chrono::high_resolution_clock::now();
            return (unsigned int)(double(t.time_since_epoch().count()) / 1000000.00f);
        }

        boost::mutex mu;
        boost::condition_variable_any cond_not_full, cond_not_empty;
        std::queue<data_t> cp_queue;
        unsigned int max_size;
};