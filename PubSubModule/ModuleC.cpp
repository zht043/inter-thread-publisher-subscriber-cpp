#include "ModuleC.hpp"

#include <iostream>
using namespace std;


void Module_C::task() {
    ITPS::Subscriber<double> subA("sensorA data", 100); // set buffer queue size to 100 
    ITPS::Subscriber<double> subB("sensorB data", 100);

    while(!subA.subscribe());
    while(!subB.subscribe());

    for(int i = 0; i < 100; i++) {
        cout << "<================>" << endl;
        cout << "A: " << subA.pop_msg() << endl;
        cout << "B: " << subB.pop_msg(100, -1) << endl; // pop with timeout of 100 milliseconds, default return is -1 on timed out
        // pubB only sends 50 data, so half of the output for B should be timed out -1
    }


}