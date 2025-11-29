#ifndef THREE_ADDRESS_CODE_H
#define THREE_ADDRESS_CODE_H

#include "AST/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Quadruple structure
typedef struct Quadruple {
    char *op;
    char *arg1;
    char *arg2;
    char *result;
} Quadruple;

typedef struct QuadList {
    Quadruple **quads;
    int count;
    int capacity;
} QuadList;

// Function declarations
QuadList* createQuadList();
void addQuad(QuadList *list, char *op, char *arg1, char *arg2, char *result);
char* newTemp();
char* generate3AC(ASTNode *node, QuadList *list);
void print3AC(QuadList *list);
void free3AC(QuadList *list);
char* generateArithExprRest(char *leftTemp, ASTNode *node, QuadList *list);
#endif
