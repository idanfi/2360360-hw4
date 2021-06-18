//
// Created by Idan Fischman on 18/06/2021.
//

#include "registerAllocator.h"

/*
 * value could be either a register name, number or a bool.
 */

static string& replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return str;
    str.replace(start_pos, from.length(), to);
    return str;
}

string RegisterAllocator::createRegister(string type, string value) {
    stringstream code;
    string registerName = getNextRegisterName();
    code << registerName << " = ";
    if (type == TYPE_INT) {
        code << "add i32 " << value << ", 0";
    } else if (type == TYPE_BYTE) {
        code << "trunc i32 " << replace(value,"b", "") << "to i8";
    } else if (type == TYPE_BOOL) {
        if (value == "true") {
            code << "add i32 1, 0";
        } else { // value == false or default value
            code << "add i32 0, 0";
        }
    } else { // should never happen todo: remove
        cout << "ERROR: invalid type: " << type << " with value = " << value << endl;
        exit(-1);
    }
    return code.str();
}
