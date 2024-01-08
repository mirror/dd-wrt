
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <typedefs.h>
#include <bcmnvram.h>
#include <utils.h>

static int NVRAMSPACE;
#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "=" VALUE(var)
#pragma message(VAR_NAME_VALUE(NVRAMSPACE))

#if 0
static int *nvram_hash;

static INLINE uint hash(const char *s, int mul)
{
	uint hash = 0;

	while (*s)
		hash = mul * hash + *s++;

	return hash;
}

void main(int argc, char *argv[])
{
	int i;
	char buf[NVRAMSPACE], *name;
	int mul, max;
	int lowest = 255;
	int lowesti = 0;
	int mod = atoi(argv[1]);
	nvram_hash = malloc(mod * 4);
	for (mul = 0; mul < mod; mul++) {
		memset(buf, 0, NVRAMSPACE);
		memset(nvram_hash, 0, mod * 4);
		nvram_getall(buf, sizeof(buf));
		int len;
		for (name = buf; *name; name += len + 1) {
			len = strlen(name);
			for (i = 0; i < len; i++)
				if (name[i] == '=')
					name[i] = 0;
			nvram_hash[hash(name, mul) % mod]++;
		}

		max = 0;
		for (i = 0; i < mod; i++) {
			if (nvram_hash[i] > max)
				max = nvram_hash[i];
			//fprintf(stderr, "%d ",nvram_hash[i]);
		}
		if (max < lowest) {
			lowest = max;
			lowesti = mul;
		}
		fprintf(stderr, "\n%d = %d\n", mul, max);
	}

	fprintf(stderr, "\n%d = %d\n", lowesti, lowest);
}
#endif

/* 
 * NVRAM utility 
 */
static int nvram_main(int argc, char **argv)
{
	char *name, *value, *buf;
	int size;
	NVRAMSPACE = nvram_size();
	if (NVRAMSPACE < 0) {
		fprintf(stderr, "error in nvram driver\n");
		return -1;
	}
	buf = malloc(NVRAMSPACE);

	/* 
	 * Skip program name 
	 */
	--argc;
	++argv;

	if (!*argv) {
		fprintf(stderr,
			"Usage: nvram [get name] [set name=value] [unset name] [commit] [show|getall]\n" //
			"             [clear|erase] [backup filename] [restore filename]\n" //
			"\n" //
			"get name         : Displays current value or string for the given variable name.\n" //
			"set name=value   : Insert new value or \"string\" for the variable name specified.\n" //
			"unset name       : Removes by name both NVRAM variable name and value or string.\n" //
			"commit           : Writes pending data operations to NVRAM, flash or filesystem.\n" //
			"show | getall    : Displays a list of all current NVRAM variable names and data.\n" //
			"clear | erase    : Delete all NVRAM names and data, but retain system variables.\n" //
			"backup filename  : Backup all stored NVRAM variables data to specified filename.\n" //
			"restore filename : Restore NVRAM names and data, not overwrite system variables.\n" //
			"--force          : WARNING override device name compatibility check for restore.\n");
		exit(0);
	}

	/* 
	 * Process the remaining arguments. 
	 */
	int force = 0;
	for (; *argv; argv++) {
		if (!strncmp(*argv, "get", 3) && strncmp(*argv, "getall", 6)) {
			if (*++argv) {
				if (nvram_exists(*argv))
					puts(nvram_safe_get(*argv));
			}
		} else if (!strncmp(*argv, "set", 3)) {
			if (*++argv) {
				strncpy(value = buf, *argv, NVRAM_SPACE);
				name = strsep(&value, "=");
				nvram_set(name, value);
			}
		} else if (!strncmp(*argv, "unset", 5)) {
			if (*++argv)
				nvram_unset(*argv);
		} else if (!strncmp(*argv, "commit", 5)) {
			nvram_commit();
		} else if (!strncmp(*argv, "clear", 5) ||
			   !strncmp(*argv, "erase", 5)) {
			nvram_clear();
			nvram_commit();
		} else if (!strncmp(*argv, "show", 4) ||
			   !strncmp(*argv, "getall", 6)) {
			nvram_getall(buf, NVRAMSPACE);
			for (name = buf; *name; name += strlen(name) + 1)
				puts(name);
			size = sizeof(struct nvram_header) + (long)name -
			       (long)buf;
			fprintf(stderr, "size: %d bytes (%d left)\n", size,
				NVRAMSPACE - size);
		} else if (!strncmp(*argv, "backup", 6)) {
			if (*++argv) {
				int ret = nvram_backup(*argv);
				if (ret < 0) {
					fprintf(stderr, "can't write %s\n",
						*argv);
					free(buf);
					return 1;
				}
			}
		} else if (!strncmp(*argv, "--force", 7)) {
			force = 1;
		} else if (!strncmp(*argv, "restore", 7)) {
			if (*++argv) {
				int ret = nvram_restore(*argv, force);
				if (ret == -1) {
					fprintf(stderr, "can't write %s\n",
						*argv);
					free(buf);
					return 1;
				}
				if (ret == -2) {
					fprintf(stderr, "file %s broken\n",
						*argv);
					free(buf);
					return 1;
				}
			}
		}
		if (!*argv)
			break;
	}

	free(buf);
	return 0;
}
