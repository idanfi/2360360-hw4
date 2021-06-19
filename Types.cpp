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
    cout << "found invalid type: " << type << endl;
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
        llvmType = "void";
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

void Node::addBreak() {
    int jmpInstr = buffer.emit("br @");
    this->nextList.push_back({jmpInstr, FIRST});
}

void Node::addContinue() {
    int jmpInstr = buffer.emit("br @");
    this->startLoopList.push_back({jmpInstr, FIRST});
}

void Node::finishWhile(string whileStartLabel, Node *whileExp) {
    // backpatch and jump to the start of the loop
    cout << "finishWhile" << endl;
    buffer.bpatch(this->startLoopList, whileStartLabel);
    stringstream code;
    code << "br %" << whileStartLabel;
    buffer.emit(code.str());
    // label the loop exit
    string whileEndLabel = buffer.genLabel();
    buffer.bpatch(this->nextList, whileEndLabel);
    buffer.bpatch(whileExp->falseList, whileEndLabel);
    buffer.bpatch(whileExp->trueList, whileStartLabel);
}

void Node::emitWhileExp(string cmpReg) {
    cout << "emitWhileExp: start" << endl;
    stringstream code;
    code << "br i1 " << cmpReg << ", label @, label @";
    int cmpAddress = buffer.emit(code.str());
    string whileStartCodeLabel = buffer.genLabel();
    vector<pair<int,BranchLabelIndex>> v1 = {{cmpAddress, FIRST}};
    buffer.bpatch(v1, whileStartCodeLabel);
    this->falseList.push_back({cmpAddress, SECOND});
    cout << "emitWhileExp: finish" << endl;
}

void Node::mergeLists(Node *node_a, Node *node_b) {
    cout << "merging lists" << endl;
    this->nextList = buffer.merge(node_a->nextList, node_b->nextList);
    this->trueList = buffer.merge(node_a->trueList, node_b->trueList);
    this->falseList = buffer.merge(node_a->falseList, node_b->falseList);
    this->startLoopList = buffer.merge(node_a->startLoopList, node_b->startLoopList);
}

void Node::emitCallCode(Node* node) {
    cout << "start emitCallCode" << endl;
    ExpList* expListNode = nullptr;
    vector<Node *> *expressions = nullptr;
    int size = 0;
    if (node) {
        expListNode = (ExpList*)node;
        size = expListNode->size();
        expressions = expListNode->getExpressions();
    }
    stringstream code;
    string retType = symbolTable.getReturnType(this->id);
    string llvmType = getLlvmType(retType);
    // save the function call result if needed
    if (retType != TYPE_VOID) {
        string reg = regAllocator.getNextRegisterName();
        code << reg << " = ";
        this->value = reg;
    }
    code << "call " << llvmType << " @" << this->id;
    createLlvmArguments(size, code, expressions);
    buffer.emit(code.str());
    cout << "finish emitCallCode - " << code.str() << endl;
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
        // todo: implement
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
        int brInstr = buffer.emit(code2.str());

        // the false label
        string label = buffer.genLabel();
        compReg = regAllocator.getNextRegisterName();
        stringstream code3;
        code3 << compReg << " = icmp ne i32 " << regAllocator.getVarRegister(left->id) << ", 0";
        buffer.emit(code3.str());
        int jmpInstr = buffer.emit("br label @");

        // end of short circuit evaluation, create the phi command and backpatch all
        string label2 = buffer.genLabel();
        stringstream code4;
        string resReg = regAllocator.getNextRegisterName();
        code4 << resReg << " phi i1 [true @], [" << compReg << ", @]";
        int phiInstr = buffer.emit(code4.str());
        vector<pair<int,BranchLabelIndex>> v1 = {{brInstr, SECOND}, {jmpInstr, FIRST}, {phiInstr, SECOND}};
        buffer.bpatch(v1, label2);
        vector<pair<int,BranchLabelIndex>> v2 = {{brInstr, FIRST}};
        buffer.bpatch(v2, label);
        vector<pair<int,BranchLabelIndex>> v3 = {{phiInstr, FIRST}};
        buffer.bpatch(v3, marker->nextInstruction);
        // update the result value to be the last register
        this->value = resReg;
    }
    delete left;
    delete right;
}

RelOp::RelOp(Node *left, Node *right, string op) {
    // cout<< left->id <<" "<< left->type <<" "<< left->realtype() <<" "<< right->id <<" "<< right->type <<" "<< right->realtype()<<endl;
    // cout << "op = " << op << endl;
    if (isNumeric(left->realtype()) && isNumeric(right->realtype())) {
        this->type = TYPE_BOOL;
        this->is_numeric = false;
    } else {
        errorMismatch(yylineno);
        exit(-1);
    }
    // write to the buffer the operation, and update the value todo: check different types - int vs byte
    this->value = regAllocator.emitCmpCode(left->value, right->value, op);
    delete left;
    delete right;
}
