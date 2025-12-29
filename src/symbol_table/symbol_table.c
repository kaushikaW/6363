// symbol_table.c


#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../semantic_error/semantic_error.h"

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
                  int line, int column, int isDeclared) {

    // cheking duplicates
    Symbol *existing = lookupSymbol(table, name, scope);

    if (existing) {
        addSemanticError(line, column, "Scope Error",
                         "Duplicate declaration of '%s' in scope '%s'", name, scope);
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
    sym->isDeclared = isDeclared;
    sym->params = NULL;
    sym->next = NULL;  //  tail insertion

    // Insert at tail
    if (!table->head) {
        table->head = sym;
    } else {
        Symbol *cur = table->head;

        while (cur->next) {
            cur = cur->next;
        }
        cur->next = sym;
    }
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
    printf("| %-12s | %-12s | %-12s | %-12s | %-10s | %-5s | %-5s | %-10s | %-6s |\n",
           "Name", "Kind", "Type", "Scope", "Visibility", "Line", "Col", "Declared?", "Offset");
    printf("|--------------|--------------|--------------|--------------|------------|-------|-------|------------|--------|\n");

    for (Symbol *s = table->head; s; s = s->next) {
        const char *kindStr =
            (s->kind == SYM_CLASS) ? "Class" :
            (s->kind == SYM_FUNCTION) ? "Function" :
            (s->kind == SYM_ATTRIBUTE) ? "Attribute" : "Variable";

        printf("| %-12s | %-12s | %-12s | %-12s | %-10s | %-5d | %-5d | %-10s | %-6d |\n",
               s->name,
               kindStr,
               s->type ? s->type : "-",
               s->scope ? s->scope : "-",
               s->visibility ? s->visibility : "-",
               s->line,
               s->column,
               (s->kind == SYM_FUNCTION) ? (s->isDeclared ? "Yes" : "No") : "-",
               s->offset = 0);
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
        // Free parameters
        Param *p = cur->params;
        while (p) {
            Param *nextP = p->next;
            free(p->name);
            free(p->type);
            free(p);
            p = nextP;
        }
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

// get the node with node kind

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

    insertSymbol(currentTable, varName, SYM_ATTRIBUTE, varType, scope, visibility, node->line, node->column, 0);
}

void handleLocalVarDecl(ASTNode *localVarNode, SymbolTable *funcTable, const char *scope) {
    ASTNode *varDecl = findChild(localVarNode, "varDecl");
    if (!varDecl || varDecl->childCount < 2) return;


    const char *varName = varDecl->children[0]->value;
    const char *varType = varDecl->children[1]->value;

    insertSymbol(funcTable, varName, SYM_VARIABLE, varType, scope, NULL, localVarNode->line, localVarNode->column, 0);
}

// collect local variable declarations
void collectLocals(ASTNode *node, SymbolTable *funcTable, const char *funcName) {
    if (!node) return;

    if (strcmp(node->kind, "varDeclOrStmtListWrapper") == 0 ||
        strcmp(node->kind, "varDeclOrStmtList") == 0) {
        for (int i = 0; i < node->childCount; i++) {
            collectLocals(node->children[i], funcTable, funcName);
        }
        return;
    }

    if (strcmp(node->kind, "varDeclOrStmt") == 0 && node->childCount > 0) {
        collectLocals(node->children[0], funcTable, funcName);
        return;
    }

    if (strcmp(node->kind, "localVarDecl") == 0) {
        handleLocalVarDecl(node, funcTable, funcName);
        return;
    }

    for (int i = 0; i < node->childCount; i++) {
        collectLocals(node->children[i], funcTable, funcName);
    }
}

void collectFuncParams(ASTNode *node, SymbolTable *funcTable, const char *funcName) {
    if (!node) return;

    if (strcmp(node->kind, "fParams") == 0) {
        ASTNode *idNode = findChild(node, "IDENTIFIER");
        ASTNode *typeNode = findChild(node, "type");
        if (idNode && typeNode) {
            insertSymbol(funcTable, idNode->value, SYM_VARIABLE, typeNode->value, funcName, "param", idNode->line, idNode->column, 0);
        }
        for (int i = 0; i < node->childCount; i++) {
            if (strcmp(node->children[i]->kind, "fParamsTailList") == 0 ||
                strcmp(node->children[i]->kind, "fParamsTailListWrapper") == 0)
                collectFuncParams(node->children[i], funcTable, funcName);
        }
        return;
    }

    if (strcmp(node->kind, "fParamsTailList") == 0 || strcmp(node->kind, "fParamsTailListWrapper") == 0) {
        for (int i = 0; i < node->childCount; i++)
            collectFuncParams(node->children[i], funcTable, funcName);
        return;
    }

    if (strcmp(node->kind, "fParamsTail") == 0) {
        ASTNode *idNode = findChild(node, "IDENTIFIER");
        ASTNode *typeNode = findChild(node, "type");
        if (idNode && typeNode) {
            insertSymbol(funcTable, idNode->value, SYM_VARIABLE, typeNode->value, funcName, "param", idNode->line, idNode->column, 0);
        }
        return;
    }

    for (int i = 0; i < node->childCount; i++)
        collectFuncParams(node->children[i], funcTable, funcName);
}

void handleFuncDecl(ASTNode *node, SymbolTable *currentTable,
                    const char *scope, const char *visibility, int isDeclaration) {
    ASTNode *funcHead = findChild(node, "funcHead");
    if (!funcHead) return;

    ASTNode *funcIdentifier = findChild(funcHead, "funcIdentifier");
    ASTNode *returnTypeNode = findChild(funcHead, "returnType");
    if (!funcIdentifier) return;

    const char *funcName = funcIdentifier->value;
    const char *returnType = returnTypeNode ? returnTypeNode->value : "void";

    // Lookup if function already exists
    Symbol *existing = lookupSymbol(currentTable, funcName, scope);
    SymbolTable *funcTable = NULL;

    if (!existing) {
        // Not declared yet, insert symbol
        insertSymbol(currentTable, funcName, SYM_FUNCTION, returnType,
                     scope ? scope : "GLOBAL", visibility,
                     funcIdentifier->line, funcIdentifier->column, isDeclaration);

        // Create nested table for this function
        funcTable = createSymbolTable(funcName);
        addNestedTable(currentTable, funcTable);
    } else {
        // Function already exists
        if (!isDeclaration) existing->isDeclared = 0;  // mark as implemented

        // Reuse existing function table
        for (int i = 0; i < currentTable->childCount; i++) {
            if (strcmp(currentTable->children[i]->scopeName, funcName) == 0) {
                funcTable = currentTable->children[i];
                break;
            }
        }
        if (!funcTable) { // fallback
            funcTable = createSymbolTable(funcName);
            addNestedTable(currentTable, funcTable);
        }
    }

    // Collect parameters
    ASTNode *fParams = findChild(funcHead, "fParams");
    if (fParams) collectFuncParams(fParams, funcTable, funcName);

    // Collect local variables from body
    ASTNode *funcBody = findChild(node, "funcBody");

    if (funcBody) {
        for (int i = 0; i < funcBody->childCount; i++)
            collectLocals(funcBody->children[i], funcTable, funcName);
    }
}



void handleClassDecl(ASTNode *node, SymbolTable *currentTable) {
    ASTNode *classIdNode = findChild(node, "ClassIdentifier");
    if (!classIdNode) return;

    const char *className = classIdNode->value;
    insertSymbol(currentTable, className, SYM_CLASS, "class-type", "Global", "public",
                 classIdNode->line, classIdNode->column, 0);

    SymbolTable *classTable = createSymbolTable(className);

    addNestedTable(currentTable, classTable);

    // Default visibility for class members
    const char *currentVisibility = "public";

    // Traverse class members
    for (int i = 0; i < node->childCount; i++) {
        ASTNode *child = node->children[i];
        if (strcmp(child->kind, "MemberListWrapper") == 0 || strcmp(child->kind, "MemberList") == 0) {
            for (int j = 0; j < child->childCount; j++) {
                ASTNode *member = child->children[j];
                if (strcmp(member->kind, "visibility") == 0) {
                    currentVisibility = member->value; // update visibility
                } else {
                    buildSymbolTable(member, classTable, className, currentVisibility);
                }
            }
        }
    }
}

void handleImplDef(ASTNode *node, SymbolTable *currentTable) {
    ASTNode *classIdNode = findChild(node, "ClassIdentifier");
    if (!classIdNode) return;
    const char *className = classIdNode->value;

    // Find class symbol table
    SymbolTable *classTable = NULL;
    for (int i = 0; i < currentTable->childCount; i++) {
        if (strcmp(currentTable->children[i]->scopeName, className) == 0) {
            classTable = currentTable->children[i];
            break;
        }
    }
    if (!classTable) {
        addSemanticError(node->line, node->column, "Scope Error",
                         "Implementation for unknown class '%s'", className, NULL);
        return;
    }

    // Traverse all FuncDefList and FuncDefListWrapper nodes
    for (int i = 0; i < node->childCount; i++) {
        ASTNode *child = node->children[i];
        if (strcmp(child->kind, "FuncDefList") == 0 ||
            strcmp(child->kind, "FuncDefListWrapper") == 0) {
            for (int j = 0; j < child->childCount; j++) {
                ASTNode *funcNode = child->children[j];
                if (strcmp(funcNode->kind, "funcDef") == 0) {
                    handleFuncDecl(funcNode, classTable, className, "public", 0);
                }
            }
        }
    }
}


void buildSymbolTable(ASTNode *root, SymbolTable *currentTable, const char *scope, const char *visibility) {
    if (!root) return;

    if (strcmp(root->kind, "classDecl") == 0) {
        handleClassDecl(root, currentTable);
        return;
    }

    if (strcmp(root->kind, "implDef") == 0) {
        handleImplDef(root, currentTable);
        return;
    }

    if (strcmp(root->kind, "funcDef") == 0) {
        handleFuncDecl(root, currentTable, scope, visibility, 0); // implementation
        return;
    }

    if (strcmp(root->kind, "funcDecl") == 0) {
        handleFuncDecl(root, currentTable, scope, visibility, 1); // declaration
        return;
    }

    if (strcmp(root->kind, "attributeDecl") == 0) {
        handleAttributeDecl(root, currentTable, scope, visibility);
        return;
    }

    if (strcmp(root->kind, "MemberList") == 0 || strcmp(root->kind, "MemberListWrapper") == 0) {
        const char *currentVisibility = visibility; // inherit from parent
        for (int i = 0; i < root->childCount; i++) {
            ASTNode *child = root->children[i];
            if (strcmp(child->kind, "visibility") == 0) {
                currentVisibility = child->value; // update visibility
            } else {
                buildSymbolTable(child, currentTable, scope, currentVisibility);
            }
        }
        return;
    }

    for (int i = 0; i < root->childCount; i++)
        buildSymbolTable(root->children[i], currentTable, scope, visibility);
}


void calculateOffsets(SymbolTable *symbolTable) {
    if (!symbolTable) return;

    Symbol *curr = symbolTable->head;
    int offset = -8;  // start from -8 for the first local/temporary

    while (curr) {
        curr->offset = offset;
        offset -= 8;
        curr = curr->next;
    }
}



