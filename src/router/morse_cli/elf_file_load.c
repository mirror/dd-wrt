/*
 * Copyright 2022-2023 Morse Micro
 */
#ifdef MORSE_WIN_BUILD
#include "win/elf.h"
#else
#include <elf.h>
#endif
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>

#include "portable_endian.h"
#include "elf_file.h"
#include "elf_file_internal.h"
#include "utilities.h"

#define HOST_FLASH_BASE_MASK        (0xFFFF0000)
#define HOST_IFLASH_BASE_ADDR       (0x00400000)
#define HOST_DFLASH_BASE_ADDR       (0x00C00000)
#define MAX_NUM_SECTION_HEADERS     (100U)

#define LOAD_BCF_SECTION_TOT    (2) /* board_config section plus regdom section */

static struct
{
    struct arg_file *file;
    struct arg_lit *load_bcf;
    struct arg_rex *country;
} args;


static void print_ehdr(Elf32_Ehdr *ehdr)
{
    mctrl_print("Elf32_Ehdr:\n");
    mctrl_print("\te_ident: 0x%02x 0x%02x 0x%02x 0x%02x\n",
            ehdr->e_ident[EI_MAG0], ehdr->e_ident[EI_MAG1],
            ehdr->e_ident[EI_MAG2], ehdr->e_ident[EI_MAG3]);
    /* elf32 and little endian */
    mctrl_print("\te_ident: 0x%02x\n", ehdr->e_ident[EI_DATA]);
    mctrl_print("\te_ident: 0x%02x\n", ehdr->e_ident[EI_CLASS]);

    mctrl_print("\te_phoff:     0x%08x\n", ehdr->e_phoff);
    mctrl_print("\te_phentsize: 0x%08x\n", ehdr->e_phentsize);
    mctrl_print("\te_phnum:     %u\n", ehdr->e_phnum);
    mctrl_print("\te_shoff:     0x%08x\n", ehdr->e_shoff);
    mctrl_print("\te_shentsize: 0x%08x\n", ehdr->e_shentsize);
    mctrl_print("\te_shnum:     %u\n", ehdr->e_shnum);
    mctrl_print("\te_shstrndx:  0x%08x\n", ehdr->e_shstrndx);
}

static void print_phdr(Elf32_Phdr *phdr)
{
    mctrl_print("Elf32_Phdr:\n");
    mctrl_print("\tp_type:   %u\n", phdr->p_type);
    mctrl_print("\tp_offset: 0x%08x\n", phdr->p_offset);
    mctrl_print("\tp_vaddr:  0x%08x\n", phdr->p_vaddr);
    mctrl_print("\tp_paddr:  0x%08x\n", phdr->p_paddr);
    mctrl_print("\tp_filesz: 0x%08x\n", phdr->p_filesz);
    mctrl_print("\tp_memsz:  0x%08x\n", phdr->p_memsz);
    mctrl_print("\tp_align:  %u\n", phdr->p_align);
}

int load_elf_blob(FILE *firmware, struct morsectrl_transport *transport,
    int idx, Elf32_Off offset, Elf32_Word size, Elf32_Addr addr)
{
    struct morsectrl_transport_buff *write;
    int ret = 0;

    mctrl_print("Loading ELF blob %d size 0x%08x into chip addr 0x%08x\n",
           idx, size, addr);

    write = morsectrl_transport_raw_write_alloc(transport, size);
    if (!write)
    {
        mctrl_err("Transport write alloc failed\n");
        return -ENOMEM;
    }

    /* Load binary blob directly into a transport buffer. */
    if (elf_file_load_binary_data(firmware, offset, size, &write->data))
    {
        mctrl_err("Load binary failed\n");
        ret = -ENOENT;
        goto exit;
    }

    if (morsectrl_transport_mem_write(transport, write, addr) != 0)
    {
        mctrl_err("Mem write failed\n");
        ret = -ENXIO;
        goto exit;
    }

exit:
    morsectrl_transport_buff_free(write);
    return ret;
}

/*
 * Load a BCF file onto a device.
 * Only the general (board_config) and regdom section for the
 * specified Regulatory domain ('country') are loaded.
 */
static int load_bcf_sections(struct morsectrl *mors, FILE *firmware, Elf32_Ehdr *ehdr,
    const char *country)
{
    struct morsectrl_transport *transport = mors->transport;
    Elf32_Off sh_offset[LOAD_BCF_SECTION_TOT] = { 0 };
    Elf32_Word sh_size[LOAD_BCF_SECTION_TOT] = { 0 };
    Elf32_Addr addr = 0;
    Elf32_Shdr sh_strtab;
    const char *sh_strs;
    uint8_t *data;
    int ii;
    int ret = 0;

    mctrl_print("Trying to load BCF file using country %s\n", country);

    load_file(firmware, &data);
    if (data == NULL)
    {
        mctrl_err("Load file failed\n");
        return -ENOENT;
    }

    if (get_section_header(data, ehdr, &sh_strtab, ehdr->e_shstrndx) != 0) {
        mctrl_err("Invalid firmware - missing string table\n");
        ret = -ENXIO;
        goto exit;
    }

    sh_strs = (const char *)data + sh_strtab.sh_offset;

    /* Sanitise the loop bound with a reasonable number for the section headers */
    if (ehdr->e_shnum > MAX_NUM_SECTION_HEADERS)
    {
        mctrl_err("Exceeded maximum number of section headers\n");
        ret = -EPERM;
        goto exit;
    }

    /* Find the required headers */
    for (ii = 0; ii < ehdr->e_shnum; ii++)
    {
        Elf32_Shdr shdr;

        if (get_section_header(data, ehdr, &shdr, ii) != 0)
            continue;

        if (strcmp(sh_strs + shdr.sh_name, ".board_config") == 0)
        {
            if (mors->debug)
                mctrl_print("Found section header %s\n", sh_strs + shdr.sh_name);

            addr = shdr.sh_addr;
            sh_offset[0] = shdr.sh_offset;
            sh_size[0] = shdr.sh_size;
        }
        else if ((strncmp(sh_strs + shdr.sh_name, ".regdom_", strlen(".regdom_")) == 0) &&
                 (strcmp(sh_strs + shdr.sh_name + strlen(".regdom_"), country) == 0))
        {
            if (mors->debug)
                mctrl_print("Found section header %s\n", sh_strs + shdr.sh_name);

            sh_offset[1] = shdr.sh_offset;
            sh_size[1] = shdr.sh_size;
        }
    }

    if ((sh_offset[0] == 0) || (sh_size[0] == 0))
    {
        mctrl_err("Board config section not found\n");
        ret = -ENXIO;
        goto exit;
    }

    if ((addr & HOST_FLASH_BASE_MASK) == 0)
    {
        mctrl_err("Board config section address (0x%08x) is invalid\n", addr);
        ret = -ENXIO;
        goto exit;
    }

    if ((sh_offset[1] == 0) || (sh_size[1] == 0))
    {
        mctrl_err("Regdom section not found for %s\n", country);
        ret = -ENXIO;
        goto exit;
    }

    for (ii = 0; ii < LOAD_BCF_SECTION_TOT; ii++)
    {
        int ret = load_elf_blob(firmware, transport, ii, sh_offset[ii], sh_size[ii], addr);
        if (ret < 0)
        {
            goto exit;
        }
        addr += sh_size[ii];
    }

exit:
    free(data);
    return ret;
}

/*
 * Load ELF program sections onto a device.
 */
static int load_blobs(struct morsectrl *mors, FILE *firmware, Elf32_Ehdr *ehdr)
{
    struct morsectrl_transport *transport = mors->transport;
    Elf32_Phdr *phdr = NULL;
    int ii;
    int ret;

    mctrl_print("%d blobs to try to load\n", ehdr->e_phnum);

    phdr = elf_file_load_program_headers(firmware, ehdr->e_phoff, ehdr->e_phentsize *ehdr->e_phnum);
    if (!phdr)
    {
        return -ENXIO;
    }

    for (ii = 0; ii < ehdr->e_phnum; ii++)
    {
        if (mors->debug)
            print_phdr(&phdr[ii]);

        /* Skip empty or unloadable blobs. Filter out external flash. */
        if ((phdr[ii].p_type != PT_LOAD) ||
            (phdr[ii].p_memsz == 0) ||
            !(phdr[ii].p_flags & (PF_X | PF_W | PF_R)) ||
            ((phdr[ii].p_paddr & HOST_FLASH_BASE_MASK) == HOST_IFLASH_BASE_ADDR) ||
            ((phdr[ii].p_paddr & HOST_FLASH_BASE_MASK) == HOST_DFLASH_BASE_ADDR))
        {
            mctrl_print("Loading ELF blob %d - unloadable, skipping\n", ii);
            continue;
        }

        ret = load_elf_blob(firmware, transport, ii, phdr[ii].p_offset,
            align_size(phdr[ii].p_memsz, phdr[ii].p_align), phdr[ii].p_paddr);
        if (ret < 0)
        {
            free(phdr);
            return ret;
        }
    }

    free(phdr);
    return 0;
}

int load_elf_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Read an ELF file and load it onto a chip",
                     args.file = arg_file1("f", "file", NULL, "filename of the elf file to load"),
                     args.load_bcf = arg_lit0("b", "bcf", "load a BCF (Board Configuration File)"),
                     args.country = arg_rex0("c", "country", "([A-Z]{2})", "<country code>", 0,
                                             "BCF country code"));
    return 0;
}

int load_elf(struct morsectrl *mors, int argc, char *argv[])
{
    FILE *firmware;
    Elf32_Ehdr ehdr;
    int ret = 0;

    firmware = fopen(args.file->filename[0], "rb");
    if (!firmware)
    {
        mctrl_err("Failed to open %s\n", args.file->filename[0]);
        return -ENOENT;
    }

    if (load_file_header(firmware, &ehdr))
    {
        ret = -ENXIO;
        goto exit;
    }

    if (mors->debug)
        print_ehdr(&ehdr);

    if (args.load_bcf->count)
    {
        if (args.country->count == 0)
        {
            mctrl_err("Country code must be specified for BCF load\n");
            fclose(firmware);
            return -EINVAL;
        }
        ret = load_bcf_sections(mors, firmware, &ehdr, args.country->sval[0]);
    }
    else
    {
        if (args.country->count)
        {
            mctrl_err("Country code can only be specified for BCF load\n");
            fclose(firmware);
            return -EINVAL;
        }
        ret = load_blobs(mors, firmware, &ehdr);
    }

exit:
    fclose(firmware);
    return ret;
}

MM_CLI_HANDLER(load_elf, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
