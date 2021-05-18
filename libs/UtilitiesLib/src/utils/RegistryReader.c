#ifndef _XBOX

#include "utilitieslib/utils/RegistryReader.h"
#include "utilitieslib/utils/wininclude.h"
#include "utilitieslib/utils/regfile.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct RegReaderImp
{
    HKEY key;
    unsigned int keyOpened;
    unsigned int keyExists;
    char* keyName;
} RegReaderImp;

RegReader createRegReader(void)
{
    return calloc(1, sizeof(RegReaderImp));
}

void destroyRegReader(RegReaderImp* reader)
{
    rrClose(reader);
    if (reader->keyName)
    {
        free(reader->keyName);
    }
    free(reader);
}

typedef struct
{
    char* keyName;
    HKEY key;
} PredefinedKey;

PredefinedKey predefinedKeys[] = {
    {"HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT},
    {"HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG},
    {"HKEY_CURRENT_USER", HKEY_CURRENT_USER},
    {"HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE},
    {"HKEY_USERS", HKEY_USERS},
};

int initRegReader(RegReaderImp* reader, const char* keyName)
{
    if (!regfileIsInit())
        regfileInit(REGFILE_DEFAULT_PATH);

    PredefinedKey* predefKey;

    // Seperate the predefined key name from the rest of the the key name.
    int matchedKnownKey = 0;
    int predefKeyNameLen = 0;

    // Look through all of known predefined keys.
    for (predefKey = predefinedKeys; predefKey < predefinedKeys + ARRAY_SIZE(predefinedKeys); predefKey++)
    {

        // Compare each predefined key names to the beginning of the key string.
        // If they match, we've found the correct predefined key to be used to open the given key.
        predefKeyNameLen = (int)strlen(predefKey->keyName);
        if (0 == strnicmp(predefKey->keyName, keyName, predefKeyNameLen))
        {
            matchedKnownKey = 1;
            break;
        }
    }

    if (!matchedKnownKey)
        return 0;

    char keyBuffer[REGFILE_PATH_LEN];

    REGFILE_CAT_PATH(keyBuffer, keyName, REGFILE_PATH_EXISTANCE_FILE);

    regfileNormalizeKey(keyBuffer);

    reader->keyName = strdup(keyName);

    reader->keyExists = regfileDoesKeyExist(keyBuffer);
    reader->keyOpened = reader->keyExists;

    return 1;
}

int rrLazyWriteInit(RegReaderImp* reader)
{
    if (!regfileIsInit())
        regfileInit(REGFILE_DEFAULT_PATH);
    
    PredefinedKey* predefKey;

    // Seperate the predefined key name from the rest of the the key name.
    int matchedKnownKey = 0;
    int predefKeyNameLen = 0;

    if (!reader->keyExists && reader->keyName)
    {
        reader->keyExists = 1;

        // Look through all of known predefined keys.
        for(predefKey = predefinedKeys; predefKey < predefinedKeys + (sizeof(predefinedKeys) / sizeof(predefinedKeys[0])); predefKey++){

            // Compare each predefined key names to the beginning of the key string.
            // If they match, we've found the correct predefined key to be used to open the given key.
            predefKeyNameLen = (int)strlen(predefKey->keyName);
            if(0 == strnicmp(predefKey->keyName, reader->keyName, predefKeyNameLen)){
                matchedKnownKey = 1;
                break;
            }
        }

        if(!matchedKnownKey)
            return 0;

        char keyBuffer[REGFILE_PATH_LEN];

        REGFILE_CAT_PATH(keyBuffer, reader->keyName, REGFILE_PATH_EXISTANCE_FILE);

        regfileNormalizeKey(keyBuffer);

        // Write of 0 bytes will create the file and do nothing.

        if (regfileStoreKeyValue(keyBuffer, &matchedKnownKey, 0) == -1)
        {
            return 0;
        }

        reader->keyOpened = 1;
        return 1;
    }
    return 1;
}

int initRegReaderEx(RegReaderImp* reader, const char* templateString, ...)
{
    va_list va;
    char buffer[1024];

    va_start(va, templateString);
    vsprintf(buffer, templateString, va);
    va_end(va);

    return initRegReader(reader, buffer);
}

int rrReadString(RegReaderImp* reader, const char* valueName, char* outBuffer, int bufferSize)
{
    if (!reader->keyOpened)
        return 0;

    char keyBuffer[REGFILE_PATH_LEN];

    REGFILE_CAT_PATH(keyBuffer, reader->keyName, valueName);

    regfileNormalizeKey(keyBuffer);

    int bytesRead = regfileLoadKeyValue(keyBuffer, outBuffer, bufferSize);

    if (bytesRead == -1)
        return 0;

    outBuffer[bytesRead] = 0;
    return 1;
}

int rrReadMultibyteString(RegReaderImp* reader, const char* valueName, char* outBuffer, int bufferSize)
{
    if (!reader->keyOpened)
        return 0;

    char keyBuffer[REGFILE_PATH_LEN];

    REGFILE_CAT_PATH(keyBuffer, reader->keyName, valueName);

    regfileNormalizeKey(keyBuffer);

    int bytesRead = regfileLoadKeyValue(keyBuffer, outBuffer, bufferSize);

    if (bytesRead == -1)
        return 0;

    outBuffer[bytesRead] = 0;
    return 1;
}

int rrWriteString(RegReaderImp* reader, const char* valueName, const char* str)
{
    rrLazyWriteInit(reader);

    if (!reader->keyOpened)
        return 0;

    char keyBuffer[REGFILE_PATH_LEN];

    REGFILE_CAT_PATH(keyBuffer, reader->keyName, valueName);

    regfileNormalizeKey(keyBuffer);

    size_t length = strlen(str);
    int bytesWritten = regfileStoreKeyValue(keyBuffer, (void*)str, length);

    if (bytesWritten != length)
        return 0;
    return 1;
}

int rrReadInt(RegReaderImp* reader, const char* valueName, unsigned int* value, unsigned int defValue)
{
    if (!value)
        return 0;

    if (!reader->keyOpened)
    {
        *value = defValue;
        return 0;
    }

    char keyBuffer[REGFILE_PATH_LEN];

    REGFILE_CAT_PATH(keyBuffer, reader->keyName, valueName);

    regfileNormalizeKey(keyBuffer);

    size_t valueSize = sizeof(*value);
    int bytesRead = regfileLoadKeyValue(keyBuffer, value, valueSize);

    if (bytesRead != valueSize)
    {
        // Uncertain if I should modify behavior to set value to 0 on read failure.
        *value = defValue;
        return 0;
    }
    return 1;
}

int rrReadInt64(RegReaderImp* reader, const char* valueName, S64* value, S64 defValue)
{
    if (!value)
        return 0;

    if (!reader->keyOpened)
    {
        *value = defValue;
        return 0;
    }

    char keyBuffer[REGFILE_PATH_LEN];

    REGFILE_CAT_PATH(keyBuffer, reader->keyName, valueName);

    regfileNormalizeKey(keyBuffer);

    size_t valueSize = sizeof(*value);
    int bytesRead = regfileLoadKeyValue(keyBuffer, value, valueSize);

    if (bytesRead != valueSize)
    {
        *value = defValue;
        return 0;
    }
    return 1;
}

int rrWriteInt(RegReaderImp* reader, const char* valueName, unsigned int value)
{
    rrLazyWriteInit(reader);

    char keyBuffer[REGFILE_PATH_LEN];
    REGFILE_CAT_PATH(keyBuffer, reader->keyName, valueName);

    regfileNormalizeKey(keyBuffer);

    size_t valueSize = sizeof(value);
    int bytesWritten = regfileStoreKeyValue(keyBuffer, &value, valueSize);

    if (bytesWritten != valueSize)
        return 0;
    return 1;
}

int rrWriteInt64(RegReaderImp* reader, const char* valueName, S64 value)
{
    rrLazyWriteInit(reader);

    char keyBuffer[REGFILE_PATH_LEN];

    REGFILE_CAT_PATH(keyBuffer, reader->keyName, valueName);
    
    regfileNormalizeKey(keyBuffer);

    size_t valueSize = sizeof(value);
    int bytesWritten = regfileStoreKeyValue(keyBuffer, &value, valueSize);

    if (bytesWritten != valueSize)
        return 0;
    return 1;
}

// Function seems to be unused.
int rrFlush(RegReaderImp* reader)
{
    return reader->keyOpened;
}

int rrDelete(RegReaderImp* reader, const char* valueName)
{
    if (reader->keyOpened)
        return 0;

    char keyBuffer[REGFILE_PATH_LEN];

    REGFILE_CAT_PATH(keyBuffer, reader->keyName, valueName);
    
    regfileNormalizeKey(keyBuffer);

    // This returns 0 on success because original API would do the same.
    return regfileRemoveKey(keyBuffer);
}

// Original API seems to return 1 on success.
int rrClose(RegReaderImp* reader)
{
    reader->keyOpened = 0;
    return 1;
}

// Returns number of files found, -1 on error.
int registryEnumKeys(RegReaderImp* reader, char* files)
{
    char keyBuffer[REGFILE_PATH_LEN];

    REGFILE_CAT_PATH(keyBuffer, reader->keyName, "");

    regfileNormalizeKey(keyBuffer);

    return regfileList(keyBuffer, files);
}

// rrEnumStrings
//
// Return values:
//    <0: Index is too high, you're done.
//  0:    Check *inOutNameLen:
//        >= 0:    Not a string value.
//        <0:        Buffer overflow.
//  >0:    A string value.

int registryWriteInt(const char* keyName, const char* valueName, unsigned int value)
{
    char keyBuffer[REGFILE_PATH_LEN];

    REGFILE_CAT_PATH(keyBuffer, keyName, valueName);
    
    regfileNormalizeKey(keyBuffer);

    size_t valueSize = sizeof(value);
    size_t bytesWritten = regfileStoreKeyValue(keyBuffer, &value, valueSize);

    if (bytesWritten != valueSize)
        return 0;
    return 1;
}

#else

// Stubbed out for now. Should be a wrapper around the xbox registry equivalent

#include "RegistryReader.h"

RegReader createRegReader(void)
{
    return NULL;
}
void destroyRegReader(RegReader reader)
{
}

int initRegReader(RegReader reader, const char* key)
{
    return 0;
}
int initRegReaderEx(RegReader reader, const char* templateString, ...)
{
    return 0;
}

int rrReadString(RegReader reader, const char* valueName, char* outBuffer, int bufferSize)
{
    return 0;
}
int rrWriteString(RegReader reader, const char* valueName, const char* str)
{
    return 0;
}
int rrReadInt64(RegReader reader, const char* valueName, S64* value)
{
    return 0;
}
int rrReadInt(RegReader reader, const char* valueName, unsigned int* value)
{
    return 0;
}
int rrWriteInt64(RegReader reader, const char* valueName, S64 value)
{
    return 0;
}
int rrWriteInt(RegReader reader, const char* valueName, unsigned int value)
{
    return 0;
}
int rrFlush(RegReader reader)
{
    return 0;
}
int rrDelete(RegReader reader, const char* valueName)
{
    return 0;
}

int rrClose(RegReader reader)
{
    return 0;
}

int rrEnumStrings(RegReader reader, int index, char* outName, int* inOutNameLen, char* outValue, int* inOutValueLen)
{
    return 0;
}
int registryWriteInt(const char* keyName, const char* valueName, unsigned int value)
{
    return 0;
}

#endif
