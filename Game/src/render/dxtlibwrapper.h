#ifndef DXTLIBWRAPPER_H
#define DXTLIBWRAPPER_H

unsigned char * dxtDecompressC(int *w, int *h, int *depth, int *total_width, int *rowBytes, int *src_format,
                                int SpecifiedMipMaps, unsigned char *data, int data_size);

#endif
