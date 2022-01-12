#ifndef HAMCORE_H
#define HAMCORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define HAMCORE_HEADER_DATA "HamCore"
#define HAMCORE_HEADER_SIZE 7

typedef struct HAMCORE_FILE
{
	char *Path;
	size_t Offset;
	size_t Size;
	size_t OriginalSize;
} HAMCORE_FILE;

typedef struct HAMCORE_FILES
{
	size_t Num;
	HAMCORE_FILE *List;
} HAMCORE_FILES;

typedef struct HAMCORE
{
	FILE *File;
	HAMCORE_FILES Files;
} HAMCORE;

HAMCORE *HamcoreOpen(const char *path);
void HamcoreClose(HAMCORE *hamcore);

const HAMCORE_FILE *HamcoreFind(const HAMCORE *hamcore, const char *path);
bool HamcoreRead(HAMCORE *hamcore, void *dst, const HAMCORE_FILE *file);

bool HamcoreBuild(const char *dst_path, const char *base_path, const char **src_paths, const size_t num);

#endif
