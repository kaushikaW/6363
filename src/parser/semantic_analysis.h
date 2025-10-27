// semantic_analysis.h
#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H

#include "AST/ast.h"
#include "symbol_table.h"


SymbolTable* buildSymbolTable(ASTNode *root, SymbolTable *currentTable, const char *scope, const char *visibility);
#endif
