#include <iostream>
#include <fstream>
#include <sstream>
#include "inter_thread_pubsub.hpp"
#include <boost/chrono.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/thread.hpp> 

using namespace ITPS;
using namespace std;


//----- helper systime functions -----//
void delay(unsigned int milliseconds) {
    boost::this_thread::sleep_for(boost::chrono::milliseconds(milliseconds));
}

void delay_us(unsigned int microseconds) {
    boost::this_thread::sleep_for(boost::chrono::microseconds(microseconds));
}

unsigned int millis(void) {
    auto t = boost::chrono::high_resolution_clock::now();
    return (unsigned int)(double(t.time_since_epoch().count()) / 1000000.00f);
}

unsigned int micros(void) {
    auto t = boost::chrono::high_resolution_clock::now();
    return (unsigned int)(double(t.time_since_epoch().count()) / 1000.00f);
}
//------------------------------------//


int main(int argc, char *argv[]) {
    
    boost::thread pub_thread( []() {
        
        Publisher<std::string> pub1("Topic1", "Msg1");
        Publisher<double> pub2("Topic1", "Msg2");
        delay(500); // wait for a bit until subscriber is initialized

        for(int i = 0; i < 100; i++) {
            pub1.publish("Hello, I'm pub1: " + std::to_string(i));
            pub2.publish(i);    
            // delay(1); // publish one set of data at every 1 ms = 1000 us
        }
    });

    auto sub_lambda = [](std::string file_name) {

        // instantiate subscribers with message queues
        int queue_size1 = 120;
        Subscriber<std::string> sub1("Topic1", "Msg1", queue_size1);

        int queue_size2 = 50;
        Subscriber<double> sub2("Topic1", "Msg2", queue_size2);

        // wait until the subscribers are initialized
        while(!sub1.subscribe());
        while(!sub2.subscribe());

        std::ofstream file;
        std::stringstream ss;
        file.open(file_name);
        int t0 = millis();
        
        for(int i = 0; i < 100; i++) {
            ss << "<==============================>" << std::endl;
            ss << "pub1: " << sub1.pop_msg() << std::endl;
            ss << "pub2: " << sub2.pop_msg() << std::endl;      
        }

        file << ss.str();
    };

    boost::thread sub_thread1(boost::bind<void>(sub_lambda, "mq_example.thread1.txt"));
    boost::thread sub_thread2(boost::bind<void>(sub_lambda, "mq_example.thread2.txt"));
    boost::thread sub_thread3(boost::bind<void>(sub_lambda, "mq_example.thread3.txt"));




    pub_thread.join();
    sub_thread1.join();
    sub_thread2.join();
    sub_thread3.join();
    return 0;
}