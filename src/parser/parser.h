#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "../lexer/token.h"
#include "AST/ast.h"


extern Token *lookahead;
extern FILE *derivation;

void nextToken();

void match(const char *expectedType);

void syntax_error(const char *expected);

ASTNode* prog();

ASTNode* classOrImplOrFuncList();

ASTNode* classOrImplOrFunc();

ASTNode* classDecl();

void implDef();

void InheritanceOpt();

ASTNode* MemberList();

ASTNode* visibility();

ASTNode* memberDecl();

ASTNode* attributeDecl();

void funcDecl();

void funcHead();

void fParams();

void fParamsTailList();

void fParamsTail();

void returnType();

void arraySizeList();

ASTNode* varDecl();

ASTNode* type();

void funcDef();

void funcBody();

void varDeclOrStmtList();

void varDeclOrStmt();

void localVarDecl();

void statement();

void assignStat();

void variable();

void idOrSelf();

void indiceList();

void indice();

void expr();

void arithExpr();

void arithExpr_();

void term();

void factor();

void addOp();

void idOrSelfTail();

void functionCall();

void aParams();

void idNestTail();

void assignOp();

void term_();

void relOp();

void multOp();

void FuncDefList();

void aParamsTailList();

void statementTail();

void statementList();

void statBlock();

void relExpr();

void idOrSelfStatement();

void arraySize();

void idOrSelfTailWithAssignOrCall();

void BaseIdTail();


#endif
