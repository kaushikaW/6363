// symbol_table.c
#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------- Symbol Table Basic Functions ----------------

SymbolTable* createSymbolTable(const char *scopeName) {
    SymbolTable *table = malloc(sizeof(SymbolTable));
    table->head = NULL;
    table->scopeName = scopeName ? strdup(scopeName) : strdup("GLOBAL");
    table->children = NULL;
    table->childCount = 0;
    return table;
}

void insertSymbol(SymbolTable *table, const char *name, SymbolKind kind,
                  const char *type, const char *scope, const char *visibility,
                  int line, int column) {
    Symbol *existing = lookupSymbol(table, name, scope);
    if (existing) {
        fprintf(stderr, "Semantic error: Duplicate declaration of '%s' at line %d, col %d\n",
                name, line, column);
        return;
    }

    Symbol *sym = malloc(sizeof(Symbol));
    sym->name = strdup(name);
    sym->kind = kind;
    sym->type = type ? strdup(type) : NULL;
    sym->scope = scope ? strdup(scope) : NULL;
    sym->visibility = visibility ? strdup(visibility) : NULL;
    sym->line = line;
    sym->column = column;
    sym->next = table->head;
    table->head = sym;
}

void addNestedTable(SymbolTable *parent, SymbolTable *child) {
    parent->children = realloc(parent->children, sizeof(SymbolTable*) * (parent->childCount + 1));
    parent->children[parent->childCount++] = child;
}

Symbol* lookupSymbol(SymbolTable *table, const char *name, const char *scope) {
    for (Symbol *s = table->head; s; s = s->next) {
        if (strcmp(s->name, name) == 0 &&
            (!scope || (s->scope && strcmp(s->scope, scope) == 0))) {
            return s;
        }
    }
    return NULL;
}

void printSymbolTable(SymbolTable *table) {
    if (!table) return;

    printf("\n===== SYMBOL TABLE for scope: %s =====\n", table->scopeName);
    printf("| %-12s | %-12s | %-12s | %-12s | %-10s | %-5s | %-5s |\n",
           "Name", "Kind", "Type", "Scope", "Visibility", "Line", "Col");
    printf("|--------------|--------------|--------------|--------------|------------|-------|-------|\n");

    for (Symbol *s = table->head; s; s = s->next) {
        const char *kindStr =
            (s->kind == SYM_CLASS) ? "Class" :
            (s->kind == SYM_FUNCTION) ? "Function" :
            (s->kind == SYM_ATTRIBUTE) ? "Attribute" : "Variable";

        printf("| %-12s | %-12s | %-12s | %-12s | %-10s | %-5d | %-5d |\n",
               s->name,
               kindStr,
               s->type ? s->type : "-",
               s->scope ? s->scope : "-",
               s->visibility ? s->visibility : "-",
               s->line,
               s->column);
    }

    for (int i = 0; i < table->childCount; i++) {
        printSymbolTable(table->children[i]);
    }
}

void freeSymbolTable(SymbolTable *table) {
    if (!table) return;

    Symbol *cur = table->head;
    while (cur) {
        Symbol *next = cur->next;
        free(cur->name);
        free(cur->type);
        free(cur->scope);
        free(cur->visibility);
        free(cur);
        cur = next;
    }

    for (int i = 0; i < table->childCount; i++) {
        freeSymbolTable(table->children[i]);
    }

    free(table->children);
    free(table->scopeName);
    free(table);
}

// ---------------- AST Helpers ----------------

ASTNode* findChild(ASTNode *node, const char *kind) {
    if (!node) return NULL;
    for (int i = 0; i < node->childCount; i++) {
        if (strcmp(node->children[i]->kind, kind) == 0)
            return node->children[i];
    }
    return NULL;
}

// ---------------- Handlers ----------------

void handleAttributeDecl(ASTNode *node, SymbolTable *currentTable, const char *scope, const char *visibility) {
    ASTNode *varDecl = findChild(node, "varDecl");
    if (!varDecl || varDecl->childCount < 2) return;

    const char *varName = varDecl->children[0]->value;
    const char *varType = varDecl->children[1]->value;

    insertSymbol(currentTable, varName, SYM_ATTRIBUTE, varType, scope, visibility, node->line, node->column);
}

void handleLocalVarDecl(ASTNode *localVarNode, SymbolTable *funcTable, const char *scope) {
    // localVarNode is expected to be the 'localVarDecl' AST node
    ASTNode *varDecl = findChild(localVarNode, "varDecl");
    if (!varDecl || varDecl->childCount < 2) return;

    const char *varName = varDecl->children[0]->value;
    const char *varType = varDecl->children[1]->value;

    insertSymbol(funcTable, varName, SYM_VARIABLE, varType, scope, NULL, localVarNode->line, localVarNode->column);
}

// Recursively walk varDeclOrStmtList / wrappers and collect localVarDecl nodes
void collectLocals(ASTNode *node, SymbolTable *funcTable, const char *funcName) {
    if (!node) return;

    // If wrapper nodes are used in your AST, handle them:
    // - varDeclOrStmtListWrapper  : contains two children (first list and rest)
    // - varDeclOrStmtList         : a list node whose children are varDeclOrStmt nodes (or wrappers)
    // - varDeclOrStmt             : either localVarDecl or statement

    if (strcmp(node->kind, "varDeclOrStmtListWrapper") == 0 ||
        strcmp(node->kind, "varDeclOrStmtListWrapper") == 0) {
        // wrapper usually contains two children: first list and rest
        for (int i = 0; i < node->childCount; i++) {
            collectLocals(node->children[i], funcTable, funcName);
        }
        return;
    }

    if (strcmp(node->kind, "varDeclOrStmtList") == 0) {
        for (int i = 0; i < node->childCount; i++) {
            collectLocals(node->children[i], funcTable, funcName);
        }
        return;
    }

    if (strcmp(node->kind, "varDeclOrStmt") == 0) {
        // varDeclOrStmt -> localVarDecl | statement
        if (node->childCount > 0) {
            ASTNode *first = node->children[0];
            if (first) collectLocals(first, funcTable, funcName);
        }
        return;
    }

    if (strcmp(node->kind, "localVarDecl") == 0) {
        // Got a local variable declaration
        handleLocalVarDecl(node, funcTable, funcName);
        return;
    }

    // Some ASTs may embed the localVarDecl one level deeper, so we check children generically
    for (int i = 0; i < node->childCount; i++) {
        collectLocals(node->children[i], funcTable, funcName);
    }
}

void handleFuncDecl(ASTNode *node, SymbolTable *currentTable, const char *scope, const char *visibility) {
    ASTNode *funcHead = findChild(node, "funcHead");
    if (!funcHead) return;

    ASTNode *funcIdentifier = findChild(funcHead, "funcIdentifier");
    ASTNode *returnTypeNode = findChild(funcHead, "returnType");

    if (!funcIdentifier) return;

    const char *funcName = funcIdentifier->value;
    const char *returnType = returnTypeNode ? returnTypeNode->value : "void";

    // Insert function in the current table (scope is e.g. GLOBAL or class name)
    insertSymbol(currentTable, funcName, SYM_FUNCTION, returnType,
                 scope ? scope : "GLOBAL", visibility, funcIdentifier->line, funcIdentifier->column);

    // Create a nested table for function scope
    SymbolTable *funcTable = createSymbolTable(funcName);
    addNestedTable(currentTable, funcTable);

    // Find the funcBody and collect local variables recursively
    ASTNode *funcBody = findChild(node, "funcBody");
    if (!funcBody) {
        // Sometimes funcDef node might contain varDeclOrStmtList directly, so try scanning children
        for (int i = 0; i < node->childCount; i++) {
            collectLocals(node->children[i], funcTable, funcName);
        }
        return;
    }

    // funcBody usually contains one child varDeclOrStmtList or wrapper: traverse it
    for (int i = 0; i < funcBody->childCount; i++) {
        collectLocals(funcBody->children[i], funcTable, funcName);
    }
}

void handleClassDecl(ASTNode *node, SymbolTable *currentTable) {
    ASTNode *classIdNode = findChild(node, "ClassIdentifier");
    if (!classIdNode) return;

    const char *className = classIdNode->value;

    insertSymbol(currentTable, className, SYM_CLASS, NULL, NULL, "public",
                 classIdNode->line, classIdNode->column);

    SymbolTable *classTable = createSymbolTable(className);
    addNestedTable(currentTable, classTable);

    for (int i = 0; i < node->childCount; i++) {
        buildSymbolTable(node->children[i], classTable, className, "public");
    }
}

// ---------------- Main Symbol Table Builder ----------------

void buildSymbolTable(ASTNode *root, SymbolTable *currentTable, const char *scope, const char *visibility) {
    if (!root) return;

    if (strcmp(root->kind, "classDecl") == 0) {
        handleClassDecl(root, currentTable);
        return;
    }

    if (strcmp(root->kind, "funcDef") == 0 || strcmp(root->kind, "funcDecl") == 0) {
        handleFuncDecl(root, currentTable, scope, visibility);
        return;
    }

    if (strcmp(root->kind, "attributeDecl") == 0) {
        handleAttributeDecl(root, currentTable, scope, visibility);
        return;
    }

    if (strcmp(root->kind, "MemberList") == 0 || strcmp(root->kind, "MemberListWrapper") == 0) {
        for (int i = 0; i < root->childCount; i++) {
            ASTNode *child = root->children[i];
            const char *childVisibility = visibility;
            if (strcmp(child->kind, "visibility") == 0) {
                childVisibility = child->value;
            } else {
                buildSymbolTable(child, currentTable, scope, childVisibility);
            }
        }
        return;
    }

    // Generic recursion
    for (int i = 0; i < root->childCount; i++) {
        buildSymbolTable(root->children[i], currentTable, scope, visibility);
    }
}
