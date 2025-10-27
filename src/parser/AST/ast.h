#ifndef AST_H
#define AST_H
#include <stdio.h>


typedef struct ASTNode {
    char *kind;
    char *value;
    struct ASTNode **children;
    int childCount;
    int line, column;
} ASTNode;

// Function declarations
ASTNode* createASTNode( char *kind, const char* value, int line, int column);
void addChild(ASTNode* parent, ASTNode* child);
void freeAST(ASTNode* node);
void printAST(ASTNode* node, int indent);

#endif

