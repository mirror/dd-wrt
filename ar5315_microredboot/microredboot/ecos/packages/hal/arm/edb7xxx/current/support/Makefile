#
# eCos support tools for Cirrus Logic EDB7xxx eval boards
#

CFLAGS += -D__LINUX -O2

DL_FILES = dl_edb7xxx.o io.o
ALL := dl_edb7xxx


all: $(ALL)

dl_edb7xxx: $(DL_FILES)
	$(CC) -o $@ $(DL_FILES)

