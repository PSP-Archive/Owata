TARGET = owata
OBJS = cp932.o intraFont.o main.o pg.o psp2ch.o psp2chFavorite.o psp2chForm.o \
psp2chImageView.o psp2chIni.o psp2chIta.o psp2chMenu.o psp2chNet.o psp2chRes.o \
psp2chResWindow.o psp2chSearch.o psp2chThread.o psp2chTinyBrowser.o pspdialogs.o \
libCat/Cat_Network.o libCat/Cat_Resolver.o

INCDIR =
CFLAGS = -G0 -Wall -O2
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
BUILD_PRX = 1
PSP_FW_VERSION=352
LIBS= -lpsppower -lpspgu -lpspwlan -ljpeg -lpng -lz -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = 人生ｵﾜﾀ＼(^o^)／
PSP_EBOOT_ICON = icon.png

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

