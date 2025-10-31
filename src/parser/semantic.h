#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdio.h>
#include "AST/ast.h"
#include "symbol_table.h"

// Entry point for semantic analysis. errorLog may be NULL to skip file logging.
void analyzeSemantics(ASTNode *root, SymbolTable *globalTable, FILE *errorLog);

// Type inference helper for expressions
// returns string literal: "integer", "float", or NULL when unknown
const char* getExprType(ASTNode *expr, SymbolTable *currentTable);

// utility used by other semantic checks (kept public if you expand later)
void checkAssignments(ASTNode *node, SymbolTable *currentTable, FILE *errorLog);

void checkReturnStatements(ASTNode *funcDefNode, const char *declaredReturnType, SymbolTable *funcTable, FILE *errorLog);

void checkReturnTypes(ASTNode *node, SymbolTable *funcTable, const char *expectedType, FILE *errorLog);

#endif // SEMANTIC_H
