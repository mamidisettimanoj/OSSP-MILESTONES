#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/lexer.h"

static int is_operator_char(char c) {
    return c == '|' || c == '>' || c == '<' || c == '&' || 
           c == ';' || c == '(' || c == ')';
}

static void tokenstream_add(TokenStream *stream, Token *token) {
    if (stream->count >= stream->capacity) {
        stream->capacity *= 2;
        stream->tokens = realloc(stream->tokens, 
                                 stream->capacity * sizeof(Token*));
    }
    stream->tokens[stream->count++] = token;
}

TokenStream* lexer_tokenize(const char *input) {
    if (input == NULL) return NULL;

    TokenStream *stream = malloc(sizeof(TokenStream));
    stream->tokens = malloc(64 * sizeof(Token*));
    stream->capacity = 64;
    stream->count = 0;

    int i = 0;
    int len = strlen(input);

    while (i < len) {
        // Skip whitespace
        while (i < len && isspace(input[i])) {
            i++;
        }

        if (i >= len) break;

        // Handle operators
        if (input[i] == '|') {
            if (i + 1 < len && input[i + 1] == '|') {
                tokenstream_add(stream, create_token(TOKEN_OR, "||"));
                i += 2;
            } else {
                tokenstream_add(stream, create_token(TOKEN_PIPE, "|"));
                i++;
            }
        } else if (input[i] == '>') {
            if (i + 1 < len && input[i + 1] == '>') {
                tokenstream_add(stream, create_token(TOKEN_REDIRECT_APPEND, ">>"));
                i += 2;
            } else {
                tokenstream_add(stream, create_token(TOKEN_REDIRECT_OUT, ">"));
                i++;
            }
        } else if (input[i] == '<') {
            tokenstream_add(stream, create_token(TOKEN_REDIRECT_IN, "<"));
            i++;
        } else if (input[i] == '&') {
            if (i + 1 < len && input[i + 1] == '&') {
                tokenstream_add(stream, create_token(TOKEN_AND, "&&"));
                i += 2;
            } else {
                tokenstream_add(stream, create_token(TOKEN_AMPERSAND, "&"));
                i++;
            }
        } else if (input[i] == ';') {
            tokenstream_add(stream, create_token(TOKEN_SEMICOLON, ";"));
            i++;
        } else if (input[i] == '(') {
            tokenstream_add(stream, create_token(TOKEN_LPAREN, "("));
            i++;
        } else if (input[i] == ')') {
            tokenstream_add(stream, create_token(TOKEN_RPAREN, ")"));
            i++;
        } else if (input[i] == '2' && i + 1 < len && input[i + 1] == '>') {
            // Handle 2> redirection
            tokenstream_add(stream, create_token(TOKEN_REDIRECT_ERR, "2>"));
            i += 2;
        } else {
            // Handle words (including quotes)
            char word[1024];
            int j = 0;
            int in_quote = 0;
            char quote_char = 0;

            while (i < len && j < 1023) {
                if (!in_quote && (input[i] == '"' || input[i] == '\'')) {
                    in_quote = 1;
                    quote_char = input[i];
                    i++;
                } else if (in_quote && input[i] == quote_char) {
                    in_quote = 0;
                    i++;
                } else if (!in_quote && isspace(input[i])) {
                    break;
                } else if (!in_quote && is_operator_char(input[i])) {
                    break;
                } else {
                    word[j++] = input[i];
                    i++;
                }
            }
            word[j] = '\0';

            if (j > 0) {
                tokenstream_add(stream, create_token(TOKEN_WORD, word));
            }
        }
    }

    tokenstream_add(stream, create_token(TOKEN_EOF, NULL));
    return stream;
}

void tokenstream_free(TokenStream *stream) {
    if (stream != NULL) {
        for (int i = 0; i < stream->count; i++) {
            free_token(stream->tokens[i]);
        }
        free(stream->tokens);
        free(stream);
    }
}

void tokenstream_print(TokenStream *stream) {
    if (stream == NULL) return;

    printf("Token Stream (%d tokens):\n", stream->count);
    for (int i = 0; i < stream->count; i++) {
        printf("  [%d] ", i);
        print_token(stream->tokens[i]);
    }
}
