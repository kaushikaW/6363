#include "three_address_code.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int tempCount = 0;

// Create a new quadruple list
QuadList* createQuadList() {
    QuadList *list = malloc(sizeof(QuadList));
    list->count = 0;
    list->capacity = 10;
    list->quads = malloc(sizeof(Quadruple*) * list->capacity);
    return list;
}

// Add a new quadruple to the list
void addQuad(QuadList *list, char *op, char *arg1, char *arg2, char *result) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->quads = realloc(list->quads, sizeof(Quadruple*) * list->capacity);
    }
    Quadruple *q = malloc(sizeof(Quadruple));
    q->op = strdup(op);
    q->arg1 = arg1 ? strdup(arg1) : NULL;
    q->arg2 = arg2 ? strdup(arg2) : NULL;
    q->result = strdup(result);
    list->quads[list->count++] = q;
}

// Generate a new temporary variable
char* newTemp() {
    char buffer[10];
    sprintf(buffer, "t%d", tempCount++);
    return strdup(buffer);
}

// Recursive 3AC generation
char* generate3AC(ASTNode *node, QuadList *list) {
    if (!node) return NULL;

    // Assignment
    if (strcmp(node->kind, "idOrSelfTailWithAssignOrCall") == 0) {
        char *lhs = strdup(node->children[0]->value); // LHS
        char *rhs = generate3AC(node->children[1], list); // RHS expr
        addQuad(list, ":=", rhs, NULL, lhs);
        free(rhs);
        return lhs;
    }

    // Arithmetic expressions
    if (strcmp(node->kind, "arithExpr") == 0) {
        char *left = generate3AC(node->children[0], list); // term
        char *res = left;

        if (node->childCount > 1) { // arithExpr_
            ASTNode *rest = node->children[1]; // arithExpr_
            if (rest->childCount > 0) {
                char *right = generate3AC(rest->children[1], list); // term
                char *temp = newTemp();
                char *op = rest->children[0]->value; // addOp
                addQuad(list, op, left, right, temp);
                free(left);
                free(right);
                res = temp;
            }
        }
        return res;
    }

    if (strcmp(node->kind, "term") == 0 || strcmp(node->kind, "factor") == 0) {
        if (node->childCount == 1) {
            if (strcmp(node->children[0]->kind, "idOrSelf") == 0)
                return strdup(node->children[0]->value);
            else
                return generate3AC(node->children[0], list);
        } else {
            return strdup(node->value); // numeric literal
        }
    }

    // Recurse for other nodes/statements
    char *res = NULL;
    for (int i = 0; i < node->childCount; i++) {
        res = generate3AC(node->children[i], list);
    }
    return res;
}



// Print the generated 3AC
void print3AC(QuadList *list) {
    printf("\nThree Address Code (3AC):\n");
    for (int i = 0; i < list->count; i++) {
        Quadruple *q = list->quads[i];
        printf("%d: %s, %s, %s, %s\n", i, q->op,
               q->arg1 ? q->arg1 : "-",
               q->arg2 ? q->arg2 : "-",
               q->result);
    }
}

// Free the 3AC list
void free3AC(QuadList *list) {
    for (int i = 0; i < list->count; i++) {
        free(list->quads[i]->op);
        if(list->quads[i]->arg1) free(list->quads[i]->arg1);
        if(list->quads[i]->arg2) free(list->quads[i]->arg2);
        free(list->quads[i]->result);
        free(list->quads[i]);
    }
    free(list->quads);
    free(list);
}
