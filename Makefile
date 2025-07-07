#
#	Makefile
#
#	Part of the Fuji Filesystem.
#	Copyright © 2025 Base Hit Ventures LLC. All rights reserved.
#	Based on FatFs, See file LICENSE.txt for details.
#

# Project, Source & Build Identification
FUJI_PROJECT ?= FujiFS
FUJI_PROJECT_SOURCEVERSION ?= 1
FUJI_PROJECT_BUILDVERSION ?= 1

# Use Fuji toolchain.
CC = fuji-tool.sh arm-none-eabi-gcc
LD = fuji-tool.sh arm-none-eabi-ld.bfd
AR = fuji-tool.sh arm-none-eabi-ar
RANLIB = fuji-tool.sh arm-none-eabi-ranlib

FUJI_PROJECT_ID = $(FUJI_PROJECT)-$(FUJI_PROJECT_SOURCEVERSION)\~$(FUJI_PROJECT_BUILDVERSION)
FUJI_BUILDRECORD_DIR ?= /tmp/FujiBuildRecords/$(FUJI_PROJECT_ID).build
FUJI_DSTROOT ?= $(FUJI_BUILDRECORD_DIR)/dst
FUJI_SCTROOT ?= $(FUJI_BUILDRECORD_DIR)/sdk
FUJI_HSTROOT ?= $(FUJI_BUILDRECORD_DIR)/hst
DEBUG ?= 0

# Enable make DEBUG=1 to build for debugging.
CFLAGS_DEBUG_0 = -Os -DNDEBUG=1
CFLAGS_DEBUG_1 = -O0 -g -DDEBUG=1
CFLAGS_DEBUG_ = $(CFLAGS_DEBUG_0)
LDFLAGS_DEBUG_0 = --lto-O3 --strip-debug
LDFLAGS_DEBUG_1 =
LDFLAGS_DEBUG_ = $(LDFLAGS_DEBUG_0)

FUJI_SDKROOT = /opt/Fuji/Source/Fuji.sdk
CFLAGS_FUJI = \
	-mcpu=cortex-m33 \
	--sysroot $(FUJI_SDKROOT) \
	-mfdpic \
		-msingle-pic-base \
		-mno-pic-data-is-text-relative \
		-mpic-register=r9 \
	-shared \
	-Wno-multichar \
	-fvisibility=hidden \
	-D__FUJI__=1 \
	-nostdlib \
	-I/usr/lib/gcc/arm-none-eabi/15.1.0/include
LDFLAGS_FUJI = \
	--sysroot $(FUJI_SDKROOT) \
	-nostdlib \
	-L $(FUJI_SDKROOT)/usr/local/lib \
	-L $(FUJI_SDKROOT)/usr/lib

CFLAGS = $(CFLAGS_FUJI) $(CFLAGS_DEBUG_$(DEBUG)) \
	 -ISources
LDFLAGS = $(LDFLAGS_FUJI) $(LDFLAGS_DEBUG_$(DEBUG))

OBJECTS = \
	Objects/ff.o \
	Objects/ffunicode.o \
	Objects/FSGlue.o

PUBLIC_HEADERS =

PRIVATE_HEADERS = \
	source/FSGlue.h


all:	libFujiFS.a


clean:
	rm -f $(OBJECTS) \
	      libFujiFS.a


libFujiFS.a: $(OBJECTS)
	$(AR) -crs $@ $(OBJECTS)


installapi: $(PUBLIC_HEADERS) $(PRIVATE_HEADERS) libFujiFS.a
	mkdir -p $(FUJI_SCTROOT)/usr/local/include
	cp $(PRIVATE_HEADERS) $(FUJI_SCTROOT)/usr/local/include
	mkdir -p $(FUJI_SCTROOT)/usr/local/lib
	cp libFujiFS.a $(FUJI_SCTROOT)/usr/local/lib

install:
	@echo install: Nothing to do


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
