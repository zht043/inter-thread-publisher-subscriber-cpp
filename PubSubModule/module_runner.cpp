#include <iostream>
// #include "module.hpp"
#include "ModuleA.hpp"
#include "ModuleB.hpp"
#include "ModuleC.hpp"



int main(int arc, char *argv[]) {

    Module_A module_a;
    Module_B module_b;
    Module_C module_c;

    module_a.run();
    module_b.run();
    module_c.run();
    
    module_c.join();
    module_a.join();
    module_b.join();


    return 0;
}