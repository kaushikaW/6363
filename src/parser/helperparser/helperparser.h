#ifndef HELPERPARSER_H
#define HELPERPARSER_H

#include <stdio.h>
#include "../../lexer/token.h"
#include "../AST/ast.h"


extern Token *lookahead;
extern FILE *derivation;
typedef enum { NODE_EXPR, NODE_STMT, NODE_DECL, NODE_FUNC } NodeType_n;

typedef struct ASTNode_n {
    char *kind;
    char *value;
    struct ASTNode_n **children;
    NodeType_n type;
    int childCount;
    int line,
    column;
} ASTNode_n;

typedef struct {
    char *op;
    char *arg1;
    char *arg2;
    char *result;
} Quadruple;


// Function declarations
ASTNode_n* createASTNode_n(char *kind, const char *value, int line, int column);
void addChild_n(ASTNode_n* parent, ASTNode_n* child);
void freeAST_n(ASTNode_n* node);
void printAST_n(ASTNode_n* node, int indent);


void nextToken_m();

void match_m(const char *expectedType);

void syntax_error_m(const char *expected);

void foo();
char* newLabel();
//3AC code
void addQuad(char *op, char *arg1, char *arg2, char *result);

ASTNode_n * prog_n();

#endif
