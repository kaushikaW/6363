#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helpers/symbol_table.h"
#include "helpers/helperparser.h"
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
// Global flag to check if printf is needed
//-----------------------------------------------------
int needsPrintf = 0;

//-----------------------------------------------------
// Function PROLOGUE and EPILOGUE
//-----------------------------------------------------
void emitPrologue() {
    // Emit .rodata only if write() exists
    if (needsPrintf) {
        emit(".section .rodata\n");
        emit("fmt_int:\n    .string \"%%ld\\n\"\n");
        emit("fmt_nl:\n    .string \"\\n\"\n\n");
    }

    emit(".text\n");
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

    if (strcmp(node->kind, "INTEGER") == 0) {
        emit("    movq $%s, %%rax\n", node->value);
        return;
    }

    if (strcmp(node->kind, "IDENTIFIER") == 0) {
        int offset = getOffset(node->value, scope);
        emit("    movq %d(%%rbp), %%rax\n", offset);
        return;
    }

    // Handle Binary Operators (+, -, , /)

    if (!strcmp(node->kind, "PLUS") ||
        !strcmp(node->kind, "MINUS") ||
        !strcmp(node->kind, "TIMES") ||
        !strcmp(node->kind, "DIVIDE")) {

        // right operand
        generateASMExpr(node->children[0], scope);
        emit("    pushq %%rax\n");

        // left  operand
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
            emit("    cqto\n");
            emit("    idivq %%rbx\n");
        }
    }
}

//-----------------------------------------------------
// Generate assembly for statements
//-----------------------------------------------------
void generateASMStmt(ASTNode_n *node, char *scope) {
    if (!node) return;

    if (!strcmp(node->kind, "assignment")) {
        generateASMExpr(node->children[0], scope);
        int offset = getOffset(node->value, scope);
        emit("    movq %%rax, %d(%%rbp)\n", offset);
    }
    else if (!strcmp(node->kind, "statement_list")) {
        for (int i = 0; i < node->childCount; i++)
            generateASMStmt(node->children[i], scope);
    }
    else if (!strcmp(node->kind, "write")) {
        // Mark that printf is needed
        needsPrintf = 1;

        generateASMExpr(node->children[0], scope); // value in %rax
        emit("    movq $fmt_int, %%rdi\n");        // format string
        emit("    movq %%rax, %%rsi\n");           // value
        emit("    xor %%rax, %%rax\n");            // clear rax for variadic
        emit("    call printf@PLT\n");
    }
    else if (!strcmp(node->kind, "if_statement")) {
        static int labelCount = 0;
        int currentLabel = labelCount++;

        ASTNode_n *condNode = node->children[0];
        ASTNode_n *thenNode = node->children[1];
        ASTNode_n *elseNode = (node->childCount > 2) ? node->children[2] : NULL;

        generateASMExpr(condNode->children[0], scope);
        emit("    pushq %%rax\n");
        generateASMExpr(condNode->children[1], scope);
        emit("    movq %%rax, %%rbx\n");
        emit("    popq %%rax\n");

        if (!strcmp(condNode->kind, "LT"))
            emit("    cmpq %%rbx, %%rax\n    jge ELSE_%d\n", currentLabel);
        else if (!strcmp(condNode->kind, "GT"))
            emit("    cmpq %%rbx, %%rax\n    jle ELSE_%d\n", currentLabel);
        else if (!strcmp(condNode->kind, "LE"))
            emit("    cmpq %%rbx, %%rax\n    jg ELSE_%d\n", currentLabel);
        else if (!strcmp(condNode->kind, "GE"))
            emit("    cmpq %%rbx, %%rax\n    jl ELSE_%d\n", currentLabel);
        else if (!strcmp(condNode->kind, "EQ"))
            emit("    cmpq %%rbx, %%rax\n    jne ELSE_%d\n", currentLabel);
        else if (!strcmp(condNode->kind, "NE"))
            emit("    cmpq %%rbx, %%rax\n    je ELSE_%d\n", currentLabel);

        generateASMStmt(thenNode, scope);
        emit("    jmp ENDIF_%d\n", currentLabel);
        emit("ELSE_%d:\n", currentLabel);
        if (elseNode)
            generateASMStmt(elseNode, scope);
        emit("ENDIF_%d:\n", currentLabel);
    }
    else if (!strcmp(node->kind, "while_statement")) {
        static int whileLabelCount = 0;
        int currentLabel = whileLabelCount++;

        ASTNode_n *condNode = node->children[0];
        ASTNode_n *bodyNode = node->children[1];

        emit("WHILE_START_%d:\n", currentLabel);

        generateASMExpr(condNode->children[0], scope);
        emit("    pushq %%rax\n");
        generateASMExpr(condNode->children[1], scope);
        emit("    movq %%rax, %%rbx\n");
        emit("    popq %%rax\n");

        if (!strcmp(condNode->kind, "LT"))
            emit("    cmpq %%rbx, %%rax\n    jge WHILE_END_%d\n", currentLabel);
        else if (!strcmp(condNode->kind, "GT"))
            emit("    cmpq %%rbx, %%rax\n    jle WHILE_END_%d\n", currentLabel);
        else if (!strcmp(condNode->kind, "LE"))
            emit("    cmpq %%rbx, %%rax\n    jg WHILE_END_%d\n", currentLabel);
        else if (!strcmp(condNode->kind, "GE"))
            emit("    cmpq %%rbx, %%rax\n    jl WHILE_END_%d\n", currentLabel);
        else if (!strcmp(condNode->kind, "EQ"))
            emit("    cmpq %%rbx, %%rax\n    jne WHILE_END_%d\n", currentLabel);
        else if (!strcmp(condNode->kind, "NE"))
            emit("    cmpq %%rbx, %%rax\n    je WHILE_END_%d\n", currentLabel);

        generateASMStmt(bodyNode, scope);
        emit("    jmp WHILE_START_%d\n", currentLabel);
        emit("WHILE_END_%d:\n", currentLabel);
    }
}

//-----------------------------------------------------
// Main ASM generator
//-----------------------------------------------------
void generateASM(ASTNode_n *root) {
    emit("\n# ===== Generated x86-64 Assembly =====\n\n");

    emitPrologue();

    for (int i = 0; i < root->childCount; i++)
        generateASMStmt(root->children[i], "main");

    emitEpilogue();

    emit("\n# ===== END OF ASM =====\n");
}
