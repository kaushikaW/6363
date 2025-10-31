#include "semantic.h"
#include <stdio.h>
#include <string.h>

// ---------------- Entry Point ----------------
void analyzeSemantics(ASTNode *root, SymbolTable *currentTable, FILE *errorLog) {
    if (!root) return;

    // ---------------- Function Definitions ----------------
    if (strcmp(root->kind, "funcDef") == 0 || strcmp(root->kind, "funcDecl") == 0) {
        ASTNode *funcHead = findChild(root, "funcHead");
        if (!funcHead) return;

        ASTNode *funcId = findChild(funcHead, "funcIdentifier");
        ASTNode *returnTypeNode = findChild(funcHead, "returnType");
        const char *returnType = returnTypeNode ? returnTypeNode->value : "void";

        SymbolTable *funcTable = NULL;
        for (int i = 0; i < currentTable->childCount; i++) {
            if (strcmp(currentTable->children[i]->scopeName, funcId->value) == 0) {
                funcTable = currentTable->children[i];
                break;
            }
        }

        if (funcTable) {
            ASTNode *funcBody = findChild(root, "funcBody");
            if (funcBody) {
                //Check return types
                checkReturnTypes(funcBody, funcTable, returnType, errorLog);

                //Continue analyzing other semantics (assignments, nested functions, etc.)
                for (int i = 0; i < funcBody->childCount; i++) {
                    analyzeSemantics(funcBody->children[i], funcTable, errorLog);
                }
            }
        }
        return;
    }

    // ---------------- Assignment Statements ----------------
    if (strcmp(root->kind, "idOrSelfStatement") == 0) {
        checkAssignments(root, currentTable, errorLog);
        return;
    }

    // ---------------- Generic Recursion for All Children ----------------
    for (int i = 0; i < root->childCount; i++) {
        analyzeSemantics(root->children[i], currentTable, errorLog);
    }
}


// ---------------- Type Checking ----------------
void checkAssignments(ASTNode *node, SymbolTable *currentTable, FILE *errorLog) {
    if (!node) return;

    ASTNode *lhs = findChild(node, "idOrSelf");
    ASTNode *rhsWrapper = findChild(node, "idOrSelfTailWithAssignOrCall");
    if (!lhs || !rhsWrapper) return;

    ASTNode *assignOp = findChild(rhsWrapper, "assignOp");
    ASTNode *expr = findChild(rhsWrapper, "expr");

    if (assignOp && expr) {
        const char *varName = lhs->value;
        Symbol *sym = lookupSymbol(currentTable, varName, NULL);
        if (!sym) {
            fprintf(stderr, "Semantic error: Undeclared variable '%s' at line %d, col %d\n",
                    varName, lhs->line, lhs->column);
            if (errorLog)
                fprintf(errorLog, "Semantic error: Undeclared variable '%s' at line %d, col %d\n",
                        varName, lhs->line, lhs->column);
            return;
        }

        const char *rhsType = getExprType(expr, currentTable);
        if (rhsType && strcmp(rhsType, sym->type) != 0) {
            fprintf(stderr, "Type error: Cannot assign %s to variable '%s' of type %s at line %d, col %d\n",
                    rhsType, varName, sym->type, lhs->line, lhs->column);
            if (errorLog)
                fprintf(errorLog, "Type error: Cannot assign %s to variable '%s' of type %s at line %d, col %d\n",
                        rhsType, varName, sym->type, lhs->line, lhs->column);
        }
    }
}

// ---------------- Function Return Type Check ----------------
void checkReturnTypes(ASTNode *node, SymbolTable *funcTable, const char *expectedType, FILE *errorLog) {
    if (!node) return;

    // ---------------- Handle return statements ----------------
    if (strcmp(node->kind, "return") == 0) {
        ASTNode *expr = findChild(node, "expr");
        const char *actualType = expr ? getExprType(expr, funcTable) : NULL;

        if (strcmp(expectedType, "void") == 0) {
            // Void function should NOT return a value
            if (actualType) {
                fprintf(stderr, "Warning: Void function '%s' should not return a value at line %d, col %d\n",
                        funcTable->scopeName, node->line, node->column);
                if (errorLog)
                    fprintf(errorLog, "Warning: Void function '%s' should not return a value at line %d, col %d\n",
                            funcTable->scopeName, node->line, node->column);
            }
        } else {
            // Non-void function must return the expected type
            if (!actualType) {
                fprintf(stderr, "Semantic error: Function '%s' must return a value of type '%s' at line %d, col %d\n",
                        funcTable->scopeName, expectedType, node->line, node->column);
                if (errorLog)
                    fprintf(errorLog, "Semantic error: Function '%s' must return a value of type '%s' at line %d, col %d\n",
                            funcTable->scopeName, expectedType, node->line, node->column);
            } else if (strcmp(actualType, expectedType) != 0) {
                fprintf(stderr, "Semantic error: Return type mismatch in function '%s'. Expected '%s' but got '%s' at line %d, col %d\n",
                        funcTable->scopeName, expectedType, actualType, node->line, node->column);
                if (errorLog)
                    fprintf(errorLog, "Semantic error: Return type mismatch in function '%s'. Expected '%s' but got '%s' at line %d, col %d\n",
                            funcTable->scopeName, expectedType, actualType, node->line, node->column);
            }
        }

        return;
    }

    // ---------------- Recurse for all children ----------------
    for (int i = 0; i < node->childCount; i++) {
        checkReturnTypes(node->children[i], funcTable, expectedType, errorLog);
    }
}



// ---------------- Expression Type Inference ----------------
const char* getExprType(ASTNode *expr, SymbolTable *currentTable) {
    if (!expr || expr->childCount == 0) return NULL;

    ASTNode *first = expr->children[0];

    // Factor (literal)
    if (strcmp(first->kind, "factor") == 0) {
        const char *val = first->value;
        if (!val) return NULL;

        // Integer check
        int isInt = 1, i = 0;
        if (val[0] == '-' || val[0] == '+') i = 1;
        for (; val[i] != '\0'; i++) {
            if (val[i] < '0' || val[i] > '9') { isInt = 0; break; }
        }
        if (isInt) return "integer";

        // Float check
        int dotCount = 0;
        for (i = 0; val[i] != '\0'; i++) {
            if (val[i] == '.') dotCount++;
            else if (val[i] < '0' || val[i] > '9') { dotCount = -1; break; }
        }
        if (dotCount == 1) return "float";

        return NULL;
    }

    // Identifier / variable
    if (strcmp(first->kind, "idOrSelf") == 0) {
        Symbol *sym = lookupSymbol(currentTable, first->value, NULL);
        if (sym) return sym->type;
        return NULL;
    }

    // Recursively check children
    return getExprType(first, currentTable);
}
