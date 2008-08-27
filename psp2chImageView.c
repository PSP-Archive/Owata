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
extern unsigned short winPixels[BUF_WIDTH*BUF_HEIGHT*2]; // pg.c
extern int preLine; // psp2chRes.c

#define MEM_SIZE 8*1024*1024
/*****************************
	矩形範囲を拡大縮小
*****************************/
#define SLICE_SIZE 64
void blt(void *src, TEX *tex, int sw, int sh, int dw, int dh)
{
	int i, j, dy;
	struct Vertex *vertices;
	int* p = src;

	dy = BUF_HEIGHT * dw / sw;

	sceGuStart(GU_DIRECT, list);
	sceGuDrawBufferList(GU_PSM_8888, framebuffer, BUF_WIDTH);
	sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
	sceGuClearColor(GU_RGBA(128,128,128,255));
    sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuTexMode(GU_PSM_8888, 0, 0, GU_FALSE);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexWrap(GU_CLAMP, GU_CLAMP);
	if (sw == dw)
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
		vertices[1].y = dh;
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
		vertices[1].y = dh;
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_COLOR_4444 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, NULL, vertices);
	}
	sceGuFinish();
	sceGuSync(0, GU_SYNC_FINISH);
}

/*****************************
メニュー処理
RGBAデータをVRAMに転送
*****************************/
void psp2chImageViewer(int* img[], int width, int height, char* fname, SceUID vpl)
{
    SceUID src, dst;
    int padX, padY;
    int ret, menu = 1, rMenu = 0;
    char picturePath[256], buf[256];
    char *p;
    double thumb, thumbW, thumbH;
    int thumbFlag = 0;
    int imgWH, imgHW, startX, startY, width2, height2, sx, sy;
	TEX tex;
	int **imgBuf = NULL;
	int originalWidth = width;

	// Gu転送のため1行を16バイト境界にそろえる
	tex.tb = (width + 15) & 0xFFFFFFF0;
	tex.w = BUF_WIDTH;
	tex.h = BUF_HEIGHT;
    startX = 0;
    startY = 0;
	// 幅の大きい画像は縮小する
	if (originalWidth > 1024)
	{
		int i, j, k;
		void* buf;
		int* imgbuf;
		int mip = originalWidth >> 10;
		mip++;
		
		i =  height / mip;
		if (sceKernelAllocateVpl(vpl, i, &buf, NULL) < 0)
		{
			return;
		}
		imgBuf = buf;
		if (sceKernelAllocateVpl(vpl, sizeof(int) * 1024 * i + 16, &buf, NULL) < 0)
		{
			return;
		}
		imgbuf = (int*)(((int)buf + 15) & 0xFFFFFFF0);
		for (j = 0; j < i; j++)
		{
			imgBuf[j] = imgbuf + (1024 * j);
			for (k = 0; k < width; k += mip)
			{
				imgBuf[j][k / mip] = img[j * mip][k];
			}
		}
		width /= mip;
		height /= mip;
		tex.tb = 1024;
	}
    width2 = width;
    height2 = height;
	thumb = 1.0;
    thumbW = (double)width / SCR_WIDTH; // 画面幅に合わせるための拡大縮小率
    imgWH = height / thumbW; // 画面幅に合わせたときの画像高さ
    thumbH = (double)height / SCR_HEIGHT; // 画面高さにあわせるための拡大縮小率
    imgHW = width / thumbH; // 画面高さにあわせたときの画像幅
	s2ch.oldPad.Buttons = PSP_CTRL_UP;
    while (s2ch.running)
    {
        if(sceCtrlPeekBufferPositive(&s2ch.pad, 1))
        {
            if(s2ch.pad.Buttons & PSP_CTRL_RTRIGGER)
            {
                rMenu = 1;
				pgPrintMenuBar("　○ : PICTUREフォルダへコピー　");
            }
            else
            {
                rMenu = 0;
                pgPrintMenuBar("　○ : 拡大縮小　　× : 戻る　　△ : メニューオン・オフ　　□ : 削除");
            }
            if (s2ch.pad.Buttons != s2ch.oldPad.Buttons)
            {
                s2ch.oldPad = s2ch.pad;
                if (rMenu)
                {
                    if(s2ch.pad.Buttons & PSP_CTRL_CIRCLE)
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
                    if(s2ch.pad.Buttons & PSP_CTRL_CIRCLE)
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
                    else if(s2ch.pad.Buttons & PSP_CTRL_CROSS)
                    {
                        break;
                    }
                    else if(s2ch.pad.Buttons & PSP_CTRL_TRIANGLE)
                    {
                        menu = menu ? 0 : 1;
                    }
                    else if(s2ch.pad.Buttons & PSP_CTRL_SQUARE)
                    {
                        sceIoRemove(fname);
                        break;
                    }
                }
            }
            if(s2ch.pad.Buttons & PSP_CTRL_UP)
            {
                startY -= 2;
            }
            if(s2ch.pad.Buttons & PSP_CTRL_DOWN)
            {
                startY += 2;
            }
            if(s2ch.pad.Buttons & PSP_CTRL_LEFT)
            {
                startX -= 2;
            }
            if(s2ch.pad.Buttons & PSP_CTRL_RIGHT)
            {
                startX += 2;
            }
            padX = s2ch.pad.Lx - 127;
            padY = s2ch.pad.Ly - 127;
            if ((padX < -s2ch.cfg.padCutoff) || (padX > s2ch.cfg.padCutoff))
            {
                startX += (padX)/4;
            }
            if ((padY < -s2ch.cfg.padCutoff) || (padY > s2ch.cfg.padCutoff))
            {
                startY += (padY)/4;
            }
        }
		// width : 元画像幅
		// height : 元画像高さ
		// sx.sy : 元画像における表示開始位置
		// width2 : 表示画像幅
		// height2 : 表示画像高さ
		// startX.startY : 表示画像における表示開始位置
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
		sx = thumb * startX;
		sy = thumb * startY;
		if (originalWidth > 1024)
		{
			blt(imgBuf[sy] + sx, &tex, width - sx, height - sy, width2 - startX, height2 - startY);
		}
		else
		{
			blt(img[sy] + sx, &tex, width - sx, height - sy, width2 - startX, height2 - startY);
		}
        if (menu)
        {
            pgCopyMenuBar();
        }
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
}

/*****************************
jpegファイルを読み込んで32ビットRGBAに変換
*****************************/
int psp2chImageViewJpeg(char* fname)
{
    FILE* infile;
    JSAMPARRAY img;
    JSAMPROW buf, imgbuf, tmp;
	void *mem;
    int width;
    int height;
    int i;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned char header[4];
	SceUID vpl;

	vpl = sceKernelCreateVpl("ImageVpl", PSP_MEMORY_PARTITION_USER, 0, MEM_SIZE + 256, NULL);
    if (vpl < 0)
    {
        return -1;
    }
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
        return -2;
    }
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
	// Gu転送のため1行を16バイト境界にそろえる
    width = (cinfo.output_width + 15) & 0xFFFFFFF0;
    height = cinfo.output_height;
    if (sceKernelAllocateVpl(vpl, sizeof(JSAMPROW) * height, &mem, NULL) < 0)
	{
		sceKernelDeleteVpl(vpl);
        fclose(infile);
		return -3;
	}
	img = mem;
    if (sceKernelAllocateVpl(vpl, sizeof(JSAMPLE) * 3 * width, &mem, NULL) < 0)
	{
		sceKernelDeleteVpl(vpl);
        fclose(infile);
		return -4;
	}
	buf = mem;
    if (sceKernelAllocateVpl(vpl, sizeof(JSAMPLE) * 4 * width * height + 16, &mem, NULL) < 0)
	{
		sceKernelDeleteVpl(vpl);
        fclose(infile);
		return -5;
	}
	imgbuf = (JSAMPROW)(((int)mem + 15) & 0xFFFFFFF0);
    tmp = imgbuf;
    for (i = 0; i < height; i++ )
    {
        img[i] = &tmp[i * width * 4];
    }
    // RGBカラーをRGBAに
    if (cinfo.out_color_components == 3)
    {
        while(cinfo.output_scanline < cinfo.output_height)
        {
            jpeg_read_scanlines(&cinfo, &buf, 1);
            for (i = 0; i < cinfo.output_width; i++)
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
            for (i = 0; i < cinfo.output_width; i++)
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
        jpeg_destroy_decompress(&cinfo);
		sceKernelDeleteVpl(vpl);
        fclose(infile);
        return -6;
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    psp2chImageViewer((int**)img, cinfo.output_width, height, fname, vpl);
	sceKernelDeleteVpl(vpl);
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
    unsigned long width, height, width2;
    int bit_depth, color_type, interlace_type;
    png_bytepp img;
    png_bytep imgbuf;
	void* mem;
    int i;
    unsigned char header[8];
	SceUID vpl;

	vpl = sceKernelCreateVpl("ImageVpl", PSP_MEMORY_PARTITION_USER, 0, MEM_SIZE + 256, NULL);
    if (vpl < 0)
    {
        return -1;
    }
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
	// Gu転送のため1行を16バイト境界にそろえる
	width2 = (width + 15) & 0xFFFFFFF0;
    if (sceKernelAllocateVpl(vpl, height * sizeof(png_bytep), &mem, NULL) < 0)
	{
		sceKernelDeleteVpl(vpl);
        fclose(infile);
		return -3;
	}
	img = mem;
    if (sceKernelAllocateVpl(vpl, png_get_rowbytes(png_ptr, info_ptr) * width2 + 16, &mem, NULL) < 0)
	{
		sceKernelDeleteVpl(vpl);
        fclose(infile);
		return -4;
	}
	imgbuf = (png_bytep)(((int)mem + 15) & 0xFFFFFFF0);
    for (i = 0; i < height; i++)
    {
        img[i] = &imgbuf[i * width2 * 4];
    }
    png_read_image(png_ptr, img);
    png_read_end(png_ptr, end_info);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(infile);
    psp2chImageViewer((int**)img, (int)width, (int)height, fname, vpl);
	sceKernelDeleteVpl(vpl);
    preLine = -2;
    return 0;
}

/*****************************
BMPファイルを読み込んで32ビットRGBAに変換
*****************************/
int psp2chImageViewBmp(char* fname)
{
    FILE* infile;
    int i, j, y, width, height, len;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    unsigned char **img, *imgbuf, *buf, *tmp;
	void* mem;
	SceUID vpl;

	vpl = sceKernelCreateVpl("ImageVpl", PSP_MEMORY_PARTITION_USER, 0, MEM_SIZE + 256, NULL);
    if (vpl < 0)
    {
        return -1;
    }
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
	// Gu転送のため1行を16バイト境界にそろえる
	width = (bi.biWidth + 15) & 0xFFFFFFF0;
    len = bi.biWidth * bi.biBitCount / 8;
    len += (4 - (len & 3)) & 3;
    if (sceKernelAllocateVpl(vpl, sizeof(unsigned char*) * height, &mem, NULL) < 0)
	{
		sceKernelDeleteVpl(vpl);
        fclose(infile);
		return -3;
	}
	img = mem;
    if (sceKernelAllocateVpl(vpl, sizeof(unsigned char) * len, &mem, NULL) < 0)
	{
		sceKernelDeleteVpl(vpl);
        fclose(infile);
		return -4;
	}
	buf = mem;
    if (sceKernelAllocateVpl(vpl, sizeof(unsigned char) * 4 * width * height + 16, &mem, NULL) < 0)
	{
		sceKernelDeleteVpl(vpl);
        fclose(infile);
		return -5;
	}
	imgbuf = (unsigned char*)(((int)mem + 15) & 0xFFFFFFF0);
    tmp = imgbuf;
    for (i = 0; i < height; i++ )
    {
        img[i] = &tmp[i * width * 4];
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
		sceKernelDeleteVpl(vpl);
        fclose(infile);
        return -1;
    }
    fclose(infile);
    psp2chImageViewer((int**)img, bi.biWidth, height, fname, vpl);
	sceKernelDeleteVpl(vpl);
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
    int i, j, Size, Row, Col, Width, Height, Count, ExtCode, width2;
    GifFileType *GifFile;
    GifRowType *ScreenBuffer, ImgBuf, GifRow;
    GifRecordType RecordType;
    GifByteType *Extension;
    GifColorType *ColorMapEntry;
    ColorMapObject *ColorMap;
    unsigned char **img, *buf, *BufferP;
	void* mem;
	SceUID vpl;

	vpl = sceKernelCreateVpl("ImageVpl", PSP_MEMORY_PARTITION_USER, 0, MEM_SIZE + 256, NULL);
    if (vpl < 0)
    {
        return -1;
    }
    if ((GifFile = DGifOpenFileName(fname)) == NULL)
    {
        return -1;
    }
    Size = GifFile->SWidth * sizeof(GifPixelType);/* Size in bytes one row.*/
	// Gu転送のため1行を16バイト境界にそろえる
	width2 = (GifFile->SWidth + 15) & 0xFFFFFFF0;
    if (sceKernelAllocateVpl(vpl, GifFile->SHeight * sizeof(GifRowType *), &mem, NULL) < 0)
	{
		sceKernelDeleteVpl(vpl);
        DGifCloseFile(GifFile);
		return -3;
	}
	ScreenBuffer = mem;
    if (sceKernelAllocateVpl(vpl, Size * GifFile->SHeight, &mem, NULL) < 0)
	{
		sceKernelDeleteVpl(vpl);
        DGifCloseFile(GifFile);
		return -4;
	}
	ImgBuf = mem;
    if (sceKernelAllocateVpl(vpl, sizeof(unsigned char*) * GifFile->SHeight, &mem, NULL) < 0)
	{
		sceKernelDeleteVpl(vpl);
        DGifCloseFile(GifFile);
		return -5;
	}
	img = mem;
    if (sceKernelAllocateVpl(vpl, 4 * width2 * GifFile->SHeight + 16, &mem, NULL) < 0)
	{
		sceKernelDeleteVpl(vpl);
        DGifCloseFile(GifFile);
		return -6;
	}
	buf = (unsigned char*)(((int)mem + 15) & 0xFFFFFFF0);
    for (i = 0; i < GifFile->SHeight; i++)
    {
        ScreenBuffer[i] = &ImgBuf[i * Size];
        img[i] = &buf[i * 4 * width2];
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
			sceKernelDeleteVpl(vpl);
            DGifCloseFile(GifFile);
            return -1;
        }
        switch (RecordType)
        {
	    case IMAGE_DESC_RECORD_TYPE:
            if (DGifGetImageDesc(GifFile) == GIF_ERROR)
            {
				sceKernelDeleteVpl(vpl);
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
				sceKernelDeleteVpl(vpl);
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
							sceKernelDeleteVpl(vpl);
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
						sceKernelDeleteVpl(vpl);
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
				sceKernelDeleteVpl(vpl);
                DGifCloseFile(GifFile);
                return -1;
            }
            while (Extension != NULL)
            {
                if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR)
                {
					sceKernelDeleteVpl(vpl);
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
		sceKernelDeleteVpl(vpl);
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
    psp2chImageViewer((int**)img, GifFile->SWidth, GifFile->SHeight, fname, vpl);
    DGifCloseFile(GifFile);
	sceKernelDeleteVpl(vpl);
    preLine = -2;
    return 0;
}
