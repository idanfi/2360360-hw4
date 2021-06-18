
#include "SymbolTable.h"
#include "SemanticCheck.h"
#include "Types.h"

// the global symboltable
SymbolTable symbolTable;

//Table Node:

/* private c'tor*/
TableNode::TableNode(const string name, bool is_function, const string type, int offset)
        : name(name), is_function(is_function), type(type), offset(offset) {
}

/* variable c'tor */
TableNode::TableNode(const string name, const string type, int offset)
        : name(name), is_function(false), type(type), offset(offset) {
}

/* function c'tor */
TableNode::TableNode(const string name, const string type, vector<string> args, int offset)
        : name(name), is_function(true), type(type), offset(offset) {
    this->args = args;
}

/* copy c'tor */
TableNode::TableNode(const TableNode &t1) {
    this->is_function = t1.is_function;
    this->type = t1.type;
    this->name = t1.name;
    this->args = t1.args;
    this->offset = t1.offset;
}

const string TableNode::getName() const {
    return this->name;
}

const string TableNode::getType() {
    if (this->is_function) {
        return makeFunctionType(this->type, this->args);
    }
    return this->type;
}

const int TableNode::getOffset() const {
    return this->offset;
}

bool TableNode::isFunction() const {
    return this->is_function;
}

bool TableNode::hasParams() const {
    return this->is_function && !this->args.empty();
}

const vector<string>& TableNode::getParams() {
    return this->args;
}

string TableNode::returnType() const {
    return this->type;
}

//SymbolTable:

SymbolTable::SymbolTable() :
    // last function will get ovverridden by addFunction, this is just to silence the compiler.
    lastFunction(TableNode("dummy name", "dummy type", -1)), whileCounter(0), switchCounter(0) {
    this->symbolsTable = new list<list<TableNode>>;
    this->offsetsTable = new list<int>;
    this->createScope(); // glboal scope.
    vector<string> print = {TYPE_STRING};
    vector<string> printi = {TYPE_INT};
    this->addFunction("print", TYPE_VOID, print);
    this->addFunction("printi", TYPE_VOID, printi);
}

void SymbolTable::createScope() {
    list<TableNode> new_list;
    this->symbolsTable->push_back(new_list);
    int offset = offsetsTable->empty() ? 0 : offsetsTable->back();
    this->offsetsTable->push_back(offset);
}

void SymbolTable::deleteScope() {
    // first we should print the scope contents
    endScope();
    auto scope = this->symbolsTable->back();
    for (auto it = scope.begin(); it != scope.end(); ++it) {
        printID((*it).getName(), (*it).getOffset(), (*it).getType());
    }
    // now let's delete the scope
    this->symbolsTable->pop_back();
    this->offsetsTable->pop_back();
}

bool SymbolTable::find(string key, TableNode *result = nullptr) {
    for (auto it = this->symbolsTable->rbegin(); it != this->symbolsTable->rend(); ++it) {
        for (auto jt = (*it).begin(); jt != (*it).end(); ++jt) {
            if ((*jt).getName() == key) {
                if (result) {
                    *result = (*jt);
                }
                return true;
            }
        }
    }
    return false;
}

bool SymbolTable::exists(string key) {
    return this->find(key);
}

/* private add */
void SymbolTable::add(TableNode node, bool is_function) {
    if (this->exists(node.getName())) {
        errorDef(yylineno, node.getName());
        exit(-1);
    }
    if (node.isFunction()) {
        checkForValidType(node.returnType()); //TODO: fix
    } else {
        // cout << "node type is " << node.getType() << endl;
        checkForValidType(node.returnType()); //TODO: fix
    }
    if (is_function) {
        this->symbolsTable->front().push_back(node);
    }
    else {
        this->symbolsTable->back().push_back(node);
    }
    //cout << node.getName() << " " << node.getType() << " " << node.getOffset() << endl;
}

void SymbolTable::addVariable(const string& name, const string& type, int offset) {
    offset = (offset == -9999) ? this->offsetsTable->back() : offset;
    // cout << "adding " << name << " of type " << type << endl;
    add(TableNode(name, type, offset));
    if (offset >= 0) {
        (this->offsetsTable->back())++;
    }
}

void SymbolTable::addFunction(const string& name, const string& retType, const vector<string>& argTypes) {
    TableNode node(name, retType, argTypes, 0);
    add(node, true);
    this->lastFunction = node;
}

void SymbolTable::addFunction(const string& name, const string& retType, FormalDeclList* expList) {
    addFunction(name, retType, expList->argTypes());
    createScope();
    int offset = -1;
    vector<IdType>* params = expList->getStatements();
    for (auto it = params->begin(); it < params->end(); it++) {
        this->addVariable((*it).id, (*it).type, offset);
        offset--;
    }
}

void SymbolTable::addConditionalStructure(const string& name) {
    //cout << name << " open" << endl;
    createScope();
    if (name == "while") {
        this->whileCounter++;
    }
    if (name == "switch") {
        this->switchCounter++;
    }
}

TableNode SymbolTable::topGlobal() {
    // assumes that symbols table isn't empty, because print, printi will always be there.
    return this->lastFunction;
}

string SymbolTable::getReturnType(string key) {
    TableNode *node = new TableNode;
    string type = "not_exists";
    if (this->find(key, node)) {
        type = node->returnType();
        delete node;
    }
    return type;
}

bool SymbolTable::inLoop(bool isContinue) {
    if (isContinue)
        return this->whileCounter > 0;
    return (this->whileCounter > 0) or (this->switchCounter > 0);
}

void SymbolTable::removeWhile() {
    //cout << "while close" << endl;
    this->whileCounter--;
    this->deleteScope();
}

void SymbolTable::removeSwitch() {
    //cout << "switch close" << endl;
    this->switchCounter--;
    this->deleteScope();
}

void SymbolTable::assertExists(string key, bool is_function) {
    TableNode *node = new TableNode;
    if (!(this->find(key, node)) || (is_function != node->isFunction())) {
        if (is_function) {
            errorUndefFunc(yylineno, key);
        } else {
            errorUndef(yylineno, key);
        }
        exit(-1);
    }
}
