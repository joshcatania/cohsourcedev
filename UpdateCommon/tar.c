/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#include <stdio.h>
#include <sys/stat.h>
#include "stdtypes.h"
#include "file.h"
#include <windows.h>
#include "tar.h"

typedef unsigned U32;
typedef unsigned char U8;

void *file_alloc(char *fname, int *pn);

Tar *tar_from_data(U8 *data, U32 len)
{
    Tar *t;
    if(!data)
        return NULL;
    t = (Tar*)calloc(sizeof(Tar),1);
    t->start = data;
    t->data = data;
    t->len = len;
    return t;
}

Tar *tar_start_archive()
{
	Tar* t = (Tar*)calloc(sizeof(Tar),1);

	t->start = NULL;
	t->data = NULL;
	t->len = 0;

	return t;
}

void Tar_Destroy(Tar *t)
{
    free(t);
}

BOOL tar_add_to_archive(Tar *t, char* fileName)
{
	int nFileSize, nFileOffset;
	BOOL bResult = FALSE;
	U8* pData;

	if (( fileName ) && ( t ))
	{
		U8* fileData = file_alloc(fileName,&nFileSize);
		if ( fileData )
		{
			nFileOffset = ((TAR_HDR_SIZE + nFileSize + 511) & ~511);
			if ( (t->data - t->start) >= 0 )
			{
				t->start = realloc( t->start, t->len + nFileOffset );
				
				if ( t->start )
				{
					t->data = t->start + t->len;
					t->len += nFileOffset;
					pData = t->data;

					// write filename
					strncpy(pData, fileName, 100);
					// skip stuff
					pData += 124;
					// write size
					sprintf(pData,"%o",nFileSize);
					// skip to data portion
					pData = t->data + 257;
					// write data
					memcpy(pData,fileData,nFileSize);

					bResult = tar_process_cur(t);
				}		
				else
				{
					// failed to allocate
					bResult = FALSE;
				}
			}
		}
	}

	return bResult;
}

// only grabs the filename, file size.
BOOL tar_process_cur(Tar *t)
{
    char *data;
    if(!t)
        return FALSE;
    data = t->data;
    strncpy(t->hdr.fname,data,100);
    data+=100;
    data+=24; // skip mode, owner and user id
    if(1!=sscanf(data,"%o",&t->hdr.file_size))
        return FALSE;
    
    data = t->data + 257;
    if(data[0] != 'u' || data[1] != 's' || data[2] != 't' || data[3] != 'a' || data[4] != 'r')
        return FALSE;
    return TRUE;
}

void *tar_cur_file(Tar *t, U32 *len)
{
    if(!tar_process_cur(t))
        return NULL;
    if(len)
        *len = t->hdr.file_size;
    return t->data + TAR_HDR_SIZE;
}

// must free returned value
void *tar_alloc_cur_file(Tar *t, U32 *len)
{
    char *s;
    char *d;
    int n;
    
    if(!tar_process_cur(t))
        return NULL;
    n = t->hdr.file_size;
    if(len)
        *len = n;
    d = (char*)malloc(n);
    s = t->data + TAR_HDR_SIZE;
    memcpy(d,s,n);
    return d;
}

BOOL tar_next(Tar *t)
{
    if(!tar_process_cur(t))
        return FALSE;
    t->data += ((TAR_HDR_SIZE + t->hdr.file_size + 511) & ~511);
    if(t->data >= t->start + t->len)
        return FALSE;
    return tar_process_cur(t);
}


// *************************************************************************
// test area
// *************************************************************************
intptr_t file_size(char *fname){
	struct _stat32 status;
    if(!_stat32(fname, &status) && status.st_mode & _S_IFREG)
        return status.st_size;
    // The file doesn't exist.
    return -1;
}

void *file_alloc(char *fname, int *pn)
{
    void *b;
    int n;
    FILE *fp;
    n = file_size(fname);
    if(n<0)
        return NULL;
    if(pn)
        *pn = n;
    b = (void*)malloc(n+1);
    fp = fopen(fname,"rb");
    if(fread(b,1,n,fp) < n)
    {
        free(b);
        b = NULL;
    }
    fclose(fp);
    return b;
}
