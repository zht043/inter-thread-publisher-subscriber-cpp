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
        delay(1);

        //while(1) {
            for(int i = 0; i < 100; i++) {
                pub1.publish("Hello, I'm pub1: " + std::to_string(i));
                pub2.publish(i + 0.888);    
                delay(1); // publish one set of data at every 1 ms = 1000 us
            }
        // }
    });

    auto sub_lambda = [](std::string file_name) {
        Subscriber<std::string> sub1("Topic1", "Msg1");
        Subscriber<double> sub2("Topic1", "Msg2");
        while(!sub1.subscribe());
        while(!sub2.subscribe());

        std::ofstream file;
        std::stringstream ss;
        file.open(file_name);
        int t0 = millis();
        // run this thread for 100 millisecond
        while(millis() - t0 < 100) {
            ss << "<==============================>" << std::endl;
            ss << "pub1: " << sub1.latest_msg() << std::endl;
            ss << "pub2: " << sub2.latest_msg() << std::endl;      
            delay_us(10); // microseconds
        }

        file << ss.str();
    };

    boost::thread sub_thread1(boost::bind<void>(sub_lambda, "trivial_example.thread1.txt"));
    boost::thread sub_thread2(boost::bind<void>(sub_lambda, "trivial_example.thread2.txt"));
    boost::thread sub_thread3(boost::bind<void>(sub_lambda, "trivial_example.thread3.txt"));




    pub_thread.join();
    sub_thread1.join();
    sub_thread2.join();
    sub_thread3.join();
    return 0;
}