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
    // バッファの幅を16バイトアラインに
    bufWidth = width + (16 - width % 16) % 16;
    height = cinfo.output_height;
    img = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * height);
    if (!img)
    {
        fclose(infile);
        return;
    }
    imgbuf = (JSAMPROW)calloc(sizeof(JSAMPLE), 4 * bufWidth * height);
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
    for (i = 0; i < height; i++ )
    {
        img[i] = &tmp[i * bufWidth * 4];
    }
    // RGBカラーをRGBAに
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
    // グレースケールをRGBAに
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
    psp2chImageViewer((int**)img, width, height, fname);
    free(imgbuf);
    free(img);
    preLine = -2;
}

void psp2chImageViewPng(char* fname)
{
    FILE* infile;
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;
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
    if (png_sig_cmp(header, 0, 8))
    {
        fclose(infile);
        return;
    }
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    end_info = png_create_info_struct(png_ptr);
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(infile);
        return;
    }
    png_init_io(png_ptr, infile);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
    // バッファの幅を16バイトアラインに
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
    if (!img)
    {
        fclose(infile);
        return;
    }
    imgbuf = (png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr) * bufWidth);
    if (!imgbuf)
    {
        free(img);
        fclose(infile);
        return;
    }
    for (i = 0; i < height; i++)
    {
        img[i] = &imgbuf[i * bufWidth * 4];
    }
    png_read_image(png_ptr, img);
    png_read_end(png_ptr, end_info);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(infile);
    psp2chImageViewer((int**)img, (int)width, (int)height, fname);
    free(imgbuf);
    free(img);
    preLine = -2;
}

void psp2chImageViewer(int* img[], int width, int height, char* fname)
{
    int w, h, startX, startY, width2, height2;
    SceCtrlData pad;
    SceCtrlData oldPad;
    unsigned int* vptr0;
    unsigned int* vptr;
    double thumb, thumbW, thumbH;
    int thumbFlag = 0;
    int menu = 1, rMenu = 0;
    int padX, padY;
    char picturePath[256], buf[256];
    char *p;
    SceUID src, dst;
    int i, j, ret, dx, dy;
    int *imgW = NULL, *imgH = NULL, *img2, imgWH, imgHW;

    width2 = width;
    height2 = height;
    img2 = img[0];
    startX = 0;
    startY = 0;
    thumbW = (double)width / SCR_WIDTH;
    imgWH = height / thumbW;
    thumbH = (double)height / SCR_HEIGHT;
    imgHW = width / thumbH;
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
                        if (thumbFlag == 1)
                        {
                            if (imgW == NULL)
                            {
                                // 画面幅用画像作成
                                pgMenuBar("画面の幅に合わせた画像を作成しています");
                                sceDisplayWaitVblankStart();
                                framebuffer = sceGuSwapBuffers();
                                imgW = (int*)malloc(sizeof(int) * SCR_WIDTH * imgWH);
                                if (imgW == NULL)
                                {
                                    psp2chErrorDialog("memorry error\nimgW");
                                    break;
                                }
                                for (i = 0; i < imgWH; i++)
                                {
                                    dy = i * thumbW;
                                    for (j = 0; j < SCR_WIDTH; j++)
                                    {
                                        dx = j * thumbW;
                                        imgW[i * SCR_WIDTH + j] = img[dy][dx];
                                    }
                                }
                            }
                            thumb = thumbW;
                            width2 = SCR_WIDTH;
                            height2 = imgWH;
                            img2 = imgW;
                        }
                        else if (thumbFlag == 2)
                        {
                            if (imgH == NULL)
                            {
                                // 画面高さ用画像作成
                                pgMenuBar("画面の高さに合わせた画像を作成しています");
                                sceDisplayWaitVblankStart();
                                framebuffer = sceGuSwapBuffers();
                                imgH = (int*)malloc(sizeof(int) * imgHW * SCR_HEIGHT);
                                if (imgH == NULL)
                                {
                                    psp2chErrorDialog("memorry error\nimgH");
                                    break;
                                }
                                for (i = 0; i < SCR_HEIGHT; i++)
                                {
                                    dy = i * thumbH;
                                    for (j = 0; j < imgHW; j++)
                                    {
                                        dx = j * thumbH;
                                        imgH[i * imgHW + j] = img[dy][dx];
                                    }
                                }
                            }
                            thumb = thumbH;
                            width2 = imgHW;
                            height2 = SCR_HEIGHT;
                            img2 = imgH;
                        }
                        else
                        {
                            thumb = 1;
                            width2 = width;
                            height2 = height;
                            img2 = img[0];
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
                startY -= 2;
            }
            if(pad.Buttons & PSP_CTRL_DOWN)
            {
                startY += 2;
            }
            if(pad.Buttons & PSP_CTRL_LEFT)
            {
                startX -= 2;
            }
            if(pad.Buttons & PSP_CTRL_RIGHT)
            {
                startX += 2;
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
        if (startX >= width2 - SCR_WIDTH)
        {
            startX = width2 - SCR_WIDTH;
        }
        if (startY >= height2 - SCR_HEIGHT)
        {
            startY = height2 - SCR_HEIGHT;
        }
        if (startX < 0)
        {
            startX = 0;
        }
        if (startY < 0)
        {
            startY = 0;
        }
        vptr0 = framebuffer + 0x04000000;
        for (i = 0, h = 0; h < height2 && h < SCR_HEIGHT; i++)
        {
            if (i < startY)
            {
                continue;
            }
            vptr = vptr0;
            for (j = 0, w = 0; w < width2 && w < SCR_WIDTH; j++)
            {
                if (j < startX)
                {
                    continue;
                }
                if (thumbFlag)
                {
                    *vptr++ = img2[i * width2 + j];
                }
                else
                {
                    *vptr++ = img[i][j];
                }
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
        if (menu)
        {
            if (rMenu)
            {
                pgMenuBar("　○ : PICTUREフォルダへコピー　");
            }
            else
            {
                pgMenuBar("　○ : 拡大縮小　　× : 戻る　　△ : メニューオン・オフ　　□ : 削除");
            }
        }
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    free(imgW);
    free(imgH);
}
