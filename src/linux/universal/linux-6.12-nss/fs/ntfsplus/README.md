## NTFS filesystem
This is the ntfs filesystem for support from the linux 6.1 kernel
to the latest kernel.

## Installing as a stand-alone module

Install prerequisite package for Fedora, RHEL:
```
	yum install kernel-devel-$(uname -r)
```

Build step:
```
	make
	sudo make install
```

To load the driver manually, run this as root:
```
	modprobe ntfs
```


## Installing as a part of the kernel

1. Let's take [linux] as the path to your kernel source dir.
```
	cd [linux]
	cp -ar ntfs [linux]/fs/
```

2. edit [linux]/fs/Kconfig
```
	source "fs/fat/Kconfig"
	+source "fs/ntfs/Kconfig"
	source "fs/ntfs3/Kconfig"
```

3. edit [linux]/fs/Makefile
```
	obj-$(CONFIG_FAT_FS)          += fat/
	+obj-$(CONFIG_NTFS_FS)       += ntfs/
	obj-$(CONFIG_BFS_FS)          += bfs/
```
4. make menuconfig and set ntfs
```
	File systems  --->
		DOS/FAT/NT Filesystems  --->
			<M> NTFS filesystem support
			[ ]   NTFS debugging support
			[ ]   NTFS POSIX Access Control Lists
```

build your kernel
