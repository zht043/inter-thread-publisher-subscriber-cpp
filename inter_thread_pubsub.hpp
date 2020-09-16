/*
 * An Inter-Thread Publisher Subscriber Pattern (ITPS) C++ implementation using boost library
 */


#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/signals2.hpp>

#include "cp_queue.hpp"


/* Synchronization for Reader/Writer problems */
// get exclusive access
#define ITPS_writer_lock(mutex) do { \
    boost::upgrade_lock<boost::shared_mutex> __writer_lock(mutex); \
    boost::upgrade_to_unique_lock<boost::shared_mutex> __unique_writer_lock( __writer_lock ); \
}while(0)
// get shared access
#define ITPS_reader_lock(mutex) boost::shared_lock<boost::shared_mutex>  __reader_lock(mutex); 

#define Default_Topic "DefaultTopic"






namespace ITPS {


    /*
     * 3 Modes
     *  * Trivial Mode: msg channel only use 1 field to store the msg, every time a new msg
     *      is sent from the publisher, the field gets overwritten, hence only the latest msg
     *      is saved.
     *  * Message Queue Mode: use a MQ to store a series of msgs, MQ is instantiated by subscriber
     *      Each subscriber gets its own MQ. When MQ is full, the publisher thread is suspended until
     *      the queue is consumed(pop) by a subscriber to give room for new msgs. Check cp_queue.hpp 
     *      for implementation details of the consumer-producer queue.
     *  * Observer Mode: 
     *      Everytime a publisher sends a new message to its subscribers, the publisher invokes the callback
     *      functions of the subcribers. Note that this way both the publisher & its subscribers run on the same
     *      thread, unlike the other 2 modes. (of course there will be multiple threads when having multiple publishers)
     *      (usually observer pattern in java is implemented through class inheritance, but complication arises when
     *       dealing with multithreading, so here we use function pointer & callback to implement the features of a observer pattern)
     *      
     * 
     *      The publisher-subcriber pattern and the observer pattern are not the same, but highly related. Here we treat the 
     *      observer pattern as a special case of the general pub-sub pattern. One big distinction between the 2 pattern is 
     *      that pub-sub is anonymous, i.e. pub doesn't know which subs subcribed it, and vice versa, where in observer pattern
     *      everything is transparent. Here small tricks are used to make observer pattern implementation "anonymous".
     */

    /*
     *  * MsgChannel utilizes a HashTable to manage messgaes
     *  * publisher instantiates a MsgChannel
     *  * subscriber contains constructors to instantiate a message queue
     * 
     *  * One important distinction between Trivial Mode and MQ Mode:
     *      * Trivial mode's subcriber's getter function "Msg latest_msg(void)" 
     *        is non-blocking, and might get garbage value when publisher hasn't published anything
     *      * MQ mode's getter function "Msg pop_msg(void)" is conditionally blocking, when
     *        the queue is empty, getter's thread gets blocked until something is published into the queue.
     *        Similarly, when the queue is full, the publisher is blocked instead. 
     */


    /* This class serves as a bridge between the Publisher class and the Subcribe class*/ 
    template<class Msg>
    class MsgChannel {
        protected:

            // unordered map == hash map
            typedef std::unordered_map<std::string, MsgChannel<Msg>*> msg_table_t;
        public:

            MsgChannel(std::string topic_name, std::string msg_name) {
                ITPS_writer_lock(table_mutex);
                this->key = topic_name + "." + msg_name;
                // std::cout << key << std::endl;
                
                // if key doesn't exist
                if(msg_table.find(key) == msg_table.end()) {
                    msg_table[key] = this;
                }
            }

            static MsgChannel *get_channel(std::string topic_name, std::string msg_name) {
                ITPS_reader_lock(table_mutex);
                std::string key = topic_name + "." + msg_name;
                
                // if key doesn't exist
                if(msg_table.find(key) == msg_table.end()) {
                    return nullptr;
                }
                return msg_table[key];
            }

            void add_msg_queue(boost::shared_ptr<ConsumerProducerQueue<Msg>> queue) {
                ITPS_writer_lock(msg_mutex); 
                msg_queues.push_back(queue);
            }

            void add_slot(boost::function<void(Msg)> callback_function) {
                callback_funcs.push_back(callback_function);
            }

            void set_msg(Msg msg) {
                ITPS_writer_lock(msg_mutex);
                this->message = msg;
                
                /* enqueue MQ*/
                for(auto& queue: msg_queues) {
                    queue->produce(msg);
                }

                /* invoke observer's callback functions */
                for(auto& func: callback_funcs) {
                    func(msg);
                }

            }

            Msg get_msg() { 
                ITPS_reader_lock(msg_mutex);
                return this->message;
            }

        protected:
            Msg message;
            static msg_table_t msg_table;
            
            boost::shared_mutex msg_mutex;
            static boost::shared_mutex table_mutex;

            std::string key;

            std::vector< boost::shared_ptr<ConsumerProducerQueue<Msg>> > msg_queues;
            std::vector< boost::function<void(Msg)> > callback_funcs;
    };





    template <class Msg>
    class Publisher {
        public:

            Publisher(std::string topic_name, std::string msg_name) {
                channel = boost::shared_ptr<ITPS::MsgChannel<Msg>>(new ITPS::MsgChannel<Msg>(topic_name, msg_name));
            }

            Publisher(std::string msg_name) : Publisher(Default_Topic, msg_name) {}

            ~Publisher() {}

            void publish(Msg message) {
                channel->set_msg(message);
            }


        protected:
            boost::shared_ptr<ITPS::MsgChannel<Msg>> channel;
            
    };

    template <class Msg>
    class Subscriber {
        public:

            Subscriber(std::string topic_name, std::string msg_name) {
                this->topic_name = topic_name;
                this->msg_name = msg_name;
            }
            Subscriber(std::string msg_name) : Subscriber(Default_Topic, msg_name){}

            //with message queue
            Subscriber(std::string topic_name, std::string msg_name, unsigned int queue_size) 
                : Subscriber(topic_name, msg_name) {
                msg_queue = boost::shared_ptr<ConsumerProducerQueue<Msg>>(
                    new ConsumerProducerQueue<Msg>(queue_size) 
                );
                use_msg_queue = true;
            } 
            Subscriber(std::string msg_name, unsigned int queue_size) : Subscriber(Default_Topic, msg_name, queue_size){}
            
            ~Subscriber() {}




            /* return true if finding a msg channel with matching key string.
             * key = "topic_name.msg_name".
             * the msg channel is created during the constructing phase
             * of the corresponding publisher with the same key string.
             * The MsgChannel object is stored in a internally global hash-map
             */
            bool subscribe() {
                channel = MsgChannel<Msg>::get_channel(topic_name, msg_name);
                if(channel == nullptr) {
                    return false;
                }
                if(use_msg_queue) {
                    channel->add_msg_queue(msg_queue);
                }
                return true;
            }

            // For Trivial Mode only
            Msg latest_msg() {
                return channel->get_msg();
            }   

            // For Message Queue Mode only
            Msg pop_msg() {
                return msg_queue->consume();
            }

            // with time limit, if surpassing the timeout limit, return dft_rtn (default return value) 
            Msg pop_msg(unsigned int timeout_ms, Msg dft_rtn) {
                return msg_queue->consume(timeout_ms, dft_rtn);
            }

            /* For Observer Mode: function pointer version.
             *
             * Add callback function to be invoked whenever 
             * a new Msg is published to the msg channel. The 
             * callback function should be of the format 
             *  void func_name(Msg msg);
             * where the msg param is the published message to be 
             * handled in the callback.
             * 
             * must call subcribe() and get a return of true, before calling this function
             */
            bool add_on_published_callback(boost::function<void(Msg)> callback_function) {
                if(channel == nullptr) return false;
                channel->add_slot(callback_function);    
                return true;            
            }    


        protected:
            MsgChannel<Msg> *channel = nullptr;
            boost::shared_ptr<ConsumerProducerQueue<Msg>> msg_queue;
            std::string topic_name, msg_name;
            bool use_msg_queue = false;
    };


}


// hash table storing messages with topic_name+msg_name as key
template <class Msg>
std::unordered_map<std::string, ITPS::MsgChannel<Msg>*> ITPS::MsgChannel<Msg>::msg_table;

template <class Msg>
boost::shared_mutex ITPS::MsgChannel<Msg>::table_mutex;