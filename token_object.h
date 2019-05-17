#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct token_obj
{
    int num_of_tokens;
    char * tokens[41]; //MAX_CMD is 80, assuming half will be used for spaces (Even here it's too much)
}token_obj;

token_obj getTokens (char * string);