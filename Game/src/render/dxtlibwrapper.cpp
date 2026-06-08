#include <utilitieslib/utils/wininclude.h>

#include "dd.h"
#include <stdlib.h>
#include <string.h>

namespace squish {
typedef unsigned char u8;
void DecompressColour( u8* rgba, void const* block, bool isDxt1 );
void DecompressAlphaDxt3( u8* rgba, void const* block );
void DecompressAlphaDxt5( u8* rgba, void const* block );
}

#ifndef DDS_RGB
#define DDS_RGB    0x00000040
#endif
#ifndef DDS_RGBA
#define DDS_RGBA   0x00000041
#endif

static int ddsChoosePitch(int header_pitch, int row_bytes, int height, size_t payload_size)
{
    if (height <= 0)
        return 0;

    if (header_pitch >= row_bytes && (size_t)header_pitch * (size_t)height <= payload_size)
        return header_pitch;

    if ((size_t)row_bytes * (size_t)height <= payload_size)
        return row_bytes;

    return 0;
}

static void ddsDecompressBlock(unsigned char *dst, int width, int height, int bx, int by, const unsigned char *block, DWORD fourCC)
{
    squish::u8 rgba[16 * 4];
    const squish::u8 *source = rgba;
    int px;
    int py;

    if (fourCC == FOURCC_DXT1)
    {
        squish::DecompressColour(rgba, block, true);
    }
    else if (fourCC == FOURCC_DXT3)
    {
        squish::DecompressColour(rgba, block + 8, false);
        squish::DecompressAlphaDxt3(rgba, block);
    }
    else
    {
        squish::DecompressColour(rgba, block + 8, false);
        squish::DecompressAlphaDxt5(rgba, block);
    }

    for (py = 0; py < 4; py++)
    {
        for (px = 0; px < 4; px++)
        {
            int x = bx + px;
            int y = by + py;
            if (x < width && y < height)
            {
                unsigned char *pixel = dst + (y * width + x) * 4;
                pixel[0] = source[2];
                pixel[1] = source[1];
                pixel[2] = source[0];
                pixel[3] = source[3];
            }
            source += 4;
        }
    }
}

extern "C" unsigned char * dxtDecompressC(int *w, int *h, int *depth, int *total_width, int *rowBytes, int *src_format,
                                          int SpecifiedMipMaps, unsigned char *data, int data_size)
{
    DDSURFACEDESC2 ddsd;
    const unsigned char *src;
    unsigned char *dst;
    size_t header_size = 4 + sizeof(ddsd);
    size_t payload_size;
    int width;
    int height;
    int blockSize;
    int blocksX;
    int blocksY;
    int x;
    int y;

    UNREFERENCED_PARAMETER(SpecifiedMipMaps);

    if (!data || data_size < (int)header_size || memcmp(data, "DDS ", 4) != 0)
        return NULL;

    memcpy(&ddsd, data + 4, sizeof(ddsd));
    width = (int)ddsd.dwWidth;
    height = (int)ddsd.dwHeight;
    src = data + 4 + sizeof(ddsd);
    payload_size = (size_t)data_size - header_size;

    if (width <= 0 || height <= 0 || width > 16384 || height > 16384)
        return NULL;

    *w = width;
    *h = height;
    *total_width = width;
    *src_format = (int)ddsd.ddpfPixelFormat.dwFourCC;

    if (ddsd.ddpfPixelFormat.dwFourCC == FOURCC_DXT1 ||
        ddsd.ddpfPixelFormat.dwFourCC == FOURCC_DXT3 ||
        ddsd.ddpfPixelFormat.dwFourCC == FOURCC_DXT5)
    {
        dst = (unsigned char*)malloc(width * height * 4);
        if (!dst)
            return NULL;

        *depth = 4;
        *rowBytes = width * 4;
        blockSize = (ddsd.ddpfPixelFormat.dwFourCC == FOURCC_DXT1) ? 8 : 16;
        blocksX = (width + 3) / 4;
        blocksY = (height + 3) / 4;

        if ((size_t)blocksX * (size_t)blocksY * (size_t)blockSize > payload_size)
        {
            free(dst);
            return NULL;
        }

        for (y = 0; y < blocksY; y++) {
            for (x = 0; x < blocksX; x++) {
                const unsigned char *block = src + (y * blocksX + x) * blockSize;
                ddsDecompressBlock(dst, width, height, x * 4, y * 4, block, ddsd.ddpfPixelFormat.dwFourCC);
            }
        }
        return dst;
    }

    if (ddsd.ddpfPixelFormat.dwFlags == DDS_RGBA && ddsd.ddpfPixelFormat.dwRGBBitCount == 32 && ddsd.ddpfPixelFormat.dwRGBAlphaBitMask == 0xff000000) {
        int row_bytes = width * 4;
        int pitch = ddsChoosePitch(ddsd.lPitch, row_bytes, height, payload_size);
        if (!pitch)
            return NULL;
        dst = (unsigned char*)malloc(width * height * 4);
        if (!dst)
            return NULL;
        for (y = 0; y < height; y++)
            memcpy(dst + y * row_bytes, src + y * pitch, row_bytes);
        *depth = 4;
        *rowBytes = row_bytes;
        return dst;
    }

    if (ddsd.ddpfPixelFormat.dwFlags == DDS_RGB && ddsd.ddpfPixelFormat.dwRGBBitCount == 24) {
        int row_bytes = width * 3;
        int pitch = ddsChoosePitch(ddsd.lPitch, row_bytes, height, payload_size);
        if (!pitch)
            return NULL;
        dst = (unsigned char*)malloc(width * height * 3);
        if (!dst)
            return NULL;
        for (y = 0; y < height; y++)
            memcpy(dst + y * row_bytes, src + y * pitch, row_bytes);
        *depth = 3;
        *rowBytes = row_bytes;
        return dst;
    }

    return NULL;
}
