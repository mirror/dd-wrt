#define EX_UTILS_NO_USE_OPEN  1
#define EX_UTILS_NO_USE_LIMIT 1
#define EX_UTILS_NO_USE_GET   1
#include "ex_utils.h"

#include <sys/types.h>
#include <dirent.h>

static void app_netstr_cstr(Vstr_base *s1, const char *val)
{
  size_t nb = vstr_add_netstr_beg(s1, s1->len);
  vstr_add_cstr_buf(s1, s1->len, val);
  vstr_add_netstr_end(s1, nb, s1->len);
}
static void app_netstr_uintmax(Vstr_base *s1, VSTR_AUTOCONF_uintmax_t val)
{
  char buf[sizeof(val) * 8];
  size_t nb = vstr_add_netstr_beg(s1, s1->len);
  size_t len = vstr_sc_conv_num10_uintmax(buf, sizeof(buf), val);
  vstr_add_buf(s1, s1->len, buf, len);
  vstr_add_netstr_end(s1, nb, s1->len);
}

static int stat_from(struct stat64 *buf, const char *from, const char *name,
                     Vstr_base *tmp)
{
  const char *full_name = NULL;
  
  vstr_del(tmp, 1, tmp->len);
  vstr_add_fmt(tmp, tmp->len, "%s/%s", from, name);
    
  if (!(full_name = vstr_export_cstr_ptr(tmp, 1, tmp->len)) ||
      stat64(full_name, buf))
  {
    warn("stat(%s)", name);
    return (FALSE);
  }

  vstr_del(tmp, 1, tmp->len);
  return (TRUE);
}

/* This is "dir_list", without any command line options */
int main(int argc, char *argv[])
{
  Vstr_base *tmp = NULL;
  Vstr_base *s1 = ex_init(&tmp); /* init the library etc. */
  int count = 1; /* skip the program name */
  DIR *dir = NULL;
  struct dirent *ent = NULL;
  int sizes = FALSE;
  int follow = FALSE;
  
  /* parse command line arguments... */
  while (count < argc)
  { /* quick hack getopt_long */
    if (!strcmp("--", argv[count]))
    {
      ++count;
      break;
    }
    else if (!strcmp("--size", argv[count]))
      sizes = !sizes;
    else if (!strcmp("--follow", argv[count]))
      follow = !follow;
    else if (!strcmp("--version", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
jdir_list 1.0.0\n\
Written by James Antill\n\
\n\
Uses Vstr string library.\n\
");
      goto out;
    }
    else if (!strcmp("--help", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
Usage: jdir_list <DIRECTORY>...\n\
   or: jdir_list OPTION\n\
Output filenames.\n\
\n\
      --help     Display this help and exit\n\
      --version  Output version information and exit\n\
      --size     Stat files to output size information\n\
      --follow   Stat symlinks to get type information\n\
      --         Treat rest of cmd line as input filenames\n\
\n\
Report bugs to James Antill <james@and.org>.\n\
");
      goto out;
    }
    else
      break;
    ++count;
  }
  
  /* if no arguments are given just do stdin to stdout */
  if (count >= argc)
    errx(EXIT_FAILURE, "No directory given");

  if (!(dir = opendir(argv[count])))
    err(EXIT_FAILURE, "opendir(%s)", argv[count]);

  {
    size_t nb = vstr_add_netstr_beg(s1, s1->len);

    app_netstr_cstr(s1, "version");
    app_netstr_cstr(s1, "1");

    vstr_add_netstr_end(s1, nb, s1->len);
  }

  /* readdir() == blocking, dirfd() for poll() ? */
  while (!s1->conf->malloc_bad && (ent = readdir(dir)))
  {
    size_t nb = vstr_add_netstr_beg(s1, s1->len);
    struct stat64 buf;
    int use_stat = FALSE;
    
    app_netstr_cstr(s1, "name");
    app_netstr_cstr(s1, ent->d_name);

    if (sizes &&
        ((ent->d_type == DT_REG) || (ent->d_type == DT_UNKNOWN)))
      use_stat = TRUE;
    
    if (follow &&
        ((ent->d_type == DT_LNK) || (ent->d_type == DT_UNKNOWN)))
      use_stat = TRUE;
    
    if (use_stat && stat_from(&buf, argv[count], ent->d_name, tmp))
    {
      app_netstr_cstr(s1, "inode");
      app_netstr_uintmax(s1, buf.st_ino);
      
      if ((ent->d_type != DT_UNKNOWN) && !follow)
      {
        app_netstr_cstr(s1, "type");
        app_netstr_uintmax(s1, DTTOIF(ent->d_type));
      }
      else
      {
        app_netstr_cstr(s1, "type");
        app_netstr_uintmax(s1, buf.st_mode);
      }
      
      app_netstr_cstr(s1, "size");
      app_netstr_uintmax(s1, buf.st_size);
    }
    else
    { 
      app_netstr_cstr(s1, "inode");
      app_netstr_uintmax(s1, ent->d_ino);

      if (ent->d_type != DT_UNKNOWN)
      {
        app_netstr_cstr(s1, "type");
        app_netstr_uintmax(s1, DTTOIF(ent->d_type));
      }
    }
    
    vstr_add_netstr_end(s1, nb, s1->len);
  }

  if (s1->conf->malloc_bad)
    errno = ENOMEM, err(EXIT_FAILURE, "readdir(%s)", argv[count]);
    
 out:
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, tmp));
}
