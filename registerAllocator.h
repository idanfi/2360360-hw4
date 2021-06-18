//
// Created by Idan Fischman on 18/06/2021.
//

#ifndef INC_236360_HW5_REGISTERALLOCATION_H
#define INC_236360_HW5_REGISTERALLOCATION_H

#include <string>
#include <sstream>
#include "Types.h"
#include <unordered_map>

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
    unordered_map<string, string> varToRegMapping;
public:
    RegisterAllocator() : counter(0) {}
    string createRegister(Node* node, string value);
    string getCurrentRegisterName() {
        return getNextRegisterName(0);
    }
    string createArithmeticCode(Node *left_node, Node *right_node, string op);
    string getVarRegister(string var) {
        return this->varToRegMapping[var];
    }
};


#endif //INC_236360_HW5_REGISTERALLOCATION_H
