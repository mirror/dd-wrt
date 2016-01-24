
#include "ex_utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static const char *def_prefix = "";

static const char *css_fname = "dir_list.css";

#define SUB_CODE(x) do {                                \
      if (!vstr_sub_cstr_buf(s1, pos, 1, x))            \
        return (FALSE);                                 \
                                                        \
      count = strlen(x);                                \
    } while (FALSE)
static int html_conv_encode_data(Vstr_base *s1, size_t pos, size_t len)
{
  if (vstr_cmp_cstr_eq(s1, pos, len, ".."))
    return (vstr_sub_cstr_buf(s1, pos, len, "Parent directory"));

  while (len)
  {
    size_t count = vstr_cspn_cstr_chrs_fwd(s1, pos, len, "&<>");
    len -= count; pos += count;

    if (len)
    {
      switch (vstr_export_chr(s1, pos))
      {
        case '&': SUB_CODE("&amp;"); break;
        case '<': SUB_CODE("&lt;");  break;
        case '>': SUB_CODE("&gt;");  break;
        default:
          ASSERT_NOT_REACHED();
      }
      len -= count; pos += count;
    }
  }

  return (TRUE);
}

static int ex_dir_list2html_process(Vstr_base *s1, Vstr_base *s2,
                                    int *parsed_header, int *row_num)
{
  size_t pos = 0;
  size_t len = 0;
  size_t ns1 = vstr_parse_netstr(s2, 1, s2->len, &pos, &len);
  Vstr_sect_node name[1] = {{0,0}};
  Vstr_sect_node size[1] = {{0,0}};
  Vstr_sect_node type[1] = {{0,0}};
  
  if (!ns1)
  {
    if ((len     > EX_MAX_R_DATA_INCORE) || 
        (s2->len > EX_MAX_R_DATA_INCORE))
      errx(EXIT_FAILURE, "bad input");
  
    return (FALSE);
  }

  if (!*parsed_header)
  {
    size_t vpos = 0;
    size_t vlen = 0;
    size_t nst  = 0;
    
    if (!(nst = vstr_parse_netstr(s2, pos, len, &vpos, &vlen)))
      errx(EXIT_FAILURE, "bad input");
    pos += nst; len -= nst;
    if (!vstr_cmp_cstr_eq(s2, vpos, vlen, "version"))
      errx(EXIT_FAILURE, "bad input");
    if (!(nst = vstr_parse_netstr(s2, pos, len, &vpos, &vlen)))
      errx(EXIT_FAILURE, "bad input");
    pos += nst; len -= nst;
    if (!vstr_cmp_cstr_eq(s2, vpos, vlen, "1"))
      errx(EXIT_FAILURE, "Unsupported version");
    *parsed_header = TRUE;
    len = 0;
  }
  
  while (len)
  {
    size_t kpos = 0;
    size_t klen = 0;
    size_t vpos = 0;
    size_t vlen = 0;
    size_t nst  = 0;
    
    if (!(nst = vstr_parse_netstr(s2, pos, len, &kpos, &klen)))
      errx(EXIT_FAILURE, "bad input");
    pos += nst; len -= nst;
    
    if (!(nst = vstr_parse_netstr(s2, pos, len, &vpos, &vlen)))
      errx(EXIT_FAILURE, "bad input");    
    pos += nst; len -= nst;

    if (0) { }
    else if (vstr_cmp_cstr_eq(s2, kpos, klen, "name"))
    {
      name->pos = vpos;
      name->len = vlen;
    }
    else if (vstr_cmp_cstr_eq(s2, kpos, klen, "type"))
    {
      type->pos = vpos;
      type->len = vlen;
    }
    else if (vstr_cmp_cstr_eq(s2, kpos, klen, "size"))
    {
      size->pos = vpos;
      size->len = vlen;
    }
  }

  if (name->pos)
  {
    Vstr_base *href = vstr_dup_vstr(NULL, s2, name->pos, name->len, 0);
    Vstr_base *text = vstr_dup_vstr(NULL, s2, name->pos, name->len, 0);
    VSTR_AUTOCONF_uintmax_t val = 0;
      
    *row_num %= 2;
    ++*row_num;

    if (!href || !vstr_conv_encode_uri(href, 1, href->len))
      errno = ENOMEM, err(EXIT_FAILURE, "html");
    if (!text || !html_conv_encode_data(text, 1, text->len))
      errno = ENOMEM, err(EXIT_FAILURE, "html");

    vstr_add_fmt(s1, s1->len, "  <tr class=\"r%d\"> <td class=\"c1\">"
                 "<a href=\"%s${vstr:%p%zu%zu%u}\">${vstr:%p%zu%zu%u}</a></td>",
                 *row_num,
                 def_prefix,
                 href, (size_t)1, href->len, 0,
                 text, (size_t)1, text->len, 0);
    vstr_free_base(href); href = NULL;
    vstr_free_base(text); text = NULL;
    
    if (!size->pos)
      vstr_add_cstr_buf(s1, s1->len, " <td class=\"c2\"></td>");
    else
    {
      val = vstr_parse_uintmax(s2, size->pos, size->len, 10, NULL, NULL);

      vstr_add_fmt(s1, s1->len, " <td class=\"c2\">${BKMG.ju:%ju}</td>", val);
    }
    
    if (!type->pos)
      vstr_add_cstr_buf(s1, s1->len, " <td class=\"c3\"></td>");
    else
    { /* FIXME: add option to skip for block devices etc. */
      val = vstr_parse_uintmax(s2, type->pos, type->len, 10, NULL, NULL);
      
      vstr_add_cstr_buf(s1, s1->len, " <td class=\"c3\">");
      if (0) { }
      else if (S_ISREG(val))
      { /* do nothing */ }
      else if (S_ISDIR(val))
        vstr_add_cstr_buf(s1, s1->len, "(directory)");
      else if (S_ISCHR(val))
        vstr_add_cstr_buf(s1, s1->len, "(character device)");
      else if (S_ISBLK(val))
        vstr_add_cstr_buf(s1, s1->len, "(block device)");
      else if (S_ISLNK(val))
        vstr_add_cstr_buf(s1, s1->len, "(symbolic link)");
      else if (S_ISSOCK(val))
        vstr_add_cstr_buf(s1, s1->len, "(socket)");
      vstr_add_cstr_buf(s1, s1->len, "</td>");
    }
      
    vstr_add_cstr_buf(s1, s1->len, "</tr>\n");
  }
  
  vstr_del(s2, 1, ns1);

  return (TRUE);
}

static void ex_dir_list2html_process_limit(Vstr_base *s1, Vstr_base *s2,
                                           int *parsed_header, int *row_num)
{
  while (s2->len)
  { /* Finish processing read data (try writing if we need memory) */
    int proc_data = ex_dir_list2html_process(s1, s2, parsed_header, row_num);

    if (!proc_data && (io_put(s1, STDOUT_FILENO) == IO_BLOCK))
      io_block(-1, STDOUT_FILENO);
  }
}

static void ex_dir_list2html_beg(Vstr_base *s1, const char *fname)
{
  /* DTD from: http://www.w3.org/QA/2002/04/valid-dtd-list.html */
  vstr_add_fmt(s1, s1->len, "\
<!doctype html public \"-//W3C//DTD HTML 4.01//EN\"\n\
                      \"http://www.w3.org/TR/html4/strict.dtd\">\n\
<html>\n\
 <head>\n\
  <title>Directory listing of %s</title>\n\
 <link rel=\"stylesheet\" type=\"text/css\" href=\"%s\">\
 </head>\n\
 <body>\n\
  \n\
  <h1>Directory listing of %s</h1>\n\
  \n\
  <table class=\"dir_list\">\n\
  \n\
  <thead>\n\
  <tr class=\"rh\"> <th class=\"c1\">Name</th> <th class=\"c2\">Size</th>  <th class=\"c3\">Type</th> </tr>\n\
  </thead>\n\
  \n\
  <tbody>\n\
", fname, css_fname, fname);
}

static void ex_dir_list2html_end(Vstr_base *s1)
{
  vstr_add_cstr_buf(s1, s1->len, "\n\
  </tbody>\n\
  </table>\n\
 </body>\n\
</html>\n\
");
}

static void ex_dir_list2html_read_fd_write_stdout(Vstr_base *s1, Vstr_base *s2,
                                                  int fd)
{
  int parsed_header[1] = {FALSE};
  int row_num[1] = {0};
  
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s2, fd);

    if (io_r_state == IO_EOF)
      break;

    ex_dir_list2html_process(s1, s2, parsed_header, row_num);
    
    io_w_state = io_put(s1, STDOUT_FILENO);

    io_limit(io_r_state, fd, io_w_state, STDOUT_FILENO, s1);    
  }
  
  ex_dir_list2html_process_limit(s1, s2, parsed_header, row_num);
}

int main(int argc, char *argv[])
{
  Vstr_base *s1 = NULL;
  Vstr_base *s2 = ex_init(&s1); /* init the library etc. */
  int count = 1; /* skip the program name */
  const char *def_name = "&lt;stdin&gt;";
  
  if (!vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$') ||
      !vstr_sc_fmt_add_all(s1->conf))
    errno = ENOMEM, err(EXIT_FAILURE, "init");
    
  /* parse command line arguments... */
  while (count < argc)
  { /* quick hack getopt_long */
    if (!strcmp("--", argv[count]))
    {
      ++count;
      break;
    }
    EX_UTILS_GETOPT_CSTR("prefix-path",  def_prefix);
    EX_UTILS_GETOPT_CSTR("css-file",     css_fname);
    EX_UTILS_GETOPT_CSTR("cssfile",      css_fname);
    EX_UTILS_GETOPT_CSTR("css-filename", css_fname);
    EX_UTILS_GETOPT_CSTR("cssfilename",  css_fname);
    EX_UTILS_GETOPT_CSTR("name",         def_name);
    else if (!strcmp("--version", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
jdir_list2html 1.0.0\n\
Written by James Antill\n\
\n\
Uses Vstr string library.\n\
");
      goto out;
    }
    else if (!strcmp("--help", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
Usage: jdir_list2html [FILENAME]...\n\
   or: jdir_list2html OPTION\n\
Output filenames.\n\
\n\
      --help         - Display this help and exit\n\
      --version      - Output version information and exit\n\
      --css-filename - Location of css used HTML.\n\
      --name         - Name to be used if input from stdin\n\
      --prefix-path  - Prefix for href on each name in directory listing\n\
      --             - Treat rest of cmd line as input filenames\n\
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
  {
    io_fd_set_o_nonblock(STDIN_FILENO);
    ex_dir_list2html_beg(s1, def_name);
    ex_dir_list2html_read_fd_write_stdout(s1, s2, STDIN_FILENO);
    ex_dir_list2html_end(s1);
  }
  
  /* loop through all arguments, open the dir specified
   * and do the read/write loop */
  while (count < argc)
  {
    int fd = io_open(argv[count]);

    ex_dir_list2html_beg(s1, argv[count]);
    ex_dir_list2html_read_fd_write_stdout(s1, s2, fd);
    ex_dir_list2html_end(s1);

    if (close(fd) == -1)
      warn("close(%s)", argv[count]);

    ++count;
  }

  /* output all remaining data */
 out:
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, s2));
}
