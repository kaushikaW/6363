#ifndef TOKEN_H
#define TOKEN_H

typedef struct Token {
    char* tokenType;
    char* lexeme;
    int line;
    int column;
} Token;

Token* create_token(const char* tokenType, const char* yytext, int length);

#endif
