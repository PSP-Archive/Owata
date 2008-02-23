TARGET_LIB = libCat.a
OBJS =\
	Cat_Network.o \
	Cat_Resolver.o

#USE_PSPSDK_LIBC = 1

INCDIR =
CFLAGS = -O6 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS =

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
