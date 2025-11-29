#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "helperparser.h"
#include <stdarg.h>

extern SymbolTable *symbolTable;   // global symbol table

//-----------------------------------------------------
// Emit helper (console only)
//-----------------------------------------------------
void emit(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);  // print to console
    va_end(args);
}

//-----------------------------------------------------
// Function PROLOGUE and EPILOGUE
//-----------------------------------------------------
void emitPrologue() {
    emit(".globl main\n");
    emit("main:\n");
    emit("    pushq %%rbp\n");
    emit("    movq %%rsp, %%rbp\n");

    int totalBytes = totalLocalBytes();
    if (totalBytes > 0)
        emit("    subq $%d, %%rsp    # allocate local variables\n", totalBytes);
}

void emitEpilogue() {
    emit("    leave\n");  // equivalent to mov %rbp,%rsp ; pop %rbp
    emit("    ret\n");
}

//-----------------------------------------------------
// Get variable offset from symbol table
//-----------------------------------------------------
int getOffset(char *name, char *scope) {
    Symbol *s = symbolTable->head;
    while (s) {
        if (strcmp(s->name, name) == 0 && strcmp(s->scope, scope) == 0)
            return s->offset;
        s = s->next;
    }
    fprintf(stderr, "ERROR: variable %s not found in scope %s\n", name, scope);
    exit(1);
}

//-----------------------------------------------------
// Generate assembly for expressions
//-----------------------------------------------------
void generateASMExpr(ASTNode_n *node, char *scope) {
    if (!node) return;

    // INTEGER literal
    if (strcmp(node->kind, "INTEGER") == 0) {
        emit("    movq $%s, %%rax\n", node->value);
        return;
    }

    // Identifier (variable)
    if (strcmp(node->kind, "IDENTIFIER") == 0) {
        int offset = getOffset(node->value, scope);
        emit("    movq %d(%%rbp), %%rax\n", offset);
        return;
    }

    // Binary operators
    if (!strcmp(node->kind, "PLUS") ||
        !strcmp(node->kind, "MINUS") ||
        !strcmp(node->kind, "TIMES") ||
        !strcmp(node->kind, "DIVIDE")) {

        // Evaluate left
        generateASMExpr(node->children[0], scope);
        emit("    pushq %%rax\n");

        // Evaluate right
        generateASMExpr(node->children[1], scope);
        emit("    movq %%rax, %%rbx\n");

        emit("    popq %%rax\n");

        if (!strcmp(node->kind, "PLUS"))
            emit("    addq %%rbx, %%rax\n");
        else if (!strcmp(node->kind, "MINUS"))
            emit("    subq %%rbx, %%rax\n");
        else if (!strcmp(node->kind, "TIMES"))
            emit("    imulq %%rbx, %%rax\n");
        else if (!strcmp(node->kind, "DIVIDE")) {
            emit("    cqto\n");          // sign extend rax into rdx
            emit("    idivq %%rbx\n");   // divide rdx:rax by rbx
        }
    }
}

//-----------------------------------------------------
// Generate assembly for statements
//-----------------------------------------------------
void generateASMStmt(ASTNode_n *node, char *scope) {
    if (!node) return;

    if (!strcmp(node->kind, "assignment")) {
        // compute RHS into RAX
        generateASMExpr(node->children[0], scope);

        // store into variable
        int offset = getOffset(node->value, scope);
        emit("    movq %%rax, %d(%%rbp)\n", offset);
    }
    else if (!strcmp(node->kind, "statement_list")) {
        for (int i = 0; i < node->childCount; i++)
            generateASMStmt(node->children[i], scope);
    }
}

//-----------------------------------------------------
// Main ASM generator (console output ONLY)
//-----------------------------------------------------
void generateASM(ASTNode_n *root) {
    emit("\n# ===== Generated x86-64 Assembly =====\n\n");

    emitPrologue();

    for (int i = 0; i < root->childCount; i++)
        generateASMStmt(root->children[i], "main");

    emitEpilogue();

    emit("\n# ===== END OF ASM =====\n");
}
