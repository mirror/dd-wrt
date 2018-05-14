#ifndef ANDROID_PERMS_H
# define ANDROID_PERMS_H

# include "config.h"
# include <ext2fs/ext2fs.h>

typedef void (*fs_config_f)(const char *path, int dir,
			    const char *target_out_path,
			    unsigned *uid, unsigned *gid,
			    unsigned *mode, uint64_t *capabilities);

# ifdef _WIN32
struct selabel_handle;
static inline errcode_t android_configure_fs(ext2_filsys fs,
					     char *src_dir,
					     char *target_out,
					     char *mountpoint,
					     void *seopts,
					     unsigned int nopt,
					     char *fs_config_file,
					     time_t fixed_time)
{
	return 0;
}
# else
#  include <selinux/selinux.h>
#  include <selinux/label.h>
#  if defined(__ANDROID__)
#   include <selinux/android.h>
#  endif
#  include <private/android_filesystem_config.h>
#  include <private/canned_fs_config.h>

errcode_t android_configure_fs(ext2_filsys fs, char *src_dir,
			       char *target_out,
			       char *mountpoint,
			       struct selinux_opt *seopts,
			       unsigned int nopt,
			       char *fs_config_file, time_t fixed_time);

# endif
#endif /* !ANDROID_PERMS_H */
