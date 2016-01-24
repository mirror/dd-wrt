/* automatically decompresses data from either gzip of bzip2 formats
 * very usefull so you can do tar -xzvf <foo> and it just works */

#include "ex_utils.h"


/* compression libraries */

#include <zlib.h>
#define BZ_NO_STDIO 1
#include <bzlib.h>


#define EX_ZCAT_USE_MMAP 1

#define EX_ZCAT_USAGE_ENCOMPRESS 1
#define EX_ZCAT_USAGE_DECOMPRESS 2

#define EX_ZCAT_TYPE_ERROR    0
#define EX_ZCAT_TYPE_NONE     1
#define EX_ZCAT_TYPE_BZIP2    2
#define EX_ZCAT_TYPE_GZIP     3
#define EX_ZCAT_TYPE_COMPRESS 4


static int ex_zcat_f_type = EX_ZCAT_TYPE_ERROR;

union ex_zcat_lib_decomp_u
{
 z_stream gzip;
 bz_stream bzip2;
} ex_zcat_lib_decomp_data;

static int ex_zcat_data_open = FALSE;

static int ex_zcat_open(Vstr_base *s2)
{
  int not_enough = FALSE;
  int f_type = EX_ZCAT_TYPE_NONE;

  if (ex_zcat_data_open) /* allow open to be called many times */
    return (TRUE);
  
  if (s2->len < 3) not_enough = TRUE; else
  {
    unsigned char buf[3] = {0x1F, 0x8B, 0x08};
    if (vstr_cmp_buf_eq(s2, 1, sizeof(buf), buf, sizeof(buf)))
      f_type = EX_ZCAT_TYPE_GZIP;
  }

  if (s2->len < 4) not_enough = TRUE; else
  {
    unsigned char buf[3] = {0x42, 0x5A, 0x68};

    if (vstr_cmp_buf_eq(s2, 1, sizeof(buf), buf, sizeof(buf)) &&
        /* byte four is the level -1 .. -9 */
        (vstr_export_chr(s2, 4) >= 0x31) && (vstr_export_chr(s2, 4) <= 0x39))
      f_type = EX_ZCAT_TYPE_BZIP2;

  }

  memset(&ex_zcat_lib_decomp_data, 0, sizeof(ex_zcat_lib_decomp_data));

  switch (f_type)
  {
    case EX_ZCAT_TYPE_NONE:
      if (not_enough)
        return (FALSE);
      f_type = EX_ZCAT_TYPE_NONE;
      break;

    case EX_ZCAT_TYPE_GZIP:
    {
      union ex_zcat_lib_decomp_u *u = &ex_zcat_lib_decomp_data;
      unsigned int method = vstr_export_chr(s2, 3);
      unsigned int flags  = vstr_export_chr(s2, 4);
      size_t pos = 1;

      /* magic  | magic | method | flags |
         stamp  | stamp | stamp  | stamp |
         Xflags | OS    | */
      pos += 10;

      ASSERT(method == 0x08);

      if (flags & 0x04)
      { /* extra field */
        char buf[2];
        unsigned int xfield_len = 0;

        if ((pos <= s2->len) && (vstr_sc_posdiff(pos, s2->len) < 2))
          return (FALSE); /* not all of gzip header */

        vstr_export_buf(s2, pos, 2, buf, sizeof(buf));
        xfield_len |= buf[1]; xfield_len <<= 8;
        xfield_len |= buf[0];

        xfield_len += 2;
        if ((pos <= s2->len) && (vstr_sc_posdiff(pos, s2->len) < xfield_len))
          return (FALSE); /* not all of gzip header */
        pos += xfield_len;
      }
      if (flags & 0x08)
      { /* name is present */
        size_t len = vstr_sc_posdiff(pos, s2->len);
        size_t end_name = vstr_srch_chr_fwd(s2, pos, len, 0);
        if (!end_name)
          return (FALSE);
        pos += vstr_sc_posdiff(pos, end_name);
      }
      if (flags & 0x10)
      { /* comment is present */
        size_t len = vstr_sc_posdiff(pos, s2->len);
        size_t end_name = vstr_srch_chr_fwd(s2, pos, len, 0);
        if (!end_name)
          return (FALSE);
        pos += vstr_sc_posdiff(pos, end_name);
      }
      if (flags & 0x02) /* continuation/header CRC --
                         * gzip and zlib do it differently */
        pos += 2;
      vstr_del(s2, 1, pos - 1);

      if (inflateInit2(&u->gzip, -MAX_WBITS) != Z_OK)
        errx(EXIT_FAILURE, "gzip init: failed");
    }
    break;

    case EX_ZCAT_TYPE_BZIP2:
    {
      union ex_zcat_lib_decomp_u *u = &ex_zcat_lib_decomp_data;

      if (BZ2_bzDecompressInit(&u->bzip2, FALSE, FALSE) != BZ_OK)
        errx(EXIT_FAILURE, "bzip2 beg: failed");
    }
    break;

    case EX_ZCAT_TYPE_COMPRESS:

    default:
      assert(FALSE);
  }

  ex_zcat_f_type    = f_type;
  ex_zcat_data_open = TRUE;

  return (TRUE);
}

static void ex_zcat_close(void)
{
  union ex_zcat_lib_decomp_u *u = &ex_zcat_lib_decomp_data;

  if (!ex_zcat_data_open)
    return;

  switch (ex_zcat_f_type)
  {
    case EX_ZCAT_TYPE_NONE:
      break;

    case EX_ZCAT_TYPE_GZIP:
      if (inflateEnd(&u->gzip) != Z_OK)
        errx(EXIT_FAILURE, "gzip end: failed");
      break;

    case EX_ZCAT_TYPE_BZIP2:
      if (BZ2_bzDecompressEnd(&u->bzip2) != BZ_OK)
        errx(EXIT_FAILURE, "bzip2 end: failed");
      break;

    case EX_ZCAT_TYPE_COMPRESS:

    default:
      assert(FALSE);
  }

  ex_zcat_data_open = FALSE;
}

static int ex_zcat_process(Vstr_base *s1, Vstr_base *s2, int last)
{
  union ex_zcat_lib_decomp_u *u = &ex_zcat_lib_decomp_data;
  struct iovec *io_s2 = NULL;
  struct iovec *io_s1 = NULL;
  unsigned char  *io_r_ptr = NULL;
  size_t io_r_len = 0;
  unsigned char  *io_w_ptr = NULL;
  size_t io_w_len = 0;
  size_t bytes_io_r = 0;
  size_t bytes_io_w = 0;
  int at_end = FALSE;

  if (!ex_zcat_open(s2))
    return (FALSE);

  if (ex_zcat_f_type == EX_ZCAT_TYPE_NONE)
  { /* special quick case ... */
    if (!s2->len)
      return (FALSE);

    vstr_mov(s1, s1->len, s2, 1, s2->len);
    return (TRUE);
  }

  if (ex_zcat_f_type == EX_ZCAT_TYPE_GZIP)
  { /* gzip needs a bunch of data at a time... */
    if (!last && (s2->len < (4 * 1024)))
      return (TRUE);
  }

  /* setup decompressor input... */
  if (vstr_export_iovec_ptr_all(s2, &io_s2, NULL))
  {
    io_r_ptr = io_s2[0].iov_base;
    io_r_len = io_s2[0].iov_len;
  }
  else if (s2->len)
    errno = ENOMEM, err(EXIT_FAILURE, "decompress");

  /* setup decompressor output... */
  {
    unsigned int dummy_num;

    if (!vstr_add_iovec_buf_beg(s1, s1->len, 1, 2, &io_s1, &dummy_num))
      errno = ENOMEM, err(EXIT_FAILURE, "decompress");

    io_w_ptr = io_s1[0].iov_base;
    io_w_len = io_s1[0].iov_len;
  }

  switch (ex_zcat_f_type)
  {
    case EX_ZCAT_TYPE_GZIP:
    {
      int ret = Z_STREAM_ERROR;

      if (!io_r_len)
      {
        ex_zcat_close();
        break;
      }

      u->gzip.next_in   = io_r_ptr;
      u->gzip.avail_in  = io_r_len;
      u->gzip.next_out  = io_w_ptr;
      u->gzip.avail_out = io_w_len;
      ret = inflate(&u->gzip, last ? Z_SYNC_FLUSH : Z_NO_FLUSH);
      bytes_io_r = io_r_len - u->gzip.avail_in;
      bytes_io_w = io_w_len - u->gzip.avail_out;
      if (ret == Z_STREAM_END)
      {
        bytes_io_r += 8; /* remove crap from end of gzip files */
        at_end = TRUE;
        ex_zcat_close();
      }
      else if (ret != Z_OK)
        errx(EXIT_FAILURE, "failed %d = %s", (int)ret, u->gzip.msg);
    }
    break;

    case EX_ZCAT_TYPE_BZIP2:
    {
      int ret = BZ_SEQUENCE_ERROR;

      u->bzip2.next_in   = (char *)io_r_ptr;
      u->bzip2.avail_in  = io_r_len;
      u->bzip2.next_out  = (char *)io_w_ptr;
      u->bzip2.avail_out = io_w_len;
      ret = BZ2_bzDecompress(&u->bzip2);
      bytes_io_r = io_r_len - u->bzip2.avail_in;
      bytes_io_w = io_w_len - u->bzip2.avail_out;

      if (ret == BZ_STREAM_END)
      {
        at_end = TRUE;
        ex_zcat_close();
      }
      else if (ret != BZ_OK)
        errx(EXIT_FAILURE, "failed %d", (int)ret);
    }
    break;

    case EX_ZCAT_TYPE_NONE:
      assert(FALSE); /* dealt with above ... as we don't do anything */

    case EX_ZCAT_TYPE_COMPRESS:

    default:
      assert(FALSE);
      return (FALSE);
  }

  vstr_add_iovec_buf_end(s1, s1->len, bytes_io_w);
  vstr_del(s2, 1, bytes_io_r);

  return (!at_end);
}

static void ex_zcat_process_limit(Vstr_base *s1, Vstr_base *s2,
                                  unsigned int lim)
{
  while (s2->len > lim)
  {
    int proc_data = ex_zcat_process(s1, s2, !lim);
    
    if (!proc_data && (io_put(s1, STDOUT_FILENO) == IO_BLOCK))
      io_block(-1, STDOUT_FILENO);
  }
}

static void ex_zcat_read_fd_write_stdout(Vstr_base *s1, Vstr_base *s2, int fd)
{
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s2, fd);

    if (io_r_state == IO_EOF)
      break;

    ex_zcat_process(s1, s2, FALSE);

    io_w_state = io_put(s1, 1);

    io_limit(io_r_state, fd, io_w_state, 1, s1);
  }

  ex_zcat_process_limit(s1, s2, 0);
}

int main(int argc, char *argv[])
{
  Vstr_base *s2 = NULL;
  Vstr_base *s1 = NULL;
  int count = 1; /* skip the program name */
  int ex_zcat_direction = EX_ZCAT_USAGE_ENCOMPRESS;

  if (argc == 1)
    ex_zcat_direction = EX_ZCAT_USAGE_ENCOMPRESS;
  else if ((argc >= 2) && !strcmp(argv[1], "-d"))
  {
    ++argv;
    --argc;
    ex_zcat_direction = EX_ZCAT_USAGE_DECOMPRESS;
  }

  if (ex_zcat_direction == EX_ZCAT_USAGE_ENCOMPRESS)
  {
    execvp("gzip", argv);
    exit (EXIT_FAILURE);
  }

  /* init library etc. */
  s1 = ex_init(&s2);
  
  if (count >= argc)
  {
    io_fd_set_o_nonblock(STDIN_FILENO);
    ex_zcat_read_fd_write_stdout(s1, s2, STDIN_FILENO);

    ex_zcat_process_limit(s1, s2, 0);
  }

  /* loop through all arguments, open the file specified
   * and do the read/write loop */
  while (count < argc)
  {
    unsigned int ern = 0;

    /* try to mmap the file, as that is faster ... */
    if (EX_ZCAT_USE_MMAP && (s2->len < EX_MAX_R_DATA_INCORE))
      vstr_sc_mmap_file(s2, s2->len, argv[count], 0, 0, &ern);

    if ((ern == VSTR_TYPE_SC_MMAP_FILE_ERR_FSTAT_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_MMAP_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_TOO_LARGE))
    {
      int fd = io_open(argv[count]);

      ex_zcat_read_fd_write_stdout(s1, s2, fd);

      if (close(fd) == -1)
        warn("close(%s)", argv[count]);
    }
    else if (ern && (ern != VSTR_TYPE_SC_MMAP_FILE_ERR_CLOSE_ERRNO))
      err(EXIT_FAILURE, "add");
    else
      ex_zcat_process_limit(s1, s2, 0);
    
    ++count;
  }

  ex_zcat_process_limit(s1, s2, 0);
  io_put_all(s1, STDOUT_FILENO);
  
  exit (ex_exit(s1, s2));
}
