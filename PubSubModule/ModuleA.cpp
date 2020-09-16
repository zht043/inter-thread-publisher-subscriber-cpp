
#include "ModuleA.hpp"

#include <iostream>
using namespace std;

static void delay(unsigned int milliseconds) {
    boost::this_thread::sleep_for(boost::chrono::milliseconds(milliseconds));
}

static double get_fake_sensor_data() {
    static double i = 1.00;
    i *= 1.50;
    return i;
}

void Module_A::task() {
    ITPS::Publisher<double> pub("sensorA data");

    delay(1000);

    double data;
    for(int i = 0; i < 100; i++) {
        // data = get_fake_sensor_data();
        pub.publish(double(i));
    }


}