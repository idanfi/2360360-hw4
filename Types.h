
#ifndef HW3_TYPES_H
#define HW3_TYPES_H

#include <string>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "SymbolTable.h"
#include "hw5_output.hpp"
using namespace output;

#define YYSTYPE Node*
// hw types that were defined. Should be upper case
#define TYPE_BOOL "BOOL"
#define TYPE_VOID "VOID"
#define TYPE_INT  "INT"
#define TYPE_STRING "STRING"
#define TYPE_BYTE "BYTE"
// our own types that we use in the code. Should be lower case to distinguish
#define TYPE_ID "id"
#define INVALID_ID "$invalid_id"

extern int yylineno;
extern SymbolTable symbolTable;
extern CodeBuffer& buffer;

class Node;
bool isNumeric(const string &type);
void checkForValidType(const string &type);
bool assertAssignableTypes(const string &leftType, const  string &leftId, const string &rightType, const string &rightId, bool toExit = true, bool printError = true);
void assertBoolean(Node *exp);
void errorByteTooLargeAndExit(int lineno, int64_t value);
string getLlvmType(string type);
void createLlvmArguments(int numArguments, stringstream &code, vector<Node *> *argNames = nullptr);

struct IdType {
    string id;
    string type;
};

// parent class for the objects we create with bison on the fly
class Node {
public:
    Node() {
        /*
         * the initialization is happening before the constructor of each derived class,
         * so basically this are the default junk values indicating that the field is invalid.
         */
        this->type = "unknown_type";
        this->id = INVALID_ID;
        this->is_numeric = false;
        this->is_valid_numeric_value = false;
        this->numeric_value = -1;
        this->value = "%invalid_value";
    }

    Node(string type, string id) : Node() {
        this->type = type;
        this->id = id;
    }

    string realtype();
    void emitReturnCode();
    void emitCallCode(Node *node);
    ~Node() = default;
    string type;
    string id;
    bool is_numeric;
    int64_t numeric_value;
    bool is_valid_numeric_value;
    string value;
};

// all relationship operators - "<", ">", "==" etc.
class RelOp : public Node {
public:
    RelOp(Node *left, Node *right) {
        //cout<< left->id <<" "<< left->type <<" "<< left->realtype() <<" "<< right->id <<" "<< right->type <<" "<< right->realtype()<<endl;
        if (isNumeric(left->realtype()) && isNumeric(right->realtype())) {
            this->type = TYPE_BOOL;
            this->is_numeric = false;
        } else {
            errorMismatch(yylineno);
            exit(-1);
        }
        delete left;
        delete right;
    }
    ~RelOp() = default;
};

// binary logic operators - "and" and "or" .
class BinaryLogicOp : public Node {
public:
    BinaryLogicOp(Node *left, Node *right) {
        if (left->realtype() == TYPE_BOOL && right->realtype() == TYPE_BOOL) {
            this->type = TYPE_BOOL;
            this->is_numeric = false;
        } else {
            errorMismatch(yylineno);
            exit(-1);
        }
        delete left;
        delete right;
    }
    ~BinaryLogicOp() = default;
};

// unary logic operator - "not" .
class UnaryLogicOp : public Node {
public:
    UnaryLogicOp(Node *exp) {
        string type = exp->realtype();
        if (type == TYPE_BOOL) {
            this->type = TYPE_BOOL;
            this->is_numeric = false;
        } else {
            //cout << "UnaryLogicOp: found type of " << type << endl;
            errorMismatch(yylineno);
            exit(-1);
        }
        delete exp;
    }
    ~UnaryLogicOp() = default;
};

// binary arithmetic operator - "+", "-", "*", etc.
class BinaryArithmeticOp : public Node {
public:
    BinaryArithmeticOp(Node *left, Node *right) {
        if (isNumeric(left->realtype()) && isNumeric(right->realtype())) {
            // check if type should be byte or int
            if (left->realtype() == TYPE_BYTE && right->realtype() == TYPE_BYTE) {
                this->type = TYPE_BYTE;
            } else {
                this->type = TYPE_INT;
            }
            this->is_numeric = true;
        } else {
        //cout << "BinaryArithmeticOp: invalid types left_is_numeric=" << left->is_numeric <<  " right=" << right->is_numeric << endl;
        //cout << "\tleft_realtype=" << left->realtype() <<  " right: " << right->realtype() << endl;
            errorMismatch(yylineno);
            exit(-1);
        }
        delete left;
        delete right;
        //cout << "\t" << this->type << endl;
    }
    ~BinaryArithmeticOp() = default;
};

// assign operator
class AssignOp : public Node {
public:
    AssignOp(Node *left, Node *right) {
        // can assign only to ids
        if (left->type != TYPE_ID) {
            //cout << "AssignOp: left type is " << left->type << " instead of id" << endl;
            errorMismatch(yylineno);
            exit(-1);
        }
        assertAssignableTypes(left->type, left->id, right->type, right->id);
        // check that we are not assigning < 255 to a byte type
        if (left->realtype() == TYPE_BYTE && right->is_valid_numeric_value && right->numeric_value > 255) {
            errorByteTooLargeAndExit(yylineno, right->numeric_value);
        }
        this->type = left->type;
        this->is_numeric = isNumeric(realtype());

        delete left;
        delete right;
    }
    ~AssignOp() = default;
};


class IDExp : public Node {
public:
    IDExp(string _id) {
        this->id = _id;
        this->type = TYPE_ID;
    }
    ~IDExp() = default;
};

class TypeExp : public Node {
public:
    TypeExp(string _type) {
        checkForValidType(_type);
        this->type = _type;
        this->is_numeric = isNumeric(type);
    }
    ~TypeExp() = default;
};

class NumberExp : public Node {
public:
    NumberExp(Node *exp, bool is_byte = false) {
        this->is_numeric = true;
        this->numeric_value = stoll(exp->id, nullptr, 10);
        this->value = exp->id;
        this->is_valid_numeric_value = true;
        if (is_byte) {
            if (numeric_value > 255) {
                errorByteTooLargeAndExit(yylineno, numeric_value);
            }
            this->type = TYPE_BYTE;
        } else {
            this->type = TYPE_INT;
        }
        //cout << "type is: " << type << endl;
        delete exp;
    }
    ~NumberExp() = default;
};

class StringExp : public Node {
public:
    StringExp(string _str) {
        this->type = TYPE_STRING;
        this->id = _str;
    }
    ~StringExp() = default;
};

class ExpList : public Node {
public:
    ExpList() {
        this->expressions = new vector<Node *>;
    }
    ExpList(Node *exp) {
        this->expressions = new vector<Node *>;
        addExp(exp);
    }
    ~ExpList() = default; // todo : should probably delete the pointers in expressions
    void addExp(Node *exp) {
        this->expressions->insert(this->expressions->begin(), exp);
    }
    vector<Node *>* getExpressions() {
        return this->expressions;
    }
    unsigned int size() {
        return this->expressions->size();
    }
private:
    vector<Node *>* expressions;
};

// function call operator.
class CallOp : public Node {
public:
    CallOp(Node *exp, ExpList* params) {
        this->type = symbolTable.getReturnType(exp->id);
        // can use strings only to call print
        assertCall(exp->id, params);
        this->is_numeric = isNumeric(type);
        delete exp;
        delete params;
    }
    ~CallOp() = default;

    void assertCall(string id, ExpList* params) {
        symbolTable.assertExists(id, true);
        TableNode node;
        symbolTable.find(id, &node);
        vector<string> argTypes = node.getParams();
        if (argTypes.size() != params->size()) {
            // cout << "sizes don't match" << endl;
            errorPrototypeMismatch(yylineno, id, argTypes);
            exit(-1);
        }

        vector<Node *> callTypes = *params->getExpressions();
        for (unsigned int i = 0; i < argTypes.size(); i++) {
            if (callTypes[i]->realtype() == TYPE_STRING && id != "print") {
                // cout << "using string not in print" << endl;
                errorPrototypeMismatch(yylineno, id, argTypes);
                exit(-1);
            }

            if (!assertAssignableTypes(argTypes[i], "dummyLeft", callTypes[i]->realtype(), "dummyRight" , false, false)) {
                // cout << "using types that are not assignable (" << argTypes[i] << ") to (" << callTypes[i]->realtype() << ")" << endl;
                errorPrototypeMismatch(yylineno, id, argTypes);
                exit(-1);
            }
        }
    }
};

class FormalDeclList : public Node {
public:
    FormalDeclList() {
        this->decls = new vector<IdType>;
    }

    FormalDeclList(Node *exp) {
        this->decls = new vector<IdType>;
        addFormalDecl(exp);
    }
    ~FormalDeclList() = default; // todo : should probably delete the pointers in expressions
    void addFormalDecl(Node *exp) {
        addFormalDecl(exp->id, exp->realtype());
    }

    void addFormalDecl(const string id, const string type) {
        IdType node = {id, type};
        this->decls->insert(this->decls->begin(), node);
    }
    vector<IdType>* getStatements() {
        return this->decls;
    }
    const vector<string> argTypes() {
        vector<string> v;
        for (auto it = decls->begin(); it < decls->end(); it++) {
            v.push_back((*it).type);
        }

        return v;
    }
    const vector<string> ids() {
        vector<string> v;
        for (auto it = decls->begin(); it < decls->end(); it++) {
            v.push_back((*it).id);
        }

        return v;
    }
private:
    vector<IdType>* decls;
};

#endif //HW3_TYPES_H
