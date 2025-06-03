/*
	FSGlue.h

	Part of the Fuji Filesystem.
	Copyright © 2025 Base Hit Ventures LLC. All rights reserved.
	Based on FatFs, See file LICENSE.txt for details.
 */

#ifndef __FUJI_FS__FujiGlue_h__
#define __FUJI_FS__FujiGlue_h__

#include <Types.h>


/* MARK: Initialization */

FUJI_EXPORT void _FSInit(void);


/* MARK: Filesystem Glue */
/* These functions are invoked both by the OS and via system calls. */

FUJI_EXPORT OSErr _FSOpen(const char *path, FSPerm perm, FileRef *f);
FUJI_EXPORT OSErr _FSOpenRF(const char *path, FSPerm perm, FileRef *f);
FUJI_EXPORT OSErr _FSClose(FileRef f);
FUJI_EXPORT OSErr _FSRead(FileRef f, void *buf, Size *count);
FUJI_EXPORT OSErr _FSWrite(FileRef f, void *buf, Size *count);
FUJI_EXPORT OSErr _FSSetPos(FileRef f, FSPosMode whence, int64_t offset);
FUJI_EXPORT OSErr _FSGetPos(FileRef f, int64_t *pos);
FUJI_EXPORT OSErr _FSSetEOF(FileRef f, int64_t eof);
FUJI_EXPORT OSErr _FSGetEOF(FileRef f, int64_t *eof);
FUJI_EXPORT OSErr _FSFlush(FileRef f);
FUJI_EXPORT OSErr _FSCreate(const char *path);
FUJI_EXPORT OSErr _FSDelete(const char *path);
FUJI_EXPORT OSErr _FSRename(const char *oldPath, const char *newPath);

FUJI_EXPORT OSErr _FSDirCreate(const char *path);
FUJI_EXPORT OSErr _FSDirOpen(const char *path, DirRef *d);
FUJI_EXPORT OSErr _FSDirClose(DirRef d);
FUJI_EXPORT OSErr _FSDirRead(DirRef d, DirEntry *dent);


#endif /* __FUJI_FS__FujiGlue_h__ */
