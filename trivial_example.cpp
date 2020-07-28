#include <iostream>
#include "generic_pubsub.hpp"
#include <boost/chrono.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/thread.hpp> 

using namespace SPS;
using namespace std;


void delay(unsigned int milliseconds) {
    boost::this_thread::sleep_for(boost::chrono::milliseconds(milliseconds));
}

int main(int argc, char *argv[]) {

    boost::thread pub_thread( []() {
        
        Publisher<std::string> pub1("Topic1", "Msg1");
        Publisher<double> pub2("Topic1", "Msg2");
        Publisher<int> pub3("Topic2", "ID");
        
        
        while(1) {
            pub1.publish("Hello, I'm pub1");
            pub2.publish(888.888);
            pub3.publish(5);
            
        }
    });

    boost::thread sub_thread1( []() {

    });

    boost::thread sub_thread2( []() {
 
    });

    boost::thread sub_thread3( []() {

    });



    pub_thread.join();
    sub_thread1.join();
    sub_thread2.join();
    sub_thread3.join();
    return 0;
}