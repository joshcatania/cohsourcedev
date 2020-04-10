#ifndef TAR_H
#define TAR_H

#define TAR_HDR_SIZE 512

typedef struct TarHdr
{
    char fname[100];
    U32 file_size;
    U32 last_mod_time;    
} TarHdr;

typedef struct Tar
{
    U8 *start;
    U8 *data;
    U32 len;
    TarHdr hdr;
} Tar;

Tar *tar_from_data(U8 *data, U32 len);
void Tar_Destroy(Tar *t);
BOOL tar_process_cur(Tar *t);
void *tar_cur_file(Tar *t, U32 *len);
void *tar_alloc_cur_file(Tar *t, U32 *len);
BOOL tar_next(Tar *t);

#endif