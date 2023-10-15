// SPDX-License-Identifier: GPL-2.0
#include <linux/libfdt_env.h>
#include <asm/setup.h>
#include <libfdt.h>

#if defined(CONFIG_ARM_ATAG_DTB_COMPAT_CMDLINE_EXTEND)
#define do_extend_cmdline 1
#elif defined(CONFIG_ARM_ATAG_DTB_COMPAT_CMDLINE_MANGLE)
#define do_extend_cmdline 1
#else
#define do_extend_cmdline 0
#endif

#define NR_BANKS 16

static int node_offset(void *fdt, const char *node_path)
{
	int offset = fdt_path_offset(fdt, node_path);
	if (offset == -FDT_ERR_NOTFOUND)
		/* Add the node to root if not found, dropping the leading '/' */
		offset = fdt_add_subnode(fdt, 0, node_path + 1);
	return offset;
}

#ifndef CONFIG_ARM_ATAG_DTB_COMPAT_CMDLINE_MANGLE
static int setprop(void *fdt, const char *node_path, const char *property,
		   void *val_array, int size)
{
	int offset = node_offset(fdt, node_path);
	if (offset < 0)
		return offset;
	return fdt_setprop(fdt, offset, property, val_array, size);
}
#endif

static int setprop_string(void *fdt, const char *node_path,
			  const char *property, const char *string)
{
	int offset = node_offset(fdt, node_path);
	if (offset < 0)
		return offset;
	return fdt_setprop_string(fdt, offset, property, string);
}

#ifndef CONFIG_ARM_ATAG_DTB_COMPAT_CMDLINE_MANGLE
static int setprop_cell(void *fdt, const char *node_path,
			const char *property, uint32_t val)
{
	int offset = node_offset(fdt, node_path);
	if (offset < 0)
		return offset;
	return fdt_setprop_cell(fdt, offset, property, val);
}
#endif

static const void *getprop(const void *fdt, const char *node_path,
			   const char *property, int *len)
{
	int offset = fdt_path_offset(fdt, node_path);

	if (offset == -FDT_ERR_NOTFOUND)
		return NULL;

	return fdt_getprop(fdt, offset, property, len);
}

#ifndef CONFIG_ARM_ATAG_DTB_COMPAT_CMDLINE_MANGLE
static uint32_t get_cell_size(const void *fdt)
{
	int len;
	uint32_t cell_size = 1;
	const __be32 *size_len =  getprop(fdt, "/", "#size-cells", &len);

	if (size_len)
		cell_size = fdt32_to_cpu(*size_len);
	return cell_size;
}
#endif

#if defined(CONFIG_ARM_ATAG_DTB_COMPAT_CMDLINE_MANGLE)
/**
 * taken from arch/x86/boot/string.c
 * local_strstr - Find the first substring in a %NUL terminated string
 * @s1: The string to be searched
 * @s2: The string to search for
 */
static char *local_strstr(const char *s1, const char *s2)
{
	size_t l1, l2;

	l2 = strlen(s2);
	if (!l2)
		return (char *)s1;
	l1 = strlen(s1);
	while (l1 >= l2) {
		l1--;
		if (!memcmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}
	return NULL;
}

static char *append_rootblock(char *dest, const char *str, int len, void *fdt)
{
	char *ptr, *end, *tmp;
	const char *root="root=";
	const char *find_rootblock;
	int i, l;
	const char *rootblock;

	find_rootblock = getprop(fdt, "/chosen", "find-rootblock", &l);
	if (!find_rootblock)
		find_rootblock = root;

	//ARM doesn't have __HAVE_ARCH_STRSTR, so it was copied from x86
	ptr = local_strstr(str, find_rootblock);

	if(!ptr)
		return dest;

	end = strchr(ptr, ' ');
	end = end ? (end - 1) : (strchr(ptr, 0) - 1);

	// Some boards ubi.mtd=XX,ZZZZ, so let's check for '," too.
	tmp = strchr(ptr, ',');

	if(tmp)
		end = end < tmp ? end : tmp - 1;

	//find partition number (assumes format root=/dev/mtdXX | /dev/mtdblockXX | yy:XX | ubi.mtd=XX,ZZZZ )
	for( i = 0; end >= ptr && *end >= '0' && *end <= '9'; end--, i++);
	ptr = end + 1;

	/* if append-rootblock property is set use it to append to command line */
	rootblock = getprop(fdt, "/chosen", "append-rootblock", &l);
	if(rootblock != NULL) {
		if(*dest != ' ') {
			*dest = ' ';
			dest++;
			len++;
		}
		if (len + l + i <= COMMAND_LINE_SIZE) {
			memcpy(dest, rootblock, l);
			dest += l - 1;
			memcpy(dest, ptr, i);
			dest += i;
		}
	}
#ifdef CONFIG_ARCH_MVEBU
	else {
		len = strlen(str);
		if (len + 1 < COMMAND_LINE_SIZE) {
			memcpy(dest, str, len);
			dest += len;
		}
	}
#endif
	return dest;
}
#endif

static void merge_fdt_bootargs(void *fdt, const char *fdt_cmdline)
{
	char cmdline[COMMAND_LINE_SIZE];
	const char *fdt_bootargs;
	char *ptr = cmdline;
	int len = 0;

	/* copy the fdt command line into the buffer */
	fdt_bootargs = getprop(fdt, "/chosen", "bootargs", &len);
	if (fdt_bootargs)
		if (len < COMMAND_LINE_SIZE) {
			memcpy(ptr, fdt_bootargs, len);
			/* len is the length of the string
			 * including the NULL terminator */
			ptr += len - 1;
		}

	/* and append the ATAG_CMDLINE */
	if (fdt_cmdline) {

#if defined(CONFIG_ARM_ATAG_DTB_COMPAT_CMDLINE_MANGLE)
		//save original bootloader args
		//and append ubi.mtd with root partition number to current cmdline
		setprop_string(fdt, "/chosen", "bootloader-args", fdt_cmdline);
		ptr = append_rootblock(ptr, fdt_cmdline, len, fdt);

#else
		len = strlen(fdt_cmdline);
		if (ptr - cmdline + len + 2 < COMMAND_LINE_SIZE) {
			*ptr++ = ' ';
			memcpy(ptr, fdt_cmdline, len);
			ptr += len;
		}
#endif
	}
	*ptr = '\0';

	setprop_string(fdt, "/chosen", "bootargs", cmdline);
}

#ifndef CONFIG_ARM_ATAG_DTB_COMPAT_CMDLINE_MANGLE
static void hex_str(char *out, uint32_t value)
{
	uint32_t digit;
	int idx;

	for (idx = 7; idx >= 0; idx--) {
		digit = value >> 28;
		value <<= 4;
		digit &= 0xf;
		if (digit < 10)
			digit += '0';
		else
			digit += 'A'-10;
		*out++ = digit;
	}
	*out = '\0';
}
#endif

/*
 * Convert and fold provided ATAGs into the provided FDT.
 *
 * Return values:
 *    = 0 -> pretend success
 *    = 1 -> bad ATAG (may retry with another possible ATAG pointer)
 *    < 0 -> error from libfdt
 */
int atags_to_fdt(void *atag_list, void *fdt, int total_space)
{
	struct tag *atag = atag_list;
	/* In the case of 64 bits memory size, need to reserve 2 cells for
	 * address and size for each bank */
#ifndef CONFIG_ARM_ATAG_DTB_COMPAT_CMDLINE_MANGLE
	__be32 mem_reg_property[2 * 2 * NR_BANKS];
	int memsize, memcount = 0;
#endif
	int ret;

	/* make sure we've got an aligned pointer */
	if ((u32)atag_list & 0x3)
		return 1;

	/* if we get a DTB here we're done already */
	if (*(__be32 *)atag_list == cpu_to_fdt32(FDT_MAGIC))
	       return 0;

	/* validate the ATAG */
	if (atag->hdr.tag != ATAG_CORE ||
	    (atag->hdr.size != tag_size(tag_core) &&
	     atag->hdr.size != 2))
		return 1;

	/* let's give it all the room it could need */
	ret = fdt_open_into(fdt, fdt, total_space);
	if (ret < 0)
		return ret;

	for_each_tag(atag, atag_list) {
		if (atag->hdr.tag == ATAG_CMDLINE) {
			/* Append the ATAGS command line to the device tree
			 * command line.
			 * NB: This means that if the same parameter is set in
			 * the device tree and in the tags, the one from the
			 * tags will be chosen.
			 */
			if (do_extend_cmdline)
				merge_fdt_bootargs(fdt,
						   atag->u.cmdline.cmdline);
			else
				setprop_string(fdt, "/chosen", "bootargs",
					       atag->u.cmdline.cmdline);
		}
#ifndef CONFIG_ARM_ATAG_DTB_COMPAT_CMDLINE_MANGLE
		else if (atag->hdr.tag == ATAG_MEM) {
			if (memcount >= sizeof(mem_reg_property)/4)
				continue;
			if (!atag->u.mem.size)
				continue;
			memsize = get_cell_size(fdt);

			if (memsize == 2) {
				/* if memsize is 2, that means that
				 * each data needs 2 cells of 32 bits,
				 * so the data are 64 bits */
				__be64 *mem_reg_prop64 =
					(__be64 *)mem_reg_property;
				mem_reg_prop64[memcount++] =
					cpu_to_fdt64(atag->u.mem.start);
				mem_reg_prop64[memcount++] =
					cpu_to_fdt64(atag->u.mem.size);
			} else {
				mem_reg_property[memcount++] =
					cpu_to_fdt32(atag->u.mem.start);
				mem_reg_property[memcount++] =
					cpu_to_fdt32(atag->u.mem.size);
			}

		} else if (atag->hdr.tag == ATAG_INITRD2) {
			uint32_t initrd_start, initrd_size;
			initrd_start = atag->u.initrd.start;
			initrd_size = atag->u.initrd.size;
			setprop_cell(fdt, "/chosen", "linux,initrd-start",
					initrd_start);
			setprop_cell(fdt, "/chosen", "linux,initrd-end",
					initrd_start + initrd_size);
		} else if (atag->hdr.tag == ATAG_SERIAL) {
			char serno[16+2];
			hex_str(serno, atag->u.serialnr.high);
			hex_str(serno+8, atag->u.serialnr.low);
			setprop_string(fdt, "/", "serial-number", serno);
		}
	}

	if (memcount) {
		setprop(fdt, "/memory", "reg", mem_reg_property,
			4 * memcount * memsize);
	}
#else

	}
#endif

	return fdt_pack(fdt);
}
