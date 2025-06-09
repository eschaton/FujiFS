#
#	Makefile
#
#	Part of the Fuji Filesystem.
#	Copyright © 2025 Base Hit Ventures LLC. All rights reserved.
#	Based on FatFs, See file LICENSE.txt for details.
#

# Use Fuji toolchain.
CC = fuji-tool.sh clang
LD = fuji-tool.sh ld.lld

# Enable make DEBUG=1 to build for debugging.
CFLAGS_DEBUG_0 = -Os -DNDEBUG=1
CFLAGS_DEBUG_1 = -O0 -g -DDEBUG=1
CFLAGS_DEBUG_ = $(CFLAGS_DEBUG_0)

FUJI_SDKROOT = /opt/Fuji/Source/Fuji.sdk
CFLAGS_FUJI = \
	--target=armv8m.main-unknown-none-eabihf \
	-mfloat-abi=hard \
	--sysroot $(FUJI_SDKROOT) \
	-Xclang -internal-isystem -Xclang $(FUJI_SDKROOT)/usr/local/include \
	-Xclang -internal-externc-isystem -Xclang $(FUJI_SDKROOT)/usr/include \
	-fPIC \
	-fvisibility=hidden
LDFLAGS_FUJI = \
	--sysroot $(FUJI_SDKROOT) \
	-L $(FUJI_SDKROOT)/usr/local/lib \
	-L $(FUJI_SDKROOT)/usr/lib \
	-L $(FUJI_SDKROOT)/lib/arm-unknown-none-eabihf \
	-lclang_rt.builtins \
	-lclang_rt.atomic

CFLAGS = $(CFLAGS_FUJI) $(CFLAGS_DEBUG_$(DEBUG)) -ISources
LDFLAGS = $(LDFLAGS_FUJI) --relocatable


OBJECTS = \
	Objects/ff.o \
	Objects/ffunicode.o \
	Objects/FSGlue.o

PUBLIC_HEADERS =

PRIVATE_HEADERS = \
	source/FSGlue.h


all:	FujiFS.o


clean:
	rm -f $(OBJECTS) FujiFS.o


count:
	wc -l source/*.[ch] Makefile


FujiFS.o: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)


install: FujiFS.o
	mkdir -p $(DSTROOT)/usr/local/include
	cp $(PRIVATE_HEADERS) $(DSTROOT)/usr/local/include
	mkdir -p $(DSTROOT)/usr/local/lib
	cp FujiFS.o $(DSTROOT)/usr/local/lib


# Individual file dependencies.

Objects/FSGlue.o:	source/FSGlue.c
	@mkdir -p Objects
	$(CC) $(CFLAGS) -c $< -o $@

source/FSGlue.c:	source/FSGlue.h \
			source/diskio.h \
			source/ff.h

Objects/ff.o:		source/ff.c
	@mkdir -p Objects
	$(CC) $(CFLAGS) -c $< -o $@

source/ff.c:		source/ff.h \
			source/diskio.h

Objects/ffunicode.o:	source/ffunicode.c
	@mkdir -p Objects
	$(CC) $(CFLAGS) -c $< -o $@

source/ffunicode.c:	source/ff.h

source/ff.h:		source/ffconf.h
