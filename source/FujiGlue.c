/*
	FujiGlue.c

	Part of the Fuji Filesystem.
	Copyright © 2025 Base Hit Ventures LLC. All rights reserved.
	Based on FatFs, See file LICENSE.txt for details.
 */
 
#include "FujiGlue.h"

#include "ff.h"
#include "diskio.h"


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
