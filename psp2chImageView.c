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
int psp2chImageViewJpeg(char* fname)
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
        return -1;
    }
    // 一応ヘッダのチェック
    fread(header, 1, 2, infile);
    fseek(infile, 0, SEEK_SET);
    if (header[0] != 0xFF || header[1] != 0xD8)
    {
        fclose(infile);
        return -1;
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
        return -1;
    }
    imgbuf = (JSAMPROW)calloc(sizeof(JSAMPLE), 4 * bufWidth * height);
    if (!imgbuf)
    {
        free(img);
        fclose(infile);
        return -1;
    }
    buf = (JSAMPROW)calloc(sizeof(JSAMPLE), 3 * bufWidth);
    if (!buf)
    {
        free(imgbuf);
        free(img);
        fclose(infile);
        return -1;
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
        return -1;
    }
    free(buf);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    psp2chImageViewer((int**)img, width, height, fname);
    free(imgbuf);
    free(img);
    preLine = -2;
    return 0;
}

/*****************************
PNGファイルを読み込んで32ビットRGBAに変換
*****************************/
int psp2chImageViewPng(char* fname)
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
        return -1;
    }
    fread(header, 1, 8, infile);
    // PNGチェック
    if (png_sig_cmp(header, 0, 8))
    {
        fclose(infile);
        return -1;
    }
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    end_info = png_create_info_struct(png_ptr);
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(infile);
        return -1;
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
        return -1;
    }
    imgbuf = (png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr) * bufWidth);
    if (!imgbuf)
    {
        free(img);
        fclose(infile);
        return -1;
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
    return 0;
}

/*****************************
BMPファイルを読み込んで32ビットRGBAに変換
*****************************/
int psp2chImageViewBmp(char* fname)
{
    FILE* infile;
    int i, j, y, bufWidth, height, len;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    unsigned char **img, *imgbuf, *buf, *tmp;

    infile = fopen(fname, "rb" );
    if (!infile)
    {
        return -1;
    }
    if (fread(&bf, sizeof(BITMAPFILEHEADER), 1, infile) != 1)
    {
        fclose(infile);
        return -1;
    }
    // BITMAP 認識文字 "BM"
    if (memcmp(bf.bfType, "BM", 2) != 0)
    {
        fclose(infile);
        return -1;
    }
    if (fread(&bi, sizeof(BITMAPINFOHEADER), 1, infile) != 1)
    {
        fclose(infile);
        return -1;
    }
    // 非圧縮のみ
    if (bi.biCompression)
    {
        fclose(infile);
        return -1;
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
        return -1;
    }
    imgbuf = (unsigned char*)calloc(sizeof(unsigned char), 4 * bufWidth * height);
    if (!imgbuf)
    {
        free(img);
        fclose(infile);
        return -1;
    }
    len = bi.biWidth * bi.biBitCount / 8;
    len += (4 - (len & 3)) & 3;
    buf = (unsigned char*)calloc(sizeof(unsigned char), len);
    if (!buf)
    {
        free(imgbuf);
        free(img);
        fclose(infile);
        return -1;
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
        return -1;
    }
    free(buf);
    fclose(infile);
    psp2chImageViewer((int**)img, bi.biWidth, height, fname);
    free(imgbuf);
    free(img);
    preLine = -2;
    return 0;
}

/*****************************
GIFファイルを読み込んで32ビットRGBAに変換
*****************************/
int psp2chImageViewGif(char* fname)
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

    if ((GifFile = DGifOpenFileName(fname)) == NULL)
    {
        return -1;
    }
    if ((ScreenBuffer = (GifRowType *)malloc(GifFile->SHeight * sizeof(GifRowType *))) == NULL)
    {
        DGifCloseFile(GifFile);
        return -1;
    }
    Size = GifFile->SWidth * sizeof(GifPixelType);/* Size in bytes one row.*/
    if ((ImgBuf = (GifRowType)malloc(Size * GifFile->SHeight)) == NULL)
    {
        free(ScreenBuffer);
        DGifCloseFile(GifFile);
        return -1;
    }
    if ((img = (unsigned char**)malloc(sizeof(unsigned char*) * GifFile->SHeight)) == NULL)
    {
        free(ImgBuf);
        free(ScreenBuffer);
        DGifCloseFile(GifFile);
        return -1;
    }
    if ((buf = (unsigned char*)malloc(4 * GifFile->SWidth * GifFile->SHeight)) == NULL)
    {
        free(img);
        free(ImgBuf);
        free(ScreenBuffer);
        DGifCloseFile(GifFile);
        return -1;
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
            return -1;
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
                return -1;
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
                return -1;
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
                            return -1;
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
                        return -1;
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
                return -1;
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
                    return -1;
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
        return -1;
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
    return 0;
}

/*--------------------------------------------------------
	矩形範囲を拡大縮小
--------------------------------------------------------*/
#define SLICE_SIZE 64
void blt(void *src, TEX *tex, RECT *src_rect, double thumb)
{
	int i, j, sw, dw, sh, dh, dy;
	struct Vertex *vertices;
	int* p = src;

	sw = src_rect->right;
	dw = (double)sw / thumb;
	sh = src_rect->bottom;
	dh = (double)sh / thumb;
	dy = (double)BUF_HEIGHT / thumb;

	sceGuStart(GU_DIRECT, list);
	sceGuDrawBufferList(GU_PSM_8888, framebuffer, BUF_WIDTH);
	sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
	sceGuClearColor(RGB8888(128,128,128,255));
    sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuTexMode(GU_PSM_8888, 0, 0, GU_FALSE);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	if (thumb == 1.0)
		sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	else
		sceGuTexFilter(GU_LINEAR, GU_LINEAR);
	i = 0;
	/* textureをSLICE_SIZE * BUF_HEIGHTに切り取ってループ */
	while (sh > BUF_HEIGHT)
	{
		for (j = 0; (j + SLICE_SIZE) < sw; j = j + SLICE_SIZE)
		{
			sceGuTexImage(0, tex->w, tex->h, tex->tb, p + j);
			vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));
			vertices[0].u = 0;
			vertices[0].v = 0;
			vertices[0].x = j * dw / sw;
			vertices[0].y = dy * i;
			vertices[1].u = SLICE_SIZE;
			vertices[1].v = BUF_HEIGHT;
			vertices[1].x = (j + SLICE_SIZE) * dw / sw;
			vertices[1].y = dy * (i + 1);
			sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_COLOR_4444 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, NULL, vertices);
		}
		if (j < sw)
		{
			sceGuTexImage(0, tex->w, tex->h, tex->tb, p + j);
			vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));
			vertices[0].u = 0;
			vertices[0].v = 0;
			vertices[0].x = j * dw / sw;
			vertices[0].y = dy * i;
			vertices[1].u = sw - j;
			vertices[1].v = BUF_HEIGHT;
			vertices[1].x = dw;
			vertices[1].y = dy * (i + 1);
			sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_COLOR_4444 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, NULL, vertices);
		}
		sh -= BUF_HEIGHT;
		p += tex->tb * BUF_HEIGHT;
		i++;
	}
		for (j = 0; (j + SLICE_SIZE) < sw; j = j + SLICE_SIZE)
		{
			sceGuTexImage(0, tex->w, tex->h, tex->tb, p + j);
			vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));
			vertices[0].u = 0;
			vertices[0].v = 0;
			vertices[0].x = j * dw / sw;
			vertices[0].y = dy * i;
			vertices[1].u = SLICE_SIZE;
			vertices[1].v = sh;
			vertices[1].x = (j + SLICE_SIZE) * dw / sw;
			vertices[1].y = dy * i + sh * dw / sw;
			sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_COLOR_4444 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, NULL, vertices);
		}
		if (j < sw)
		{
			sceGuTexImage(0, tex->w, tex->h, tex->tb, p + j);
			vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));
			vertices[0].u = 0;
			vertices[0].v = 0;
			vertices[0].x = j * dw / sw;
			vertices[0].y = dy * i;
			vertices[1].u = sw - j;
			vertices[1].v = sh;
			vertices[1].x = dw;
			vertices[1].y = dy * i + sh * dw / sw;
			sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_COLOR_4444 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, NULL, vertices);
		}
	sceGuFinish();
	sceGuSync(0, GU_SYNC_FINISH);
}

/*****************************
メニュー処理
RGBAデータをVRAMに転送
*****************************/
void psp2chImageViewer(int* img[], int width, int height, char* fname)
{
    SceCtrlData pad;
    SceCtrlData oldPad;
    SceUID src, dst;
    int padX, padY;
    int ret, menu = 1, rMenu = 0;
    char picturePath[256], buf[256];
    char *p;
    double thumb, thumbW, thumbH;
    int thumbFlag = 0;
    int imgWH, imgHW, startX, startY, width2, height2, sx, sy;
	RECT srcRect;
	TEX tex;

    startX = 0;
    startY = 0;
    width2 = width;
    height2 = height;
	thumb = 1.0;
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
					pgPrintMenuBar("　○ : PICTUREフォルダへコピー　");
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
                            thumb = thumbW;
                            width2 = SCR_WIDTH;
                            height2 = imgWH;
                        }
                        else if (thumbFlag == 2)
                        {
                            thumb = thumbH;
                            width2 = imgHW;
                            height2 = SCR_HEIGHT;
                        }
                        else
                        {
                            thumb = 1.0;
                            width2 = width;
                            height2 = height;
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
                pgPrintMenuBar("　○ : 拡大縮小　　× : 戻る　　△ : メニューオン・オフ　　□ : 削除");
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
		sx = startX * thumb;
		sy = startY * thumb;
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = width - sx;
		srcRect.bottom = height - sy;
		tex.w = width;
		tex.h = height;
		tex.tb = width;
		blt(img[0]+sx+sy*width, &tex, &srcRect, thumb);
        if (menu)
        {
            pgCopyMenuBar();
        }
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
}
