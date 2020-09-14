#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/chrono.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/thread.hpp> 

#include "oop_observer.hpp"

using namespace std;



class MyDataGenerator : public Subject<string> {};

class MyDataCollector : public Observer<string> {
    public:
        MyDataCollector(ofstream* file) {
            this->file = file;
        }

        void on_update(string data) {
            stringstream ss;
            ss << data << endl;
            *file << ss.str();
        }

    private:
        ofstream *file;
};



int main(int argc, char *argv[]) {
    

    ofstream file;
    file.open("oop_observer_example.txt");

    MyDataGenerator gen;
    MyDataCollector coll(&file);
    gen.add_observer(coll);

    for(int i = 0; i < 100; i++) {
        gen.notify_observers(to_string(i));
    }


    return 0;
}