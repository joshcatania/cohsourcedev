#include <utilitieslib/components/earray.h>
#include <utilitieslib/components/EString.h>
#include <utilitieslib\utils\memcheck.h>

#include "json.h"

JsonNode* jsonNode(const char* name, const char* value, bool quote, bool isarray)
{
    JsonNode* ret = calloc(1, sizeof(JsonNode));
    if (name)
        ret->name = _strdup(name);
    if (value)
    {
        if (quote)
        {
            size_t len = strlen(value);
            ret->value = malloc(len + 3);
            ret->value[0] = '\"';
            memcpy(&ret->value[1], value, len);
            ret->value[len + 1] = '\"';
            ret->value[len + 2] = 0;
        }
        else
            ret->value = _strdup(value);
    }
    ret->isarray = isarray;
    return ret;
}

void jsonDestroy(JsonNode* node)
{
    if (node->children)
    {
        eaDestroyEx(&node->children, jsonDestroy);
    }

    SAFE_FREE(node->name);
    SAFE_FREE(node->value);
    free(node);
}

static char* indentStr(int indent)
{
    static char buf[256];
    int i;

    for (i = 0; i <= indent; i += 16)
    {
        memcpy(buf + i, "                ", 16);
    }
    buf[indent] = 0;
    return buf;
}

#define jsonPrint(fmt, ...)                                                                                                                                    \
    if (estr)                                                                                                                                                  \
    {                                                                                                                                                          \
        estrConcatf(estr, fmt, __VA_ARGS__);                                                                                                                   \
    }                                                                                                                                                          \
    else                                                                                                                                                       \
    {                                                                                                                                                          \
        printf(fmt, __VA_ARGS__);                                                                                                                              \
    }

static void jsonNodeOut(char** estr, int indent, JsonNode* node, const char* comma)
{
    int i, sz;
    char aopen = '{', aclose = '}';

    jsonPrint("%s", indentStr(indent));
    if (node->name)
    {
        jsonPrint("\"%s\": ", node->name);
    }
    if (node->value)
    {
        jsonPrint("%s%s\n", node->value, comma);
    }
    else if (node->children)
    {
        if (node->isarray)
        {
            aopen = '[';
            aclose = ']';
        }
        jsonPrint("%c\n", aopen);
        sz = eaSize(&node->children);
        for (i = 0; i < sz; i++)
        {
            jsonNodeOut(estr, indent + 2, node->children[i], (i == sz - 1) ? "" : ",");
        }
        jsonPrint("%s%c%s\n", indentStr(indent), aclose, comma);
    }
    else if (node->isarray)
    {
        jsonPrint("[]%s\n", comma);
    }
    else
    {
        jsonPrint("{}%s\n", comma);
    }
}

void jsonOutput(JsonNode* root)
{
    jsonNodeOut(NULL, 0, root, "");
}

char* jsonEStr(JsonNode* root)
{
    char* out = 0;
    jsonNodeOut(&out, 0, root, "");
    return out;
}