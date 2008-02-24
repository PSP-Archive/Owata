/*
* $Id$
*/

#include "pspdialogs.h"
#include <stdio.h>
#include <malloc.h>
#include <jpeglib.h>
#include <png.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <pspgu.h>
#include "psp2ch.h"
#include "pg.h"
#include "psp2chImageView.h"

extern int running; //main.c
extern void* framebuffer; // pg.c
extern unsigned int list[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int pixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern int preLine;

void psp2chImageViewJpeg(char* fname)
{
    FILE* infile;
    JSAMPARRAY img;
    JSAMPROW buf, imgbuf, tmp;
    int width;
    int height;
    int i;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned char header[4];

    infile = fopen(fname, "rb" );
    if (!infile)
    {
        return;
    }
    // 一応ヘッダのチェック
    fread(header, 1, 2, infile);
    fseek(infile, 0, SEEK_SET);
    if (header[0] != 0xFF || header[1] != 0xD8)
    {
        fclose(infile);
        return;
    }
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
    if (cinfo.out_color_components != 3)
    {
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return;
    }
    width = cinfo.output_width;
    height = cinfo.output_height;
    img = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * height);
    if (!img)
    {
        fclose(infile);
        return;
    }
    imgbuf = (JSAMPROW)calloc(sizeof(JSAMPLE), 4 * width * height + 16);
    if (!imgbuf)
    {
        free(img);
        fclose(infile);
        return;
    }
    buf = (JSAMPROW)calloc(sizeof(JSAMPLE), 3 * width);
    if (!buf)
    {
        free(imgbuf);
        free(img);
        fclose(infile);
        return;
    }
    tmp = imgbuf;
    while (((int)tmp & 0xF) != 0)
    {
        tmp++;
    }
    for (i = 0; i < height; i++ )
    {
        img[i] = &tmp[i * width * 4];
    }
    while(cinfo.output_scanline < cinfo.output_height)
    {
        jpeg_read_scanlines(&cinfo, &buf, 1);
        for (i = 0; i < width; i++)
        {
            img[cinfo.output_scanline-1][i * 4 + 0] = buf[i * 3 + 0];
            img[cinfo.output_scanline-1][i * 4 + 1] = buf[i * 3 + 1];
            img[cinfo.output_scanline-1][i * 4 + 2] = buf[i * 3 + 2];
            img[cinfo.output_scanline-1][i * 4 + 3] = 0xFF;
        }
    }
    free(buf);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    psp2chImageViewer((int**)img, width, height);
    free(imgbuf);
    free(img);
    preLine = -2;
}

void psp2chImageViewPng(char* fname)
{
    FILE* infile;
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned long width, height;
    int bit_depth, color_type, interlace_type;
    png_bytepp img;
    png_bytep imgbuf;
    int i;
    unsigned char header[8];

    infile = fopen(fname, "rb");
    if (!infile)
    {
        return;
    }
    fread(header, 1, 8, infile);
    fseek(infile, 0, SEEK_SET);
    if (png_sig_cmp(header, 0, 8))
    {
        fclose(infile);
        return;
    }
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    png_init_io(png_ptr, infile);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
    //パレット系->RGB系に拡張
    if (color_type == PNG_COLOR_TYPE_PALETTE ||
        (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) ||
        png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    {
        png_set_expand(png_ptr);
    }
    //16ビット->8ビットに落とす
    if (bit_depth == 16)
    {
        png_set_strip_16(png_ptr);
    }
    //グレースケール->RGBに拡張
    if (color_type==PNG_COLOR_TYPE_GRAY ||
        color_type==PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        png_set_gray_to_rgb(png_ptr);
    }
    if (color_type != PNG_COLOR_TYPE_RGB_ALPHA)
    {
        png_set_add_alpha(png_ptr, 0xFFFF, PNG_FILLER_AFTER);
    }
    png_read_update_info(png_ptr, info_ptr);
    img = (png_bytepp)malloc(height * sizeof(png_bytep));
    imgbuf = (png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr) * width);
    for (i = 0; i < height; i++)
    {
        img[i] = &imgbuf[i * width * 4];
    }
    png_read_image(png_ptr, img);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose(infile);
    psp2chImageViewer((int**)img, (int)width, (int)height);
    free(imgbuf);
    free(img);
    preLine = -2;
}

struct Vertex
{
    unsigned short u, v;
    unsigned short color;
    short x, y, z;
};
void psp2chBitBlt(int sx, int sy, int sw, int sh, int dx, int dy)
{
    struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));

    vertices[0].u = sx; vertices[0].v = sy;
    vertices[0].color = 0;
    vertices[0].x = dx; vertices[0].y = dy; vertices[0].z = 0;

    vertices[1].u = sx+sw; vertices[1].v = sy+sh;
    vertices[1].color = 0;
    vertices[1].x = dx+sw; vertices[1].y = dy+sh; vertices[1].z = 0;

    sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_4444|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
}

void psp2chImageViewer(int* img[], int width, int height)
{
    int i, j, w, h, startX, startY;
    SceCtrlData pad;
    SceCtrlData oldPad;
    unsigned int* vptr0;
    unsigned int* vptr;
    int thumb;
    int thumbFlag = 0;
    int menu = 1;
    int padX, padY;

    thumb = (width/SCR_WIDTH > height/SCR_HEIGHT) ? width/SCR_WIDTH : height/SCR_HEIGHT;
    thumb++;
    startX = 0;
    startY = 0;
    sceCtrlPeekBufferPositive(&oldPad, 1);
    while (running)
    {
        if(sceCtrlPeekBufferPositive(&pad, 1))
        {
            if (pad.Buttons != oldPad.Buttons)
            {
                oldPad = pad;
                if(pad.Buttons & PSP_CTRL_CIRCLE && thumb > 1)
                {
                    thumbFlag = thumbFlag ? 0 : 1;
                    startX = 0;
                    startY = 0;
                }
                else if(pad.Buttons & PSP_CTRL_CROSS)
                {
                    break;
                }
                else if(pad.Buttons & PSP_CTRL_TRIANGLE)
                {
                    menu = menu ? 0 : 1;
                }
            }
            if(pad.Buttons & PSP_CTRL_UP)
            {
                startY--;
            }
            if(pad.Buttons & PSP_CTRL_DOWN)
            {
                startY++;
            }
            if(pad.Buttons & PSP_CTRL_LEFT)
            {
                startX--;
            }
            if(pad.Buttons & PSP_CTRL_RIGHT)
            {
                startX++;
            }
            padX = pad.Lx - 127;
            padY = pad.Ly - 127;
            if ((padX < -PAD_CUTOFF) || (padX > PAD_CUTOFF))
            {
                startX += (padX)/4;
            }
            if ((padY < -PAD_CUTOFF) || (padY > PAD_CUTOFF))
            {
                startY += (padY)/4;
            }
            if (startX >= width - SCR_WIDTH)
            {
                startX = width - SCR_WIDTH;
            }
            if (startY >= height - SCR_HEIGHT)
            {
                startY = height - SCR_HEIGHT;
            }
            if (startX < 0)
            {
                startX = 0;
            }
            if (startY < 0)
            {
                startY = 0;
            }
        }
        if (thumbFlag)
        {
            vptr0 = framebuffer + 0x04000000;
            for (i = 0, h = 0;i < height &&  h < height && h < SCR_HEIGHT; i += thumb)
            {
                vptr = vptr0;
                for (j = 0, w = 0;j < width && w < width && w < SCR_WIDTH; j += thumb)
                {
                    *vptr++ = img[i][j];
                    w++;
                }
                for (; w < SCR_WIDTH; w++)
                {
                    *vptr++ = GRAY;
                }
                vptr0 += BUF_WIDTH;
                h++;
            }
            for (; h < SCR_HEIGHT; h++)
            {
                vptr = vptr0;
                for (i = 0; i < SCR_WIDTH; i++)
                {
                    *vptr++ = GRAY;
                }
                vptr0 += BUF_WIDTH;
            }
        }
        else
        {
            /*
            sceGuStart(GU_DIRECT,list);
            sceGuCopyImage(GU_PSM_8888,startX,startY,width,height,width,img[0],0,0,width,(void*)(0x04000000+(u32)framebuffer));
            sceGuTexSync();
            sceGuFinish();
            sceGuSync(0,0);
            */
            vptr0 = framebuffer + 0x04000000;
            for (i = 0, h = 0; h < height && h < SCR_HEIGHT; i++)
            {
                if (i < startY)
                {
                    continue;
                }
                vptr = vptr0;
                for (j = 0, w = 0; w < width && w < SCR_WIDTH; j++)
                {
                    if (j < startX)
                    {
                        continue;
                    }
                    *vptr++ = img[i][j];
                    w++;
                }
                for (; w < SCR_WIDTH; w++)
                {
                    *vptr++ = GRAY;
                }
                vptr0 += BUF_WIDTH;
                h++;
            }
            for (; h < SCR_HEIGHT; h++)
            {
                vptr = vptr0;
                for (i = 0; i < SCR_WIDTH; i++)
                {
                    *vptr++ = GRAY;
                }
                vptr0 += BUF_WIDTH;
            }
        }
        if (menu)
        {
            pgMenuBar("　○ : 拡大縮小　　　× : 戻る　　　△ : メニューオン・オフ　　　");
        }
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
}
