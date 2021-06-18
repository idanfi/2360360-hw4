

#ifndef HW3_SEMANTICCHECK_H
#define HW3_SEMANTICCHECK_H

#include <string>
#include <vector>
#include "hw5_output.hpp"
#include "Types.h"
#include "SymbolTable.h"

extern int yylineno;
extern SymbolTable symbolTable;


using namespace std;
using namespace output;

namespace SemanticCheck {
    void isInLoop(bool isContinue); // isContinue ? continue : break;
    void isValidReturn(string returnType, bool isExp);
    void isValidByteValue(string value);
    void isMainMissing();
}


#endif //HW3_SEMANTICCHECK_H
