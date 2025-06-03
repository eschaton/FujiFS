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

FUJI_SDKROOT = /opt/Fuji/Source/FujiSDK
FUJI_ABI_FLAGS = -fPIC -fvisibility=hidden -mfloat-abi=hard
CFLAGS = -Isource $(FUJI_ABI_FLAGS) -isysroot $(FUJI_SDKROOT) $(CFLAGS_DEBUG_$(DEBUG))
LDFLAGS = --relocatable --sysroot $(FUJI_SDKROOT)


OBJECTS = \
	Objects/ff.o \
	Objects/ffunicode.o \
	Objects/FSGlue.o

PUBLIC_HEADERS =

PRIVATE_HEADERS = \
	source/FSGlue.h


all: FujiFS.o


clean:
	rm -f $(OBJECTS) FujiFS.o


count:
	wc -l source/*.[ch] Makefile


FujiFS.o: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)


install: FujiFS.o
	mkdir -p $(DSTROOT)$(FUJI_SDKROOT)/usr/local/include
	cp $(PRIVATE_HEADERS) $(DSTROOT)$(FUJI_SDKROOT)/usr/local/include
	mkdir -p $(DSTROOT)$(FUJI_SDKROOT)/usr/local/lib
	cp FujiFS.o $(DSTROOT)$(FUJI_SDKROOT)/usr/local/lib


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
