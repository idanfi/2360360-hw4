
#ifndef HW3_SYMBOLTABLE_H
#define HW3_SYMBOLTABLE_H

#include <string>
#include <list>
#include <unordered_map>
#include <vector>
#include "hw5_output.hpp"

using namespace std;
using namespace output;

class TableNode;
class SymbolTable;
struct IdType;
class FormalDeclList;

extern SymbolTable symbolTable;

class TableNode {
public:
    TableNode(const string name, const string type, vector<string> args, int offset);
    TableNode(const string name, const string type, int offset);
    TableNode(const TableNode &t1);
    TableNode() = default;
    ~TableNode() = default;
    const string getName() const;
    const string getType();
    const int getOffset() const;
    bool isFunction() const;
    string returnType() const;
    void setFunction() const;
    bool hasParams() const;
    const vector<string>& getParams();

private:
    TableNode(const string name, bool is_function, const string type, int offset);
    string name;
    bool is_function;
    string type;
    vector<string>  args;
    int offset;
};

class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable() = default;
    void createScope();
    void deleteScope();
    void addVariable(const string& name, const string& type, int offset = -9999);
    void addFunction(const string& name, const string& retType, const vector<string>& argTypes);
    void addFunction(const string& name, const string& retType, FormalDeclList* expList);
    void addConditionalStructure(const string& name);
    bool find(string key, TableNode *node_ptr);
    bool exists(string key);
    void assertExists(string key, bool is_function);
    TableNode topGlobal();
    string getReturnType(string key);
    bool inLoop(bool isContinue);
    void removeWhile();
    void removeSwitch();

private:
    //attributes:
    list<list<TableNode>>* symbolsTable; //key is name.
    list<int>* offsetsTable;
    TableNode lastFunction; //TODO: can remove because we're using list.
    int whileCounter;
    int switchCounter;

    //methods:
    void add(TableNode node, bool is_function = false);
};



// moving operator== to here, with the static declaration made the code compile.
inline bool operator==(const TableNode& t1, const TableNode& t2) {
    return true;
}

#endif //HW3_SYMBOLTABLE_H
