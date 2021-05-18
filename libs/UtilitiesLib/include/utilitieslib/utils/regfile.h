#ifndef REGFILE_H
#define REGFILE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
#define REGFILE_SEPERATOR_CHAR '\\'
#define REGFILE_OTHER_SEPERATOR_CHAR '/'
#define REGFILE_SEPERATOR_STR "\\"
#else
#define REGFILE_SEPERATOR_CHAR '/'
#define REGFILE_OTHER_SEPERATOR_CHAR '\\'
#define REGFILE_SEPERATOR_STR "/"
#endif
#define REGFILE_PATH_LEN 512
#define REGFILE_DEFAULT_PATH "." REGFILE_SEPERATOR_STR "registry-keys" REGFILE_SEPERATOR_STR
#define REGFILE_PATH_EXISTANCE_FILE ".regfile-path-exists"
#define REGFILE_CAT_PATH(buff, path, filename)                                                                                                                 \
    strcpy(buff, path);                                                                                                                                        \
    if (buff[strlen(buff) - 1] == REGFILE_OTHER_SEPERATOR_CHAR)                                                                                                \
        buff[strlen(buff) - 1] = REGFILE_SEPERATOR_CHAR;                                                                                                       \
    if (buff[strlen(buff) - 1] != REGFILE_SEPERATOR_CHAR)                                                                                                      \
        strcat(buff, REGFILE_SEPERATOR_STR);                                                                                                                   \
    strcat(buff, filename)

    int regfileIsInit(void);
    int regfileInit(const char* filepath);
    int regfileLoadKeyValue(const char* key, void* buffer, size_t len);
    int regfileStoreKeyValue(const char* key, void* value, size_t len);
    void regfileNormalizeKey(char* mutable_key);
    int regfileRemoveKey(const char* key);
    int regfileList(const char* key, char* files);
    int regfileDoesKeyExist(const char* key);

#ifdef __cplusplus
}
#endif

#endif
