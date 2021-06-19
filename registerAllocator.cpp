//
// Created by Idan Fischman on 18/06/2021.
//

#include "registerAllocator.h"

RegisterAllocator regAllocator;

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

string RegisterAllocator::createRegister(Node *node, string value, string id) {
    stringstream code;
    string registerName = getNextRegisterName();
    code << registerName << " = ";
    string type = node->realtype(); // todo: should it be type of realtype?
    if (id != INVALID_ID && symbolTable.exists(id)) {
        value = this->getVarRegister(id);
    }
    if (type == TYPE_INT) {
        code << "add i32 " << value << ", 0";
    } else if (type == TYPE_BYTE) {
        code << "trunc i32 " << replace(value,"b", "") << " to i8" << endl;
        string curr_reg = this->getCurrentRegisterName();
        code << this->getNextRegisterName() << " = " << "zext i8 " << curr_reg << " to i32";
    } else if (type == TYPE_BOOL) {
        code << "add i32 0, " << value;
    } else { // should never happen todo: remove
        cout << "ERROR: invalid type: " << type << " with value = " << value << endl;
        exit(-1);
    }
    // update the register mapping
    // cout << "working on " << node->id << " type " << node->type << endl;
    if (node->type == TYPE_ID) {
        // cout << "inserting [" << node->id << "] = " << registerName << endl;
        this->varToRegMapping[node->id] = registerName;
    }
    node->value = registerName;
    return code.str();
}

// add, sub, mul, udiv, sdiv
string RegisterAllocator::createArithmeticCode(Node *left_node, Node *right_node, string op) {
    stringstream code;
    string left;
    string right;
    // get the left string
    auto search = varToRegMapping.find(left_node->id);
    if (search != varToRegMapping.end()) {
        left = search->second;
    } else {
        left = left_node->value;
    }
    // get the right string
    search = varToRegMapping.find(right_node->id);
    if (search != varToRegMapping.end()) {
        right = search->second;
    } else {
        right = right_node->value;
    }

    // write the code
    if (op == "sdiv" || op == "udiv") {
        // todo: special treatment for divide by zero
        code << "call void @assertDiv(i32 " << right << ")" << endl;
    }
    code << this->getNextRegisterName() << " = " << op << " i32 " << left << ", " << right;
    if (left_node->realtype() == TYPE_BYTE && right_node->realtype() == TYPE_BYTE) {
        string curr_reg = this->getCurrentRegisterName();
        code << endl << this->getNextRegisterName() << " = " << "trunc i32 " << curr_reg << " to i8";
        curr_reg = this->getCurrentRegisterName();
        code << endl << this->getNextRegisterName() << " = " << "zext i8 " << curr_reg << " to i32";
    }
    return code.str();
}
