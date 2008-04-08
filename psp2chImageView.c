/*
* $Id$
*/

#include <stdio.h>
#include <malloc.h>
#include <jpeglib.h>
#include <png.h>
#include "giflib/gif_lib.h"
#include "psp2ch.h"
#include "pg.h"
#include "psp2chImageView.h"

extern S_2CH s2ch; //psp2ch.c
extern unsigned int list[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern unsigned int pixels[BUF_WIDTH*BUF_HEIGHT]; // pg.c
extern int preLine; // psp2chRes.c

/*****************************
jpegファイルを読み込んで32ビットRGBAに変換
*****************************/
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
    bufWidth = width + ((16 - (width & 0x0F)) & 0x0F);
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

/*****************************
PNGファイルを読み込んで32ビットRGBAに変換
*****************************/
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
    bufWidth = width + ((16 - (width & 0x0F)) & 0x0F);
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

/*****************************
BMPファイルを読み込んで32ビットRGBAに変換
*****************************/
void psp2chImageViewBmp(char* fname)
{
    FILE* infile;
    int i, j, y, bufWidth, height, len;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    unsigned char **img, *imgbuf, *buf, *tmp;

    infile = fopen(fname, "rb" );
    if (!infile)
    {
        return;
    }
    if (fread(&bf, sizeof(BITMAPFILEHEADER), 1, infile) != 1)
    {
        fclose(infile);
        return;
    }
    // BITMAP 認識文字 "BM"
    if (memcmp(bf.bfType, "BM", 2) != 0)
    {
        fclose(infile);
        return;
    }
    if (fread(&bi, sizeof(BITMAPINFOHEADER), 1, infile) != 1)
    {
        fclose(infile);
        return;
    }
    // 非圧縮のみ
    if (bi.biCompression)
    {
        fclose(infile);
        return;
    }
    if (bi.biHeight < 0)
    {
        height = -bi.biHeight;
    }
    else
    {
        height = bi.biHeight;
    }
    // バッファの幅を16バイトアラインに
    bufWidth = bi.biWidth + ((16 - (bi.biWidth & 0x0F)) & 0x0F);
    img = (unsigned char**)malloc(sizeof(unsigned char*) * height);
    if (!img)
    {
        fclose(infile);
        return;
    }
    imgbuf = (unsigned char*)calloc(sizeof(unsigned char), 4 * bufWidth * height);
    if (!imgbuf)
    {
        free(img);
        fclose(infile);
        return;
    }
    len = bi.biWidth * bi.biBitCount / 8;
    len += (4 - (len & 3)) & 3;
    buf = (unsigned char*)calloc(sizeof(unsigned char), len);
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
    fseek(infile, bf.bfOffBits, SEEK_SET);
    // 24ビットBMPをRGBAに
    if (bi.biBitCount == 24)
    {
        for (j = 0; j < height; j++)
        {
            if (bi.biHeight < 0)
            {
                y = j;
            }
            else
            {
                y = height - j - 1;
            }
            fread(buf, 1, len, infile);
            for (i = 0; i < bi.biWidth; i++)
            {
                img[y][i * 4 + 0] = buf[i * 3 + 2];
                img[y][i * 4 + 1] = buf[i * 3 + 1];
                img[y][i * 4 + 2] = buf[i * 3 + 0];
                img[y][i * 4 + 3] = 0xFF;
            }
        }
    }
    // 32ビットBMPをRGBAに
    else if (bi.biBitCount == 32)
    {
        for (j = 0; j <height; j++)
        {
            if (bi.biHeight < 0)
            {
                y = j;
            }
            else
            {
                y = height - j - 1;
            }
            fread(buf, 1, len, infile);
            for (i = 0; i < bi.biWidth; i++)
            {
                img[y][i * 4 + 0] = buf[i * 4 + 2];
                img[y][i * 4 + 1] = buf[i * 4 + 1];
                img[y][i * 4 + 2] = buf[i * 4 + 0];
                img[y][i * 4 + 3] = 0xFF;
            }
        }
    }
    // 未対応
    else
    {
        free(buf);
        free(imgbuf);
        free(img);
        fclose(infile);
        return;
    }
    free(buf);
    fclose(infile);
    psp2chImageViewer((int**)img, bi.biWidth, height, fname);
    free(imgbuf);
    free(img);
    preLine = -2;
}

/*****************************
GIFファイルを読み込んで32ビットRGBAに変換
*****************************/
void psp2chImageViewGif(char* fname)
{
    int InterlacedOffset[] = { 0, 4, 2, 1 }; /* The way Interlaced image should. */
    int InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */
    int i, j, Size, Row, Col, Width, Height, Count, ExtCode;
    GifFileType *GifFile;
    GifRowType *ScreenBuffer, ImgBuf, GifRow;
    GifRecordType RecordType;
    GifByteType *Extension;
    GifColorType *ColorMapEntry;
    ColorMapObject *ColorMap;
    unsigned char **img, *buf, *BufferP;

    GifFile = DGifOpenFileName(fname);
    if ((ScreenBuffer = (GifRowType *)malloc(GifFile->SHeight * sizeof(GifRowType *))) == NULL)
    {
        DGifCloseFile(GifFile);
        return;
    }
    Size = GifFile->SWidth * sizeof(GifPixelType);/* Size in bytes one row.*/
    if ((ImgBuf = (GifRowType)malloc(Size * GifFile->SHeight)) == NULL)
    {
        free(ScreenBuffer);
        DGifCloseFile(GifFile);
        return;
    }
    if ((img = (unsigned char**)malloc(sizeof(unsigned char*) * GifFile->SHeight)) == NULL)
    {
        free(ImgBuf);
        free(ScreenBuffer);
        DGifCloseFile(GifFile);
        return;
    }
    if ((buf = (unsigned char*)malloc(4 * GifFile->SWidth * GifFile->SHeight)) == NULL)
    {
        free(img);
        free(ImgBuf);
        free(ScreenBuffer);
        DGifCloseFile(GifFile);
        return;
    }
    for (i = 0; i < GifFile->SHeight; i++)
    {
        ScreenBuffer[i] = &ImgBuf[i * Size];
        img[i] = &buf[i * 4 * GifFile->SWidth];
    }
    for (i = 0; i < GifFile->SWidth; i++)  /* Set its color to BackGround. */
    {
        ScreenBuffer[0][i] = GifFile->SBackGroundColor;
    }
    for (i = 1; i < GifFile->SHeight; i++)
    {
        memcpy(ScreenBuffer[i], ScreenBuffer[0], Size);
    }
    /* Scan the content of the GIF file and load the image(s) in: */
    do
    {
        if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR)
        {
            free(buf);
            free(img);
            free(ImgBuf);
            free(ScreenBuffer);
            DGifCloseFile(GifFile);
            return;
        }
        switch (RecordType)
        {
	    case IMAGE_DESC_RECORD_TYPE:
            if (DGifGetImageDesc(GifFile) == GIF_ERROR)
            {
                free(buf);
                free(img);
                free(ImgBuf);
                free(ScreenBuffer);
                DGifCloseFile(GifFile);
                return;
            }
            Row = GifFile->Image.Top; /* Image Position relative to Screen. */
            Col = GifFile->Image.Left;
            Width = GifFile->Image.Width;
            Height = GifFile->Image.Height;
            if (GifFile->Image.Left + GifFile->Image.Width > GifFile->SWidth ||
               GifFile->Image.Top + GifFile->Image.Height > GifFile->SHeight)
            {
                free(buf);
                free(img);
                free(ImgBuf);
                free(ScreenBuffer);
                DGifCloseFile(GifFile);
                return;
            }
            if (GifFile->Image.Interlace) {
                /* Need to perform 4 passes on the images: */
                for (Count = i = 0; i < 4; i++)
                {
                    for (j = Row + InterlacedOffset[i]; j < Row + Height; j += InterlacedJumps[i])
                    {
                        if (DGifGetLine(GifFile, &ScreenBuffer[j][Col], Width) == GIF_ERROR)
                        {
                            free(buf);
                            free(img);
                            free(ImgBuf);
                            free(ScreenBuffer);
                            DGifCloseFile(GifFile);
                            return;
                        }
                    }
                }
            }
            else {
                for (i = 0; i < Height; i++)
                {
                    if (DGifGetLine(GifFile, &ScreenBuffer[Row++][Col], Width) == GIF_ERROR)
                    {
                        free(buf);
                        free(img);
                        free(ImgBuf);
                        free(ScreenBuffer);
                        DGifCloseFile(GifFile);
                        return;
                    }
                }
            }
            break;
	    case EXTENSION_RECORD_TYPE:
            /* Skip any extension blocks in file: */
            if (DGifGetExtension(GifFile, &ExtCode, &Extension) == GIF_ERROR)
            {
                free(buf);
                free(img);
                free(ImgBuf);
                free(ScreenBuffer);
                DGifCloseFile(GifFile);
                return;
            }
            while (Extension != NULL)
            {
                if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR)
                {
                    free(buf);
                    free(img);
                    free(ImgBuf);
                    free(ScreenBuffer);
                    DGifCloseFile(GifFile);
                    return;
                }
            }
            break;
	    case TERMINATE_RECORD_TYPE:
            break;
	    default:		    /* Should be traps by DGifGetRecordType. */
            break;
        }
    } while (RecordType != TERMINATE_RECORD_TYPE);
    ColorMap = (GifFile->Image.ColorMap ? GifFile->Image.ColorMap : GifFile->SColorMap);
    if (ColorMap == NULL)
    {
        free(buf);
        free(img);
        free(ImgBuf);
        free(ScreenBuffer);
        DGifCloseFile(GifFile);
        return;
    }
    for (i = 0; i < GifFile->SHeight; i++) {
        GifRow = ScreenBuffer[i];
        BufferP = img[i];
        for (j = 0; j < GifFile->SWidth; j++) {
            ColorMapEntry = &ColorMap->Colors[GifRow[j]];
            *BufferP++ = ColorMapEntry->Red;
            *BufferP++ = ColorMapEntry->Green;
            *BufferP++ = ColorMapEntry->Blue;
            *BufferP++ = 0xFF;
        }
    }
    psp2chImageViewer((int**)img, GifFile->SWidth, GifFile->SHeight, fname);
    DGifCloseFile(GifFile);
    free(buf);
    free(img);
    free(ImgBuf);
    free(ScreenBuffer);
    preLine = -2;
}

/*****************************
メニュー処理
BMPデータをVRAMに転送
*****************************/
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
