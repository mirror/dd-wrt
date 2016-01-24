/* Prints a rainbow in html span tags, using css...
 *
 * red    (0xF00), orange, yellow (0xFF0),
 * yellow (0xFF0),         green  (0x0F0),
 * green  (0x0F0),         cyan   (0x0FF),
 * cyan   (0x0FF),         blue   (0x00F),
 * blue   (0x00F), indigo, violet (0xF0F)
 *
 * ... repeat.
 */
#include "ex_utils.h"

#define EX_RAINBOW_USE_256 0 /* use a 256 or a 16 step gradient color change */
#define EX_RAINBOW_USE_PUNCT 1 /* color punctuation chars */

#define EX_RAINBOW_PUNCT_CHARS \
    "!\"#$%&'()*+,-./:;<=>?[\\]^_`{|}~"
#define EX_RAINBOW_ALPHA_CHARS \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"                            \
    "abcdefghijklmnopqrstuvwxyz"                            \
    "0123456789"

#if EX_RAINBOW_USE_PUNCT
#define EX_RAINBOW_CHARS EX_RAINBOW_ALPHA_CHARS EX_RAINBOW_PUNCT_CHARS
#else
#define EX_RAINBOW_CHARS EX_RAINBOW_ALPHA_CHARS
#endif

/* states */
#define EX_RAINBOW_ST_R2Y 0
#define EX_RAINBOW_ST_Y2G 1
#define EX_RAINBOW_ST_G2C 2
#define EX_RAINBOW_ST_C2B 3
#define EX_RAINBOW_ST_B2V 4
#define EX_RAINBOW_ST_V2R 5

static struct 
{
 unsigned int r;
 unsigned int g;
 unsigned int b;
 int state;
} color = {(EX_RAINBOW_USE_256 ? 0xFF : 0xF), 0, 0, 0};


static void ex_rainbow_process(Vstr_base *s1, Vstr_base *s2)
{
  while (s2->len)
  {
    size_t skip = vstr_cspn_cstr_chrs_fwd(s2, 1, s2->len, EX_RAINBOW_CHARS);
    int hexsz = EX_RAINBOW_USE_256 * 2;
    unsigned int hexmax = (EX_RAINBOW_USE_256 ? 0xFF : 0xF);
    unsigned int hexmin = 0;
    
    vstr_add_vstr(s1, s1->len, s2, 1, skip, VSTR_TYPE_ADD_DEF);
    vstr_del(s2, 1, skip);
  
    if (!s2->len)
      return;

    ASSERT(color.r <= hexmax);
    ASSERT(color.g <= hexmax);
    ASSERT(color.b <= hexmax);
  
    vstr_add_fmt(s1, s1->len,
                 "<span style=\"color: #%0*x%0*x%0*x\">"
                 "${vstr:%p%zu%zu%u}"
                 "</span>",
                 hexsz, color.r, hexsz, color.g, hexsz, color.b,
                 s2, 1, 1, VSTR_TYPE_ADD_DEF);
    
    if (s1->conf->malloc_bad)
      errno = ENOMEM, err(EXIT_FAILURE, "adding data");
    
    vstr_del(s2, 1, 1);
    
    switch (color.state)
    {
      case EX_RAINBOW_ST_R2Y:
        if (++color.g == hexmax) color.state = EX_RAINBOW_ST_Y2G; break;
      case EX_RAINBOW_ST_Y2G:
        if (--color.r == hexmin) color.state = EX_RAINBOW_ST_G2C; break;
      case EX_RAINBOW_ST_G2C:
        if (++color.b == hexmax) color.state = EX_RAINBOW_ST_C2B; break;
      case EX_RAINBOW_ST_C2B:
        if (--color.g == hexmin) color.state = EX_RAINBOW_ST_B2V; break;
      case EX_RAINBOW_ST_B2V:
        if (++color.r == hexmax) color.state = EX_RAINBOW_ST_V2R; break;
      case EX_RAINBOW_ST_V2R:
        if (--color.b == hexmin) color.state = EX_RAINBOW_ST_R2Y; break;
        
      default:
        ASSERT(FALSE);
    }
  }
}

static void ex_rainbow_read_fd_write_stdout(Vstr_base *s1,
                                            Vstr_base *s2, int fd)
{
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s2, fd);

    if (io_r_state == IO_EOF)
      break;
    
    ex_rainbow_process(s1, s2);

    io_w_state = io_put(s1, 1);
    
    io_limit(io_r_state, fd, io_w_state, 1, s1);    
  }
  
  ex_rainbow_process(s1, s2);
}

int main(int argc, char *argv[])
{
  Vstr_base *s2 = NULL;
  Vstr_base *s1 = ex_init(&s2);
  int count = 1;

  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_sc_fmt_add_all(NULL);

  if (!s1->conf)
    errno = ENOMEM, err(EXIT_FAILURE, "custom formatters");

  if (count >= argc)
  {
    io_fd_set_o_nonblock(STDIN_FILENO);
    ex_rainbow_read_fd_write_stdout(s1, s2, STDIN_FILENO);
  }  

  while (count < argc)
  {
    int fd = io_open(argv[count]);

    ex_rainbow_read_fd_write_stdout(s1, s2, fd);

    if (close(fd) == -1)
      warn("close(%s)", argv[count]);

    ++count;
  }

  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, s2));
}
