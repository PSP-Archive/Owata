/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include <jpeglib.h>
#include <png.h>
#include <pspdebug.h>
#include <pspgu.h>
#include "psp2ch.h"
#include "pg.h"
#include "psp2chImageView.h"

extern S_2CH s2ch; //psp2ch.c
extern unsigned int list[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int pixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern int preLine; // psp2chRes.c

void psp2chImageViewJpeg(char* fname)
{
    FILE* infile;
    JSAMPARRAY img;
    JSAMPROW buf, imgbuf, tmp;
    int width, bufWidth;
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
    width = cinfo.output_width;
    bufWidth = width + (16 - width % 16) % 16;
    height = cinfo.output_height;
    img = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * height);
    if (!img)
    {
        fclose(infile);
        return;
    }
    imgbuf = (JSAMPROW)calloc(sizeof(JSAMPLE), 4 * bufWidth * height + 16);
    if (!imgbuf)
    {
        free(img);
        fclose(infile);
        return;
    }
    buf = (JSAMPROW)calloc(sizeof(JSAMPLE), 3 * bufWidth);
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
        img[i] = &tmp[i * bufWidth * 4];
    }
    if (cinfo.out_color_components == 3)
    {
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
    }
    else if (cinfo.out_color_components == 1)
    {
        while(cinfo.output_scanline < cinfo.output_height)
        {
            jpeg_read_scanlines(&cinfo, &buf, 1);
            for (i = 0; i < width; i++)
            {
                img[cinfo.output_scanline-1][i * 4 + 0] = buf[i];
                img[cinfo.output_scanline-1][i * 4 + 1] = buf[i];
                img[cinfo.output_scanline-1][i * 4 + 2] = buf[i];
                img[cinfo.output_scanline-1][i * 4 + 3] = 0xFF;
            }
        }
    }
    else
    {
        free(buf);
        free(imgbuf);
        free(img);
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return;
    }
    free(buf);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    psp2chImageViewer((int**)img, width, height, bufWidth, fname);
    free(imgbuf);
    free(img);
    preLine = -2;
}

void psp2chImageViewPng(char* fname)
{
    FILE* infile;
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned long width, height, bufWidth;
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
    bufWidth = width + (16 - width % 16) % 16;
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
    imgbuf = (png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr) * bufWidth);
    for (i = 0; i < height; i++)
    {
        img[i] = &imgbuf[i * bufWidth * 4];
    }
    png_read_image(png_ptr, img);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose(infile);
    psp2chImageViewer((int**)img, (int)width, (int)height, (int)bufWidth, fname);
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
void psp2chBitBlt(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh)
{
    struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));

    vertices[0].u = sx; vertices[0].v = sy;
    vertices[0].color = 0;
    vertices[0].x = dx; vertices[0].y = dy; vertices[0].z = 0;

    vertices[1].u = sx+sw; vertices[1].v = sy+sh;
    vertices[1].color = 0;
    vertices[1].x = dx+dw; vertices[1].y = dy+dh; vertices[1].z = 0;

    sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_4444|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
}

void psp2chImageViewer(int* img[], int width, int height, int bufWidth, char* fname)
{
    int w, h, startX, startY;
    SceCtrlData pad;
    SceCtrlData oldPad;
    double thumb, thumbW, thumbH;
    double sx, sy, sw, sh, dx, dy, dw, dh;
    int thumbFlag = 0;
    int menu = 1, rMenu = 0;
    int padX, padY;
    char picturePath[256], buf[256];
    char *p;
    SceUID src, dst;
    int ret;

    thumbW = (double)width/SCR_WIDTH;
    thumbH = (double)height/SCR_HEIGHT;
    startX = 0;
    startY = 0;
    sceCtrlPeekBufferPositive(&oldPad, 1);
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&pad, 1))
        {
            if(pad.Buttons & PSP_CTRL_RTRIGGER)
            {
                rMenu = 1;
            }
            else
            {
                rMenu = 0;
            }
            if (pad.Buttons != oldPad.Buttons)
            {
                oldPad = pad;
                if (rMenu)
                {
                    if(pad.Buttons & PSP_CTRL_CIRCLE)
                    {
                        if (s2ch.cfg.imageDir[0])
                        {
                            sprintf(picturePath, "ms0:/PICTURE/%s", s2ch.cfg.imageDir);
                            src = sceIoDopen(picturePath);
                            if (src < 0)
                            {
                                ret = sceIoMkdir(picturePath, 0777);
                                if (ret < 0)
                                {
                                    continue;
                                }
                            }
                            else
                            {
                                sceIoDclose(src);
                            }
                        }
                        else
                        {
                            strcpy(picturePath, "ms0:/PICTURE");
                        }
                        p = strrchr(fname, '/');
                        if (p == NULL)
                        {
                            continue;
                        }
                        strcat(picturePath, p);
                        src = sceIoOpen(fname, PSP_O_RDONLY, 0777);
                        if (src < 0)
                        {
                            continue;
                        }
                        dst = sceIoOpen(picturePath, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
                        if (dst < 0)
                        {
                            continue;
                        }
                        while((ret = sceIoRead(src, buf, 256)))
                        {
                            sceIoWrite(dst, buf, ret);
                        }
                        sceIoClose(dst);
                        sceIoClose(src);
                    }
                }
                else
                {
                    if(pad.Buttons & PSP_CTRL_CIRCLE)
                    {
                        thumbFlag++;
                        if (thumbFlag > 2)
                        {
                            thumbFlag = 0;
                        }
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
                    else if(pad.Buttons & PSP_CTRL_SQUARE)
                    {
                        sceIoRemove(fname);
                        break;
                    }
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
            if ((padX < -s2ch.cfg.padCutoff) || (padX > s2ch.cfg.padCutoff))
            {
                startX += (padX)/4;
            }
            if ((padY < -s2ch.cfg.padCutoff) || (padY > s2ch.cfg.padCutoff))
            {
                startY += (padY)/4;
            }
        }
        if (thumbFlag)
        {
            if (thumbFlag == 1)
            {
                thumb = thumbW;
            }
            else
            {
                thumb = thumbH;
            }
            if (startX >= width / thumb - SCR_WIDTH)
            {
                startX = width / thumb - SCR_WIDTH;
            }
            if (startY >= height / thumb - SCR_HEIGHT)
            {
                startY = height / thumb - SCR_HEIGHT;
            }
            if (startX < 0)
            {
                startX = 0;
            }
            if (startY < 0)
            {
                startY = 0;
            }
            sceGuStart(GU_DIRECT,list);
            sceGuClearColor(GRAY);
            sceGuClear(GU_COLOR_BUFFER_BIT);
            sceGuTexMode(GU_PSM_8888,0,0,0);
            sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
            sceGuTexFilter(GU_NEAREST,GU_NEAREST);
            for (sx = 0; sx < width; sx += BUF_WIDTH)
            {
                dx = sx / thumb - startX;
                if ((width - sx) < BUF_WIDTH)
                {
                    sw = width - sx;
                }
                else
                {
                    sw = 512;
                }
                dw = sw / thumb;
                for (sy = 0; sy < height; sy += BUF_HEIGHT)
                {
                    dy = sy / thumb - startY;
                    if ((height - sy) < BUF_HEIGHT)
                    {
                        sh = height - sy;
                    }
                    else
                    {
                        sh = 512;
                    }
                    dh = sh / thumb;
                    sceGuTexImage(0, BUF_WIDTH, BUF_HEIGHT, bufWidth, img[0] + (int)sx + (int)sy * bufWidth);
                    psp2chBitBlt((int)sx, (int)sy, (int)sw, (int)sh, (int)dx, (int)dy, (int)dw, (int)dh);
                }
            }
            sceGuFinish();
            sceGuSync(0,0);
        }
        else
        {
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
            w = (width > SCR_WIDTH) ? SCR_WIDTH : width;
            h = (height > SCR_HEIGHT) ? SCR_HEIGHT : height;
            sceGuStart(GU_DIRECT,list);
            sceGuClearColor(GRAY);
            sceGuClear(GU_COLOR_BUFFER_BIT);
            sceGuCopyImage(GU_PSM_8888, startX, startY, w, h, bufWidth, img[0], 0, 0, BUF_WIDTH, framebuffer + 0x04000000);
            sceGuTexSync();
            sceGuFinish();
            sceGuSync(0,0);
        }
        if (menu)
        {
            if (rMenu)
            {
                pgMenuBar("　○ : PICTUREフォルダへコピー　");
            }
            else
            {
                pgMenuBar("　○ : 拡大縮小　　　× : 戻る　　　△ : メニューオン・オフ　　　□ : 削除");
            }
        }
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
}
