/*
 * Copyright 2022-2026 Morse Micro
 *
 */

#pragma once

#include "transport/transport.h"

/**
 * @brief Get the ELF32 file header
 *
 * @param data Pointer to the start of the ELF file loaded into memory.
 * @param ehdr Pointer to ELF header struct to populate.
 * @return      0 on success otherwise relevant error.
 */
int get_file_header(const uint8_t *data, Elf32_Ehdr *ehdr);

/*
* Get the ii-th section header and fill in the details in shdr
*/
int get_section_header(const uint8_t *data, const Elf32_Ehdr *ehdr, Elf32_Shdr *shdr, int ii);

/**
 * @brief Get the program headers from an ELF32 file.
 *
 * @param infile    The ELF file to get program headers from.
 * @param offset    The offset from the start of the ELF file the program headers are located.
 * @param count     The number of program headers to get.
 * @return          a pointer to a dynamically allocated array of program headers on success,
 *                  otherwise NULL.
 */
Elf32_Phdr *elf_file_load_program_headers(FILE *infile, Elf32_Off offset, Elf32_Half count);

/**
 * @brief Get the section headers from an ELF32 file.
 *
 * @param infile    The ELF file to get section headers from.
 * @param offset    The offset from the start of the ELF file the section headers are located.
 * @param count     The number of program headers to get.
 * @return          a pointer to a dynamically allocated array of section headers on success,
 *                  otherwise NULL.
 */
Elf32_Shdr *elf_file_load_section_headers(FILE *infile, Elf32_Off offset, Elf32_Half count);

/**
 * @brief Load the ELF32 header into a given struct.
 *
 * @param infile    The ELF file to load the header from.
 * @param ehdr      Pointer to the ELF header structure to populate.
 * @return          0 on success, otherwise relevant error.
 */
int load_file_header(FILE *infile, Elf32_Ehdr *ehdr);

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
int elf_file_load_binary_data(FILE *infile, Elf32_Off offset, size_t size, uint8_t **buf);

/**
 * @brief Get the program headers from an ELF32 file.
 *
 * @param infile    The ELF file to get program headers from.
 * @param offset    The offset from the start of the ELF file the program headers are located.
 * @param count     The number of program headers to get.
 * @return          a pointer to a dynamically allocated array of program headers on success,
 *                  otherwise NULL.
 */
Elf32_Phdr *elf_file_load_program_headers(FILE *infile, Elf32_Off offset, Elf32_Half count);
