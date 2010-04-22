// This is used to pack YAFFS1 tags, not YAFFS2 tags.

#ifndef __YAFFS_PACKEDTAGS1_H__
#define __YAFFS_PACKEDTAGS1_H__

#include "yaffs_guts.h"

typedef struct {
	unsigned chunkId:20;
	unsigned serialNumber:2;
	unsigned byteCount:10;
	unsigned objectId:18;
	unsigned ecc:12;
	unsigned deleted:1;
	unsigned unusedStuff:1;
	unsigned shouldBeFF;

} yaffs_PackedTags1;

void yaffs_PackTags1(yaffs_PackedTags1 * pt, const yaffs_ExtendedTags * t);
void yaffs_UnpackTags1(yaffs_ExtendedTags * t, const yaffs_PackedTags1 * pt);
#endif
