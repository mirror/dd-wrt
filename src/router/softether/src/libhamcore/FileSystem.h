#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdbool.h>
#include <stdio.h>

FILE *Ham_FileOpen(const char *path, const bool write);
bool Ham_FileClose(FILE *file);

bool Ham_FileRead(FILE *file, void *dst, const size_t size);
bool Ham_FileWrite(FILE *file, const void *src, const size_t size);

bool Ham_FileSeek(FILE *file, const size_t offset);

size_t Ham_FileSize(const char *path);

const char *Ham_PathRelativeToBase(const char *full, const char *base);

#endif
