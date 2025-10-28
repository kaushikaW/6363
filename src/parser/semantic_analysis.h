// semantic_analysis.h
#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H

#include "AST/ast.h"
#include "symbol_table.h"

// Entry point for semantic analysis
void performSemanticAnalysis(ASTNode *root, SymbolTable *globalTable);

#endif
