#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/token.h"

Token* create_token(TokenType type, const char *value) {
    Token *token = malloc(sizeof(Token));
    if (token == NULL) {
        perror("malloc failed");
        return NULL;
    }

    token->type = type;
    if (value != NULL) {
        token->value = malloc(strlen(value) + 1);
        if (token->value == NULL) {
            perror("malloc failed");
            free(token);
            return NULL;
        }
        strcpy(token->value, value);
    } else {
        token->value = NULL;
    }

    return token;
}

void free_token(Token *token) {
    if (token != NULL) {
        if (token->value != NULL) {
            free(token->value);
        }
        free(token);
    }
}

void print_token(Token *token) {
    if (token == NULL) return;

    const char *type_names[] = {
        "WORD", "PIPE", "REDIRECT_OUT", "REDIRECT_IN",
        "REDIRECT_APPEND", "REDIRECT_ERR", "AMPERSAND", "SEMICOLON",
        "AND", "OR", "LPAREN", "RPAREN", "EOF"
    };

    printf("Token(%s, \"%s\")\n", type_names[token->type], 
           token->value ? token->value : "");
}
