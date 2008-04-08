/*
* $Id$
*/

#ifndef __PSP2CH_IMAGE_VIEW__
#define __PSP2CH_IMAGE_VIEW__

typedef struct tagBITMAPFILEHEADER {
    unsigned char bfType[2];
    unsigned long  bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned long  bfOffBits;
} __attribute__ ((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
    unsigned long  biSize;
    long           biWidth;
    long           biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned long  biCompression;
    unsigned long  biSizeImage;
    long           biXPixPerMeter;
    long           biYPixPerMeter;
    unsigned long  biClrUsed;
    unsigned long  biClrImporant;
} __attribute__ ((packed)) BITMAPINFOHEADER;

void psp2chImageViewJpeg(char* fname);
void psp2chImageViewPng(char* fname);
void psp2chImageViewBmp(char* fname);
void psp2chImageViewGif(char* fname);
void psp2chImageViewer(int* img[], int width, int height, char* fname);

#endif
