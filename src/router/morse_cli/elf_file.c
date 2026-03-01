/*
 * Copyright 2022-2023 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
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

/**
 * @brief Get the ELF32 file header
 *
 * @param data Pointer to the start of the ELF file loaded into memory.
 * @param ehdr Pointer to ELF header struct to populate.
 * @return      0 on success otherwise relevant error.
 */
static int get_file_header(const uint8_t *data, Elf32_Ehdr *ehdr)
{
    Elf32_Ehdr *p = (Elf32_Ehdr *)data;

    /* Magic check */
    if ((p->e_ident[EI_MAG0] != ELFMAG0) ||
        (p->e_ident[EI_MAG1] != ELFMAG1) ||
        (p->e_ident[EI_MAG2] != ELFMAG2) ||
        (p->e_ident[EI_MAG3] != ELFMAG3))
    {
        mctrl_err("Magic check failed 0x%02X 0x%02X 0x%02X 0x%02X\n",
               p->e_ident[EI_MAG0], p->e_ident[EI_MAG1], p->e_ident[EI_MAG2], p->e_ident[EI_MAG3]);
        return -EBADF;
    }

    /* elf32 and little endian */
    if ((p->e_ident[EI_DATA]  != ELFDATA2LSB) ||
        (p->e_ident[EI_CLASS] != ELFCLASS32))
    {
        mctrl_err("ELF not LE and 32bit\n");
        return -EINVAL;
    }

    memcpy(ehdr->e_ident, p->e_ident, sizeof(ehdr->e_ident));

    ehdr->e_phoff     = le32toh((__force __le32)p->e_phoff);
    ehdr->e_phentsize = le16toh((__force __le16)p->e_phentsize);
    ehdr->e_phnum     = le16toh((__force __le16)p->e_phnum);
    ehdr->e_shoff = le32toh((__force __le32)p->e_shoff);
    ehdr->e_shentsize = le16toh((__force __le16)p->e_shentsize);
    ehdr->e_shnum     = le16toh((__force __le16)p->e_shnum);
    ehdr->e_shstrndx = le16toh((__force __le16)p->e_shstrndx);

    return 0;
}


/*
* Get the ii-th section header and fill in the details in shdr
*/
static int get_section_header(const uint8_t *data, const Elf32_Ehdr *ehdr, Elf32_Shdr *shdr, int ii)
{
    Elf32_Shdr *p = (Elf32_Shdr *)(data + ehdr->e_shoff +
                       (ii * ehdr->e_shentsize));

    shdr->sh_name = le32toh((__force __le32)p->sh_name);
    shdr->sh_type = le32toh((__force __le32)p->sh_type);
    shdr->sh_offset = le32toh((__force __le32)p->sh_offset);
    shdr->sh_addr = le32toh((__force __le32)p->sh_addr);
    shdr->sh_size = le32toh((__force __le32)p->sh_size);
    shdr->sh_flags = le32toh((__force __le32)p->sh_flags);

    return 0;
}

/**
 * @brief Load an arbitray section of an ELF file into memory.
 *
 * @note This is useful for loading program/section headers or the sections themselves.
 *
 * @param infile    The ELF file to load data from.
 * @param offset    Offset from the start of the ELF file.
 * @param size      Size of the data to read into memory.
 * @param buf       Pointer to buffer to read memory into. If the buffer is NULL then one will be
 *                  allocated.
 * @return          0 on success otherwise relevant error.
 */
static int elf_file_load_binary_data(FILE *infile, Elf32_Off offset, size_t size, uint8_t **buf)
{
    size_t octets_read = 0;
    struct stat file_stats;
    size_t filesize;
    bool mallocd = false;

    if (!buf || !infile || fstat(fileno(infile), &file_stats))
        return -ENOENT;

    filesize = file_stats.st_size;

    if (fseek(infile, offset, SEEK_SET))
        return -EIO;

    if (!*buf)
    {
        *buf = malloc(size);
        mallocd = true;
    }

    if (!*buf)
    {
        return -ENXIO;
    }

    if ((filesize - offset) < size)
    {
        mctrl_err("Error file read size greater than remaining file size (%d < %d)\n",
               (int) (filesize - offset), (int) size);
        mctrl_err("File size, offset, read size: 0x%08x, 0x%08x, 0x%08x\n",
               (int) filesize, offset, (int) size);
    }

    while ((octets_read < (filesize - offset)) && (octets_read < size))
    {
        octets_read += fread(*buf + octets_read, sizeof(uint8_t),  size - octets_read, infile);
        if (ferror(infile))
        {
            mctrl_err("Error reading file\n");
            if (mallocd)
            {
                free(*buf);
                *buf = NULL;
            }

            return -EIO;
        }
    }

    return 0;
}

/**
 * @brief Get the program headers from an ELF32 file.
 *
 * @param infile    The ELF file to get program headers from.
 * @param offset    The offset from the start of the ELF file the program headers are located.
 * @param count     The number of program headers to get.
 * @return          a pointer to a dynamically allocated array of program headers on success,
 *                  otherwise NULL.
 */
static Elf32_Phdr *elf_file_load_program_headers(FILE *infile, Elf32_Off offset, Elf32_Half count)
{
    Elf32_Phdr *phdr = NULL;
    int ii;

    if (elf_file_load_binary_data(infile, offset, sizeof(*phdr) * count, (uint8_t **)(&phdr)))
        return NULL;

    /* Fix endianess. */
    for (ii = 0; ii < count; ii++)
    {
        phdr[ii].p_type = le32toh((__force __le32)phdr[ii].p_type);
        phdr[ii].p_offset = le32toh((__force __le32)phdr[ii].p_offset);
        phdr[ii].p_vaddr = le32toh((__force __le32)phdr[ii].p_vaddr);
        phdr[ii].p_paddr = le32toh((__force __le32)phdr[ii].p_paddr);
        phdr[ii].p_filesz = le32toh((__force __le32)phdr[ii].p_filesz);
        phdr[ii].p_memsz = le32toh((__force __le32)phdr[ii].p_memsz);
        phdr[ii].p_align = le32toh((__force __le32)phdr[ii].p_align);
    }

    return phdr;
}

/**
 * @brief Get the section headers from an ELF32 file.
 *
 * @param infile    The ELF file to get section headers from.
 * @param offset    The offset from the start of the ELF file the section headers are located.
 * @param count     The number of program headers to get.
 * @return          a pointer to a dynamically allocated array of section headers on success,
 *                  otherwise NULL.
 */
Elf32_Shdr *elf_file_load_section_headers(FILE *infile, Elf32_Off offset, Elf32_Half count)
{
    Elf32_Shdr *shdr = NULL;
    int ii;

    if (elf_file_load_binary_data(infile, offset, sizeof(*shdr) * count, (uint8_t **)(&shdr)))
        return NULL;

    /* Fix endianess. */
    for (ii = 0; ii < count; ii++)
    {
        shdr[ii].sh_name = le32toh((__force __le32)shdr[ii].sh_name);
        shdr[ii].sh_type = le32toh((__force __le32)shdr[ii].sh_type);
        shdr[ii].sh_flags = le32toh((__force __le32)shdr[ii].sh_flags);
        shdr[ii].sh_addr = le32toh((__force __le32)shdr[ii].sh_addr);
        shdr[ii].sh_offset = le32toh((__force __le32)shdr[ii].sh_offset);
        shdr[ii].sh_size = le32toh((__force __le32)shdr[ii].sh_size);
        shdr[ii].sh_link = le32toh((__force __le32)shdr[ii].sh_link);
        shdr[ii].sh_info = le32toh((__force __le32)shdr[ii].sh_info);
        shdr[ii].sh_addralign = le32toh((__force __le32)shdr[ii].sh_addralign);
        shdr[ii].sh_entsize = le32toh((__force __le32)shdr[ii].sh_entsize);
    }

    return shdr;
}

/**
 * @brief Load the ELF32 header into a given struct.
 *
 * @param infile    The ELF file to load the header from.
 * @param ehdr      Pointer to the ELF header structure to populate.
 * @return          0 on success, otherwise relevant error.
 */
static int load_file_header(FILE *infile, Elf32_Ehdr *ehdr)
{
    uint8_t *data = NULL;

    if (elf_file_load_binary_data(infile, 0, sizeof(*ehdr), &data))
        return -ENOENT;

    if (get_file_header(data, ehdr))
    {
        free(data);
        return -ENXIO;
    }

    free(data);

    return 0;
}

/*
 * Load the offchip statistics from an ELF data structure.
 *
 * An array of n_rec struct statistics_offchip_data elements is allocated in the stats_handle.
 * It is the caller's responsibility to free the array.
 */
int morse_stats_load(struct statistics_offchip_data **stats_handle, size_t *n_rec,
                     const uint8_t *data)
{
    int ii;
    int ret = 0;
    Elf32_Ehdr ehdr;
    Elf32_Shdr shdr;
    Elf32_Shdr sh_strtab;
    const char *sh_strs;

    if (get_file_header((const uint8_t *)data, &ehdr) != 0) {
        mctrl_err("Wrong file format\n");
        return -ENOENT;
    }

    if (get_section_header(data, &ehdr, &sh_strtab, ehdr.e_shstrndx) != 0) {
        mctrl_err("Invalid firmware - missing string table\n");
        return -ENXIO;
    }

    sh_strs = (const char *)data + sh_strtab.sh_offset;

    /* first run through the ELF file to get the total size of the stats */
    size_t total_stats_size = 0;
    for (ii = 0; ii < ehdr.e_shnum; ii++)
    {
        if (get_section_header(data, &ehdr, &shdr, ii) != 0)
            continue;

        if (strstr(sh_strs + shdr.sh_name, "_offchip_"))
        {
            total_stats_size += shdr.sh_size;
        }
    }
    void *blob = malloc(total_stats_size);
    memset(blob, 0, total_stats_size);
    *n_rec = 0;
    if (blob)
    {
        int offset = 0;
        for (ii = 0; ii < ehdr.e_shnum; ii++)
        {
            if (get_section_header(data, &ehdr, &shdr, ii) != 0)
                continue;

            if (strstr(sh_strs + shdr.sh_name, "_offchip_"))
            {
                memcpy(blob + offset, data + shdr.sh_offset, shdr.sh_size);
                offset += shdr.sh_size;
                *n_rec += shdr.sh_size / sizeof(struct statistics_offchip_data);
            }
        }
        *stats_handle = blob;
    }
    else
    {
        *stats_handle = 0;
        ret = -ENXIO;
    }
    return ret;
}

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

void print_shdr(Elf32_Shdr *shdr)
{
    mctrl_print("Elf32_Shdr:\n");
    mctrl_print("\tsh_name:      %u\n", shdr->sh_name);
    mctrl_print("\tsh_type:      %u\n", shdr->sh_type);
    mctrl_print("\tsh_flags:     0x%08x\n", shdr->sh_flags);
    mctrl_print("\tsh_addr:      0x%08x\n", shdr->sh_addr);
    mctrl_print("\tsh_offset:    0x%08x\n", shdr->sh_offset);
    mctrl_print("\tsh_size:      0x%08x\n", shdr->sh_size);
    mctrl_print("\tsh_link:      0x%08x\n", shdr->sh_link);
    mctrl_print("\tsh_info:      %u\n", shdr->sh_info);
    mctrl_print("\tsh_addralign: %u\n", shdr->sh_addralign);
    mctrl_print("\tsh_entsize:   0x%08x\n", shdr->sh_entsize);
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
