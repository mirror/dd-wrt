#include "Hamcore.h"

#include "FileSystem.h"
#include "Memory.h"

#include <stdlib.h>
#include <string.h>

#include <zlib.h>

typedef struct COMPRESSED_FILE
{
	void *Data;
	HAMCORE_FILE File;
} COMPRESSED_FILE;

HAMCORE *HamcoreOpen(const char *path)
{
	if (!path)
	{
		return NULL;
	}

	HAMCORE *hamcore = malloc(sizeof(HAMCORE));
	if (!hamcore)
	{
		return NULL;
	}
	memset(hamcore, 0, sizeof(HAMCORE));

	hamcore->File = Ham_FileOpen(path, false);
	if (!hamcore->File)
	{
		free(hamcore);
		return NULL;
	}

	bool ok = false;

	uint8_t header[HAMCORE_HEADER_SIZE];
	if (!Ham_FileRead(hamcore->File, header, sizeof(header)))
	{
		goto FINAL;
	}

	if (memcmp(header, HAMCORE_HEADER_DATA, sizeof(header)) != 0)
	{
		goto FINAL;
	}

	uint32_t tmp;
	if (!Ham_FileRead(hamcore->File, &tmp, sizeof(tmp)))
	{
		goto FINAL;
	}

	HAMCORE_FILES *files = &hamcore->Files;

	files->Num = BigEndian32(tmp);
	files->List = malloc(sizeof(HAMCORE_FILE) * files->Num);
	if (!files->List)
	{
		return NULL;
	}
	memset(files->List, 0, sizeof(HAMCORE_FILE) * files->Num);

	for (size_t i = 0; i < files->Num; ++i)
	{
		if (!Ham_FileRead(hamcore->File, &tmp, sizeof(tmp)))
		{
			goto FINAL;
		}

		HAMCORE_FILE *file = &files->List[i];

		tmp = BigEndian32(tmp);
		file->Path = malloc(tmp);
		if (tmp >= 1)
		{
			memset(file->Path, 0, tmp);
			--tmp;
		}

		if (!Ham_FileRead(hamcore->File, file->Path, tmp))
		{
			goto FINAL;
		}

		if (!Ham_FileRead(hamcore->File, &tmp, sizeof(tmp)))
		{
			goto FINAL;
		}

		file->OriginalSize = BigEndian32(tmp);

		if (!Ham_FileRead(hamcore->File, &tmp, sizeof(tmp)))
		{
			goto FINAL;
		}

		file->Size = BigEndian32(tmp);

		if (!Ham_FileRead(hamcore->File, &tmp, sizeof(tmp)))
		{
			goto FINAL;
		}

		file->Offset = BigEndian32(tmp);
	}

	ok = true;
FINAL:
	if (!ok)
	{
		HamcoreClose(hamcore);
		hamcore = NULL;
	}

	return hamcore;
}

void HamcoreClose(HAMCORE *hamcore)
{
	if (!hamcore)
	{
		return;
	}

	Ham_FileClose(hamcore->File);

	HAMCORE_FILES *files = &hamcore->Files;
	if (!files->List)
	{
		return;
	}

	for (size_t i = 0; i < files->Num; ++i)
	{
		HAMCORE_FILE *file = &files->List[i];
		if (file->Path)
		{
			free(file->Path);
		}
	}

	free(files->List);
	free(hamcore);
}

const HAMCORE_FILE *HamcoreFind(const HAMCORE *hamcore, const char *path)
{
	if (!hamcore || !path)
	{
		return NULL;
	}

	const HAMCORE_FILES *files = &hamcore->Files;

	for (size_t i = 0; i < files->Num; ++i)
	{
		const HAMCORE_FILE *file = &files->List[i];
		if (strcmp(file->Path, path) == 0)
		{
			return file;
		}
	}

	return NULL;
}

bool HamcoreRead(HAMCORE *hamcore, void *dst, const HAMCORE_FILE *hamcore_file)
{
	if (!hamcore || !dst || !hamcore_file)
	{
		return false;
	}

	if (!Ham_FileSeek(hamcore->File, hamcore_file->Offset))
	{
		return false;
	}

	bool ok = false;

	void *buf = malloc(hamcore_file->Size);
	if (!Ham_FileRead(hamcore->File, buf, hamcore_file->Size))
	{
		goto FINAL;
	}

	uLong dst_size = (uLong)hamcore_file->OriginalSize;
	if (uncompress(dst, &dst_size, buf, (uLong)hamcore_file->Size) != Z_OK)
	{
		goto FINAL;
	}

	if (dst_size != hamcore_file->OriginalSize)
	{
		goto FINAL;
	}

	ok = true;
FINAL:
	free(buf);
	return ok;
}

bool HamcoreBuild(const char *dst_path, const char *base_path, const char **src_paths, const size_t num)
{
	if (!dst_path || !src_paths || num == 0)
	{
		return false;
	}

	COMPRESSED_FILE *compressed_files = calloc(num, sizeof(COMPRESSED_FILE));
	if (!compressed_files)
	{
		return false;
	}

	void *buffer = NULL;
	size_t buffer_size = 0;

	for (size_t i = 0; i < num; ++i)
	{
		const char *path = src_paths[i];
		if (!path)
		{
			continue;
		}

		FILE *handle = Ham_FileOpen(path, false);
		if (!handle)
		{
			fprintf(stderr, "HamcoreBuild(): Failed to open \"%s\", skipping...\n", path);
			continue;
		}

		COMPRESSED_FILE *compressed_file = &compressed_files[i];
		HAMCORE_FILE *file = &compressed_file->File;

		file->OriginalSize = Ham_FileSize(path);
		void *content = malloc(file->OriginalSize);
		int ret = Ham_FileRead(handle, content, file->OriginalSize);
		Ham_FileClose(handle);

		if (!ret)
		{
			fprintf(stderr, "HamcoreBuild(): Failed to read \"%s\", skipping...\n", path);
			free(content);
			continue;
		}

		const size_t wanted_size = CompressionBufferSize(file->OriginalSize);
		if (buffer_size < wanted_size)
		{
			const size_t prev_size = buffer_size;
			buffer_size = wanted_size;
			buffer = realloc(buffer, buffer_size);
			memset((uint8_t *)buffer + prev_size, 0, buffer_size - prev_size);
		}

		file->Size = buffer_size;
		ret = compress(buffer, (uLongf *)&file->Size, content, (uLong)file->OriginalSize);
		free(content);

		if (ret != Z_OK)
		{
			fprintf(stderr, "HamcoreBuild(): Failed to compress \"%s\" with error %d, skipping...\n", path, ret);
			file->Size = 0;
			continue;
		}

		const char *relative_path = base_path ? Ham_PathRelativeToBase(path, base_path) : path;
		if (!relative_path)
		{
			fprintf(stderr, "HamcoreBuild(): Failed to get relative path for \"%s\", skipping...\n", path);
			file->Size = 0;
			continue;
		}

		const size_t path_size = strlen(relative_path) + 1;
		file->Path = malloc(path_size);
		if (!file->Path)
		{
			free(compressed_files);
			free(buffer);
			return false;
		}

		memcpy(file->Path, relative_path, path_size);

		compressed_file->Data = malloc(file->Size);
		if (!compressed_file->Data)
		{
			free(compressed_files);
			free(buffer);
			return false;
		}

		memcpy(compressed_file->Data, buffer, file->Size);
	}

	size_t offset = HAMCORE_HEADER_SIZE;
	// Number of files
	offset += sizeof(uint32_t);

	// File table
	for (size_t i = 0; i < num; ++i)
	{
		const HAMCORE_FILE *file = &compressed_files[i].File;
		if (file->Size == 0)
		{
			continue;
		}

		// Path (length + string)
		offset += sizeof(uint32_t) + strlen(file->Path);
		// Original size
		offset += sizeof(uint32_t);
		// Size
		offset += sizeof(uint32_t);
		// Offset
		offset += sizeof(uint32_t);
	}

	for (size_t i = 0; i < num; ++i)
	{
		HAMCORE_FILE *file = &compressed_files[i].File;
		if (file->Size == 0)
		{
			continue;
		}

		file->Offset = offset;
		offset += file->Size;
	}

	if (buffer_size < offset)
	{
		buffer_size = offset;
		buffer = realloc(buffer, buffer_size);
	}

	void *ptr = buffer;
	Ham_WriteAndSeek(&ptr, HAMCORE_HEADER_DATA, HAMCORE_HEADER_SIZE);
	uint32_t tmp = BigEndian32((uint32_t)num);
	Ham_WriteAndSeek(&ptr, &tmp, sizeof(tmp));

	for (size_t i = 0; i < num; ++i)
	{
		const HAMCORE_FILE *file = &compressed_files[i].File;
		if (file->Size == 0)
		{
			continue;
		}

		const size_t path_length = strlen(file->Path);
		tmp = BigEndian32((uint32_t)path_length + 1);
		Ham_WriteAndSeek(&ptr, &tmp, sizeof(tmp));
		Ham_WriteAndSeek(&ptr, file->Path, path_length);
		free(file->Path);

		tmp = BigEndian32((uint32_t)file->OriginalSize);
		Ham_WriteAndSeek(&ptr, &tmp, sizeof(tmp));

		tmp = BigEndian32((uint32_t)file->Size);
		Ham_WriteAndSeek(&ptr, &tmp, sizeof(tmp));

		tmp = BigEndian32((uint32_t)file->Offset);
		Ham_WriteAndSeek(&ptr, &tmp, sizeof(tmp));
	}

	for (size_t i = 0; i < num; ++i)
	{
		COMPRESSED_FILE *compressed_file = &compressed_files[i];
		Ham_WriteAndSeek(&ptr, compressed_file->Data, compressed_file->File.Size);
		free(compressed_file->Data);
	}

	free(compressed_files);

	bool ok = false;

	FILE *handle = Ham_FileOpen(dst_path, true);
	if (!handle)
	{
		fprintf(stderr, "HamcoreBuild(): Failed to open \"%s\"!\n", dst_path);
		goto FINAL;
	}

	if (!Ham_FileWrite(handle, buffer, buffer_size))
	{
		fprintf(stderr, "HamcoreBuild(): Failed to write \"%s\"!\n", dst_path);
		goto FINAL;
	}

	ok = true;
FINAL:
	Ham_FileClose(handle);
	free(buffer);
	return ok;
}
