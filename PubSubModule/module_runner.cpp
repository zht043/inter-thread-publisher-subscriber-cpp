#include <iostream>
#include "thread_pool.hpp"
#include "ModuleA.hpp"
#include "ModuleB.hpp"
#include "ModuleC.hpp"




static void delay(unsigned int milliseconds) {
    boost::this_thread::sleep_for(boost::chrono::milliseconds(milliseconds));
}

int main(int arc, char *argv[]) {


/* create new thread version
    Module_A module_a;
    Module_B module_b;
    Module_C module_c;

    module_a.run();
    module_b.run();
    module_c.run();
    
    module_c.join();
    module_a.join();
    module_b.join();
*/

/* thread pool version */
    ThreadPool thread_pool(10); // pre-allocate 10 threads in a pool

    Module_A module_a;
    Module_B module_b;
    Module_C module_c;

    module_a.run(thread_pool);
    module_b.run(thread_pool);
    module_c.run(thread_pool);

    delay(8000); // wait 8 seconds until every thread is finished


    return 0;
}