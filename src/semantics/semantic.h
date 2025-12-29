#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdio.h>
#include "../parser/ast.h"
#include "../symbol_table/symbol_table.h"

// Entry point for semantic analysis. errorLog may be NULL to skip file logging.
void analyzeSemantics(ASTNode *root, SymbolTable *globalTable);

// Type inference helper for expressions
// returns string literal: "integer", "float", or NULL when unknown
const char* getExprType(ASTNode *expr, SymbolTable *currentTable);

// utility used by other semantic checks (kept public if you expand later)
void checkAssignments(ASTNode *node, SymbolTable *currentTable);

void checkReturnStatements(ASTNode *funcDefNode, const char *declaredReturnType, SymbolTable *funcTable, FILE *errorLog);

void checkReturnTypes(ASTNode *node, SymbolTable *funcTable, const char *expectedType);

int isFunctionDeclared(SymbolTable *classTable, const char *funcName);

void handleImplDef(ASTNode *node, SymbolTable *currentTable, FILE *errorLog);

void checkFunctionCalls(ASTNode *node, SymbolTable *currentTable, FILE *errorLog);

Symbol* lookupFunctionBeforeUse(SymbolTable *table, const char *name, int currentLine);

Symbol* lookupSymbolRecursive(SymbolTable *table, const char *name, const char *scope);
#endif // SEMANTIC_H
