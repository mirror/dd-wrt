// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2016, 2019 Google LLC
 * Author: Eric Biggers <ebiggers@google.com>
 */

#ifdef OVERRIDE_SYSTEM_FSCRYPT_ADD_KEY_ARG
#  define fscrypt_add_key_arg sys_fscrypt_add_key_arg
#endif
#include "platform_defs.h"
#include "command.h"
#include "init.h"
#include "libfrog/paths.h"
#include "io.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

/*
 * Declare the fscrypt ioctls if needed, since someone may be compiling xfsprogs
 * with old kernel headers.  But <linux/fs.h> has already been included, so be
 * careful not to declare things twice.
 */

/* first batch of ioctls (Linux headers v4.6+) */
#ifndef FS_IOC_SET_ENCRYPTION_POLICY
#define fscrypt_policy fscrypt_policy_v1
#define FS_IOC_SET_ENCRYPTION_POLICY		_IOR('f', 19, struct fscrypt_policy)
#define FS_IOC_GET_ENCRYPTION_PWSALT		_IOW('f', 20, __u8[16])
#define FS_IOC_GET_ENCRYPTION_POLICY		_IOW('f', 21, struct fscrypt_policy)
#endif

/*
 * Second batch of ioctls (Linux headers v5.4+), plus some renamings from FS_ to
 * FSCRYPT_.  We don't bother defining the old names here.
 */
#ifndef FS_IOC_GET_ENCRYPTION_POLICY_EX

#define FSCRYPT_POLICY_FLAGS_PAD_4		0x00
#define FSCRYPT_POLICY_FLAGS_PAD_8		0x01
#define FSCRYPT_POLICY_FLAGS_PAD_16		0x02
#define FSCRYPT_POLICY_FLAGS_PAD_32		0x03
#define FSCRYPT_POLICY_FLAGS_PAD_MASK		0x03
#define FSCRYPT_POLICY_FLAG_DIRECT_KEY		0x04

#define FSCRYPT_MODE_AES_256_XTS		1
#define FSCRYPT_MODE_AES_256_CTS		4
#define FSCRYPT_MODE_AES_128_CBC		5
#define FSCRYPT_MODE_AES_128_CTS		6
#define FSCRYPT_MODE_ADIANTUM			9

/*
 * In the headers for Linux v4.6 through v5.3, 'struct fscrypt_policy_v1' is
 * already defined under its old name, 'struct fscrypt_policy'.  But it's fine
 * to define it under its new name too.
 *
 * Note: "v1" policies really are version "0" in the API.
 */
#define FSCRYPT_POLICY_V1		0
#define FSCRYPT_KEY_DESCRIPTOR_SIZE	8
struct fscrypt_policy_v1 {
	__u8 version;
	__u8 contents_encryption_mode;
	__u8 filenames_encryption_mode;
	__u8 flags;
	__u8 master_key_descriptor[FSCRYPT_KEY_DESCRIPTOR_SIZE];
};

#define FSCRYPT_POLICY_V2		2
#define FSCRYPT_KEY_IDENTIFIER_SIZE	16
struct fscrypt_policy_v2 {
	__u8 version;
	__u8 contents_encryption_mode;
	__u8 filenames_encryption_mode;
	__u8 flags;
	__u8 __reserved[4];
	__u8 master_key_identifier[FSCRYPT_KEY_IDENTIFIER_SIZE];
};

#define FSCRYPT_MAX_KEY_SIZE		64

#define FS_IOC_GET_ENCRYPTION_POLICY_EX		_IOWR('f', 22, __u8[9]) /* size + version */
struct fscrypt_get_policy_ex_arg {
	__u64 policy_size; /* input/output */
	union {
		__u8 version;
		struct fscrypt_policy_v1 v1;
		struct fscrypt_policy_v2 v2;
	} policy; /* output */
};

#define FSCRYPT_KEY_SPEC_TYPE_DESCRIPTOR	1
#define FSCRYPT_KEY_SPEC_TYPE_IDENTIFIER	2
struct fscrypt_key_specifier {
	__u32 type;	/* one of FSCRYPT_KEY_SPEC_TYPE_* */
	__u32 __reserved;
	union {
		__u8 __reserved[32]; /* reserve some extra space */
		__u8 descriptor[FSCRYPT_KEY_DESCRIPTOR_SIZE];
		__u8 identifier[FSCRYPT_KEY_IDENTIFIER_SIZE];
	} u;
};

/* FS_IOC_ADD_ENCRYPTION_KEY is defined later */

#define FS_IOC_REMOVE_ENCRYPTION_KEY		_IOWR('f', 24, struct fscrypt_remove_key_arg)
#define FS_IOC_REMOVE_ENCRYPTION_KEY_ALL_USERS	_IOWR('f', 25, struct fscrypt_remove_key_arg)
struct fscrypt_remove_key_arg {
	struct fscrypt_key_specifier key_spec;
#define FSCRYPT_KEY_REMOVAL_STATUS_FLAG_FILES_BUSY	0x00000001
#define FSCRYPT_KEY_REMOVAL_STATUS_FLAG_OTHER_USERS	0x00000002
	__u32 removal_status_flags;	/* output */
	__u32 __reserved[5];
};

#define FS_IOC_GET_ENCRYPTION_KEY_STATUS	_IOWR('f', 26, struct fscrypt_get_key_status_arg)
struct fscrypt_get_key_status_arg {
	/* input */
	struct fscrypt_key_specifier key_spec;
	__u32 __reserved[6];

	/* output */
#define FSCRYPT_KEY_STATUS_ABSENT		1
#define FSCRYPT_KEY_STATUS_PRESENT		2
#define FSCRYPT_KEY_STATUS_INCOMPLETELY_REMOVED	3
	__u32 status;
#define FSCRYPT_KEY_STATUS_FLAG_ADDED_BY_SELF   0x00000001
	__u32 status_flags;
	__u32 user_count;
	__u32 __out_reserved[13];
};

#endif /* !FS_IOC_GET_ENCRYPTION_POLICY_EX */

/*
 * Since the key_id field was added later than struct fscrypt_add_key_arg
 * itself, we may need to override the system definition to get that field.
 */
#if !defined(FS_IOC_ADD_ENCRYPTION_KEY) || \
	defined(OVERRIDE_SYSTEM_FSCRYPT_ADD_KEY_ARG)
#undef fscrypt_add_key_arg
struct fscrypt_add_key_arg {
	struct fscrypt_key_specifier key_spec;
	__u32 raw_size;
	__u32 key_id;
	__u32 __reserved[8];
	__u8 raw[];
};
#endif

#ifndef FS_IOC_ADD_ENCRYPTION_KEY
#  define FS_IOC_ADD_ENCRYPTION_KEY		_IOWR('f', 23, struct fscrypt_add_key_arg)
#endif

static const struct {
	__u8 mode;
	const char *name;
} available_modes[] = {
	{FSCRYPT_MODE_AES_256_XTS, "AES-256-XTS"},
	{FSCRYPT_MODE_AES_256_CTS, "AES-256-CTS"},
	{FSCRYPT_MODE_AES_128_CBC, "AES-128-CBC"},
	{FSCRYPT_MODE_AES_128_CTS, "AES-128-CTS"},
	{FSCRYPT_MODE_ADIANTUM, "Adiantum"},
};

static cmdinfo_t get_encpolicy_cmd;
static cmdinfo_t set_encpolicy_cmd;
static cmdinfo_t add_enckey_cmd;
static cmdinfo_t rm_enckey_cmd;
static cmdinfo_t enckey_status_cmd;

static void
get_encpolicy_help(void)
{
	printf(_(
"\n"
" display the encryption policy of the current file\n"
"\n"
" -1 -- Use only the old ioctl to get the encryption policy.\n"
"       This only works if the file has a v1 encryption policy.\n"
" -t -- Test whether v2 encryption policies are supported.\n"
"       Prints \"supported\", \"unsupported\", or an error message.\n"
"\n"));
}

static void
set_encpolicy_help(void)
{
	int i;

	printf(_(
"\n"
" assign an encryption policy to the currently open file\n"
"\n"
" Examples:\n"
" 'set_encpolicy' - assign v1 policy with default key descriptor\n"
"                   (0000000000000000)\n"
" 'set_encpolicy -v 2' - assign v2 policy with default key identifier\n"
"                        (00000000000000000000000000000000)\n"
" 'set_encpolicy 0000111122223333' - assign v1 policy with given key descriptor\n"
" 'set_encpolicy 00001111222233334444555566667777' - assign v2 policy with given\n"
"                                                    key identifier\n"
"\n"
" -c MODE -- contents encryption mode\n"
" -n MODE -- filenames encryption mode\n"
" -f FLAGS -- policy flags\n"
" -v VERSION -- policy version\n"
"\n"
" MODE can be numeric or one of the following predefined values:\n"));
	printf("    ");
	for (i = 0; i < ARRAY_SIZE(available_modes); i++) {
		printf("%s", available_modes[i].name);
		if (i != ARRAY_SIZE(available_modes) - 1)
			printf(", ");
	}
	printf("\n");
	printf(_(
" FLAGS and VERSION must be numeric.\n"
"\n"
" Note that it's only possible to set an encryption policy on an empty\n"
" directory.  It's then inherited by new files and subdirectories.\n"
"\n"));
}

static void
add_enckey_help(void)
{
	printf(_(
"\n"
" add an encryption key to the filesystem\n"
"\n"
" Examples:\n"
" 'add_enckey' - add key for v2 policies\n"
" 'add_enckey -d 0000111122223333' - add key for v1 policies w/ given descriptor\n"
"\n"
"Unless -k is given, the key in binary is read from standard input.\n"
" -d DESCRIPTOR -- master_key_descriptor\n"
" -k KEY_ID -- ID of fscrypt-provisioning key containing the raw key\n"
"\n"));
}

static void
rm_enckey_help(void)
{
	printf(_(
"\n"
" remove an encryption key from the filesystem\n"
"\n"
" Examples:\n"
" 'rm_enckey 0000111122223333' - remove key for v1 policies w/ given descriptor\n"
" 'rm_enckey 00001111222233334444555566667777' - remove key for v2 policies w/ given identifier\n"
"\n"
" -a -- remove key for all users who have added it (privileged operation)\n"
"\n"));
}

static void
enckey_status_help(void)
{
	printf(_(
"\n"
" get the status of a filesystem encryption key\n"
"\n"
" Examples:\n"
" 'enckey_status 0000111122223333' - get status of v1 policy key\n"
" 'enckey_status 00001111222233334444555566667777' - get status of v2 policy key\n"
"\n"));
}

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

static int
hexchar2bin(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + (c - 'a');
	if (c >= 'A' && c <= 'F')
		return 10 + (c - 'A');
	return -1;
}

static bool
hex2bin(const char *hex, __u8 *bin, size_t bin_len)
{
	if (strlen(hex) != 2 * bin_len)
		return false;

	while (bin_len--) {
		int hi = hexchar2bin(*hex++);
		int lo = hexchar2bin(*hex++);

		if (hi < 0 || lo < 0)
			return false;
		*bin++ = (hi << 4) | lo;
	}
	return true;
}

static const char *
keydesc2str(const __u8 master_key_descriptor[FSCRYPT_KEY_DESCRIPTOR_SIZE])
{
	static char buf[2 * FSCRYPT_KEY_DESCRIPTOR_SIZE + 1];
	int i;

	for (i = 0; i < FSCRYPT_KEY_DESCRIPTOR_SIZE; i++)
		sprintf(&buf[2 * i], "%02x", master_key_descriptor[i]);

	return buf;
}

static const char *
keyid2str(const __u8 master_key_identifier[FSCRYPT_KEY_IDENTIFIER_SIZE])
{
	static char buf[2 * FSCRYPT_KEY_IDENTIFIER_SIZE + 1];
	int i;

	for (i = 0; i < FSCRYPT_KEY_IDENTIFIER_SIZE; i++)
		sprintf(&buf[2 * i], "%02x", master_key_identifier[i]);

	return buf;
}

static const char *
keyspectype(const struct fscrypt_key_specifier *key_spec)
{
	switch (key_spec->type) {
	case FSCRYPT_KEY_SPEC_TYPE_DESCRIPTOR:
		return _("descriptor");
	case FSCRYPT_KEY_SPEC_TYPE_IDENTIFIER:
		return _("identifier");
	}
	return _("[unknown]");
}

static const char *
keyspec2str(const struct fscrypt_key_specifier *key_spec)
{
	switch (key_spec->type) {
	case FSCRYPT_KEY_SPEC_TYPE_DESCRIPTOR:
		return keydesc2str(key_spec->u.descriptor);
	case FSCRYPT_KEY_SPEC_TYPE_IDENTIFIER:
		return keyid2str(key_spec->u.identifier);
	}
	return _("[unknown]");
}

static bool
str2keydesc(const char *str,
	    __u8 master_key_descriptor[FSCRYPT_KEY_DESCRIPTOR_SIZE])
{
	if (!hex2bin(str, master_key_descriptor, FSCRYPT_KEY_DESCRIPTOR_SIZE)) {
		fprintf(stderr, _("invalid key descriptor: %s\n"), str);
		return false;
	}
	return true;
}

static bool
str2keyid(const char *str,
	  __u8 master_key_identifier[FSCRYPT_KEY_IDENTIFIER_SIZE])
{
	if (!hex2bin(str, master_key_identifier, FSCRYPT_KEY_IDENTIFIER_SIZE)) {
		fprintf(stderr, _("invalid key identifier: %s\n"), str);
		return false;
	}
	return true;
}

/*
 * Parse a key specifier (descriptor or identifier) given as a hex string.
 *
 *  8 bytes (16 hex chars) == key descriptor == v1 encryption policy.
 * 16 bytes (32 hex chars) == key identifier == v2 encryption policy.
 *
 * If a policy_version is given (>= 0), then the corresponding type of key
 * specifier is required.  Otherwise the specifier type and policy_version are
 * determined based on the length of the given hex string.
 *
 * Returns the policy version, or -1 on error.
 */
static int
str2keyspec(const char *str, int policy_version,
	    struct fscrypt_key_specifier *key_spec)
{
	if (policy_version < 0) { /* version unspecified? */
		size_t len = strlen(str);

		if (len == 2 * FSCRYPT_KEY_DESCRIPTOR_SIZE) {
			policy_version = FSCRYPT_POLICY_V1;
		} else if (len == 2 * FSCRYPT_KEY_IDENTIFIER_SIZE) {
			policy_version = FSCRYPT_POLICY_V2;
		} else {
			fprintf(stderr, _("invalid key specifier: %s\n"), str);
			return -1;
		}
	}
	if (policy_version == FSCRYPT_POLICY_V2) {
		if (!str2keyid(str, key_spec->u.identifier))
			return -1;
		key_spec->type = FSCRYPT_KEY_SPEC_TYPE_IDENTIFIER;
	} else {
		if (!str2keydesc(str, key_spec->u.descriptor))
			return -1;
		key_spec->type = FSCRYPT_KEY_SPEC_TYPE_DESCRIPTOR;
	}
	return policy_version;
}

static int
parse_key_id(const char *arg)
{
	long value;
	char *tmp;

	value = strtol(arg, &tmp, 0);
	if (value <= 0 || value > INT_MAX || tmp == arg || *tmp != '\0') {
		fprintf(stderr, _("invalid key ID: %s\n"), arg);
		/* 0 is never a valid Linux key ID. */
		return 0;
	}
	return value;
}

static void
test_for_v2_policy_support(void)
{
	struct fscrypt_get_policy_ex_arg arg;

	arg.policy_size = sizeof(arg.policy);

	if (ioctl(file->fd, FS_IOC_GET_ENCRYPTION_POLICY_EX, &arg) == 0 ||
	    errno == ENODATA /* file unencrypted */) {
		printf(_("supported\n"));
		return;
	}
	if (errno == ENOTTY) {
		printf(_("unsupported\n"));
		return;
	}
	fprintf(stderr,
		_("%s: unexpected error checking for FS_IOC_GET_ENCRYPTION_POLICY_EX support: %s\n"),
		file->name, strerror(errno));
	exitcode = 1;
}

static void
show_v1_encryption_policy(const struct fscrypt_policy_v1 *policy)
{
	printf(_("Encryption policy for %s:\n"), file->name);
	printf(_("\tPolicy version: %u\n"), policy->version);
	printf(_("\tMaster key descriptor: %s\n"),
	       keydesc2str(policy->master_key_descriptor));
	printf(_("\tContents encryption mode: %u (%s)\n"),
	       policy->contents_encryption_mode,
	       mode2str(policy->contents_encryption_mode));
	printf(_("\tFilenames encryption mode: %u (%s)\n"),
	       policy->filenames_encryption_mode,
	       mode2str(policy->filenames_encryption_mode));
	printf(_("\tFlags: 0x%02x\n"), policy->flags);
}

static void
show_v2_encryption_policy(const struct fscrypt_policy_v2 *policy)
{
	printf(_("Encryption policy for %s:\n"), file->name);
	printf(_("\tPolicy version: %u\n"), policy->version);
	printf(_("\tMaster key identifier: %s\n"),
	       keyid2str(policy->master_key_identifier));
	printf(_("\tContents encryption mode: %u (%s)\n"),
	       policy->contents_encryption_mode,
	       mode2str(policy->contents_encryption_mode));
	printf(_("\tFilenames encryption mode: %u (%s)\n"),
	       policy->filenames_encryption_mode,
	       mode2str(policy->filenames_encryption_mode));
	printf(_("\tFlags: 0x%02x\n"), policy->flags);
}

static int
get_encpolicy_f(int argc, char **argv)
{
	int c;
	struct fscrypt_get_policy_ex_arg arg;
	bool only_use_v1_ioctl = false;
	int res;

	while ((c = getopt(argc, argv, "1t")) != EOF) {
		switch (c) {
		case '1':
			only_use_v1_ioctl = true;
			break;
		case 't':
			test_for_v2_policy_support();
			return 0;
		default:
			return command_usage(&get_encpolicy_cmd);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 0)
		return command_usage(&get_encpolicy_cmd);

	/* first try the new ioctl */
	if (only_use_v1_ioctl) {
		res = -1;
		errno = ENOTTY;
	} else {
		arg.policy_size = sizeof(arg.policy);
		res = ioctl(file->fd, FS_IOC_GET_ENCRYPTION_POLICY_EX, &arg);
	}

	/* fall back to the old ioctl */
	if (res != 0 && errno == ENOTTY)
		res = ioctl(file->fd, FS_IOC_GET_ENCRYPTION_POLICY,
			    &arg.policy.v1);

	if (res != 0) {
		fprintf(stderr, _("%s: failed to get encryption policy: %s\n"),
			file->name, strerror(errno));
		exitcode = 1;
		return 0;
	}

	switch (arg.policy.version) {
	case FSCRYPT_POLICY_V1:
		show_v1_encryption_policy(&arg.policy.v1);
		break;
	case FSCRYPT_POLICY_V2:
		show_v2_encryption_policy(&arg.policy.v2);
		break;
	default:
		printf(_("Encryption policy for %s:\n"), file->name);
		printf(_("\tPolicy version: %u (unknown)\n"),
		       arg.policy.version);
		break;
	}
	return 0;
}

static int
set_encpolicy_f(int argc, char **argv)
{
	int c;
	__u8 contents_encryption_mode = FSCRYPT_MODE_AES_256_XTS;
	__u8 filenames_encryption_mode = FSCRYPT_MODE_AES_256_CTS;
	__u8 flags = FSCRYPT_POLICY_FLAGS_PAD_16;
	int version = -1; /* unspecified */
	struct fscrypt_key_specifier key_spec;
	union {
		__u8 version;
		struct fscrypt_policy_v1 v1;
		struct fscrypt_policy_v2 v2;
	} policy;

	while ((c = getopt(argc, argv, "c:n:f:v:")) != EOF) {
		switch (c) {
		case 'c':
			if (!parse_mode(optarg, &contents_encryption_mode)) {
				fprintf(stderr,
					_("invalid contents encryption mode: %s\n"),
					optarg);
				exitcode = 1;
				return 0;
			}
			break;
		case 'n':
			if (!parse_mode(optarg, &filenames_encryption_mode)) {
				fprintf(stderr,
					_("invalid filenames encryption mode: %s\n"),
					optarg);
				exitcode = 1;
				return 0;
			}
			break;
		case 'f':
			if (!parse_byte_value(optarg, &flags)) {
				fprintf(stderr, _("invalid flags: %s\n"),
					optarg);
				exitcode = 1;
				return 0;
			}
			break;
		case 'v': {
			__u8 val;

			if (!parse_byte_value(optarg, &val)) {
				fprintf(stderr,
					_("invalid policy version: %s\n"),
					optarg);
				exitcode = 1;
				return 0;
			}
			if (val == 1) /* Just to avoid annoying people... */
				val = FSCRYPT_POLICY_V1;
			version = val;
			break;
		}
		default:
			exitcode = 1;
			return command_usage(&set_encpolicy_cmd);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 1) {
		exitcode = 1;
		return command_usage(&set_encpolicy_cmd);
	}

	/*
	 * If unspecified, the key descriptor or identifier defaults to all 0's.
	 * If the policy version is additionally unspecified, it defaults to v1.
	 */
	memset(&key_spec, 0, sizeof(key_spec));
	if (argc > 0) {
		version = str2keyspec(argv[0], version, &key_spec);
		if (version < 0) {
			exitcode = 1;
			return 0;
		}
	}
	if (version < 0) /* version unspecified? */
		version = FSCRYPT_POLICY_V1;

	memset(&policy, 0, sizeof(policy));
	policy.version = version;
	if (version == FSCRYPT_POLICY_V2) {
		policy.v2.contents_encryption_mode = contents_encryption_mode;
		policy.v2.filenames_encryption_mode = filenames_encryption_mode;
		policy.v2.flags = flags;
		memcpy(policy.v2.master_key_identifier, key_spec.u.identifier,
		       FSCRYPT_KEY_IDENTIFIER_SIZE);
	} else {
		/*
		 * xfstests passes .version = 255 for testing.  Just use
		 * 'struct fscrypt_policy_v1' for both v1 and unknown versions.
		 */
		policy.v1.contents_encryption_mode = contents_encryption_mode;
		policy.v1.filenames_encryption_mode = filenames_encryption_mode;
		policy.v1.flags = flags;
		memcpy(policy.v1.master_key_descriptor, key_spec.u.descriptor,
		       FSCRYPT_KEY_DESCRIPTOR_SIZE);
	}

	if (ioctl(file->fd, FS_IOC_SET_ENCRYPTION_POLICY, &policy) != 0) {
		fprintf(stderr, _("%s: failed to set encryption policy: %s\n"),
			file->name, strerror(errno));
		exitcode = 1;
	}
	return 0;
}

static ssize_t
read_until_limit_or_eof(int fd, void *buf, size_t limit)
{
	size_t bytes_read = 0;
	ssize_t res;

	while (limit) {
		res = read(fd, buf, limit);
		if (res < 0)
			return res;
		if (res == 0)
			break;
		buf += res;
		bytes_read += res;
		limit -= res;
	}
	return bytes_read;
}

static int
add_enckey_f(int argc, char **argv)
{
	int c;
	struct fscrypt_add_key_arg *arg;
	ssize_t raw_size;
	int retval = 0;

	arg = calloc(1, sizeof(*arg) + FSCRYPT_MAX_KEY_SIZE + 1);
	if (!arg) {
		perror("calloc");
		exitcode = 1;
		return 0;
	}

	arg->key_spec.type = FSCRYPT_KEY_SPEC_TYPE_IDENTIFIER;

	while ((c = getopt(argc, argv, "d:k:")) != EOF) {
		switch (c) {
		case 'd':
			arg->key_spec.type = FSCRYPT_KEY_SPEC_TYPE_DESCRIPTOR;
			if (!str2keydesc(optarg, arg->key_spec.u.descriptor))
				goto out;
			break;
		case 'k':
			arg->key_id = parse_key_id(optarg);
			if (arg->key_id == 0)
				goto out;
			break;
		default:
			exitcode = 1;
			retval = command_usage(&add_enckey_cmd);
			goto out;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 0) {
		exitcode = 1;
		retval = command_usage(&add_enckey_cmd);
		goto out;
	}

	if (arg->key_id == 0) {
		raw_size = read_until_limit_or_eof(STDIN_FILENO, arg->raw,
						   FSCRYPT_MAX_KEY_SIZE + 1);
		if (raw_size < 0) {
			fprintf(stderr, _("Error reading key from stdin: %s\n"),
				strerror(errno));
			exitcode = 1;
			goto out;
		}
		if (raw_size > FSCRYPT_MAX_KEY_SIZE) {
			fprintf(stderr,
				_("Invalid key; got > FSCRYPT_MAX_KEY_SIZE (%d) bytes on stdin!\n"),
				FSCRYPT_MAX_KEY_SIZE);
			exitcode = 1;
			goto out;
		}
		arg->raw_size = raw_size;
	} /* else, raw key is given via key with ID 'key_id' */

	if (ioctl(file->fd, FS_IOC_ADD_ENCRYPTION_KEY, arg) != 0) {
		fprintf(stderr, _("Error adding encryption key: %s\n"),
			strerror(errno));
		exitcode = 1;
		goto out;
	}
	printf(_("Added encryption key with %s %s\n"),
	       keyspectype(&arg->key_spec), keyspec2str(&arg->key_spec));
out:
	memset(arg->raw, 0, FSCRYPT_MAX_KEY_SIZE + 1);
	free(arg);
	return retval;
}

static int
rm_enckey_f(int argc, char **argv)
{
	int c;
	struct fscrypt_remove_key_arg arg;
	int ioc = FS_IOC_REMOVE_ENCRYPTION_KEY;

	memset(&arg, 0, sizeof(arg));

	while ((c = getopt(argc, argv, "a")) != EOF) {
		switch (c) {
		case 'a':
			ioc = FS_IOC_REMOVE_ENCRYPTION_KEY_ALL_USERS;
			break;
		default:
			exitcode = 1;
			return command_usage(&rm_enckey_cmd);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1) {
		exitcode = 1;
		return command_usage(&rm_enckey_cmd);
	}

	if (str2keyspec(argv[0], -1, &arg.key_spec) < 0) {
		exitcode = 1;
		return 0;
	}

	if (ioctl(file->fd, ioc, &arg) != 0) {
		fprintf(stderr, _("Error removing encryption key: %s\n"),
			strerror(errno));
		exitcode = 1;
		return 0;
	}
	if (arg.removal_status_flags &
	    FSCRYPT_KEY_REMOVAL_STATUS_FLAG_OTHER_USERS) {
		printf(_("Removed user's claim to encryption key with %s %s\n"),
		       keyspectype(&arg.key_spec), keyspec2str(&arg.key_spec));
	} else if (arg.removal_status_flags &
		   FSCRYPT_KEY_REMOVAL_STATUS_FLAG_FILES_BUSY) {
		printf(_("Removed encryption key with %s %s, but files still busy\n"),
		       keyspectype(&arg.key_spec), keyspec2str(&arg.key_spec));
	} else {
		printf(_("Removed encryption key with %s %s\n"),
		       keyspectype(&arg.key_spec), keyspec2str(&arg.key_spec));
	}
	return 0;
}

static int
enckey_status_f(int argc, char **argv)
{
	struct fscrypt_get_key_status_arg arg;

	memset(&arg, 0, sizeof(arg));

	if (str2keyspec(argv[1], -1, &arg.key_spec) < 0) {
		exitcode = 1;
		return 0;
	}

	if (ioctl(file->fd, FS_IOC_GET_ENCRYPTION_KEY_STATUS, &arg) != 0) {
		fprintf(stderr, _("Error getting encryption key status: %s\n"),
			strerror(errno));
		exitcode = 1;
		return 0;
	}

	switch (arg.status) {
	case FSCRYPT_KEY_STATUS_PRESENT:
		printf(_("Present"));
		if (arg.user_count || arg.status_flags) {
			printf(" (user_count=%u", arg.user_count);
			if (arg.status_flags &
			    FSCRYPT_KEY_STATUS_FLAG_ADDED_BY_SELF)
				printf(", added_by_self");
			arg.status_flags &=
				~FSCRYPT_KEY_STATUS_FLAG_ADDED_BY_SELF;
			if (arg.status_flags)
				printf(", unknown_flags=0x%08x",
				       arg.status_flags);
			printf(")");
		}
		printf("\n");
		return 0;
	case FSCRYPT_KEY_STATUS_ABSENT:
		printf(_("Absent\n"));
		return 0;
	case FSCRYPT_KEY_STATUS_INCOMPLETELY_REMOVED:
		printf(_("Incompletely removed\n"));
		return 0;
	default:
		printf(_("Unknown status (%u)\n"), arg.status);
		return 0;
	}
}

void
encrypt_init(void)
{
	get_encpolicy_cmd.name = "get_encpolicy";
	get_encpolicy_cmd.cfunc = get_encpolicy_f;
	get_encpolicy_cmd.args = _("[-1] [-t]");
	get_encpolicy_cmd.argmin = 0;
	get_encpolicy_cmd.argmax = -1;
	get_encpolicy_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	get_encpolicy_cmd.oneline =
		_("display the encryption policy of the current file");
	get_encpolicy_cmd.help = get_encpolicy_help;

	set_encpolicy_cmd.name = "set_encpolicy";
	set_encpolicy_cmd.cfunc = set_encpolicy_f;
	set_encpolicy_cmd.args =
		_("[-c mode] [-n mode] [-f flags] [-v version] [keyspec]");
	set_encpolicy_cmd.argmin = 0;
	set_encpolicy_cmd.argmax = -1;
	set_encpolicy_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	set_encpolicy_cmd.oneline =
		_("assign an encryption policy to the current file");
	set_encpolicy_cmd.help = set_encpolicy_help;

	add_enckey_cmd.name = "add_enckey";
	add_enckey_cmd.cfunc = add_enckey_f;
	add_enckey_cmd.args = _("[-d descriptor] [-k key_id]");
	add_enckey_cmd.argmin = 0;
	add_enckey_cmd.argmax = -1;
	add_enckey_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	add_enckey_cmd.oneline = _("add an encryption key to the filesystem");
	add_enckey_cmd.help = add_enckey_help;

	rm_enckey_cmd.name = "rm_enckey";
	rm_enckey_cmd.cfunc = rm_enckey_f;
	rm_enckey_cmd.args = _("[-a] keyspec");
	rm_enckey_cmd.argmin = 0;
	rm_enckey_cmd.argmax = -1;
	rm_enckey_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	rm_enckey_cmd.oneline =
		_("remove an encryption key from the filesystem");
	rm_enckey_cmd.help = rm_enckey_help;

	enckey_status_cmd.name = "enckey_status";
	enckey_status_cmd.cfunc = enckey_status_f;
	enckey_status_cmd.args = _("keyspec");
	enckey_status_cmd.argmin = 1;
	enckey_status_cmd.argmax = 1;
	enckey_status_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	enckey_status_cmd.oneline =
		_("get the status of a filesystem encryption key");
	enckey_status_cmd.help = enckey_status_help;

	add_command(&get_encpolicy_cmd);
	add_command(&set_encpolicy_cmd);
	add_command(&add_enckey_cmd);
	add_command(&rm_enckey_cmd);
	add_command(&enckey_status_cmd);
}
