TARGET = owata
OBJS = src/cp932.o src/intraFont.o src/main.o src/pg.o src/psp2ch.o src/psp2chFavorite.o src/psp2chForm.o \
src/psp2chIni.o src/psp2chIta.o src/psp2chMenu.o src/psp2chNet.o src/psp2chRes.o src/psp2chResWindow.o \
src/psp2chSearch.o src/psp2chThread.o src/pspdialogs.o src/psphtmlviewer.o \
src/libCat/Cat_Network.o src/libCat/Cat_Resolver.o \
src/giflib/dgif_lib.o src/giflib/gif_err.o src/giflib/gifalloc.o

INCDIR =
CFLAGS = -Wall -O2
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
BUILD_PRX = 1
PSP_FW_VERSION=352
LIBS= -lpsppower -lpspgu -lpspssl -lpsphttp -lpspwlan -ljpeg -lpng -lz -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = 人生ｵﾜﾀ＼(^o^)／
PSP_EBOOT_ICON = icon.png

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

