/* do a rot13 of ASCII text */

#include "ex_utils.h"

#define CONF_USE_MMAP_DEF FALSE

/* configuration:
   how to do it ... */
#define EX_ROT13_USE_ITER        1
#define EX_ROT13_USE_EXPORT_CHR  0
#define EX_ROT13_USE_SUB_CHR     0
#define EX_ROT13_USE_CSTR_MALLOC 0

#define ROT13_LETTER(x) ( \
 (((x) >= 'A' && (x) <= 'M') || \
  ((x) >= 'a' && (x) <= 'm')) ? ((x) + 13) : ((x) - 13) \
 )

#if 0
#define ROT13_MAP(x) ( \
 ((((x) >= 'A') && ((x) <= 'Z')) || \
  (((x) >= 'a') && ((x) <= 'z'))) ? ROT13_LETTER(x) : (x) \
 )
#else
static char rot13_map[] = {
 1*   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
 1*  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
 1*  32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
 1*  48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
 1*  64, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 65, 66,
 1*  67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 91, 92, 93, 94, 95,
 1*  96,110,111,112,113,114,115,116,117,118,119,120,121,122, 97, 98,
 1*  99,100,101,102,103,104,105,106,107,108,109,123,124,125,126,127,
 1* 128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
 1* 144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
 1* 160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
 1* 176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
 1* 192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
 1* 208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
 1* 224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
 1* 240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255};
#define ROT13_MAP(x) rot13_map[(unsigned char)(x)]
#endif

static int ex_rot13_process(Vstr_base *s1, Vstr_base *s2)
{
  size_t count = 0;
  static const char chrs[] = ("abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  /* we don't want to create more data, if we are over our limit */
  if (s1->len > EX_MAX_W_DATA_INCORE)
    return (FALSE);

  if (!s2->len)
    return (FALSE);

  if (EX_ROT13_USE_ITER)
  {
    Vstr_iter iter[1];

    if (!vstr_iter_fwd_beg(s2, 1, s2->len, iter))
      abort();

    do
    {
      unsigned int scan = 0;
      while (scan < iter->len)
      {
        char tmp = ROT13_MAP(iter->ptr[scan]);
        vstr_add_rep_chr(s1, s1->len, tmp, 1);
        ++scan;
      }
    } while (vstr_iter_fwd_nxt(iter));

    vstr_del(s2, 1, s2->len);
  }

  if (EX_ROT13_USE_EXPORT_CHR)
  {
    unsigned int scan = 0;
    while (scan++ < s2->len)
    {
      char tmp = vstr_export_chr(s2, scan);
      tmp = ROT13_MAP(tmp);
      vstr_add_rep_chr(s1, s1->len, tmp, 1);
    }
    vstr_del(s2, 1, s2->len);
  }
  
  if (EX_ROT13_USE_SUB_CHR)
  {
    unsigned int scan = 0;

    while (scan++ < s2->len)
    {
      char tmp = vstr_export_chr(s2, scan);
      tmp = ROT13_MAP(tmp);
      vstr_sub_rep_chr(s2, scan, 1, tmp, 1);
    }
    vstr_add_vstr(s1, s1->len, s2, 1, s2->len,
                  VSTR_TYPE_ADD_BUF_REF);
    vstr_del(s2, 1, s2->len);
  }
  
  if (EX_ROT13_USE_CSTR_MALLOC)
  while (s2->len)
  {
    if ((count = VSTR_CSPN_CSTR_CHRS_FWD(s2, 1, s2->len, chrs)))
    {
      vstr_add_vstr(s1, s1->len, s2, 1, count,
                    VSTR_TYPE_ADD_BUF_REF);
      vstr_del(s2, 1, count);

      if (s1->len > EX_MAX_W_DATA_INCORE)
        return (TRUE);
    }

    if ((count = VSTR_SPN_CSTR_CHRS_FWD(s2, 1, s2->len, chrs)))
    {
      char *ptr = vstr_export_cstr_malloc(s2, 1, count);
      Vstr_ref *ref = vstr_ref_make_ptr(ptr, vstr_ref_cb_free_ptr_ref);

      if (!ref || !ref->ptr)
        errno = ENOMEM, err(EXIT_FAILURE, "vstr_make_conf");

      while (*ptr)
      {
        *ptr = ROT13_LETTER(*ptr);
        ++ptr;
      }

      vstr_add_ref(s1, s1->len, ref, 0, count);
      vstr_del(s2, 1, count);
    }
  }

  return (TRUE);
}

static void ex_rot13_read_fd_write_stdout(Vstr_base *s1, Vstr_base *s2, int fd)
{
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s2, fd);

    if (io_r_state == IO_EOF)
      break;

    ex_rot13_process(s1, s2);

    io_w_state = io_put(s1, 1);

    io_limit(io_r_state, fd, io_w_state, 1, s1);
  }
}

static void ex_rot13_process_limit(Vstr_base *s1, Vstr_base *s2,
                                   unsigned int lim)
{
  while (s2->len > lim)
  {
    int proc_data = ex_rot13_process(s1, s2);
    if (!proc_data && (io_put(s1, STDOUT_FILENO) == IO_BLOCK))
      io_block(-1, STDOUT_FILENO);
  }
}

int main(int argc, char *argv[])
{
  Vstr_base *s1 = NULL;
  Vstr_base *s2 = ex_init(&s1);
  int count = 1;
  int use_mmap = CONF_USE_MMAP_DEF;  
  
  /* parse command line arguments... */
  while (count < argc)
  { /* quick hack getopt_long */
    if (!strcmp("--", argv[count]))
    {
      ++count;
      break;
    }
    else if (!strcmp("--mmap", argv[count])) /* toggle use of mmap */
      use_mmap = !use_mmap;
    else if (!strcmp("--version", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
jrot13 1.0.0\n\
Written by James Antill\n\
\n\
Uses Vstr string library.\n\
");
      goto out;
    }
    else if (!strcmp("--help", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
Usage: jrot13 [FILENAME]...\n\
   or: jrot13 OPTION\n\
Output filenames.\n\
\n\
      --help     Display this help and exit\n\
      --version  Output version information and exit\n\
      --mmap     Toggle use of mmap() to load input files\n\
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
  
  if (count >= argc)  /* use stdin */
  {
    io_fd_set_o_nonblock(STDIN_FILENO);
    ex_rot13_read_fd_write_stdout(s1, s2, STDIN_FILENO);
  }

  /* loop through all arguments, open the file specified
   * and do the read/write loop */
  while (count < argc)
  {
    unsigned int ern = 0;

    if (use_mmap)
      vstr_sc_mmap_file(s2, s2->len, argv[count], 0, 0, &ern);

    if (!use_mmap ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_FSTAT_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_MMAP_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_TOO_LARGE))
    {
      int fd = io_open(argv[count]);

      ex_rot13_read_fd_write_stdout(s1, s2, fd);

      if (close(fd) == -1)
        warn("close(%s)", argv[count]);
    }
    else if (ern && (ern != VSTR_TYPE_SC_MMAP_FILE_ERR_CLOSE_ERRNO))
      err(EXIT_FAILURE, "add");
    else
      ex_rot13_process_limit(s1, s2, EX_MAX_R_DATA_INCORE);

    ++count;
  }

  ex_rot13_process_limit(s1, s2, 0);

 out:
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, s2));
}
