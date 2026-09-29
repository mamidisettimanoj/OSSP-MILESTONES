#ifndef LEXER_H
#define LEXER_H

#include "token.h"

typedef struct {
    Token **tokens;
    int count;
    int capacity;
} TokenStream;

TokenStream* lexer_tokenize(const char *input);
void tokenstream_free(TokenStream *stream);
void tokenstream_print(TokenStream *stream);

#endif
