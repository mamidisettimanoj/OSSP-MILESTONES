#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIRECT_OUT,      // >
    TOKEN_REDIRECT_IN,       // <
    TOKEN_REDIRECT_APPEND,   // >>
    TOKEN_REDIRECT_ERR,      // 2>
    TOKEN_AMPERSAND,         // &
    TOKEN_SEMICOLON,         // ;
    TOKEN_AND,               // &&
    TOKEN_OR,                // ||
    TOKEN_LPAREN,            // (
    TOKEN_RPAREN,            // )
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char *value;
} Token;

Token* create_token(TokenType type, const char *value);
void free_token(Token *token);
void print_token(Token *token);

#endif
