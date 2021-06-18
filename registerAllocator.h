//
// Created by Idan Fischman on 18/06/2021.
//

#ifndef INC_236360_HW5_REGISTERALLOCATION_H
#define INC_236360_HW5_REGISTERALLOCATION_H

#include <string>
#include <sstream>
#include "Types.h"

using namespace std;

class RegisterAllocator {
private:
    unsigned int counter;
    string getNextRegisterName(int increment = 1) {
        this->counter += increment;
        stringstream s1;
        s1 << "%t" << this->counter;
        return s1.str();
    }

public:
    RegisterAllocator() : counter(0) {}
    string createRegister(string type, string value);
    string getCurrentRegisterName() {
        return getNextRegisterName(0);
    }
};


#endif //INC_236360_HW5_REGISTERALLOCATION_H
