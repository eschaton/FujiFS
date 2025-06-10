/*
	FSGlue.c

	Part of the Fuji Filesystem.
	Copyright © 2025 Base Hit Ventures LLC. All rights reserved.
	Based on FatFs, See file LICENSE.txt for details.
 */

#include "FSGlue.h"

#include <Errors.h>

#include <stdatomic.h>
#include <string.h>

#include "ff.h"
#include "diskio.h"


/* MARK: Initialization */

void FSInit(void)
{
	// TODO: See if there's any initialization we can do for performance.
}


/* MARK: Integration Glue between FatFs and Fuji */

DWORD get_fattime(void)
{
#warning Implement get_fattime
	/* This needs to get the current time and translate it to a FAT time. */
	return 0;
}


/* MARK: - Disk Access Glue */

PARTITION VolToPart[FF_VOLUMES];

DSTATUS disk_initialize (BYTE pdrv)
{
	return STA_NODISK;
}

DSTATUS disk_status (BYTE pdrv)
{
	return STA_NODISK;
}

DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
	return RES_NOTRDY;
}

DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
	return RES_NOTRDY;
}

DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff)
{
	return RES_NOTRDY;
}


/* MARK: - Multiprocessing Glue */

int Mutexes[FF_VOLUMES];
atomic_flag MutexesLock;

int ff_mutex_create (int vol)
{
    atomic_flag_test_and_set(&MutexesLock);
    Mutexes[vol] = 0;
    atomic_flag_clear(&MutexesLock);

	return vol;
}

void ff_mutex_delete (int vol)
{
    atomic_flag_test_and_set(&MutexesLock);
    Mutexes[vol] = 0;
    atomic_flag_clear(&MutexesLock);
}

int ff_mutex_take (int vol)
{
    int mutex;
    do {
        atomic_flag_test_and_set(&MutexesLock);
        mutex = Mutexes[vol];
        if (mutex == 0) {
            Mutexes[vol] = 1;
            mutex = 1;
        }
        atomic_flag_clear(&MutexesLock);
    } while (mutex == 0);

	return vol;
}

void ff_mutex_give (int vol)
{
    int mutex;
    do {
        atomic_flag_test_and_set(&MutexesLock);
        mutex = Mutexes[vol];
        if (mutex != 0) {
            Mutexes[vol] = 0;
            mutex = 0;
        }
        atomic_flag_clear(&MutexesLock);
    } while (mutex != 0);
}


/* MARK: - Filesystem Glue */

typedef struct _FSFileControlBlock {
	FIL fil;
} _FSFileControlBlock;
#define _FSMAXFILES 64
_FSFileControlBlock fcbs[_FSMAXFILES] = { 0 };
uint64_t fcbsInUse = 0;

static FileRef _FSAcquireFileRef(void)
{
	FileRef f = -1;

	// TODO: Acquire spinlock
	{
		for (int i = 0; (i < _FSMAXFILES) && (f == -1); i++) {
			if (((fcbsInUse >> i) & 1) == 0) {
				f = i;
			}
		}
	}
	// TODO: Release spinlock

	return f;
}

static void _FSReleaseFileRef(FileRef f)
{
	// TODO: Acquire spinlock
	{
		fcbsInUse &= ~(1 << f);
	}
	// TODO: Release spinlock
}

static bool _FSFileRefIsValid(FileRef f)
{
	return (f >= 0) && (f < _FSMAXFILES);
}

static BYTE _FSPermToMode(FSPerm perm)
{
	switch (perm) {
		case fsRd: return FA_READ;
		case fsWr: return FA_WRITE;
		case fsRdWr: return FA_READ|FA_WRITE;
		default: return 0;
	}
}

static OSErr _FSResultToErr(FRESULT res)
{
#warning Finish _FSResultToErr
	switch (res) {
		case FR_OK:                  return noErr;
		case FR_NO_FILE:             return fnfErr;
		case FR_TOO_MANY_OPEN_FILES: return tmfoErr;
		case FR_INVALID_PARAMETER:   return paramErr;
		default:                     return paramErr;
	}
}

OSErr _FSOpen(const char *path, FSPerm perm, FileRef *f)
{
	if ((path == NULL) || (f == NULL)) return paramErr;

	FileRef fr = _FSAcquireFileRef();
	if (fr == -1) return tmfoErr;

	_FSFileControlBlock *fcbp = &fcbs[fr];
	FIL *fp = &fcbp->fil;

	FRESULT res = f_open(fp, path, _FSPermToMode(perm) | FA_OPEN_EXISTING);
	if (res != FR_OK) {
		_FSReleaseFileRef(fr);
		return _FSResultToErr(res);
	}

	*f = fr;

	return noErr;
}

OSErr _FSOpenRF(const char *path, FSPerm perm, FileRef *f)
{
#warning Implement _FSOpenRF
	return _FSOpen(path, perm, f);
}

OSErr _FSClose(FileRef f)
{
	if (!_FSFileRefIsValid(f)) return paramErr;

	_FSFileControlBlock *fcbp = &fcbs[f];
	FIL *fp = &fcbp->fil;

	FRESULT res = f_close(fp);
	_FSReleaseFileRef(f);

	return _FSResultToErr(res);
}

OSErr _FSRead(FileRef f, void *buf, Size *count)
{
	if (!_FSFileRefIsValid(f)) return paramErr;
	if ((buf == NULL) || (count == NULL)) return paramErr;

	_FSFileControlBlock *fcbp = &fcbs[f];
	FIL *fp = &fcbp->fil;

	UINT btr = *count;
	UINT br = btr;

	FRESULT res = f_read(fp, buf, btr, &br);
	*count = br;

	return _FSResultToErr(res);
}

OSErr _FSWrite(FileRef f, void *buf, Size *count)
{
	if (!_FSFileRefIsValid(f)) return paramErr;
	if ((buf == NULL) || (count == NULL)) return paramErr;

	_FSFileControlBlock *fcbp = &fcbs[f];
	FIL *fp = &fcbp->fil;

	UINT btw = *count;
	UINT bw = btw;

	FRESULT res = f_write(fp, buf, btw, &bw);
	*count = bw;

	return _FSResultToErr(res);
}

OSErr _FSSetPos(FileRef f, FSPosMode whence, int64_t offset)
{
	if (!_FSFileRefIsValid(f)) return paramErr;

	_FSFileControlBlock *fcbp = &fcbs[f];
	FIL *fp = &fcbp->fil;

	FSIZE_t curPos = f_tell(fp);
	FSIZE_t newPos;
	switch (whence) {
		case fsAtMark:
			// Do nothing. Why does this exist?!
			newPos = curPos;
			break;

		case fsFromStart:
			newPos = offset;
			break;

		case fsFromEOF:
			newPos = f_size(fp) + offset;
			break;

		case fsFromMark:
			newPos = curPos + offset;
			break;
	}

	FRESULT res = f_lseek(fp, newPos);

	return _FSResultToErr(res);
}

OSErr _FSGetPos(FileRef f, int64_t *pos)
{
	if (!_FSFileRefIsValid(f)) return paramErr;
	if (pos == NULL) return paramErr;

	_FSFileControlBlock *fcbp = &fcbs[f];
	FIL *fp = &fcbp->fil;

	*pos = f_tell(fp);

	return noErr;
}

OSErr _FSSetEOF(FileRef f, int64_t eof)
{
	if (!_FSFileRefIsValid(f)) return paramErr;
	if (eof < 0) return paramErr;

	_FSFileControlBlock *fcbp = &fcbs[f];
	FIL *fp = &fcbp->fil;

	FRESULT res = f_truncate(fp);
	if ((res == FR_OK) && (eof > 0)) {
		res = f_expand(fp, eof, 1);
	}

	return _FSResultToErr(res);
}

OSErr _FSGetEOF(FileRef f, int64_t *eof)
{
	if (!_FSFileRefIsValid(f)) return paramErr;
	if (eof == NULL) return paramErr;

	_FSFileControlBlock *fcbp = &fcbs[f];
	FIL *fp = &fcbp->fil;

	*eof = f_size(fp);

	return noErr;
}

OSErr _FSFlush(FileRef f)
{
	if (!_FSFileRefIsValid(f)) return paramErr;

	_FSFileControlBlock *fcbp = &fcbs[f];
	FIL *fp = &fcbp->fil;

	FRESULT res = f_sync(fp);

	return _FSResultToErr(res);
}

OSErr _FSCreate(const char *path)
{
	if (path == NULL) return paramErr;

	FileRef fr = _FSAcquireFileRef();
	if (fr == -1) return tmfoErr;

	_FSFileControlBlock *fcbp = &fcbs[fr];
	FIL *fp = &fcbp->fil;

	FRESULT res = f_open(fp, path, FA_WRITE | FA_CREATE_NEW);
	if (res != FR_OK) {
		_FSReleaseFileRef(fr);
		return _FSResultToErr(res);
	}

	res = f_close(fp);

	return _FSResultToErr(res);
}

OSErr _FSDelete(const char *path)
{
	if (path == NULL) return paramErr;

	FRESULT res = f_unlink(path);

	return _FSResultToErr(res);
}

OSErr _FSRename(const char *oldPath, const char *newPath)
{
	if ((oldPath == NULL) || (newPath == NULL)) return paramErr;

	FRESULT res = f_rename(oldPath, newPath);

	return _FSResultToErr(res);
}

OSErr _FSDirCreate(const char *path)
{
	if (path == NULL) return paramErr;

	FRESULT res = f_mkdir(path);

	return _FSResultToErr(res);
}


typedef struct _FSDirControlBlock {
	DIR dir;
} _FSDirControlBlock;
#define _FSMAXDIRS 32
_FSDirControlBlock dcbs[_FSMAXDIRS] = { 0 };
uint32_t dcbsInUse = 0;

static DirRef _FSAcquireDirRef(void)
{
	DirRef d = -1;

	// TODO: Acquire spinlock
	{
		for (int i = 0; (i < _FSMAXDIRS) && (d == -1); i++) {
			if (((dcbsInUse >> i) & 1) == 0) {
				d = i;
			}
		}
	}
	// TODO: Release spinlock

	return d;
}

static void _FSReleaseDirRef(DirRef d)
{
	// TODO: Acquire spinlock
	{
		dcbsInUse &= ~(1 << d);
	}
	// TODO: Release spinlock
}

static bool _FSDirRefIsValid(DirRef d)
{
	return (d >= 0) && (d < _FSMAXDIRS);
}


OSErr _FSDirOpen(const char *path, DirRef *d)
{
	if ((path == NULL) || (d == NULL)) return paramErr;

	DirRef dr = _FSAcquireDirRef();
	if (dr == -1) return tmfoErr;

	_FSDirControlBlock *dcbp = &dcbs[dr];
	DIR *dp = &dcbs->dir;

	FRESULT res = f_opendir(dp, path);
	if (res != FR_OK) {
		_FSReleaseDirRef(dr);
		return _FSResultToErr(res);
	}

	*d = dr;

	return noErr;
}

OSErr _FSDirClose(DirRef d)
{
	if (!_FSDirRefIsValid(d)) return paramErr;

	_FSDirControlBlock *dcbp = &dcbs[d];
	DIR *dp = &dcbp->dir;

	FRESULT res = f_closedir(dp);
	_FSReleaseDirRef(d);

	return _FSResultToErr(res);
}

OSErr _FSDirRead(DirRef d, DirEntry *dent)
{
	if (!_FSDirRefIsValid(d)) return paramErr;
	if (dent == NULL) return paramErr;

	_FSDirControlBlock *dcbp = &dcbs[d];
	DIR *dp = &dcbp->dir;

	FILINFO info;
	FRESULT res = f_readdir(dp, &info);
	if (res == FR_OK) {
        strncpy(dent->name, info.fname, 256);
	}

	return _FSResultToErr(res);
}
