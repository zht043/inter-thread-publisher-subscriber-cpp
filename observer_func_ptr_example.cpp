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




void thread1_msg1_callback(std::ofstream* file, std::string msg) {
    std::stringstream ss;
    ss << "<==============================>" << std::endl;
    ss << "pub1: " << msg << std::endl;   
    *file << ss.str();
}

void thread1_msg2_callback(std::ofstream* file, double msg) {
    std::stringstream ss;
    ss << "<==============================>" << std::endl;
    ss << "pub2: " << msg << std::endl;      
    *file << ss.str();
}


void thread2_msg1_callback(std::ofstream* file, std::string msg) {
    std::stringstream ss;
    ss << "<==============================>" << std::endl;
    ss << "pub1: " << msg << std::endl;   
    *file << ss.str();
}

void thread2_msg2_callback(std::ofstream* file, double msg) {
    std::stringstream ss;
    ss << "<==============================>" << std::endl;
    ss << "pub2: " << msg << std::endl;      
    *file << ss.str();
}


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

    boost::thread sub_thread1([]() {
        Subscriber<std::string> sub1("Topic1", "Msg1");
        Subscriber<double> sub2("Topic1", "Msg2");

        // wait until the subscribers are initialized
        while(!sub1.subscribe());
        while(!sub2.subscribe());

        std::ofstream file;
        file.open("observer_example.thread1.txt");


        sub1.add_on_published_callback(boost::bind(&thread1_msg1_callback, &file,  _1));
        sub2.add_on_published_callback(boost::bind(&thread1_msg2_callback, &file,  _1));

        delay(1000); // wait for 1 second
    });

    boost::thread sub_thread2([]() {
        Subscriber<std::string> sub1("Topic1", "Msg1");
        Subscriber<double> sub2("Topic1", "Msg2");

        // wait until the subscribers are initialized
        while(!sub1.subscribe());
        while(!sub2.subscribe());

        std::ofstream file;
        file.open("observer_example.thread2.txt");


        sub1.add_on_published_callback(boost::bind(&thread2_msg1_callback, &file,  _1));
        sub2.add_on_published_callback(boost::bind(&thread2_msg2_callback, &file,  _1));

        delay(1000); // wait for 1 second
    });



    pub_thread.join();
    sub_thread1.join();
    sub_thread2.join();
    return 0;
}