#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "../lexer/token.h"

extern Token *lookahead;
extern FILE *derivation;

void nextToken();

void match(const char *expectedType);

void syntax_error(const char *expected);

void prog();

void classOrImplOrFuncList();

void classOrImplOrFunc();

void classDecl();

void implDef();

void InheritanceOpt();

void MemberList();

void visibility();

void memberDecl();

void attributeDecl();

void funcDecl();

void funcHead();

void fParams();

void fParamsTailList();

void fParamsTail();

void returnType();

void arraySizeList();

void varDecl();

void type();

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
