#ifndef __YAFFS_CHECKPTRW_H__
#define __YAFFS_CHECKPTRW_H__

#include "yaffs_guts.h"

int yaffs_CheckpointOpen(yaffs_Device *dev, int forWriting);

int yaffs_CheckpointWrite(yaffs_Device *dev,const void *data, int nBytes);

int yaffs_CheckpointRead(yaffs_Device *dev,void *data, int nBytes);

int yaffs_CheckpointClose(yaffs_Device *dev);

int yaffs_CheckpointInvalidateStream(yaffs_Device *dev);


#endif

