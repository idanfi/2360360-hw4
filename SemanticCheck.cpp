
#include "SemanticCheck.h"

void SemanticCheck::isInLoop(bool isContinue) {
    if (!symbolTable.inLoop(isContinue)) {
        if (isContinue) {
            errorUnexpectedContinue(yylineno);
        }
        else {
            errorUnexpectedBreak(yylineno);
        }
        exit(-1);
    }
}

void SemanticCheck::isValidReturn(string returnType, bool isExp) {
    string currentFuncReturnType = symbolTable.topGlobal().returnType();
    if (isExp && currentFuncReturnType == TYPE_VOID) {
        // if the function is of type void, dont allow to return Exp;
        errorMismatch(yylineno);
        exit(-1);
    }
    assertAssignableTypes(symbolTable.topGlobal().returnType(), "dummyLeft", returnType, "dummyRight");
}

void SemanticCheck::isValidByteValue(string value) {
    return; //Idan already implemented.
}

void SemanticCheck::isMainMissing() {
    TableNode node;
    int result = symbolTable.find("main", &node);
    if (!result || !(node.isFunction()) || (node.returnType() != TYPE_VOID) || (node.hasParams())) {
        errorMainMissing();
        exit(-1);
    }
}
