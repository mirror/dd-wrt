/* this program shows all possible names for given numbers */

#include "ex_utils.h"
#include <limits.h>


static char phone_state[CHAR_MAX] = {
 ['2'] = 'a',
 ['a'] = 'b',
 ['b'] = 'c',
 ['c'] = '2',

 ['3'] = 'd',
 ['d'] = 'e',
 ['e'] = 'f',
 ['f'] = '3',

 ['4'] = 'g',
 ['g'] = 'h',
 ['h'] = 'i',
 ['i'] = '4',

 ['5'] = 'j',
 ['j'] = 'k',
 ['k'] = 'l',
 ['l'] = '5',

 ['6'] = 'm',
 ['m'] = 'n',
 ['n'] = 'o',
 ['o'] = '6',

 ['7'] = 'p',
 ['p'] = 'r',
 ['r'] = 's',
 ['s'] = '7',

 ['8'] = 't',
 ['t'] = 'u',
 ['u'] = 'v',
 ['v'] = '8',

 ['9'] = 'w',
 ['w'] = 'x',
 ['x'] = 'y',
 ['y'] = 'z',
 ['z'] = '9', 
};


#define xisdigit(x) (((x) >= '2') && ((x) <= '9'))

static int prnt_phone_name(Vstr_base *s1,
                           Vstr_base *s2, size_t pos, size_t len,
                           Vstr_sects *sects, unsigned int off)
{
  size_t cpos = 0;
  char chr = 0;
  char nxt_chr = 0;
  
  if (off > sects->num)
    return (FALSE);

  cpos = VSTR_SECTS_NUM(sects, off)->pos;
  chr = vstr_export_chr(s2, cpos);
  nxt_chr = phone_state[(unsigned char) chr];
  
  vstr_sub_rep_chr(s2, cpos, 1, nxt_chr, 1);
  if (xisdigit(nxt_chr))
  {
    if (!prnt_phone_name(s1, s2, pos, len, sects, off + 1))
      return (FALSE);
  }
  
  return (TRUE);
}

static void split_num(Vstr_sects *sects, Vstr_base *s2, size_t pos, size_t len)
{
  Vstr_iter iter[1];
  
  if (!vstr_iter_fwd_beg(s2, pos, len, iter))
    err(EXIT_FAILURE, "INTERNAL ERROR");

  while (len--)
  {
    char chr = vstr_iter_fwd_chr(iter, NULL);

    if (xisdigit(chr))
      vstr_sects_add(sects, pos, 1);

    ++pos;
  }
}

#define PRNT_LOOP_NAMES() do {                                          \
                                                                        \
      while (prnt_phone_name(s1, s2, 1, llen, sects, 1))                \
      {                                                                 \
        ret = TRUE;                                                     \
                                                                        \
        vstr_add_vstr(s1, s1->len, s2, 1, llen, VSTR_TYPE_ADD_DEF);     \
        if (s1->len > EX_MAX_W_DATA_INCORE)                             \
        {                                                               \
          if (sects->malloc_bad || s1->conf->malloc_bad)                \
            errno = ENOMEM, err(EXIT_FAILURE, "adding data");           \
                                                                        \
          return (ret);                                                 \
        }                                                               \
      }                                                                 \
                                                                        \
      if (sects->malloc_bad || s1->conf->malloc_bad)                    \
        errno = ENOMEM, err(EXIT_FAILURE, "adding data");               \
                                                                        \
      vstr_del(s2, 1, llen);                                            \
      llen = 0;                                                         \
      if (!s2->len)                                                     \
      {                                                                 \
        vstr_sects_free(sects);                                         \
        sects = NULL;                                                   \
      }                                                                 \
} while (FALSE)

static int ex_phones_name_process(Vstr_base *s1, Vstr_base *s2, int last)
{
  static Vstr_sects *sects = NULL;
  static size_t llen = 0;
  int ret = FALSE;
    
  /* we don't want to create more data, if we are over our limit */
  if (s1->len > EX_MAX_W_DATA_INCORE)
    return (FALSE);

  if (llen)
  {
    assert(sects);

    PRNT_LOOP_NAMES();
  }
  
  if (!s2->len)
    return (ret);

  while ((llen = vstr_srch_chr_fwd(s2, 1, s2->len, '\n')))
  {
    if (!sects && !(sects = vstr_sects_make(llen - 1)))
      errno = ENOMEM, err(EXIT_FAILURE, "adding data");
      
    split_num(sects, s2, 1, llen);
    PRNT_LOOP_NAMES();
  }

  if (s2->len && last)
  {
    llen = s2->len + 1;
    if (!vstr_add_cstr_buf(s2, s2->len, "\n") &&
        !sects && !(sects = vstr_sects_make(s2->len - 1)))
      errno = ENOMEM, err(EXIT_FAILURE, "adding data");
      
    ret = TRUE;

    split_num(sects, s2, 1, llen);
    PRNT_LOOP_NAMES();
  }

  vstr_sects_free(sects);
  sects = NULL;
      
  return (ret);
}

/* files are merged */
static void ex_phones_name_read_fd_write_stdout(Vstr_base *s1, Vstr_base *s2,
                                                int fd)
{
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s2, fd);

    if (io_r_state == IO_EOF)
      break;

    ex_phones_name_process(s1, s2, FALSE);

    io_w_state = io_put(s1, 1);

    io_limit(io_r_state, fd, io_w_state, 1, s1);
  }
}

static void ex_phones_name_process_limit(Vstr_base *s1, Vstr_base *s2,
                                         unsigned int lim)
{
  while (s2->len > lim)
  {
    int proc_data = ex_phones_name_process(s1, s2, !lim);
    if (!proc_data && (io_put(s1, STDOUT_FILENO) == IO_BLOCK))
      io_block(-1, STDOUT_FILENO);
  }
}


int main(int argc, char *argv[])
{
  Vstr_base *s2 = NULL;
  Vstr_base *s1 = ex_init(&s2);
  int count = 1;

  /* if no arguments are given just do stdin to stdout */
  if (count >= argc)
  {
    io_fd_set_o_nonblock(STDIN_FILENO);
    ex_phones_name_read_fd_write_stdout(s1, s2, STDIN_FILENO);
  }

  /* loop through all arguments, open the file specified
   * and do the read/write loop */
  while (count < argc)
  {
    unsigned int ern = 0;

    if (s2->len <= EX_MAX_R_DATA_INCORE)
      vstr_sc_mmap_file(s2, s2->len, argv[count], 0, 0, &ern);

    if ((ern == VSTR_TYPE_SC_MMAP_FILE_ERR_FSTAT_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_MMAP_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_TOO_LARGE))
    {
      int fd = io_open(argv[count]);

      ex_phones_name_read_fd_write_stdout(s1, s2, fd);

      if (close(fd) == -1)
        warn("close(%s)", argv[count]);
    }
    else if (ern && (ern != VSTR_TYPE_SC_MMAP_FILE_ERR_CLOSE_ERRNO))
      err(EXIT_FAILURE, "add");
    else
      ex_phones_name_process_limit(s1, s2, EX_MAX_R_DATA_INCORE);
    
    ++count;
  }

  ex_phones_name_process_limit(s1, s2, 0);

  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, s2));
}
