#include "token_object.h"

token_obj getTokens (char * string)
{
    token_obj tokens;

    int i = 0;
    char * token = strtok(string, " /");

    while (token != NULL) {
        tokens.tokens[i] = token;

        i++;
        token = strtok(NULL, " /");
    }

    for (int j = 0; tokens.tokens[i-1][j] != '\0'; j++) {
        if (tokens.tokens[i-1][j] == 0x0A)
            tokens.tokens[i-1][j] = '\0';
    }

    tokens.num_of_tokens = i;

    return tokens;
}