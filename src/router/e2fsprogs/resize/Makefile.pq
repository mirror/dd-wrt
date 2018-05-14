TOPSRC=..
LIBNAME=RESIZE.LIB
OBJFILE=RESIZE.LST

OBJS= extent.obj \
	ext2_block_move.obj \
	ext2_inode_move.obj \
	resize2fs.obj

!include $(TOPSRC)\powerquest\MCONFIG

