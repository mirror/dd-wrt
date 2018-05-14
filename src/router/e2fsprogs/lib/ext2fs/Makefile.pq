TOPSRC=..\..
LIBNAME=EXT2.LIB
OBJFILE=EXT2.LST

OBJS= 	alloc.obj \
	alloc_tables.obj \
	badblocks.obj \
	bb_compat.obj \
	bb_inode.obj \
	bitmaps.obj \
	bitops.obj \
	block.obj \
	bmap.obj \
	bmove.obj \
	check_desc.obj \
	closefs.obj \
	cmp_bitmaps.obj \
	dblist.obj \
	dblist_dir.obj \
	dirblock.obj \
	dir_iterate.obj \
	dupfs.obj \
	expanddir.obj \
	fileio.obj \
	freefs.obj \
	get_pathname.obj \
	icount.obj \
	initialize.obj \
	inline.obj \
	inline_data.obj \
	inode.obj \
	ismounted.obj \
	link.obj \
	lookup.obj \
	mkdir.obj \
	namei.obj \
	native.obj \
	newdir.obj \
	openfs.obj \
	read_bb.obj \
	read_bb_file.obj \
	rs_bitmap.obj \
	rw_bitmaps.obj \
	swapfs.obj \
	unlink.obj \
	valid_blk.obj \
	version.obj

!include $(TOPSRC)\powerquest\MCONFIG

