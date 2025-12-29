// semantic.c

#include "semantic.h"
#include <stdio.h>
#include <string.h>
#include "../semantic_error/semantic_error.h"

void analyzeSemantics(ASTNode *root, SymbolTable *currentTable) {

    if (!root) return;

//    // ---------------- Handle Class Implementations ----------------
//    if (strcmp(root->kind, "implDef") == 0) {
//
//        // Find the class identifier node
//        ASTNode *classId = findChild(root, "ClassIdentifier");
//        if (!classId) return;
//
//        // Find the class symbol table
//        SymbolTable *classTable = NULL;
//
//        for (int i = 0; i < currentTable->childCount; i++) {
//            if (strcmp(currentTable->children[i]->scopeName, classId->value) == 0) {
//                classTable = currentTable->children[i];
//                break;
//            }
//        }
//
//        if (!classTable) return;
//
//        ASTNode *funcList = findChild(root, "FuncDefList");
//
//        // Find the list of function definitions inside the class
//
//        if (funcList) {
//            for (int i = 0; i < funcList->childCount; i++) {
//                ASTNode *funcDef = funcList->children[i];
//                if (strcmp(funcDef->kind, "funcDef") == 0) {
//                    analyzeSemantics(funcDef, classTable);
//                }
//            }
//        }
//        return;
//    }

    // ---------------- Function Definitions ----------------
    if (strcmp(root->kind, "funcDef") == 0 || strcmp(root->kind, "funcDecl") == 0) {

        // Get function header (contains name and return type)
        ASTNode *funcHead = findChild(root, "funcHead");
        if (!funcHead) return;

        // Extract function identifier
        ASTNode *funcId = findChild(funcHead, "funcIdentifier");

        // Extract return type (default to void if not specified)
        ASTNode *returnTypeNode = findChild(funcHead, "returnType");
        const char *returnType =  returnTypeNode->value;

        // Find the correct function scope table

        /*
This code locates the symbol table corresponding to the current function by matching the function name
from the AST with the scope names of the child symbol tables, enabling semantic analysis within the correct
function scope.
 */
        SymbolTable *funcTable = NULL;
        for (int i = 0; i < currentTable->childCount; i++) {
            if (strcmp(currentTable->children[i]->scopeName, funcId->value) == 0) {
                funcTable = currentTable->children[i];
                break;
            }
        }

        // If function scope exists, analyze function body
        if (funcTable) {
            ASTNode *funcBody = findChild(root, "funcBody");
            if (funcBody) {
                // Check return types returnType- expectedType
                checkReturnTypes(funcBody, funcTable, returnType);

                // Continue analyzing inside function body
                for (int i = 0; i < funcBody->childCount; i++) {
                    analyzeSemantics(funcBody->children[i], funcTable);
                }
            }
        }
        return;
    }

    // ---------------- Assignment Statements ----------------
    if (strcmp(root->kind, "idOrSelfStatement") == 0) {
        checkAssignments(root, currentTable);
        return;
    }

    // ---------------- Recurse DFS through all children ----------------
    for (int i = 0; i < root->childCount; i++) {
        analyzeSemantics(root->children[i], currentTable);
    }
}

// ---------------- Type Checking - assignment statements ----------------
void checkAssignments(ASTNode *node, SymbolTable *currentTable) {
    if (!node) return;


    // take  node of the local variable
    ASTNode *lhs = findChild(node, "idOrSelf");

    ASTNode *rhsWrapper = findChild(node, "idOrSelfTailWithAssignOrCall");

    if (!lhs || !rhsWrapper) return;

    ASTNode *assignOp = findChild(rhsWrapper, "assignOp");

    ASTNode *expr = findChild(rhsWrapper, "expr");

    if (assignOp && expr) {

        const char *varName = lhs->value;

        //  Scope checking
        Symbol *sym = lookupSymbol(currentTable, varName, NULL);

        if (!sym) {
            addSemanticError(lhs->line, lhs->column, "Scope Error",
                             "Undeclared variable '%s'", varName);
            return;
        }

        // Type checking
        const char *rhsType = getExprType(expr, currentTable);

        // comparing expression type and varibale type
        if (rhsType && strcmp(rhsType, sym->type) != 0) {
            addSemanticError(lhs->line, lhs->column, "Type Error",
                             "Cannot assign '%s' to variable '%s' of type '%s'",
                             rhsType, varName, sym->type);
        }
    }
}

// ---------------- Function Return Type Check ----------------
void checkReturnTypes(ASTNode *node, SymbolTable *funcTable, const char *expectedType) {
    if (!node) return;

    if (strcmp(node->kind, "return") == 0) {
        ASTNode *expr = findChild(node, "expr");

        if (strcmp(expectedType, "void") == 0) {
            // void function returning a value
            if (expr != NULL) {
                addSemanticError(
                    node->line,
                    node->column,
                    "Return Type Error",
                    "Void function '%s' cannot return a value",
                    funcTable->scopeName
                );
            }
        } else {
            // Non-void function
            if (expr == NULL) {
                addSemanticError(
                    node->line,
                    node->column,
                    "Return Type Error",
                    "Function '%s' must return a value of type '%s'",
                    funcTable->scopeName,
                    expectedType
                );
            } else {
                const char *actualType = getExprType(expr, funcTable);
                if (!actualType || strcmp(actualType, expectedType) != 0) {
                    addSemanticError(
                        node->line,
                        node->column,
                        "Return Type Error",
                        "Return type mismatch in function '%s'. Expected '%s' but got '%s'",
                        funcTable->scopeName,
                        expectedType,
                        actualType ? actualType : "unknown"
                    );
                }
            }
        }
        return;
    }

    for (int i = 0; i < node->childCount; i++) {
        checkReturnTypes(node->children[i], funcTable, expectedType);
    }
}


//// ---------------- Expression Type Inference ----------------

// ---------------- Expression Type Inference ----------------
const char* getExprType(ASTNode *expr, SymbolTable *currentTable) {
    if (!expr) return NULL;

    // Literal factors
    if (strcmp(expr->kind, "integerFactor") == 0) return "integer";
    if (strcmp(expr->kind, "floatFactor") == 0) return "float";

    // Identifier / variable
    if (strcmp(expr->kind, "idOrSelf") == 0) {
        Symbol *sym = lookupSymbol(currentTable, expr->value, NULL);
        if (sym) return sym->type;
        return NULL;
    }

    // Arithmetic expressions
    if (strcmp(expr->kind, "arithExpr") == 0 || strcmp(expr->kind, "term") == 0) {
        const char *resultType = NULL;

        for (int i = 0; i < expr->childCount; i++) {
            const char *childType = getExprType(expr->children[i], currentTable);
            if (!childType) continue;

            if (!resultType) {
                resultType = childType;
            } else {
                // Type promotion: if either operand is float, result is float
                if (strcmp(resultType, "float") == 0 || strcmp(childType, "float") == 0)
                    resultType = "float";
                else
                    resultType = "integer";
            }
        }

        return resultType;
    }

    // recursively check children
    for (int i = 0; i < expr->childCount; i++) {
        const char *childType = getExprType(expr->children[i], currentTable);
        if (childType) return childType;
    }

    return NULL;
}





//const char* getExprType(ASTNode *expr, SymbolTable *currentTable) {
//    if (!expr) return NULL;
//
//    // If factor, check if it's literal or identifier
//    if (strcmp(expr->kind, "factor") == 0) {
//        if (expr->value) {
//            // Literal integer
//            int isInt = 1, i = 0;
//            if (expr->value[0] == '-' || expr->value[0] == '+') i = 1;
//            for (; expr->value[i]; i++) {
//                if (expr->value[i] < '0' || expr->value[i] > '9') { isInt = 0; break; }
//            }
//            if (isInt) return "integer";
//
//            // Literal float
//            int dotCount = 0;
//            for (i = 0; expr->value[i]; i++) {
//                if (expr->value[i] == '.') dotCount++;
//                else if (expr->value[i] < '0' || expr->value[i] > '9') { dotCount = -1; break; }
//            }
//            if (dotCount == 1) return "float";
//        }
//
//        // Factor may contain an identifier as a child
//        for (int i = 0; i < expr->childCount; i++) {
//            const char *childType = getExprType(expr->children[i], currentTable);
//            if (childType) return childType;
//        }
//        return NULL;
//    }
//
//    // Identifier / variable
//    if (strcmp(expr->kind, "idOrSelf") == 0) {
//        Symbol *sym = lookupSymbol(currentTable, expr->value, NULL);
//        if (sym) return sym->type;
//        return NULL;
//    }
//
//    // Recursively check all children
//    for (int i = 0; i < expr->childCount; i++) {
//        const char *childType = getExprType(expr->children[i], currentTable);
//        if (childType) return childType;
//    }
//
//    return NULL;
//}

