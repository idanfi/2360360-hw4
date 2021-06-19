//
// Created by lidor on 5/14/2021.
//

#include <algorithm>
#include <cctype>
#include <string>

#include "Types.h"
#include "registerAllocator.h"
using namespace std;
extern RegisterAllocator regAllocator;

bool isNumeric(const string &type) {
    if (type == TYPE_BYTE || type == TYPE_INT)
        return true;
    return false;
}

void checkForValidType(const string &type) {
    static const string valid_types[] = {
            TYPE_VOID,
            TYPE_BOOL,
            TYPE_INT,
            TYPE_STRING,
            TYPE_BYTE,
    };

    for (unsigned int i=0; i < 5; i++) {
        if (type == valid_types[i])
            return;
    }
    //cout << "found invalid type: " << type << endl;
    errorMismatch(yylineno);
    exit(-1);
}

bool assertAssignableTypes(const string &leftType, const string &leftId, const string &rightType, const string &rightId, bool toExit, bool printError) {
    string r_type = (rightType == "unknown_type" || rightType == TYPE_ID) ? symbolTable.getReturnType(rightId) : rightType;
    string l_type = (leftType  == "unknown_type"  || leftType == TYPE_ID) ? symbolTable.getReturnType(leftId) : leftType;
    //cout << "Left: " << leftId << " " << l_type << " Right: " << rightId << " " << r_type << endl;
    if (r_type != l_type) {
        if (!((l_type == TYPE_INT) && (r_type == TYPE_BYTE))) {
            if (printError)
                errorMismatch(yylineno);
            if (toExit)
                exit(-1);
            return false;
        }
    }
    return true;
}

void assertBoolean(Node *exp) {
    //cout << exp->id << exp->type << exp->realtype() << endl;
    string type = exp->realtype();
    if (type == TYPE_ID) {
        type = symbolTable.getReturnType(exp->id);
    }
    if (type != TYPE_BOOL) {
        // cout << "assertBoolean: found type of " << type << " instead of bool" << endl;
        errorMismatch(yylineno);
        exit(-1);
    }
}

void errorByteTooLargeAndExit(int lineno, int64_t value) {
    stringstream ss;
    string s;
    ss << value;
    ss >> s;
    errorByteTooLarge(lineno, s);
    exit(-1);
}

string Node::realtype() {
    string type = this->type;
    if (type == TYPE_ID) {
        // cout << "getting realtype of " << this->id << " - " << yylineno << endl;
        type = symbolTable.getReturnType(this->id);
    }
    return type;
}

string getLlvmType(string type) {
    string llvmType = type;
    if (llvmType == TYPE_VOID) {
        transform(llvmType.begin(), llvmType.end(), llvmType.begin(), ::tolower);
    } else {
        llvmType = "i32";
    }
    return llvmType;
}

void createLlvmArguments(int numArguments, stringstream &code, vector<Node *>* expressions) {
    code << "(";
    for (int i = 0; i < numArguments; i++) {
        if (i != 0) {
            code << ", ";
        }
        code << "i32";
        if (expressions) {
            code << " " << regAllocator.getVarRegister((*expressions)[i]->id);
        }
    }
    code << ")";
}

void Node::emitCallCode(Node* node) {
    ExpList* expListNode = nullptr;
    vector<Node *> *expressions = nullptr;
    int size = 0;
    if (node) {
        expListNode = (ExpList*)node;
        size = expListNode->size();
        expressions = expListNode->getExpressions();
    }
    stringstream code;
    string llvmType = getLlvmType(symbolTable.getReturnType(this->id));
    code << "call " << llvmType;
    code << " " << this->id;
    createLlvmArguments(size, code, expressions);
    buffer.emit(code.str());
}

void Node::emitReturnCode() {
    stringstream code;
    string reg;
    code << "ret ";
    if (this->id != INVALID_ID) {
        reg = regAllocator.getVarRegister(this->id);
    } else {
        reg = this->value;
    }
    code << reg;
    buffer.emit(code.str());
}

BinaryLogicOp::BinaryLogicOp(Node *left, Node *right, bool isAnd, Node *marker) {
    if (left->realtype() == TYPE_BOOL && right->realtype() == TYPE_BOOL) {
        this->type = TYPE_BOOL;
        this->is_numeric = false;
    } else {
        errorMismatch(yylineno);
        exit(-1);
    }
    if (isAnd) {
        buffer.bpatch(left->trueList, marker->nextInstruction);
        this->trueList = right->trueList;
        this->falseList = buffer.merge(left->falseList, right->falseList);
    } else { // or op
        buffer.bpatch(left->falseList, marker->nextInstruction);
        this->trueList = buffer.merge(left->trueList, right->trueList);
        this->falseList = right->falseList;
        // check for short circuit evaluation
        string compReg = regAllocator.getNextRegisterName();
        stringstream code;
        code << compReg << " = icmp ne i32 " << regAllocator.getVarRegister(left->id) << ", 0";
        buffer.emit(code.str());
        stringstream code2;
        code2 << "br i1 " << compReg << ", label @, label @";
        buffer.emit(code2.str());
    }
    delete left;
    delete right;
}