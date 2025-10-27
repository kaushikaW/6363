#include "AST/ast.h"
#include "symbol_table.h"
#include <string.h>

void buildSymbolTable(ASTNode *root, SymbolTable *currentTable, const char *scope, const char *visibility) {
    if (!root) return;

    // --- Handle class declarations ---
    // When we find a classDecl node in the AST, we’re entering a new scope.
    if (strcmp(root->kind, "classDecl") == 0) {
        ASTNode *classIdNode = NULL;

        // Find ClassIdentifier
        for (int i = 0; i < root->childCount; i++) {
            if (strcmp(root->children[i]->kind, "ClassIdentifier") == 0)
                classIdNode = root->children[i];
        }

        if (!classIdNode) return;

        const char *className = classIdNode->value;

        // Insert class symbol
        insertSymbol(currentTable, className, SYM_CLASS, NULL, scope, "public", classIdNode->line, classIdNode->column);

        // Create nested table for class
        SymbolTable *classTable = createSymbolTable(className);
        addNestedTable(currentTable, classTable);

        // Recurse on all children (including MemberLists)
        for (int i = 0; i < root->childCount; i++)
            buildSymbolTable(root->children[i], classTable, className, "public");

        return;
    }

    // --- Handle MemberList or nested MemberListWrapper ---
    if (strcmp(root->kind, "MemberList") == 0 || strcmp(root->kind, "MemberListWrapper") == 0) {
        for (int i = 0; i < root->childCount; i++) {
            ASTNode *child = root->children[i];

            if (strcmp(child->kind, "visibility") == 0) {
                // Update current visibility
                visibility = child->value;
            } else if (strcmp(child->kind, "memberDecl") == 0) {
                buildSymbolTable(child, currentTable, scope, visibility);
            } else {
                // Recurse for nested MemberLists
                buildSymbolTable(child, currentTable, scope, visibility);
            }
        }
        return;
    }

    // --- Handle memberDecl ---
    if (strcmp(root->kind, "memberDecl") == 0) {
        for (int i = 0; i < root->childCount; i++)
            buildSymbolTable(root->children[i], currentTable, scope, visibility);
        return;
    }

    // --- Handle attributeDecl ---
    if (strcmp(root->kind, "attributeDecl") == 0) {
        ASTNode *varDecl = NULL;

        for (int i = 0; i < root->childCount; i++) {
            if (strcmp(root->children[i]->kind, "varDecl") == 0)
                varDecl = root->children[i];
        }

        if (varDecl && varDecl->childCount >= 2) {
            const char *varName = varDecl->children[0]->value;  // IDENTIFIER
            const char *varType = NULL;

            if (varDecl->children[1]->value)
                varType = varDecl->children[1]->value;

            insertSymbol(currentTable, varName, SYM_ATTRIBUTE, varType, scope, visibility, root->line, root->column);
        }
        return;
    }

    // --- Recurse on other children ---
    for (int i = 0; i < root->childCount; i++)
        buildSymbolTable(root->children[i], currentTable, scope, visibility);
}
