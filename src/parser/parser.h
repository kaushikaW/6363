#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "../lexer/token.h"
#include "../parser/ast.h"


extern Token *lookahead;
extern FILE *derivation;

void nextToken();

void match(const char *expectedType);

void syntax_error(const char *expected);

ASTNode* prog();

ASTNode* classOrImplOrFuncList();

ASTNode* classOrImplOrFunc();

ASTNode* classDecl();

ASTNode* implDef();

void InheritanceOpt();

ASTNode* MemberList();

ASTNode* visibility();

ASTNode* memberDecl();

ASTNode* attributeDecl();

ASTNode* funcDecl();

ASTNode* funcHead();

ASTNode* fParams();

ASTNode* fParamsTailList();

ASTNode* fParamsTail();

ASTNode* returnType();

void arraySizeList();

ASTNode* varDecl();

ASTNode* type();

ASTNode* funcDef();

ASTNode* funcBody();

ASTNode* varDeclOrStmtList();

ASTNode* varDeclOrStmt();

ASTNode* localVarDecl();

ASTNode* statement();

void assignStat();

void variable();

ASTNode* idOrSelf();

void indiceList();

void indice();

ASTNode*  expr();

ASTNode* arithExpr();

ASTNode* arithExpr_();

ASTNode*  term();

ASTNode* factor();

ASTNode* addOp();

void idOrSelfTail();

void functionCall();

void aParams();

void idNestTail();

ASTNode* assignOp();

void term_();

void relOp();

void multOp();

ASTNode* FuncDefList();

void aParamsTailList();

void statementTail();

void statementList();

void statBlock();

void relExpr();

ASTNode* idOrSelfStatement();

void arraySize();

ASTNode* idOrSelfTailWithAssignOrCall();

void BaseIdTail();


#endif
