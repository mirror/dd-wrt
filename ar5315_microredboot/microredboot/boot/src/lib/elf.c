#include "elf.h"

static unsigned long fis_load_elf_image(unsigned long flash, unsigned long base,
					unsigned long length,
					unsigned long *entry_address,
					unsigned long *load_address,
					unsigned long *load_address_end)
{
	Elf32_Ehdr ehdr;
#define MAX_PHDR 8
	Elf32_Phdr phdr[MAX_PHDR];
	unsigned long offset = 0;
	int phx, len;
	unsigned char *addr;
	unsigned long addr_offset = 0;
	unsigned long highest_address = 0;
	unsigned long lowest_address = 0xFFFFFFFF;

	memmove(&ehdr, (char *)flash, sizeof(Elf32_Ehdr));

	offset += sizeof(Elf32_Ehdr);

	if (ehdr.e_type != ET_EXEC) {
		printf("Only absolute ELF images supported\n");
		return 0;
	}
	if (ehdr.e_phnum > MAX_PHDR) {
		printf("Too many program headers\n");
		return 0;
	}

	offset = ehdr.e_phoff;	/* =, not += !!! */

	for (phx = 0; phx < ehdr.e_phnum; phx++) {

		memmove(&phdr[phx], (char *)(flash + offset),
			sizeof(Elf32_Phdr));

		offset += sizeof(Elf32_Phdr);
	}
	if (base) {
		// Set address offset based on lowest address in file.
		addr_offset = 0xFFFFFFFF;
		for (phx = 0; phx < ehdr.e_phnum; phx++) {
			if (phdr[phx].p_vaddr < addr_offset) {
				addr_offset = phdr[phx].p_vaddr;
			}
		}
		addr_offset = (unsigned long)base - addr_offset;
	} else {
		addr_offset = 0;
	}

	for (phx = 0; phx < ehdr.e_phnum; phx++) {
		if (phdr[phx].p_type == PT_LOAD) {
			// Loadable segment
			addr = (unsigned char *)phdr[phx].p_vaddr;
			len = phdr[phx].p_filesz;
			if ((unsigned long)addr < lowest_address) {
				lowest_address = (unsigned long)addr;
			}
			addr += addr_offset;
			if (offset > phdr[phx].p_offset) {
				if ((phdr[phx].p_offset + len) < offset) {
					printf
					    ("Can't load ELF file - program headers out of order\n");
					return 0;
				}
				addr += offset - phdr[phx].p_offset;
			} else {
				offset = phdr[phx].p_offset;	/* _=_ */
			}

			memmove((char *)addr, (char *)(flash + offset), len);
			offset += len;
			len = 0;

			if ((unsigned long)(addr - addr_offset) >
			    highest_address) {
				highest_address =
				    (unsigned long)(addr - addr_offset);
			}
		}
	}			/* for (phx... */
	// Save load base/top and entry
	if (base) {
		*load_address = base;
		*load_address_end = base + (highest_address - lowest_address);
		*entry_address = base + (ehdr.e_entry - lowest_address);
	} else {
		*load_address = lowest_address;
		*load_address_end = highest_address;
		*entry_address = ehdr.e_entry;
	}

	// redboot_getc_terminate(false);
	if (addr_offset)
		printf("\nAddress offset = 0x%08X\n", (void *)addr_offset);
	printf("Entry point: 0x%08X, address range: 0x%08X-0x%08X\n",
		    (void *)*entry_address, (void *)*load_address,
		    (void *)*load_address_end);
	return 1;
}
