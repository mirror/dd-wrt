TOPSRC=..
LIBNAME=E2FSCK.LIB
OBJFILE=E2FSCK.LST

OBJS= e2fsck.obj super.obj pass1.obj pass2.obj pass3.obj \
	pass4.obj pass5.obj dirinfo.obj

!include $(TOPSRC)\powerquest\MCONFIG

