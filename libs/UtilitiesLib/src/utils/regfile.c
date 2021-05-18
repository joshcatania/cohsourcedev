// regfile.c : Defines the functions for the static library.
//

#include <utilitiesLib/utils/regfile.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LOCK_NAME "lock"

char registryPath_[1024];
char lockFile_[1024];

int regfileIsInit(void)
{
    return (strlen(lockFile_) != 0);
}

#ifdef _WIN32
#include <io.h>
#include <sys/stat.h>
#include <direct.h>
#include <windows.h>
#include <assert.h>
#include <time.h>

void printLastError_(void)
{
    DWORD errorCode = GetLastError();
    if (errorCode == 0)
    {
        return;
    }

    wchar_t messageBuffer[1024];
    size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, 0, messageBuffer,
                                 sizeof(messageBuffer) / sizeof(wchar_t), NULL);
    _wperror(messageBuffer);
}

#define LOCK_FLAGS (_O_EXCL | _O_CREAT | _O_RDWR | _O_TEMPORARY), (_S_IREAD | _S_IWRITE)

int tryLock_(int maxTries)
{
    if (!regfileIsInit())
        return -1;

    int tries = 0;
    int lfd = _open(lockFile_, LOCK_FLAGS);

    while (lfd == -1 && tries < maxTries)
    {
        Sleep(5);
        lfd = _open(lockFile_, LOCK_FLAGS);
        tries += 1;
    }
    if (tries == maxTries)
    {
        perror(__FUNCTION__);
        perror("FATAL: Could not acquire shadow registry lockfile.");
        exit(EXIT_FAILURE);
    }
    return lfd;
}

int releaseLock_(int lfd)
{
    if (!regfileIsInit())
        return -1;
    return _close(lfd);
}

int listFiles_(const char* path, char* files)
{
    int lfd = tryLock_(50);

    HANDLE findHandle;
    WIN32_FIND_DATA findData;
    char* iter = files;
    int fileCount = 0;

    findHandle = FindFirstFile((LPCWSTR)path, &findData);

    if (findHandle == INVALID_HANDLE_VALUE)
    {
        perror(__FUNCTION__);
        printLastError_();

        releaseLock_(lfd);
        return 0;
    }

    strcpy(iter, (const char*)findData.cFileName);
    iter += strlen(iter) + 1;
    fileCount += 1;

    while (FindNextFile(findHandle, findHandle))
    {
        strcpy(iter, (const char*)findData.cFileName);
        iter += strlen(iter) + 1;
        fileCount += 1;
    }

    FindClose(findHandle);

    releaseLock_(lfd);
    return fileCount;
}

#endif

// Shameless Copypasta from SO
int mkpath_(const char* path)
{
    char pathBuffer[REGFILE_PATH_LEN];
    strcpy(pathBuffer, path);
    for (char* p = strchr(pathBuffer + 1, REGFILE_SEPERATOR_CHAR); p; p = strchr(p + 1, REGFILE_SEPERATOR_CHAR))
    {
        *p = '\0';
        if (_mkdir(pathBuffer) == -1)
        {
            if (errno != EEXIST)
            {
                perror(__FUNCTION__);
                perror(strerror(errno));
                *p = REGFILE_SEPERATOR_CHAR;
                return -1;
            }
        }
        *p = REGFILE_SEPERATOR_CHAR;
    }

    return 0;
}

int regfileInit(const char* directory)
{
    strcpy(registryPath_, directory);
    regfileNormalizeKey(registryPath_);

    if (directory[strlen(directory) - 1] != REGFILE_SEPERATOR_CHAR)
        strcat(registryPath_, REGFILE_SEPERATOR_STR);

    int status = mkpath_(registryPath_);
    if (status != 0)
    {
        perror(__FUNCTION__);
        perror("Failed to initialize regfile");
        return -1;
    }

    strcpy(lockFile_, registryPath_);
    strcat(lockFile_, LOCK_NAME);

    return 0;
}

void regfileNormalizeKey(char* mutableKey)
{
    char* iter = mutableKey;
    while (*iter != 0)
    {
        *iter = tolower(*iter);
#ifdef _WIN32
        if (*iter == '/')
            *iter = '\\';
#else
        if (*iter == '\\')
            *iter = '/';
#endif
        ++iter;
    }
}

/// <summary>
/// Load value from file.
/// </summary>
/// <param name="key"> Path within registry file. </param>
/// <param name="buffer"> Buffer to read data into. </param>
/// <param name="len"> Max length in bytes to read. </param>
/// <returns> Number of bytes read from file. -1 on error. </returns>
int regfileLoadKeyValue(const char* key, void* buffer, size_t len)
{
    int lfd = tryLock_(50);

    char keyPath[REGFILE_PATH_LEN];
    strcpy(keyPath, registryPath_);
    strcat(keyPath, key);

    FILE* keyfile = fopen(keyPath, "rb");

    if (keyfile == NULL)
    {
        int errcode = errno;
        perror(__FUNCTION__);
        perror(strerror(errcode));
        releaseLock_(lfd);
        return -1;
    }

    int bytesRead = fread(buffer, 1, len, keyfile);

    fclose(keyfile);
    releaseLock_(lfd);

    return bytesRead;
}

int regfileStoreKeyValue(const char* key, void* value, size_t len)
{
    int lfd = tryLock_(50);

    char keyPath[REGFILE_PATH_LEN];
    strcpy(keyPath, registryPath_);
    strcat(keyPath, key);
    mkpath_(keyPath);

    FILE* keyfile = fopen(keyPath, "wb");

    if (keyfile == NULL)
    {
        int errcode = errno;
        perror(__FUNCTION__);
        perror(strerror(errcode));

        releaseLock_(lfd);
        return -1;
    }

    size_t bytesWritten = fwrite(value, 1, len, keyfile);

    if (bytesWritten != len)
    {
        perror(__FUNCTION__);
        perror("Error writing value to key file.");
    }

    fclose(keyfile);
    releaseLock_(lfd);

    return bytesWritten;
}

int regfileRemoveKey(const char* key)
{
    int lfd = tryLock_(50);

    char keyPath[REGFILE_PATH_LEN];

    strcpy(keyPath, registryPath_);
    strcat(keyPath, key);

    regfileNormalizeKey(keyPath);

    int returnValue = remove(keyPath);

    releaseLock_(lfd);

    return returnValue;
}

int regfileList(const char* key, char* files)
{
    return listFiles_(key, files);
}

int regfileDoesKeyExist(const char* key)
{
    int lfd = tryLock_(50);

    char keyPath[REGFILE_PATH_LEN];
    strcpy(keyPath, registryPath_);
    strcat(keyPath, key);

    regfileNormalizeKey(keyPath);

    FILE* keyfile = fopen(keyPath, "rb");

    if (keyfile == NULL)
    {
        releaseLock_(lfd);
        return 0;
    }

    fclose(keyfile);

    releaseLock_(lfd);
    return 1;
}
