#ifndef _JSON_H
#define _JSON_H

#include <stdbool.h>

typedef struct JsonNode JsonNode;
typedef struct JsonNode
{
    char* name;

    // MUTUALLY EXCLUSIVE!
    char* value;
    JsonNode** children;

    bool isarray;
} JsonNode;

JsonNode* jsonNode(const char* name, const char* value, bool quote, bool isarray);
void jsonDestroy(JsonNode* node);

void jsonOutput(JsonNode* root);
char* jsonEStr(JsonNode* root);

#endif
