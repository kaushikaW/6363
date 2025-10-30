#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "symbol_table.h"
#include "AST/ast.h"

void analyzeSemantics(ASTNode *root, SymbolTable *currentTable, FILE *errorLog);
void checkAssignments(ASTNode *node, SymbolTable *currentTable, FILE *errorLog);
const char* getExprType(ASTNode *expr, SymbolTable *currentTable);

#endif
