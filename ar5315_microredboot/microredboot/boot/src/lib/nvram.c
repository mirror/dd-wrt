/*
 * nvram.c - simple dd-wrt nvram implementation (read only)
 *
 * copyright 2009 Sebastian Gottschall / NewMedia-NET GmbH / DD-WRT.COM
 * licensed under GPL conditions
 */

struct nvram_header {
	__u32 magic;
	__u32 len;
	__u32 crc_ver_init;	/* 0:7 crc, 8:15 ver, 16:27 init, mem. test 28, 29-31 reserved */
	__u32 config_refresh;	/* 0:15 config, 16:31 refresh */
	__u32 config_ncdl;	/* ncdl values for memc */
};

struct nvram_tuple {
	char *name;
	char *value;
	struct nvram_tuple *next;
};

#define NVRAM_SPACE 0x10000
#define NVRAM_MAGIC			0x48534C46	/* 'NVFL' */

static char nvram_buf[65536] __attribute__((aligned(4096))) = {
0};				//

static void nvram_init(void)
{
	struct nvram_header *header;
	__u32 off, lim;
	int i;
	int a;
	flashdetect();
	for (i = 0; i < 4; i++) {
		header =
		    (struct nvram_header *)(flashbase + flashsize -
					    (sectorsize * (3 + i)));
		if (header->magic == NVRAM_MAGIC && header->len > 0
		    && header->len <= NVRAM_SPACE) {
			/* copy nvram area to ram */
			printf
			    ("DD-WRT NVRAM with size = %d found on [0x%08X]\n",
			     header->len, header);
			nvramdetect = (unsigned int)header;
			unsigned int *src = (unsigned int *)header;
			unsigned int *dst = (unsigned int *)nvram_buf;
			for (a = 0; a < NVRAM_SPACE / 4; a++)
				dst[a] = src[a];
			return;
		} else {
			/* set mem array to zero */
			unsigned int *dst = (unsigned int *)nvram_buf;
			for (a = 0; a < NVRAM_SPACE / 4; a++)
				dst[a] = 0;
		}
	}
}

static char *nvram_get(const char *name)
{
	char *var, *value, *end, *eq;

	if (!name)
		return NULL;

	if (!nvram_buf[0])
		return NULL;

	/* Look for name=value and return value */
	var = &nvram_buf[sizeof(struct nvram_header)];
	end = nvram_buf + sizeof(nvram_buf) - 2;
	end[0] = end[1] = '\0';
	for (; *var; var = value + strlen(value) + 1) {
		if (!(eq = strchr(var, '=')))
			break;
		value = eq + 1;
		if ((eq - var) == strlen(name)
		    && strncmp(var, name, (eq - var)) == 0)
			return value;
	}

	return NULL;
}
