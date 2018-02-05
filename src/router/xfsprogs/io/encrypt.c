/*
 * Copyright (c) 2016 Google, Inc.  All Rights Reserved.
 *
 * Author: Eric Biggers <ebiggers@google.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "platform_defs.h"
#include "command.h"
#include "init.h"
#include "path.h"
#include "io.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

/*
 * We may have to declare the fscrypt ioctls ourselves because someone may be
 * compiling xfsprogs with old kernel headers.  And since some old versions of
 * <linux/fs.h> declared the policy struct and ioctl numbers but not the flags
 * and modes, our declarations must be split into two conditional blocks.
 */

/* Policy struct and ioctl numbers */
#ifndef FS_IOC_SET_ENCRYPTION_POLICY
#define FS_KEY_DESCRIPTOR_SIZE  8

struct fscrypt_policy {
	__u8 version;
	__u8 contents_encryption_mode;
	__u8 filenames_encryption_mode;
	__u8 flags;
	__u8 master_key_descriptor[FS_KEY_DESCRIPTOR_SIZE];
} __attribute__((packed));

#define FS_IOC_SET_ENCRYPTION_POLICY	_IOR('f', 19, struct fscrypt_policy)
#define FS_IOC_GET_ENCRYPTION_PWSALT	_IOW('f', 20, __u8[16])
#define FS_IOC_GET_ENCRYPTION_POLICY	_IOW('f', 21, struct fscrypt_policy)
#endif /* FS_IOC_SET_ENCRYPTION_POLICY */

/* Policy flags and encryption modes */
#ifndef FS_ENCRYPTION_MODE_AES_256_XTS
#define FS_POLICY_FLAGS_PAD_4		0x00
#define FS_POLICY_FLAGS_PAD_8		0x01
#define FS_POLICY_FLAGS_PAD_16		0x02
#define FS_POLICY_FLAGS_PAD_32		0x03
#define FS_POLICY_FLAGS_PAD_MASK	0x03
#define FS_POLICY_FLAGS_VALID		0x03

#define FS_ENCRYPTION_MODE_INVALID	0
#define FS_ENCRYPTION_MODE_AES_256_XTS	1
#define FS_ENCRYPTION_MODE_AES_256_GCM	2
#define FS_ENCRYPTION_MODE_AES_256_CBC	3
#define FS_ENCRYPTION_MODE_AES_256_CTS	4
#endif /* FS_ENCRYPTION_MODE_AES_256_XTS */

static cmdinfo_t get_encpolicy_cmd;
static cmdinfo_t set_encpolicy_cmd;

static void
set_encpolicy_help(void)
{
	printf(_(
"\n"
" assign an encryption policy to the currently open file\n"
"\n"
" Examples:\n"
" 'set_encpolicy' - assign policy with default key [0000000000000000]\n"
" 'set_encpolicy 0000111122223333' - assign policy with specified key\n"
"\n"
" -c MODE -- contents encryption mode\n"
" -n MODE -- filenames encryption mode\n"
" -f FLAGS -- policy flags\n"
" -v VERSION -- version of policy structure\n"
"\n"
" MODE can be numeric or one of the following predefined values:\n"
"    AES-256-XTS, AES-256-CTS, AES-256-GCM, AES-256-CBC\n"
" FLAGS and VERSION must be numeric.\n"
"\n"
" Note that it's only possible to set an encryption policy on an empty\n"
" directory.  It's then inherited by new files and subdirectories.\n"
"\n"));
}

static const struct {
	__u8 mode;
	const char *name;
} available_modes[] = {
	{FS_ENCRYPTION_MODE_AES_256_XTS, "AES-256-XTS"},
	{FS_ENCRYPTION_MODE_AES_256_CTS, "AES-256-CTS"},
	{FS_ENCRYPTION_MODE_AES_256_GCM, "AES-256-GCM"},
	{FS_ENCRYPTION_MODE_AES_256_CBC, "AES-256-CBC"},
};

static bool
parse_byte_value(const char *arg, __u8 *value_ret)
{
	long value;
	char *tmp;

	value = strtol(arg, &tmp, 0);
	if (value < 0 || value > 255 || tmp == arg || *tmp != '\0')
		return false;
	*value_ret = value;
	return true;
}

static bool
parse_mode(const char *arg, __u8 *mode_ret)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(available_modes); i++) {
		if (strcmp(arg, available_modes[i].name) == 0) {
			*mode_ret = available_modes[i].mode;
			return true;
		}
	}

	return parse_byte_value(arg, mode_ret);
}

static const char *
mode2str(__u8 mode)
{
	static char buf[32];
	int i;

	for (i = 0; i < ARRAY_SIZE(available_modes); i++)
		if (mode == available_modes[i].mode)
			return available_modes[i].name;

	sprintf(buf, "0x%02x", mode);
	return buf;
}

static const char *
keydesc2str(__u8 master_key_descriptor[FS_KEY_DESCRIPTOR_SIZE])
{
	static char buf[2 * FS_KEY_DESCRIPTOR_SIZE + 1];
	int i;

	for (i = 0; i < FS_KEY_DESCRIPTOR_SIZE; i++)
		sprintf(&buf[2 * i], "%02x", master_key_descriptor[i]);

	return buf;
}

static int
get_encpolicy_f(int argc, char **argv)
{
	struct fscrypt_policy policy;

	if (ioctl(file->fd, FS_IOC_GET_ENCRYPTION_POLICY, &policy) < 0) {
		fprintf(stderr, "%s: failed to get encryption policy: %s\n",
			file->name, strerror(errno));
		exitcode = 1;
		return 0;
	}

	printf("Encryption policy for %s:\n", file->name);
	printf("\tPolicy version: %u\n", policy.version);
	printf("\tMaster key descriptor: %s\n",
	       keydesc2str(policy.master_key_descriptor));
	printf("\tContents encryption mode: %u (%s)\n",
	       policy.contents_encryption_mode,
	       mode2str(policy.contents_encryption_mode));
	printf("\tFilenames encryption mode: %u (%s)\n",
	       policy.filenames_encryption_mode,
	       mode2str(policy.filenames_encryption_mode));
	printf("\tFlags: 0x%02x\n", policy.flags);
	return 0;
}

static int
set_encpolicy_f(int argc, char **argv)
{
	int c;
	struct fscrypt_policy policy;

	/* Initialize the policy structure with default values */
	memset(&policy, 0, sizeof(policy));
	policy.contents_encryption_mode = FS_ENCRYPTION_MODE_AES_256_XTS;
	policy.filenames_encryption_mode = FS_ENCRYPTION_MODE_AES_256_CTS;
	policy.flags = FS_POLICY_FLAGS_PAD_16;

	/* Parse options */
	while ((c = getopt(argc, argv, "c:n:f:v:")) != EOF) {
		switch (c) {
		case 'c':
			if (!parse_mode(optarg,
					&policy.contents_encryption_mode)) {
				fprintf(stderr, "invalid contents encryption "
					"mode: %s\n", optarg);
				return 0;
			}
			break;
		case 'n':
			if (!parse_mode(optarg,
					&policy.filenames_encryption_mode)) {
				fprintf(stderr, "invalid filenames encryption "
					"mode: %s\n", optarg);
				return 0;
			}
			break;
		case 'f':
			if (!parse_byte_value(optarg, &policy.flags)) {
				fprintf(stderr, "invalid flags: %s\n", optarg);
				return 0;
			}
			break;
		case 'v':
			if (!parse_byte_value(optarg, &policy.version)) {
				fprintf(stderr, "invalid policy version: %s\n",
					optarg);
				return 0;
			}
			break;
		default:
			return command_usage(&set_encpolicy_cmd);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 1)
		return command_usage(&set_encpolicy_cmd);

	/* Parse key descriptor if specified */
	if (argc > 0) {
		const char *keydesc = argv[0];
		char *tmp;
		unsigned long long x;
		int i;

		if (strlen(keydesc) != FS_KEY_DESCRIPTOR_SIZE * 2) {
			fprintf(stderr, "invalid key descriptor: %s\n",
				keydesc);
			return 0;
		}

		x = strtoull(keydesc, &tmp, 16);
		if (tmp == keydesc || *tmp != '\0') {
			fprintf(stderr, "invalid key descriptor: %s\n",
				keydesc);
			return 0;
		}

		for (i = 0; i < FS_KEY_DESCRIPTOR_SIZE; i++) {
			policy.master_key_descriptor[i] = x >> 56;
			x <<= 8;
		}
	}

	/* Set the encryption policy */
	if (ioctl(file->fd, FS_IOC_SET_ENCRYPTION_POLICY, &policy) < 0) {
		fprintf(stderr, "%s: failed to set encryption policy: %s\n",
			file->name, strerror(errno));
		exitcode = 1;
		return 0;
	}

	return 0;
}

void
encrypt_init(void)
{
	get_encpolicy_cmd.name = "get_encpolicy";
	get_encpolicy_cmd.cfunc = get_encpolicy_f;
	get_encpolicy_cmd.argmin = 0;
	get_encpolicy_cmd.argmax = 0;
	get_encpolicy_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	get_encpolicy_cmd.oneline =
		_("display the encryption policy of the current file");

	set_encpolicy_cmd.name = "set_encpolicy";
	set_encpolicy_cmd.cfunc = set_encpolicy_f;
	set_encpolicy_cmd.args =
		_("[-c mode] [-n mode] [-f flags] [-v version] [keydesc]");
	set_encpolicy_cmd.argmin = 0;
	set_encpolicy_cmd.argmax = -1;
	set_encpolicy_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	set_encpolicy_cmd.oneline =
		_("assign an encryption policy to the current file");
	set_encpolicy_cmd.help = set_encpolicy_help;

	add_command(&get_encpolicy_cmd);
	add_command(&set_encpolicy_cmd);
}
