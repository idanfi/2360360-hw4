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

void RegisterAllocator::createRegister(Node *node, string value, string id) {
    stringstream code;
    bool needStore = false;
    string registerName = getNextRegisterName();
    code << registerName << " = ";
    string type = node->realtype(); // todo: should it be type of realtype?
    value = this->getVarRegister(id, value);
    if (type == TYPE_INT) {
        code << "add i32 " << value << ", 0";
    } else if (type == TYPE_BYTE) {
        code << "trunc i32 " << replace(value,"b", "") << " to i8" << endl;
        string curr_reg = this->getCurrentRegisterName();
        registerName = this->getNextRegisterName();
        code << registerName << " = " << "zext i8 " << curr_reg << " to i32";
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
    buffer.emit(code.str());
    this->storeVar(node->id, registerName);
}

// add, sub, mul, udiv, sdiv
string RegisterAllocator::createArithmeticCode(Node *left_node, Node *right_node, string op) {
    left_node->loadExp();
    right_node->loadExp();
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
    if (op == "sdiv" || op == "udiv") { //TODO: check whether there's need to use also udiv.
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

string RegisterAllocator::emitCmpCode(string left, string right, string op) {
    stringstream code;
    string reg = this->getNextRegisterName();
    code << reg << " = icmp ";
    // todo: should we care about signed vs unsigned?
    if (op == "==") {
        code << "eq ";
    } else if (op == "!=") {
        code << "ne ";
    } else if (op == "<") {
        code << "slt ";
    } else if (op == "<=") {
        code << "sle ";
    } else if (op == ">") {
        code << "sgt ";
    } else if (op == ">=") {
        code << "sge ";
    }
    code << "i32 " << left << ", " << right << endl;
    string regI32 = this->getNextRegisterName();
    code << regI32 << " = zext i1 " << reg << " to i32";
    buffer.emit(code.str());
    return regI32;
}

void RegisterAllocator::funcionEpilog() {
    this->varToStackMap.clear();
}

string RegisterAllocator::functionGetVarReg(string var) {
    auto reg = this->varToStackMap.find(var);
    string regOut;
    // first time allocating this variable in the stack
    if (reg == this->varToStackMap.end()) {
        stringstream stackRegister;
        stackRegister << "%s" << this->varToStackMap.size();
        regOut = stackRegister.str();
        this->varToStackMap[var] = regOut;
    } else {
        regOut = reg->second;
    }
    return regOut;
}

void RegisterAllocator::functionProlog() {
    for (int i=0; i<50; i++) {
        buffer.emit("%s" + to_string(i) + " = alloca i32, i32 0");
    }
}

void RegisterAllocator::storeVar(string var, string value) {
    string stackReg = functionGetVarReg(var);
    if (stackReg != "") {
        string code = "store i32 " + value + ", i32* " + stackReg;
        buffer.emit(code);
    }
}

string RegisterAllocator::loadVar(string var) {
    string stackReg = this->varToStackMap[var];
    if (stackReg != "") {
        string new_reg = this->getNextRegisterName();
        buffer.emit(new_reg + " = load i32, i32* " + stackReg);
        this->varToRegMapping[var] = new_reg;
        return new_reg;
    }
    else {
        return this->varToRegMapping[var];
    }
}