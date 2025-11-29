%{
#include "token_n.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define YY_DECL Token_n* nnlex(void)

int line_num_n = 1;
int col_num_n = 1;
int token_start_col_n;

Token_n* create_token_n(const char* tokenType, const char* yytext, int length) {
    token_start_col_n = col_num_n;
    col_num_n += length;

    Token_n* t = malloc(sizeof(Token_n));
    t->tokenType = strdup(tokenType);
    t->lexeme = strdup(yytext);
    t->line = line_num_n;
    t->column = token_start_col_n;
    return t;
}

void update_position_n(const char* text, int length) {
    for (int i = 0; i < length; i++) {
        if (text[i] == '\n') {
            line_num_n++;
            col_num_n = 1;
        } else {
            col_num_n++;
        }
    }
}
%}

%option noyywrap
%option yylineno
%option prefix="n"

letter     [a-zA-Z]
digit      [0-9]
nonzero    [1-9]
alphanum   ({letter}|{digit}|_)
id         {letter}({alphanum})*

integerNum 0|{nonzero}{digit}*
fraction   \.0|\.{digit}*{nonzero}
exp        [eE][+-]?{integerNum}
floatNum   {integerNum}{fraction}({exp})?

%%

{floatNum}      { return create_token_n("FLOAT", yytext, yyleng); }
{integerNum}    { return create_token_n("INTEGER", yytext, yyleng); }

"("             { return create_token_n("LPAREN", yytext, yyleng); }
")"             { return create_token_n("RPAREN", yytext, yyleng); }
";"             { return create_token_n("SEMICOLON", yytext, yyleng); }
","             { return create_token_n("COMMA", yytext, yyleng); }
"{"             { return create_token_n("LBRACE", yytext, yyleng); }
"}"             { return create_token_n("RBRACE", yytext, yyleng); }
"["             { return create_token_n("LBRACKET", yytext, yyleng); }
"]"             { return create_token_n("RBRACKET", yytext, yyleng); }
"."             { return create_token_n("DOT", yytext, yyleng); }
"=>"            { return create_token_n("ARROW", yytext, yyleng); }
":="            { return create_token_n("ASSIGN", yytext, yyleng); }
":"             { return create_token_n("COLON", yytext, yyleng); }

"+"             { return create_token_n("PLUS", yytext, yyleng); }
"-"             { return create_token_n("MINUS", yytext, yyleng); }
"*"             { return create_token_n("TIMES", yytext, yyleng); }
"/"             { return create_token_n("DIVIDE", yytext, yyleng); }

"=="            { return create_token_n("EQ", yytext, yyleng); }
"<>"            { return create_token_n("NEQ", yytext, yyleng); }
"<="            { return create_token_n("LE", yytext, yyleng); }
">="            { return create_token_n("GE", yytext, yyleng); }
"<"             { return create_token_n("LT", yytext, yyleng); }
">"             { return create_token_n("GT", yytext, yyleng); }

"or"            { return create_token_n("OR", yytext, yyleng); }
"and"           { return create_token_n("AND", yytext, yyleng); }
"not"           { return create_token_n("NOT", yytext, yyleng); }

"else"          { return create_token_n("ELSE", yytext, yyleng); }
"float"         { return create_token_n("FLOAT_TYPE", yytext, yyleng); }
"func"          { return create_token_n("FUNC", yytext, yyleng); }
"if"            { return create_token_n("IF", yytext, yyleng); }
"implement"     { return create_token_n("IMPLEMENT", yytext, yyleng); }
"class"         { return create_token_n("CLASS", yytext, yyleng); }
"attribute"     { return create_token_n("ATTRIBUTE", yytext, yyleng); }
"isa"           { return create_token_n("ISA", yytext, yyleng); }
"private"       { return create_token_n("PRIVATE", yytext, yyleng); }
"public"        { return create_token_n("PUBLIC", yytext, yyleng); }
"read"          { return create_token_n("READ", yytext, yyleng); }
"return"        { return create_token_n("RETURN", yytext, yyleng); }
"then"          { return create_token_n("THEN", yytext, yyleng); }
"local"         { return create_token_n("LOCAL", yytext, yyleng); }
"void"          { return create_token_n("VOID", yytext, yyleng); }
"while"         { return create_token_n("WHILE", yytext, yyleng); }
"write"         { return create_token_n("WRITE", yytext, yyleng); }
"integer"       { return create_token_n("INTEGER_TYPE", yytext, yyleng); }
"self"          { return create_token_n("SELF", yytext, yyleng); }
"constructor"   { return create_token_n("CONSTRUCTOR", yytext, yyleng); }

{id}            { return create_token_n("IDENTIFIER", yytext, yyleng); }

[ \t]+          { col_num_n += yyleng; }  /* skip whitespace */
\n              { line_num_n++; col_num_n = 1; }

\/\/[^\n]*      { /* comment ignored */ }
\/\*([^*]|\*+[^*/])*\*+\/ {update_position_n(yytext, yyleng);}

.               { return create_token_n("ILLEGAL", yytext, yyleng); }

%%

