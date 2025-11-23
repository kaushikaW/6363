#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include "../lexer/token.h"

#include "parser.h"

#include "AST/ast.h"

#include "symbol_table.h"

#include "semantic.h"

#include "semantic_error.h"


extern Token * yylex();

Token * lookahead; // current token
FILE * derivation; // output file for derivation

ASTNode * createASTNode(char * kind,
  const char * value, int line, int column) {
  ASTNode * node = malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Memory allocation failed for ASTNode\n");
    exit(EXIT_FAILURE);
  }

  node -> kind = kind ? strdup(kind) : NULL;
  node -> value = value ? strdup(value) : NULL;
  node -> children = NULL;
  node -> childCount = 0;
  node -> line = line;
  node -> column = column;

  return node;
}

void addChild(ASTNode * parent, ASTNode * child) {
  if (!parent || !child) return;
  parent -> childCount++;
  parent -> children = (ASTNode ** ) realloc(parent -> children, parent -> childCount * sizeof(ASTNode * ));
  parent -> children[parent -> childCount - 1] = child;
}

void freeAST(ASTNode * node) {
  if (!node) return;
  for (int i = 0; i < node -> childCount; i++)
    freeAST(node -> children[i]);
  free(node -> children);
  if (node -> value) free(node -> value);
  free(node);
}

void printAST(ASTNode * node, int indent) {
  if (!node) return;

  // Print current node with indentation
  for (int i = 0; i < indent; i++)
    printf("|   ");

  printf("|-- %s", node -> kind);
  if (node -> value)
    printf(" (%s)", node -> value);

  printf(" [line %d, col %d]\n", node -> line, node -> column);

  // Recursively print children
  for (int i = 0; i < node -> childCount; i++) {
    printAST(node -> children[i], indent + 1);
  }
}

// Free the memory of the current token before fetching the next one
void nextToken() {
  if (lookahead) {
    free(lookahead -> tokenType);
    free(lookahead -> lexeme);
    free(lookahead);
  }

  lookahead = yylex();
  if (lookahead) {
    printf("TOKEN: %-15s Lexeme: %-10s\n", lookahead -> tokenType, lookahead -> lexeme);
  }
}

void syntax_error(const char * expected) {
  // Print error with token info
  if (lookahead) {
    fprintf(stderr, "Syntax error: expected %s but found %s (lexeme '%s') at line %d, col %d\n",
      expected, lookahead -> tokenType, lookahead -> lexeme, lookahead -> line, lookahead -> column);
  }
  // Print error when no more tokens are left
  else {
    fprintf(stderr, "Syntax error: expected %s\n", expected);
  }
  exit(1);
}

void match(const char * expectedType) {
  if (lookahead && strcmp(lookahead -> tokenType, expectedType) == 0) {
    nextToken();
  } else {
    syntax_error(expectedType);
  }
}

// Grammar rules

// prog → classOrImplOrFuncList

ASTNode * prog() {
  fprintf(derivation, "prog -> classOrImplOrFuncList\n");
  ASTNode * node = createASTNode("prog", NULL, 0, 0);

  ASTNode * children = classOrImplOrFuncList();
  if (children) addChild(node, children);

  return node;
}

/*
classOrImplOrFuncList → classOrImplOrFunc classOrImplOrFuncList
| ε
 */

ASTNode * classOrImplOrFuncList() {
  ASTNode * node = createASTNode("classOrImplOrFuncList", NULL, 0, 0);

  while (lookahead &&
    (strcmp(lookahead -> tokenType, "CLASS") == 0 ||
      strcmp(lookahead -> tokenType, "IMPLEMENT") == 0 ||
      strcmp(lookahead -> tokenType, "FUNC") == 0 ||
      strcmp(lookahead -> tokenType, "CONSTRUCTOR") == 0)) {

    ASTNode * child_classOrImplOrFunc = classOrImplOrFunc();
    if (child_classOrImplOrFunc) addChild(node, child_classOrImplOrFunc);

  }

  if (node -> childCount == 0) {
    fprintf(derivation, "classOrImplOrFuncList -> ε\n");
    free(node);
    return NULL;
  } else {
    fprintf(derivation, "classOrImplOrFuncList -> classOrImplOrFunc classOrImplOrFuncList\n");
    return node;
  }
}

/*
classOrImplOrFunc   → classDecl
                    | implDef
                    | funcDef

 */

ASTNode * classOrImplOrFunc() {
  ASTNode * node = createASTNode("classOrImplOrFunc", NULL, 0, 0);
  if (lookahead == NULL) {
    syntax_error("classOrImplOrFunc (classDecl, implDef, or funcDef)");
    return NULL;
  }

  if (strcmp(lookahead -> tokenType, "CLASS") == 0) {
    fprintf(derivation, "classOrImplOrFunc -> classDecl\n");

    ASTNode * child = classDecl();
    if (child) addChild(node, child);

  } else if (strcmp(lookahead -> tokenType, "IMPLEMENT") == 0) {
    fprintf(derivation, "classOrImplOrFunc -> implDef\n");
    ASTNode* child_implDef = implDef();
    if(child_implDef) addChild(node, child_implDef);

  } else if (strcmp(lookahead -> tokenType, "FUNC") == 0 ||
    strcmp(lookahead -> tokenType, "CONSTRUCTOR") == 0) {
    fprintf(derivation, "classOrImplOrFunc -> funcDef\n");
    ASTNode * child_funcDef = funcDef();
    if (child_funcDef) addChild(node, child_funcDef);

  } else {
    syntax_error("classOrImplOrFunc (expected class, implement, or function)");
  }
  return node;
}

// classDecl → 'class' 'id' InheritanceOpt '{' MemberList '}' ';'
ASTNode * classDecl() {

  ASTNode * parent = createASTNode("classDecl", NULL,
    lookahead -> line, lookahead -> column);

  fprintf(derivation, "classDecl -> 'class' 'id' InheritanceOpt '{' MemberList '}' ';'\n");

  match("CLASS");
  char * className = strdup(lookahead -> lexeme);
  match("IDENTIFIER");

  ASTNode * child1 = createASTNode("ClassIdentifier", className, lookahead -> line, lookahead -> column);
  if (child1) addChild(parent, child1);

  //    ASTNode* inheritance = InheritanceOpt();
  //    if (inheritance) addChild(node, inheritance);

  match("LBRACE");

  ASTNode * child2 = MemberList();
  if (child2) addChild(parent, child2);

  match("RBRACE");
  match("SEMICOLON");
  return parent;
}

// InheritanceOpt → 'isa' 'id' BaseIdTail | ε
void InheritanceOpt() {
  if (!lookahead) return;

  if (strcmp(lookahead -> tokenType, "ISA") == 0) {
    fprintf(derivation, "InheritanceOpt -> 'isa' 'id' BaseIdTail\n");
    match("ISA");
    match("IDENTIFIER");
    BaseIdTail();
  } else {
    fprintf(derivation, "InheritanceOpt -> ε\n");
    // epsilon production, do nothing
  }
}

// BaseIdTail → ',' 'id' BaseIdTail | ε
void BaseIdTail() {
  if (!lookahead) return;

  if (strcmp(lookahead -> tokenType, "COMMA") == 0) {
    fprintf(derivation, "BaseIdTail -> ',' 'id' BaseIdTail\n");
    match("COMMA");
    match("IDENTIFIER");
    BaseIdTail(); // recursive call
  } else {
    fprintf(derivation, "BaseIdTail -> ε\n");
    // epsilon production, do nothing
  }
}

// arraySize → '[' 'intLit' ']' | '[' ']'
void arraySize() {
  if (!lookahead) return;

  if (strcmp(lookahead -> tokenType, "LBRACKET") == 0) {
    match("LBRACKET");

    if (strcmp(lookahead -> tokenType, "INTEGER") == 0) {
      fprintf(derivation, "arraySize -> '[' intLit ']'\n");
      match("INTEGER");
      match("RBRACKET");
    } else {
      fprintf(derivation, "arraySize -> '[' ']'\n");
      match("RBRACKET");
    }
  } else {
    fprintf(stderr, "Syntax error in arraySize: expected '[' but found %s\n", lookahead -> tokenType);
    exit(1);
  }
}

// MemberList -> visibility memberDecl MemberList
ASTNode * MemberList() {
  if (!lookahead ||
    (strcmp(lookahead -> tokenType, "PUBLIC") != 0 &&
      strcmp(lookahead -> tokenType, "PRIVATE") != 0)) {
    fprintf(derivation, "MemberList -> ε\n");
    return NULL;
  }

  fprintf(derivation, "MemberList -> visibility memberDecl MemberList\n");

  ASTNode * parent = createASTNode("MemberList", NULL,
    lookahead -> line, lookahead -> column);

  ASTNode * vis = visibility();
  if (vis) addChild(parent, vis);

  ASTNode * mem = memberDecl();
  if (mem) addChild(parent, mem);

  ASTNode * sibling = MemberList(); // recursive call
  if (sibling) {
    // Instead of adding as child, return as sibling to caller
    ASTNode * wrapper = createASTNode("MemberListWrapper", NULL, 0, 0);
    addChild(wrapper, parent);
    addChild(wrapper, sibling);
    return wrapper;
  }

  return parent;
}

// visibility -> public | private
ASTNode * visibility() {
  ASTNode * node = createASTNode("visibility", NULL,
    lookahead -> line, lookahead -> column);
  if (lookahead && strcmp(lookahead -> tokenType, "PUBLIC") == 0) {

    match("PUBLIC");
    fprintf(derivation, "visibility -> public\n");
    node -> value = "public";

  } else if (lookahead && strcmp(lookahead -> tokenType, "PRIVATE") == 0) {
    match("PRIVATE");
    fprintf(derivation, "visibility -> private\n");
    node -> value = "private";
  } else {
    syntax_error("visibility ('public' or 'private')");
    return NULL;
  }

  return node;
}

// implDef -> 'implement' 'id' '{' FuncDefList '}'
ASTNode* implDef() {
  ASTNode * parent_implDef = createASTNode("implDef", NULL,lookahead -> line, lookahead -> column);
  fprintf(derivation, "implDef -> 'implement' 'id' '{' FuncDefList '}'\n");
  match("IMPLEMENT");
  char * className = strdup(lookahead -> lexeme);

  match("IDENTIFIER"); // the class name

  ASTNode* child_impl_class_identifier = createASTNode("ClassIdentifier",className,lookahead -> line, lookahead -> column);
  if (child_impl_class_identifier) {
    addChild(parent_implDef, child_impl_class_identifier);
  }

  match("LBRACE");
  ASTNode* child_FuncDefList = FuncDefList(); // <-- call FuncDefList, not just funcDef
  if (child_FuncDefList) {
    addChild(parent_implDef, child_FuncDefList);
  }

  match("RBRACE");

  return parent_implDef;
}

//FuncDefList         → funcDef FuncDefList | ε
ASTNode * FuncDefList() {
  ASTNode * parent_FuncDefList = createASTNode("FuncDefList", NULL, lookahead -> line, lookahead -> column);
  if (lookahead &&
    (strcmp(lookahead -> tokenType, "FUNC") == 0 ||
      strcmp(lookahead -> tokenType, "CONSTRUCTOR") == 0)) {
    fprintf(derivation, "FuncDefList -> funcDef FuncDefList\n");
    ASTNode * child_funDef = funcDef();

    if (child_funDef) {
      addChild(parent_FuncDefList, child_funDef);
    }

    ASTNode * sibling = FuncDefList(); // recursive call
    if (sibling) {
      // Instead of adding as child, return as sibling to caller
      ASTNode * wrapper = createASTNode("FuncDefListWrapper", NULL, 0, 0);
      addChild(wrapper, parent_FuncDefList);
      addChild(wrapper, sibling);
      return wrapper;
    }
      return parent_FuncDefList;

  } else {
    fprintf(derivation, "FuncDefList -> ε\n");
    return NULL;
  }
}

// memberDecl -> funcDecl | attributeDecl

ASTNode * memberDecl() {
  ASTNode * parent_createASTNode = createASTNode("memberDecl", NULL, lookahead -> line, lookahead -> column);

  if (lookahead &&
    (strcmp(lookahead -> tokenType, "FUNC") == 0 ||
      strcmp(lookahead -> tokenType, "CONSTRUCTOR") == 0)) {
    fprintf(derivation, "memberDecl -> funcDecl\n");
    ASTNode * child_funcDecl = funcDecl();
    if (child_funcDecl) {
      addChild(parent_createASTNode, child_funcDecl);
    }
  } else if (lookahead && strcmp(lookahead -> tokenType, "ATTRIBUTE") == 0) {
    fprintf(derivation, "memberDecl -> attributeDecl\n");
    ASTNode * child1 = attributeDecl();
    if (child1) addChild(parent_createASTNode, child1);
  } else {
    syntax_error("memberDecl (funcDecl or attributeDecl)");
  }

  return parent_createASTNode;
}

//funcDecl -> funcHead ;
ASTNode * funcDecl() {
  ASTNode * parent_node = createASTNode("funcDecl", NULL, lookahead -> line, lookahead -> column);
  fprintf(derivation, "funcDecl -> funcHead ;\n");
  ASTNode * child_funcHead = funcHead();
  if (child_funcHead) addChild(parent_node, child_funcHead);
  match("SEMICOLON");

  return parent_node;
}

/* attributeDecl → 'attribute' varDecl */
ASTNode * attributeDecl() {
  ASTNode * parent = createASTNode("attributeDecl", NULL, lookahead -> line, lookahead -> column);

  fprintf(derivation, "attributeDecl -> 'attribute' varDecl\n");
  match("ATTRIBUTE");
  ASTNode * child1 = createASTNode("attribute", "attribute", lookahead -> line, lookahead -> column);
  if (child1) addChild(parent, child1);
  ASTNode * child2 = varDecl();
  if (child2) addChild(parent, child2);
  return parent;
}

/* varDecl → 'id' ':' type arraySizeList ';' */
ASTNode * varDecl() {
  ASTNode * parent = createASTNode("varDecl", NULL, lookahead -> line, lookahead -> column);

  fprintf(derivation, "varDecl -> 'id' ':' type arraySizeList ';'\n");

  ASTNode * child1 = createASTNode("IDENTIFIER", lookahead -> lexeme, lookahead -> line, lookahead -> column);
  if (child1) addChild(parent, child1);

  match("IDENTIFIER");

  match("COLON");

  ASTNode * child2 = type();
  if (child2) addChild(parent, child2);

  arraySizeList();
  match("SEMICOLON");

  return parent;
}

/* funcDef → funcHead funcBody */
ASTNode * funcDef() {
  ASTNode * parent_funcDef = createASTNode("funcDef", NULL, lookahead -> line, lookahead -> column);
  fprintf(derivation, "funcDef -> funcHead funcBody\n");

  ASTNode * child_funcHead = funcHead();
  if (child_funcHead) addChild(parent_funcDef, child_funcHead);

  ASTNode * child_funcBody = funcBody();
  if (child_funcBody) addChild(parent_funcDef, child_funcBody);

  return parent_funcDef;
}

/* funcBody -> '{' varDeclOrStmtList '}' */
ASTNode * funcBody() {
  ASTNode * parent_funcBody = createASTNode("funcBody", NULL, lookahead -> line, lookahead -> column);
  fprintf(derivation, "funcBody -> '{' varDeclOrStmtList '}'\n");
  match("LBRACE");

  ASTNode * child_varDeclOrStmtList = varDeclOrStmtList();

  if (child_varDeclOrStmtList) {
    addChild(parent_funcBody, child_varDeclOrStmtList);
  }

  match("RBRACE");

  return parent_funcBody;
}

/* varDeclOrStmtList → varDeclOrStmt varDeclOrStmtList | ε */
ASTNode * varDeclOrStmtList() {
  if (lookahead &&
    (strcmp(lookahead -> tokenType, "LOCAL") == 0 ||
      strcmp(lookahead -> tokenType, "IDENTIFIER") == 0 ||
      strcmp(lookahead -> tokenType, "IF") == 0 ||
      strcmp(lookahead -> tokenType, "WHILE") == 0 ||
      strcmp(lookahead -> tokenType, "READ") == 0 ||
      strcmp(lookahead -> tokenType, "WRITE") == 0 ||
      strcmp(lookahead -> tokenType, "RETURN") == 0 ||
      strcmp(lookahead -> tokenType, "FUNC") == 0 || // function call start
      strcmp(lookahead -> tokenType, "CONSTRUCTOR") == 0 ||
      strcmp(lookahead -> tokenType, "SELF") == 0)) {
    // Create parent node for the first element
    ASTNode * parent_varDeclOrStmtList = createASTNode("varDeclOrStmtList", NULL, lookahead -> line, lookahead -> column);

    fprintf(derivation, "varDeclOrStmtList -> varDeclOrStmt varDeclOrStmtList\n");

    // Parse first varDeclOrStmt
    ASTNode * child_varDeclOrStmt = varDeclOrStmt();
    if (child_varDeclOrStmt) addChild(parent_varDeclOrStmtList, child_varDeclOrStmt);

    // Parse rest recursively
    ASTNode * rest = varDeclOrStmtList();
    if (rest) {
      // Wrap firstNode and rest as siblings
      ASTNode * wrapper = createASTNode("varDeclOrStmtListWrapper", NULL, 0, 0);
      addChild(wrapper, parent_varDeclOrStmtList);
      addChild(wrapper, rest);
      return wrapper;
    }

    return parent_varDeclOrStmtList; // only one element
  } else {
    fprintf(derivation, "varDeclOrStmtList -> ε\n");
    return NULL; // empty list
  }
}

/* varDeclOrStmt → localVarDecl | statement */
ASTNode * varDeclOrStmt() {
  ASTNode * parent_varDeclOrStmt = createASTNode("varDeclOrStmt", NULL, lookahead -> line, lookahead -> column);
  if (lookahead && strcmp(lookahead -> tokenType, "LOCAL") == 0) {
    fprintf(derivation, "varDeclOrStmt → localVarDecl\n");
    ASTNode * child_localVarDecl = localVarDecl();
    if (child_localVarDecl) {
      addChild(parent_varDeclOrStmt, child_localVarDecl);
    }
  } else {
    fprintf(derivation, "varDeclOrStmt → statement\n");
    ASTNode * child_statement = statement();
    if (child_statement) addChild(parent_varDeclOrStmt, child_statement);
  }

  return parent_varDeclOrStmt;
}

/* localVarDecl → 'local' varDecl */
ASTNode * localVarDecl() {
  ASTNode * parent_localVarDecl = createASTNode("localVarDecl", NULL, lookahead -> line, lookahead -> column);

  fprintf(derivation, "localVarDecl -> 'local' varDecl\n");
  match("LOCAL");

  // this is always local so hardcoded is done
  parent_localVarDecl -> value = "local";

  ASTNode * child_varDecl = varDecl();
  if (child_varDecl) {
    addChild(parent_localVarDecl, child_varDecl);
  }

  return parent_localVarDecl;
}

// arraySizeList → arraySize arraySizeList | ε
void arraySizeList() {
  if (!lookahead) return;

  if (strcmp(lookahead -> tokenType, "LBRACKET") == 0) {
    fprintf(derivation, "arraySizeList -> arraySize arraySizeList\n");
    arraySize();
    arraySizeList();
  } else {
    fprintf(derivation, "arraySizeList -> ε\n");
  }
}

// type -> integer | float | id
ASTNode * type() {
  ASTNode * parent = createASTNode("type", NULL, lookahead -> line, lookahead -> column);
  if (lookahead) {
    if (strcmp(lookahead -> tokenType, "INTEGER_TYPE") == 0) {
      fprintf(derivation, "type -> integer\n");
      match("INTEGER_TYPE");

      parent -> value = "integer";

    } else if (strcmp(lookahead -> tokenType, "FLOAT_TYPE") == 0) {
      fprintf(derivation, "type -> float\n");
      match("FLOAT_TYPE");

      parent -> value = "float";

    } else if (strcmp(lookahead -> tokenType, "IDENTIFIER") == 0) {
      fprintf(derivation, "type -> id\n");
      match("IDENTIFIER");

      parent -> value = "indentifier";

    } else {
      syntax_error("type (integer, float, or id)");
    }
  } else {
    syntax_error("type (integer, float, or id)");
  }
  return parent;
}

/* funcHead → 'func' 'id' '(' fParams ')' => returnType | 'constructor' '(' fParams ') */
ASTNode * funcHead() {
  ASTNode * parent_funcHead = createASTNode("funcHead", NULL, lookahead -> line, lookahead -> column);

  if (lookahead && strcmp(lookahead -> tokenType, "FUNC") == 0) {
    fprintf(derivation, "funcHead -> 'func' 'id' '(' fParams ')' '=>' returnType\n");
    match("FUNC");
    char * functionName = strdup(lookahead -> lexeme);
    match("IDENTIFIER");

    ASTNode * child_funcIdentifier = createASTNode("funcIdentifier", functionName, lookahead -> line, lookahead -> column);
    if (child_funcIdentifier) {
      addChild(parent_funcHead, child_funcIdentifier);
    }

    match("LPAREN");

    ASTNode * child_fParams = fParams();

    if(child_fParams) addChild(parent_funcHead, child_fParams);

    match("RPAREN");
    match("ARROW");
    ASTNode * child_returnType = returnType();
    if (child_returnType) {
      addChild(parent_funcHead, child_returnType);
    }
  } else if (lookahead && strcmp(lookahead -> tokenType, "CONSTRUCTOR") == 0) {
    fprintf(derivation, "funcHead -> 'constructor' '(' fParams ')'\n");

    match("CONSTRUCTOR");
    match("LPAREN");

    ASTNode* child_fParams = fParams();
    if (child_fParams) addChild(parent_funcHead, child_fParams);
    match("RPAREN");
  } else {
    syntax_error("funcHead (func|constructor)");
  }
  return parent_funcHead;
}

/* fParams → 'id' ':' type arraySizeList fParamsTailList | ε */
ASTNode * fParams() {
  ASTNode * parent_fParams = createASTNode("fParams", NULL, lookahead -> line, lookahead -> column);
  if (lookahead && strcmp(lookahead -> tokenType, "IDENTIFIER") == 0) {
    fprintf(derivation, "fParams -> 'id' ':' type arraySizeList fParamsTailList\n");

    char * Indentifier = strdup(lookahead -> lexeme);
    ASTNode * child_identifier = createASTNode("IDENTIFIER", Indentifier, lookahead -> line, lookahead -> column);

    if (child_identifier) {
      addChild(parent_fParams, child_identifier);
    }

    match("IDENTIFIER");
    match("COLON");

    ASTNode * child_type = type();

    if (child_type) {
      addChild(parent_fParams, child_type);
    }

    arraySizeList();

    ASTNode* child_fParamsTailList = fParamsTailList();
    if(child_fParamsTailList) {
      addChild(parent_fParams, child_fParamsTailList);
    }

  } else {
    fprintf(derivation, "fParams -> ε\n");
  }

  return parent_fParams;
}

/* fParamsTailList → fParamsTail fParamsTailList | ε */
ASTNode* fParamsTailList() {
  ASTNode* parent_fParamsTailList = createASTNode("fParamsTailList", NULL, lookahead -> line, lookahead -> column);

  if (lookahead && strcmp(lookahead -> tokenType, "COMMA") == 0) {
    fprintf(derivation, "fParamsTailList -> fParamsTail fParamsTailList\n");
    ASTNode* child_fParamsTail = fParamsTail();

    if (child_fParamsTail) {
      addChild(parent_fParamsTailList, child_fParamsTail);
    }

    // Parse rest recursively
    ASTNode * rest = fParamsTailList();;
    if (rest) {
      // Wrap firstNode and rest as siblings
      ASTNode * wrapper = createASTNode("fParamsTailListWrapper", NULL, 0, 0);
      addChild(wrapper, parent_fParamsTailList);
      addChild(wrapper, rest);
      return wrapper;
    }

    return parent_fParamsTailList; // only one element


  } else {
    fprintf(derivation, "fParamsTailList -> ε\n");
    return NULL;
  }

}

/* fParamsTail → ',' 'id' ':' type arraySizeList */
ASTNode* fParamsTail() {
  ASTNode* parent_fParamsTail = createASTNode("fParamsTail", NULL, lookahead -> line, lookahead -> column);
  fprintf(derivation, "fParamsTail -> ',' 'id' ':' type arraySizeList\n");
  match("COMMA");
  char * Indentifier = strdup(lookahead -> lexeme);
  ASTNode * child_identifier = createASTNode("IDENTIFIER", Indentifier, lookahead -> line, lookahead -> column);

  if (child_identifier) {
    addChild(parent_fParamsTail, child_identifier);
  }


  match("IDENTIFIER");
  match("COLON");


  ASTNode* child_type = type();
  if (child_type) {
    addChild(parent_fParamsTail, child_type);
  }

  arraySizeList();

  return parent_fParamsTail;
}

// returnType -> type | void
ASTNode * returnType() {
  ASTNode * parent = createASTNode("returnType", NULL, lookahead -> line, lookahead -> column);

  if (lookahead &&
    (strcmp(lookahead -> tokenType, "INTEGER_TYPE") == 0 ||
      strcmp(lookahead -> tokenType, "FLOAT_TYPE") == 0 ||
      strcmp(lookahead -> tokenType, "VARIABLE") == 0)) {
    fprintf(derivation, "returnType -> type\n");
    ASTNode * child_type = type();

    // can grab return type
    char * return_type = child_type -> value;
    parent -> value = return_type;

  } else if (lookahead && strcmp(lookahead -> tokenType, "VOID") == 0) {
    fprintf(derivation, "returnType -> void\n");
    parent -> value = "void";
    match("VOID");
  } else {
    syntax_error("returnType (type|void)");
  }
  return parent;
}

/*
statement ->
      'read' '(' variable ')' ';'
    | 'write' '(' expr ')' ';'
    | 'return' '(' expr ')' ';'
    | 'if' '(' relExpr ')' 'then' statBlock 'else' statBlock ';'
    | 'while' '(' relExpr ')' statBlock ';'
    | idOrSelfStatement ';'
*/

ASTNode * statement() {
  if (!lookahead) return NULL;
  ASTNode * parent_statement = createASTNode("statement", NULL, lookahead -> line, lookahead -> column);

  if (strcmp(lookahead -> tokenType, "READ") == 0) {
    fprintf(derivation, "statement -> 'read' '(' variable ')' ';'\n");
    match("READ");
    match("LPAREN");
    variable();
    match("RPAREN");
    match("SEMICOLON");
  } else if (strcmp(lookahead -> tokenType, "WRITE") == 0) {
    fprintf(derivation, "statement -> 'write' '(' expr ')' ';'\n");
    match("WRITE");
    match("LPAREN");
    expr();
    match("RPAREN");
    match("SEMICOLON");
  } else if (strcmp(lookahead -> tokenType, "RETURN") == 0) {
    // wrap around return

    ASTNode * child_return = createASTNode("return", NULL, lookahead -> line, lookahead -> column);

    if (child_return) {
      addChild(parent_statement, child_return);
    }

    fprintf(derivation, "statement -> 'return' '(' expr ')' ';'\n");

    match("RETURN");
    match("LPAREN");
    ASTNode * child_expr = expr();

    if (child_return) {
      addChild(child_return, child_expr);
    }

    match("RPAREN");
    match("SEMICOLON");
  } else if (strcmp(lookahead -> tokenType, "IF") == 0) {
    fprintf(derivation, "statement -> 'if' '(' relExpr ')' 'then' statBlock 'else' statBlock ';'\n");
    match("IF");
    match("LPAREN");
    relExpr();
    match("RPAREN");
    match("THEN");
    statBlock(); // statBlock
    match("ELSE");
    statBlock(); // statBlock
    match("SEMICOLON");
  } else if (strcmp(lookahead -> tokenType, "WHILE") == 0) {
    fprintf(derivation, "statement -> 'while' '(' relExpr ')' statBlock ';'\n");
    match("WHILE");
    match("LPAREN");
    relExpr();
    match("RPAREN");
    statBlock();
    match("SEMICOLON");
  } else if (strcmp(lookahead -> tokenType, "IDENTIFIER") == 0 || strcmp(lookahead -> tokenType, "SELF") == 0) {
    // Start of assignStat or functionCall
    fprintf(derivation, "statement -> idOrSelfStatement ;\n");

    ASTNode * child_statement = idOrSelfStatement();
    if (child_statement) {
      addChild(parent_statement, child_statement);
    }

    match("SEMICOLON");
  } else {
    fprintf(stderr, "Syntax error in statement: unexpected token %s\n", lookahead -> tokenType);
    exit(1);
  }

  return parent_statement;
}

//idOrSelfStatement   → idOrSelf idOrSelfTailWithAssignOrCall
ASTNode * idOrSelfStatement() {
  ASTNode * parent_idOrSelfStatement = createASTNode("idOrSelfStatement", NULL, lookahead -> line, lookahead -> column);

  fprintf(derivation, "idOrSelfStatement   → idOrSelf idOrSelfTailWithAssignOrCall\n");

  ASTNode * child_idOrSelf = idOrSelf();
  if (child_idOrSelf) {
    addChild(parent_idOrSelfStatement, child_idOrSelf);
  }

  ASTNode * child_idOrSelfTailWithAssignOrCall = idOrSelfTailWithAssignOrCall();
  if (child_idOrSelfTailWithAssignOrCall) {
    addChild(parent_idOrSelfStatement, child_idOrSelfTailWithAssignOrCall);
  }

  return parent_idOrSelfStatement;
}

/*
idOrSelfTailWithAssignOrCall → assignOp expr
                             | '(' aParams ')' idNestTail
                             | indiceList idNestTail
 */

ASTNode * idOrSelfTailWithAssignOrCall() {
  ASTNode * parent_idOrSelfTailWithAssignOrCall = createASTNode("idOrSelfTailWithAssignOrCall", NULL, lookahead -> line, lookahead -> column);
  if (strcmp(lookahead -> tokenType, "ASSIGN") == 0) {
    fprintf(derivation, "idOrSelfTailWithAssignOrCall -> assignOp expr\n");
    ASTNode * child_assignOp = assignOp();
    if (child_assignOp) {
      addChild(parent_idOrSelfTailWithAssignOrCall, child_assignOp);
    }

    ASTNode * child_expr = expr();
    if (child_expr) {
      addChild(parent_idOrSelfTailWithAssignOrCall, child_expr);
    }
  } else if (strcmp(lookahead -> tokenType, "LPAREN") == 0) {
    // function call
    fprintf(derivation, "idOrSelfTailWithAssignOrCall -> '(' aParams ')' idNestTail\n");
    match("LPAREN");
    aParams();
    match("RPAREN");
    idNestTail();
  } else if (strcmp(lookahead -> tokenType, "LBRACKET") == 0 ||
    strcmp(lookahead -> tokenType, "DOT") == 0) {
    fprintf(derivation, "idOrSelfTailWithAssignOrCall -> (indiceList idNestTail | assignOp expr)\n");

    // parse indices first
    indiceList();
    idNestTail();

    // after indiceList, if assignment appears, parse assignment
    if (lookahead && strcmp(lookahead -> tokenType, "ASSIGN") == 0) {
      assignOp();
      expr();
    }
  }

  return parent_idOrSelfTailWithAssignOrCall;
}

// statementTail → '(' aParams ')' idNestTail ';' | assignOp expr ';'
void statementTail() {}

// assignStat → variable assignOp expr
void assignStat() {
  fprintf(derivation, "assignStat -> variable assignOp expr\n");
  variable();
  assignOp();
  expr();
}

// variable -> idOrSelf idOrSelfTail
void variable() {
  fprintf(derivation, "variable -> idOrSelf idOrSelfTail\n");
  idOrSelf();
  idOrSelfTail();
}

// idOrSelf → 'id' | 'self'
ASTNode * idOrSelf() {
  ASTNode * parent_idOrSelf = createASTNode("idOrSelf", NULL, lookahead -> line, lookahead -> column);
  if (lookahead && strcmp(lookahead -> tokenType, "IDENTIFIER") == 0) {
    fprintf(derivation, "idOrSelf -> id\n");
    parent_idOrSelf -> value = strdup(lookahead -> lexeme);
    match("IDENTIFIER");
  } else if (lookahead && strcmp(lookahead -> tokenType, "SELF") == 0) {
    fprintf(derivation, "idOrSelf -> self\n");
    parent_idOrSelf -> value = "self";
    match("SELF");
  } else {
    syntax_error("idOrSelf ('id' or 'self')");
  }

  return parent_idOrSelf;
}

// indiceList → indice indiceList | ε
void indiceList() {
  if (lookahead && strcmp(lookahead -> tokenType, "LBRACKET") == 0) {
    fprintf(derivation, "indiceList -> indice indiceList\n");
    indice();
    indiceList();
  } else {
    fprintf(derivation, "indiceList -> ε\n");
  }
}

// indice → '[' arithExpr ']'
void indice() {
  fprintf(derivation, "indice -> '[' arithExpr ']'\n");
  match("LBRACKET");
  arithExpr();
  match("RBRACKET");
}

// expr → arithExpr | relExpr
ASTNode * expr() {
  ASTNode * parent_expr = createASTNode("expr", NULL, lookahead -> line, lookahead -> column);
  fprintf(derivation, "expr -> arithExpr\n");
  ASTNode * child_parent = arithExpr();
  if (child_parent) {
    addChild(parent_expr, child_parent);
  }

  // After arithExpr, check if it extends into a relExpr
  if (lookahead &&
    (strcmp(lookahead -> tokenType, "EQ") == 0 ||
      strcmp(lookahead -> tokenType, "NEQ") == 0 ||
      strcmp(lookahead -> tokenType, "LT") == 0 ||
      strcmp(lookahead -> tokenType, "LEQ") == 0 ||
      strcmp(lookahead -> tokenType, "GT") == 0 ||
      strcmp(lookahead -> tokenType, "GEQ") == 0)) {
    fprintf(derivation, "expr -> relExpr\n");
    relOp();
    arithExpr();
  }

  return parent_expr;
}

// relExpr → arithExpr relOp arithExpr
void relExpr() {
  fprintf(derivation, "relExpr -> arithExpr relOp arithExpr\n");
  arithExpr();
  relOp();
  arithExpr();
}

// arithExpr → term arithExpr_
ASTNode * arithExpr() {
  ASTNode * parent_arithExpr = createASTNode("arithExpr", NULL, lookahead -> line, lookahead -> column);
  fprintf(derivation, "arithExpr -> term arithExpr_\n");
  ASTNode * child_arithExpr = term();
  if (child_arithExpr) {
    addChild(parent_arithExpr, child_arithExpr);
  }

  ASTNode * child_arithExpr_ = arithExpr_();
  if (child_arithExpr_) {
    addChild(parent_arithExpr, child_arithExpr_);
  }

  return parent_arithExpr;
}

//arithExpr_-> addOp term arithExpr_
ASTNode * arithExpr_() {
  ASTNode * parent_arithExpr_ = createASTNode("arithExpr_", NULL, lookahead -> line, lookahead -> column);
  if (lookahead &&
    (strcmp(lookahead -> tokenType, "PLUS") == 0 ||
      strcmp(lookahead -> tokenType, "MINUS") == 0 ||
      strcmp(lookahead -> tokenType, "OR") == 0)) {
    fprintf(derivation, "arithExpr_ -> addOp term arithExpr_\n");

    ASTNode * child_addOp = addOp();
    if (child_addOp) {
      addChild(parent_arithExpr_, child_addOp);
    }

    ASTNode * child_term = term();
    if (child_term) {
      addChild(parent_arithExpr_, child_term);
    }

    ASTNode * sibling = arithExpr_(); // recursive call
    if (sibling) {
      // Instead of adding as child, return as sibling to caller
      ASTNode * wrapper = createASTNode("arithExpr_Wrapper", NULL, 0, 0);
      addChild(wrapper, parent_arithExpr_);
      addChild(wrapper, sibling);
      return wrapper;
    }
    return parent_arithExpr_;
  } else {
    fprintf(derivation, "arithExpr_ -> ε\n"); // no further addition/subtraction
    return NULL;
  }

}

//aaddOp -> '+' | '-' | 'or'

ASTNode * addOp() {
  ASTNode * parent_addOp = createASTNode("addOp", NULL, lookahead -> line, lookahead -> column);

  if (lookahead && strcmp(lookahead -> tokenType, "PLUS") == 0) {
    fprintf(derivation, "addOp -> '+'\n");
    parent_addOp -> value = "+";
    match("PLUS");
  } else if (lookahead && strcmp(lookahead -> tokenType, "MINUS") == 0) {
    parent_addOp -> value = "-";

    fprintf(derivation, "addOp -> '-'\n");
    match("MINUS");
  } else if (lookahead && strcmp(lookahead -> tokenType, "OR") == 0) {
    parent_addOp -> value = "or";
    fprintf(derivation, "addOp -> 'or'\n");
    match("OR");
  } else {
    syntax_error("addOp ('+', '-', or 'or')");
  }

  return parent_addOp;
}

// term -> factor term_
ASTNode * term() {
  ASTNode * parent_term = createASTNode("term", NULL, lookahead -> line, lookahead -> column);
  fprintf(derivation, "term -> factor term_\n");
  ASTNode * child_factor = factor();
  if (child_factor) {
    addChild(parent_term, child_factor);
  }

  term_();

  return parent_term;
}

// factor -> idOrSelf idOrSelfTail | 'intLit' | 'floatLit' | '(' arithExpr ')' | 'not' factor | sign factor
ASTNode * factor() {
  ASTNode * parent_factor = createASTNode("factor", NULL, lookahead -> line, lookahead -> column);

  if (lookahead && (strcmp(lookahead -> tokenType, "IDENTIFIER") == 0 || strcmp(lookahead -> tokenType, "SELF") == 0)) {
    fprintf(derivation, "factor -> idOrSelf idOrSelfTail\n");
    ASTNode * child_idOrSelf = idOrSelf();
    if (child_idOrSelf) {
      addChild(parent_factor, child_idOrSelf);
    }

    idOrSelfTail();

  } else if (lookahead && strcmp(lookahead -> tokenType, "INTEGER") == 0) {
    fprintf(derivation, "factor -> 'intLit'\n");
    parent_factor -> value = strdup(lookahead -> lexeme);
    match("INTEGER");
  } else if (lookahead && strcmp(lookahead -> tokenType, "FLOAT") == 0) {
    fprintf(derivation, "factor -> 'floatLit'\n");
    parent_factor -> value = strdup(lookahead -> lexeme);
    match("FLOAT");
  } else if (lookahead && strcmp(lookahead -> tokenType, "LPAREN") == 0) {
    fprintf(derivation, "factor -> '(' arithExpr ')'\n");
    match("LPAREN");
    arithExpr();
    match("RPAREN");
  } else if (lookahead && strcmp(lookahead -> tokenType, "NOT") == 0) {
    fprintf(derivation, "factor -> 'not' factor\n");
    match("NOT");
    factor();
  } else if (lookahead && (strcmp(lookahead -> tokenType, "PLUS") == 0 || strcmp(lookahead -> tokenType, "MINUS") == 0)) {
    fprintf(derivation, "factor -> sign factor\n");
    match(lookahead -> tokenType);
    factor();
  } else {
    syntax_error("factor (idOrSelf, intLit, floatLit, or '(' arithExpr ')')");
  }

  return parent_factor;
}

// idOrSelfTail -> '(' aParams ')' idNestTail | indiceList idNestTail | ε
void idOrSelfTail() {
  if (lookahead && strcmp(lookahead -> tokenType, "LPAREN") == 0) {
    fprintf(derivation, "idOrSelfTail -> '(' aParams ')' idNestTail\n");
    match("LPAREN");
    aParams();
    match("RPAREN");
    idNestTail();
  } else if (lookahead && strcmp(lookahead -> tokenType, "LBRACKET") == 0 ||
    lookahead && strcmp(lookahead -> tokenType, "DOT") == 0
  ) {
    fprintf(derivation, "idOrSelfTail -> indiceList idNestTail\n");
    indiceList();
    idNestTail();
  } else {
    fprintf(derivation, "idOrSelfTail -> ε\n");
  }
}

// idNestTail -> '.' idOrSelf idOrSelfTail | ε
void idNestTail() {
  if (lookahead && strcmp(lookahead -> tokenType, "DOT") == 0) {
    fprintf(derivation, "idNestTail -> '.' idOrSelf idOrSelfTail\n");
    match("DOT");
    idOrSelf();
    idOrSelfTail();
  } else {
    fprintf(derivation, "idNestTail -> ε\n");
  }
}

// functionCall -> idOrSelf '(' aParams ')' idNestTail
void functionCall() {
  fprintf(derivation, "functionCall -> idOrSelf '(' aParams ')' idNestTail\n");
  idOrSelf();
  match("LPAREN");
  aParams();
  match("RPAREN");
  idNestTail();
}

// aParams -> expr aParamsTailList | ε
void aParams() {
  if (lookahead &&
    (strcmp(lookahead -> tokenType, "IDENTIFIER") == 0 ||
      strcmp(lookahead -> tokenType, "SELF") == 0 ||
      strcmp(lookahead -> tokenType, "INTEGER") == 0 ||
      strcmp(lookahead -> tokenType, "FLOAT") == 0 ||
      strcmp(lookahead -> tokenType, "LPAREN") == 0 ||
      strcmp(lookahead -> tokenType, "PLUS") == 0 ||
      strcmp(lookahead -> tokenType, "MINUS") == 0 ||
      strcmp(lookahead -> tokenType, "NOT") == 0)) {
    fprintf(derivation, "aParams -> expr aParamsTailList\n");
    expr();
    aParamsTailList();
  } else {
    fprintf(derivation, "aParams -> ε\n");
  }
}

// aParamsTailList -> ',' expr aParamsTailList | ε
void aParamsTailList() {
  if (lookahead && strcmp(lookahead -> tokenType, "COMMA") == 0) {
    fprintf(derivation, "aParamsTailList -> ',' expr aParamsTailList\n");
    match("COMMA");
    expr();
    aParamsTailList();
  } else {
    fprintf(derivation, "aParamsTailList -> ε\n");
  }
}

//assignOp -> :=
ASTNode * assignOp() {
  ASTNode * parent_assignOp = createASTNode("assignOp", NULL, lookahead -> line, lookahead -> column);
  fprintf(derivation, "assignOp -> := \n");
  parent_assignOp -> value = ":=";
  match("ASSIGN");

  return parent_assignOp;
}

// term_ → multOp factor term_ | ε
void term_() {
  if (lookahead &&
    (strcmp(lookahead -> tokenType, "TIMES") == 0 ||
      strcmp(lookahead -> tokenType, "DIVIDE") == 0 ||
      strcmp(lookahead -> tokenType, "AND") == 0)) {
    fprintf(derivation, "term_ -> multOp factor term_\n");
    multOp();
    factor();
    term_();
  } else {
    fprintf(derivation, "term_ -> ε\n"); // no further multiplication/division/and
  }
}

/* relOp → '==' | '<>' | '<' | '>' | '<=' | '>=' */
void relOp() {
  fprintf(derivation, "relOp -> ");

  if (lookahead == NULL) {
    syntax_error("relOp (unexpected end of input)");
    return;
  }

  if (strcmp(lookahead -> tokenType, "EQ") == 0) {
    // '=='
    fprintf(derivation, "==\n");
    match("EQ");
  } else if (strcmp(lookahead -> tokenType, "NEQ") == 0) {
    // '<>'
    fprintf(derivation, "<>\n");
    match("NEQ");
  } else if (strcmp(lookahead -> tokenType, "LT") == 0) {
    // '<'
    fprintf(derivation, "<\n");
    match("LT");
  } else if (strcmp(lookahead -> tokenType, "GT") == 0) {
    // '>'
    fprintf(derivation, ">\n");
    match("GT");
  } else if (strcmp(lookahead -> tokenType, "LE") == 0) {
    // '<='
    fprintf(derivation, "<=\n");
    match("LE");
  } else if (strcmp(lookahead -> tokenType, "GE") == 0) {
    // '>='
    fprintf(derivation, ">=\n");
    match("GE");
  } else {
    syntax_error("relOp (expected relational operator)");
  }
}

/* multOp → '*' | '/' | 'and' */
void multOp() {
  fprintf(derivation, "multOp -> ");

  if (lookahead == NULL) {
    syntax_error("multOp (unexpected end of input)");
    return;
  }

  if (strcmp(lookahead -> tokenType, "TIMES") == 0) {
    // '*'
    fprintf(derivation, "*\n");
    match("TIMES");
  } else if (strcmp(lookahead -> tokenType, "DIVIDE") == 0) {
    // '/'
    fprintf(derivation, "/\n");
    match("DIVIDE");
  } else if (strcmp(lookahead -> tokenType, "AND") == 0) {
    // 'and'
    fprintf(derivation, "and\n");
    match("AND");
  } else {
    syntax_error("multOp (expected *, /, or 'and')");
  }
}

/* statementList -> statement statementList | ε */
void statementList() {
  if (lookahead &&
    (strcmp(lookahead -> tokenType, "IF") == 0 ||
      strcmp(lookahead -> tokenType, "WHILE") == 0 ||
      strcmp(lookahead -> tokenType, "READ") == 0 ||
      strcmp(lookahead -> tokenType, "WRITE") == 0 ||
      strcmp(lookahead -> tokenType, "RETURN") == 0 ||
      strcmp(lookahead -> tokenType, "IDENTIFIER") == 0 ||
      strcmp(lookahead -> tokenType, "SELF") == 0)) {
    fprintf(derivation, "statementList -> statement statementList\n");
    statement();
    statementList();
  } else {
    fprintf(derivation, "statementList -> ε\n");
  }
}

void statBlock() {
  if (lookahead && strcmp(lookahead -> tokenType, "LBRACE") == 0) {
    /* block form: { statementList } */
    fprintf(derivation, "statBlock -> '{' statementList '}'\n");
    match("LBRACE");
    statementList();
    match("RBRACE");
  } else if (lookahead &&
    (strcmp(lookahead -> tokenType, "IF") == 0 ||
      strcmp(lookahead -> tokenType, "WHILE") == 0 ||
      strcmp(lookahead -> tokenType, "READ") == 0 ||
      strcmp(lookahead -> tokenType, "WRITE") == 0 ||
      strcmp(lookahead -> tokenType, "RETURN") == 0 ||
      strcmp(lookahead -> tokenType, "IDENTIFIER") == 0 ||
      strcmp(lookahead -> tokenType, "SELF") == 0)) {
    fprintf(derivation, "statBlock -> statement\n");
    statement();
  } else {
    fprintf(derivation, "statBlock -> ε\n");
  }
}
int main() {
  // Open derivation file
  derivation = fopen("derivation.txt", "w");
  if (!derivation) {
    perror("derivation.txt");
    return 1;
  }

  nextToken(); // read first token
  ASTNode * root = prog();

  // Check for extra tokens
  if (lookahead != NULL) {
    fprintf(stderr, "Syntax error: extra tokens at end starting at '%s' (line %d, col %d)\n",
      lookahead -> lexeme, lookahead -> line, lookahead -> column);
    fclose(derivation);
    return 1;
  }

  printf("Parsing successful! Derivation written to derivation.txt\n");
  printf("\nAbstract Syntax Tree:\n");
  printAST(root, 0);

  // Create global table and build nested symbol tables
  SymbolTable * globalTable = createSymbolTable("GLOBAL");
  buildSymbolTable(root, globalTable, "GLOBAL", NULL);
  assignOffsets(globalTable);


  printSymbolTable(globalTable);

  // Open semantic error log file
  FILE * semanticErrorLog = fopen("semantic_errors.txt", "w");
  if (!semanticErrorLog) {
    perror("semantic_errors.txt");
    freeSymbolTable(globalTable);
    fclose(derivation);
    return 1;
  }

  // Semantic analysis with type checking and error reporting
  analyzeSemantics(root, globalTable);

  printSemanticErrors();

  fclose(semanticErrorLog);
  freeSymbolTable(globalTable);
  fclose(derivation);

  return 0;
}