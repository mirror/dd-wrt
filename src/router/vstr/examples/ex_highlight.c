/* quick and dirty copy of the
 * highlight (http://freshmeat.net/projects/highlight/) command */
#include "ex_utils.h"

#include <sys/time.h>
#include <time.h>

static struct 
{
 const char *name;
 const char *seq;
 size_t off;
 size_t eoff;
 size_t len;
} ex_hl_seqs[] = 
/* This assumes a certain style...
   
   for instance neither of...

   if(foo)
   foo;if (bar)

   ...will be seen as an if statement, as there is no space before and after
   the if. And using TABs instead of spaces will make it not work.
   These are all features, as the style should be the same.
*/
#define EX_HL_SEQ_T(x, y) \
   x, " "  y " ", 1, 1, 0 }, \
 { x, "("  y " ", 1, 1, 0 }, \
 { x, " "  y ")", 1, 1, 0 }, \
 { x, "("  y ")", 1, 1, 0 }, \
 { x, "*"  y " ", 1, 1, 0 }, \
 { x, "\n" y " ", 1, 1, 0
#define EX_HL_SEQ_SP(x, y) \
   x, " " y " ",  1, 1, 0
#define EX_HL_SEQ_WS(x, y) \
   x, " " y " ",  1, 1, 0 }, \
 { x, " " y "\n", 1, 1, 0
/* custom for default and break */
#define EX_HL_SEQ_DEF(x, y) \
   x, " " y ": ", 1, 2, 0 }, \
 { x, " " y ":\n", 1, 2, 0
#define EX_HL_SEQ_BRK(x, y) \
   x, " " y ";\n", 1, 2, 0
#define EX_HL_SEQ_SB(x, y) \
   x, " " y " (", 1, 2, 0
#define EX_HL_SEQ_RET(x, y) \
   x, " " y " (",  1, 2, 0 }, \
 { x, " " y ";\n", 1, 2, 0
#define EX_HL_SEQ_B(x, y)  \
   x, " " y "(", 1, 1, 0
#define EX_HL_SEQ_VAL(x, y) \
   x, " "  y " ",  1, 1, 0 }, \
 { x, " "  y "\n", 1, 1, 0 }, \
 { x, " "  y ";",  1, 1, 0 }, \
 { x, " "  y ",",  1, 1, 0 }, \
 { x, " "  y ")",  1, 1, 0 }, \
 { x, "("  y " ",  1, 1, 0 }, \
 { x, "("  y ")",  1, 1, 0 }, \
 { x, "("  y ",",  1, 1, 0
#define EX_HL_SEQ_CPP(x, y) \
   x, "#"     y " ",  1, 1, 0 }, \
 { x, "#"     y "\n", 1, 1, 0 }, \
 { x, "# "    y " ",  2, 1, 0 }, \
 { x, "# "    y "\n", 2, 1, 0 }, \
 { x, "#  "   y " ",  3, 1, 0 }, \
 { x, "#  "   y "\n", 3, 1, 0 }, \
 { x, "#   "  y " ",  4, 1, 0 }, \
 { x, "#   "  y "\n", 4, 1, 0 }, \
 { x, "#    " y " ",  5, 1, 0 }, \
 { x, "#    " y "\n", 5, 1, 0
{
 /* order matters */
 { EX_HL_SEQ_T("vstrbase", "Vstr_base") },
 { EX_HL_SEQ_T("vstrsects", "Vstr_sects") },
 { EX_HL_SEQ_T("vstrfmt", "Vstr_fmt_spec") },
 { EX_HL_SEQ_T("pollfd", "struct pollfd") },
 { EX_HL_SEQ_T("mpzt", "mpz_t") },
 
 { EX_HL_SEQ_CPP("cppif", "if") },
 { EX_HL_SEQ_CPP("cppifndef", "ifndef") },
 { EX_HL_SEQ_CPP("cppifdef", "ifdef") },
 { EX_HL_SEQ_CPP("cppelse", "else") },
 { EX_HL_SEQ_CPP("cppendif", "endif") },
 { EX_HL_SEQ_CPP("cppdefine", "define") },
 { EX_HL_SEQ_CPP("cppinclude", "include") },
 { EX_HL_SEQ_CPP("cppelif", "elif") },
 { EX_HL_SEQ_SB("cppdefined", "defined") },
 { EX_HL_SEQ_B("cppdefined", "defined") },
 
 { EX_HL_SEQ_SB("if", "if") },
 { EX_HL_SEQ_WS("else", "else") },
 { EX_HL_SEQ_WS("do", "do") },
 { EX_HL_SEQ_SB("while", "while") },
 { EX_HL_SEQ_SB("for", "for") },
 { EX_HL_SEQ_RET("return", "return") },
 { EX_HL_SEQ_SB("switch", "switch") },
 { EX_HL_SEQ_SP("case", "case") },
 { EX_HL_SEQ_DEF("default", "default") },
 { EX_HL_SEQ_BRK("break", "break") },
 { EX_HL_SEQ_SP("goto", "goto") },
 { EX_HL_SEQ_T("extern", "extern") },
 { EX_HL_SEQ_T("static", "static") },
 { EX_HL_SEQ_T("const", "const") },
 { EX_HL_SEQ_T("restrict", "restrict") },
 { EX_HL_SEQ_T("inline", "inline") },
 { EX_HL_SEQ_T("void", "void") },
 { EX_HL_SEQ_T("unsigned", "unsigned") },
 { EX_HL_SEQ_T("char", "char") },
 { EX_HL_SEQ_T("short", "short") },
 { EX_HL_SEQ_T("int", "int") },
 { EX_HL_SEQ_T("long", "long") },
 { EX_HL_SEQ_T("float", "float") },
 { EX_HL_SEQ_T("double", "double") },
 { EX_HL_SEQ_T("ssizet", "ssize_t") },
 { EX_HL_SEQ_T("sizet", "size_t") },
 { EX_HL_SEQ_T("offt", "off_t") },
 { EX_HL_SEQ_T("off64t", "off64_t") },
 { EX_HL_SEQ_T("intmaxt", "intmax_t") },
 { EX_HL_SEQ_T("uintmaxt", "uintmax_t") },
 
 { EX_HL_SEQ_SB("exit", "exit") },
 { EX_HL_SEQ_SB("abort", "abort") },
 { EX_HL_SEQ_B("err", "err") },
 { EX_HL_SEQ_B("err", "errx") },
 { EX_HL_SEQ_B("warn", "warn") },
 { EX_HL_SEQ_B("warn", "warnx") },
 { EX_HL_SEQ_B("assert", "assert") },
 { EX_HL_SEQ_B("assert", "ASSERT") },
 { EX_HL_SEQ_B("assert", "assert_ret") },
 { EX_HL_SEQ_B("assert", "ASSERT_RET") },
 { EX_HL_SEQ_B("assert", "assert_ret_void") },
 { EX_HL_SEQ_B("assert", "ASSERT_RET_VOID") },
 { EX_HL_SEQ_B("assert", "assert_no_switch_def") },
 { EX_HL_SEQ_B("assert", "ASSERT_NO_SWITCH_DEF") },

 { EX_HL_SEQ_VAL("compdate", "__DATE__") },
 { EX_HL_SEQ_VAL("compversion", "__VERSION__") },
 { EX_HL_SEQ_VAL("compfile", "__FILE__") },
 { EX_HL_SEQ_VAL("compline", "__LINE__") },
 { EX_HL_SEQ_VAL("charbit", "CHAR_BIT") },
 { EX_HL_SEQ_VAL("intmax", "INT_MAX") },
 { EX_HL_SEQ_VAL("intmin", "INT_MIN") },
 { EX_HL_SEQ_VAL("intjmax", "INTMAX_MAX") },
 { EX_HL_SEQ_VAL("intjmin", "INTMAX_MIN") },
 { EX_HL_SEQ_VAL("stdin", "stdin") },
 { EX_HL_SEQ_VAL("stdout", "stdout") },
 { EX_HL_SEQ_VAL("stderr", "stderr") },
 { EX_HL_SEQ_VAL("stdin", "STDIN_FILENO") },
 { EX_HL_SEQ_VAL("stdout", "STDOUT_FILENO") },
 { EX_HL_SEQ_VAL("stderr", "STDERR_FILENO") },
 { EX_HL_SEQ_VAL("errno", "errno") },
 { EX_HL_SEQ_VAL("exitsucs", "EXIT_SUCCESS") },
 { EX_HL_SEQ_VAL("exitfail", "EXIT_FAILURE") },
 { EX_HL_SEQ_VAL("true", "TRUE") },
 { EX_HL_SEQ_VAL("false", "FALSE") },
 { EX_HL_SEQ_VAL("null", "NULL") },
 { EX_HL_SEQ_VAL("num0",   "0") },
 { EX_HL_SEQ_VAL("numm1", "-1") },
 
 {NULL, NULL, 0, 0, 0},
};
static size_t ex_hl_max_seq_len = 0;

#define C_DEF 0
#define C_SEQ 1
#define C_STR 2
#define C_CHR 3
#define C_CMO 4
#define C_CMN 5

static void ex_hl_mov_clean(Vstr_base *s1, Vstr_base *s2, size_t len)
{ /* html the elements... */
  while (len > 0)
  {
    size_t count = vstr_cspn_cstr_chrs_fwd(s2, 1, len, "&<>");
    
    vstr_add_vstr(s1, s1->len, s2, 1, count, VSTR_TYPE_ADD_BUF_REF);
    vstr_del(s2, 1, count);
    len -= count;

    if (count)
      continue;

    --len;
    
    switch (vstr_export_chr(s2, 1))
    {
      case '<': /* html stuff ... */
        vstr_add_cstr_ptr(s1, s1->len, "&lt;");
        vstr_del(s2, 1, 1);
        continue;
        
      case '>':
        vstr_add_cstr_ptr(s1, s1->len, "&gt;");
        vstr_del(s2, 1, 1);
        continue;
        
      case '&':
        vstr_add_cstr_ptr(s1, s1->len, "&amp;");
        vstr_del(s2, 1, 1);
        continue;
        
      default:
        ASSERT(FALSE);
        break;
    }  
  }
}

static int first_time = FALSE;

static int ex_hl_process(Vstr_base *s1, Vstr_base *s2, int last)
{
  static unsigned int state = C_DEF;

  /* we don't want to create more data, if we are over our limit */
  if (s1->len > EX_MAX_W_DATA_INCORE)
    return (FALSE);

  if (s2->len < ex_hl_max_seq_len)
  {
    if (!s2->len || !last)
      return (FALSE);
  }
  
  while (((s2->len >= ex_hl_max_seq_len) || (s2->len && last)) &&
         (s1->len <= EX_MAX_W_DATA_INCORE))
  {
    size_t len = 0;
  
    switch (state)
    {
      case C_DEF:
        /* this is pretty intimately tied with the array above */
        len = vstr_cspn_cstr_chrs_fwd(s2, 1, s2->len, "# \n(*\"'/&<>");
        vstr_add_vstr(s1, s1->len, s2, 1, len, VSTR_TYPE_ADD_BUF_REF);
        vstr_del(s2, 1, len);
        state = C_SEQ;
        break;
        
      case C_SEQ:
      {
        unsigned int scan = 0;
        const char *seq = NULL;

        switch (vstr_export_chr(s2, 1))
        {
          case '\'':
            state = C_CHR;
            vstr_add_cstr_ptr(s1, s1->len, "<span class=\"chr\">'");
            vstr_del(s2, 1, 1);
            continue;
            
          case '/':
            if (s2->len < 2)
              break;
            
            if (vstr_cmp_cstr_eq(s2, 1, 2, "/*"))
            {
              state = C_CMO;
              vstr_add_cstr_ptr(s1, s1->len, "<span class=\"comment\">/*");
              vstr_del(s2, 1, 2);
              continue;
            }
            if (vstr_cmp_cstr_eq(s2, 1, 2, "//"))
            {
              state = C_CMN;
              vstr_add_cstr_ptr(s1, s1->len, "<span class=\"comment\">//");
              vstr_del(s2, 1, 2);
              continue;
            }
            break;
            
          case '"':
            state = C_STR;
            vstr_add_cstr_ptr(s1, s1->len, "<span class=\"str\">\"");
            vstr_del(s2, 1, 1);
            continue;

          case '<': /* html stuff ... */
            vstr_add_cstr_ptr(s1, s1->len, "&lt;");
            vstr_del(s2, 1, 1);
            continue;
            
          case '>':
            vstr_add_cstr_ptr(s1, s1->len, "&gt;");
            vstr_del(s2, 1, 1);
            continue;
            
          case '&':
            vstr_add_cstr_ptr(s1, s1->len, "&amp;");
            vstr_del(s2, 1, 1);
            continue;
            
          default:
            break;
        }
        
        while (ex_hl_seqs[scan].name)
        {
          seq = ex_hl_seqs[scan].seq;
          len = ex_hl_seqs[scan].len;
          
          if ((len <= s2->len) && vstr_cmp_buf_eq(s2, 1, len, seq, len))
          {
            size_t off  = ex_hl_seqs[scan].off;
            size_t eoff = ex_hl_seqs[scan].eoff;
            unsigned int mid = len - (off + eoff);

            if (first_time)
            {
              ASSERT(off && (vstr_export_chr(s2, 1) == '\n'));
              vstr_del(s2, 1, 1);
              --off;
              first_time = FALSE;
            }
            
            vstr_add_vstr(s1, s1->len, s2, 1, off, VSTR_TYPE_ADD_BUF_REF);
            vstr_del(s2, 1, off);
            
            vstr_add_cstr_ptr(s1, s1->len, "<span class=\"");
            vstr_add_cstr_ptr(s1, s1->len, ex_hl_seqs[scan].name);
            vstr_add_cstr_ptr(s1, s1->len, "\">");

            vstr_add_vstr(s1, s1->len, s2, 1, mid, VSTR_TYPE_ADD_BUF_REF);
            vstr_del(s2, 1, mid);
            
            vstr_add_cstr_ptr(s1, s1->len, "</span>");
            /* don't output end marker */
            break;
          }
          
          ++scan;
        }

        if (!ex_hl_seqs[scan].name)
        {
          if (first_time)
          {
            ASSERT(vstr_export_chr(s2, 1) == '\n');
            first_time = FALSE;
          }
          else
            vstr_add_vstr(s1, s1->len, s2, 1, 1, VSTR_TYPE_ADD_BUF_REF);
          vstr_del(s2, 1, 1);
        }
        
        state = C_DEF;
      }
      break;

      case C_CMO:
        len = vstr_srch_cstr_buf_fwd(s2, 1, s2->len, "*/");
        if (!len)
        {
          ex_hl_mov_clean(s1, s2, s2->len);
          return (TRUE);
        }

        ++len; /* move to last character */
        
        state = C_DEF;
        ex_hl_mov_clean(s1, s2, len);
        vstr_add_cstr_ptr(s1, s1->len, "</span>");
        break;
        
      case C_CMN:
        len = vstr_srch_chr_fwd(s2, 1, s2->len, '\n');
        if (!len)
        {
          ex_hl_mov_clean(s1, s2, s2->len);
          return (TRUE);
        }
        
        state = C_DEF;
        ex_hl_mov_clean(s1, s2, len);
        vstr_add_cstr_ptr(s1, s1->len, "</span>");
        break;
        
      case C_CHR:
        len = vstr_srch_cstr_buf_fwd(s2, 1, s2->len, "'");
        if (!len)
        {
          ex_hl_mov_clean(s1, s2, s2->len);
          return (TRUE);
        }
        
        if ((len == 1) || /* even number of \'s going backwards */
            (!(vstr_spn_cstr_chrs_rev(s2, 1, len - 1, "\\") & 1)))
          state = C_DEF;
        ex_hl_mov_clean(s1, s2, len);
        if (state == C_DEF)
          vstr_add_cstr_ptr(s1, s1->len, "</span>");
        break;
        
      case C_STR:
        len = vstr_srch_cstr_buf_fwd(s2, 1, s2->len, "\"");
        if (!len)
        {
          ex_hl_mov_clean(s1, s2, s2->len);
          return (TRUE);
        }
        
        if ((len == 1) || /* even number of \'s going backwards */
            (!(vstr_spn_cstr_chrs_rev(s2, 1, len - 1, "\\") & 1)))
          state = C_DEF;
        ex_hl_mov_clean(s1, s2, len);
        if (state == C_DEF)
          vstr_add_cstr_ptr(s1, s1->len, "</span>");
        break;
        

      default:
        ASSERT(FALSE);
    }
  }
  
  return (TRUE);
}

static void ex_hl_process_limit(Vstr_base *s1, Vstr_base *s2, unsigned int lim)
{
  while (s2->len > lim)
  {
    int proc_data = ex_hl_process(s1, s2, !lim);
    if (!proc_data && (io_put(s1, STDOUT_FILENO) == IO_BLOCK))
      io_block(-1, STDOUT_FILENO);
  }
}

static void ex_hl_read_fd_write_stdout(Vstr_base *s1, Vstr_base *s2, int fd)
{
  while (TRUE)
  {
    int io_w_state = IO_OK;
    int io_r_state = io_get(s2, fd);

    if (io_r_state == IO_EOF)
      break;

    ex_hl_process(s1, s2, FALSE);

    io_w_state = io_put(s1, 1);

    io_limit(io_r_state, fd, io_w_state, 1, s1);
  }

  ex_hl_process_limit(s1, s2, 0);
}

static const char *ex_hl_ctime(time_t val)
{
  static char ret[4096];

  strftime(ret, sizeof(ret), "%c", localtime(&val));

  return (ret);
}

static const char *base_fname(const char *s1)
{
  const char *sname = strrchr(s1, '/');

  if (sname)
    ++sname;
  else
    sname = s1;

  return (sname);
}

static void ex_hl_block_beg(Vstr_base *s1, const char *block_type,
                            const char *block_beg,
                            const char *attr_id, const char *attr_class)
{
  vstr_add_fmt(s1, s1->len, "<%s", block_type);
  if (attr_id)
    vstr_add_fmt(s1, s1->len, " id=\"%s\"", attr_id);
  if (attr_class)
    vstr_add_fmt(s1, s1->len, " class=\"%s\"", attr_class);
  vstr_add_fmt(s1, s1->len, ">%s", block_beg);
}
static void ex_hl_block_end(Vstr_base *s1, const char *block_end,
                            const char *block_type,
                            unsigned int comments,
                            time_t timestamp, const char *fname)
{
  vstr_add_fmt(s1, s1->len, "%s</%s>\n", block_end, block_type);
  if (comments)
    vstr_add_fmt(s1, s1->len,
                 "<!-- C to html convertion of %s -->\n"
                 "<!--   done on %s -->\n"
                 "<!--   done by jhighlight -->\n",
                 fname, ex_hl_ctime(timestamp));
}

int main(int argc, char *argv[])
{
  Vstr_base *s2 = NULL;
  Vstr_base *s1 = ex_init(&s2);
  int count = 1;
  time_t now = time(NULL);
  unsigned int use_mmap = FALSE;
  unsigned int comments = TRUE;
  const char *cssfile = NULL;
  const char *block_type = NULL;
  const char *block_beg = "\n";
  const char *block_end = NULL;
  const char *attr_id = NULL;
  const char *attr_class = "c2html";
  
  {
    size_t scan = 0;
    
    while (ex_hl_seqs[scan].name)
    {
      size_t len = strlen(ex_hl_seqs[scan].seq);

      ex_hl_seqs[scan].len = len;
      
      if (ex_hl_max_seq_len < len)
        ex_hl_max_seq_len = len;
      
      ++scan;
    }
  }
  
  /* parse command line arguments... */
  while (count < argc)
  { /* quick hack getopt_long */
    if (!strcmp("--", argv[count]))
    {
      ++count;
      break;
    }
    else if (!strcmp("--comments", argv[count])) /* toggle use of mmap */
      comments = !comments;
    else if (!strcmp("--mmap", argv[count])) /* toggle use of mmap */
      use_mmap = !use_mmap;
    EX_UTILS_GETOPT_CSTR("beg",     block_beg);
    EX_UTILS_GETOPT_CSTR("cssfile",      cssfile);
    EX_UTILS_GETOPT_CSTR("css-file",     cssfile);
    EX_UTILS_GETOPT_CSTR("cssfilename",  cssfile);
    EX_UTILS_GETOPT_CSTR("css-filename", cssfile);
    EX_UTILS_GETOPT_CSTR("class",   attr_class);
    EX_UTILS_GETOPT_CSTR("end",     block_end);
    EX_UTILS_GETOPT_CSTR("id",      attr_id);
    EX_UTILS_GETOPT_CSTR("type",    block_type);
    else if (!strcmp("--version", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
jhighlight 1.0.0\n\
Written by James Antill\n\
\n\
Uses Vstr string library.\n\
");
      goto out;
    }
    else if (!strcmp("--help", argv[count]))
    { /* print version and exit */
      vstr_add_fmt(s1, 0, "%s", "\
Usage: jhighlight [STRING]...\n\
   or: jhighlight OPTION\n\
Output filenames in html converteed from C.\n\
\n\
      --help         - Display this help and exit\n\
      --version      - Output version information and exit\n\
      --mmap         - Toggle use of mmap() to load input files\n\
      --comments     - Toggle output of attribution comments\n\
      --beg          - Extra text to output at the begining\n\
      --css-filename - Location of css used in HTML.\n\
      --class        - Class name for block\n\
      --end          - Extra text to output at the ending\n\
      --id           - Id name for block\n\
      --type         - Name for block (Eg. \"pre\" or \"code\" etc.)\n\
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

  if (cssfile    && !*cssfile)    cssfile    = NULL;
  if (block_type && !*block_type) block_type = NULL;
  if (attr_id    && !*attr_id)    attr_id    = NULL;
  if (attr_class && !*attr_class) attr_class = NULL;
  
  if (!block_type) block_type = "pre";
  if (!block_end)  block_end  = "";
  
  if (cssfile)
  {
    int scan = count;
    
    vstr_add_cstr_ptr(s1, s1->len, "\
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n\
<html>\n\
  <head>\n\
    <title>");

    if (scan >= argc)
      vstr_add_cstr_ptr(s1, s1->len, "c2html for: STDIN");
    else
      vstr_add_fmt(s1, s1->len, "c2html for: %s", base_fname(argv[scan++]));
    
    while (scan < argc)
      vstr_add_fmt(s1, s1->len, ", %s", base_fname(argv[scan++]));
    
    vstr_add_cstr_ptr(s1, s1->len, "</title>\n\
    <link rel=\"stylesheet\" type=\"text/css\" href=\"");
    vstr_add_cstr_ptr(s1, s1->len, cssfile);
    vstr_add_cstr_ptr(s1, s1->len, "\">\n\
  </head>\n\
  <body>\n");
  }
  
  /* if no arguments are given just do stdin to stdout */
  if (count >= argc)
  {
    io_fd_set_o_nonblock(STDIN_FILENO);
    
    if (cssfile)
      vstr_add_cstr_ptr(s1, s1->len, "     <h1>STDIN</h1>\n");

    ex_hl_block_beg(s1, block_type, block_beg, attr_id, attr_class);
    first_time = TRUE;
    vstr_add_rep_chr(s2, s2->len, '\n', 1);
    ex_hl_read_fd_write_stdout(s1, s2, STDIN_FILENO);
    ex_hl_block_end(s1, block_end, block_type, comments, now, "stdin");
  }

  /* loop through all arguments, open the file specified
   * and do the read/write loop */
  while (count < argc)
  {
    unsigned int ern = 0;

    ASSERT(!s2->len);
    first_time = TRUE;
    vstr_add_rep_chr(s2, s2->len, '\n', 1); /* add to begining */
    
    if (use_mmap && (s2->len <= EX_MAX_R_DATA_INCORE))
      vstr_sc_mmap_file(s2, s2->len, argv[count], 0, 0, &ern);

    if (cssfile)
      vstr_add_fmt(s1, s1->len, "     <h1>%s</h1>\n", base_fname(argv[count]));
    
    ex_hl_block_beg(s1, block_type, block_beg, attr_id, attr_class);

    if (!use_mmap ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_FSTAT_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_MMAP_ERRNO) ||
        (ern == VSTR_TYPE_SC_MMAP_FILE_ERR_TOO_LARGE))
    {
      int fd = io_open(argv[count]);

      ex_hl_read_fd_write_stdout(s1, s2, fd);

      if (close(fd) == -1)
        warn("close(%s)", argv[count]);
    }
    else if (ern && (ern != VSTR_TYPE_SC_MMAP_FILE_ERR_CLOSE_ERRNO))
      err(EXIT_FAILURE, "mmap");
    else
      ex_hl_process_limit(s1, s2, 0);

    ex_hl_block_end(s1, block_end, block_type, comments, now, argv[count]);
    
    ++count;
  }
  
  if (cssfile)
  {
    vstr_add_cstr_ptr(s1, s1->len, "\n\
  </body>\n\
</html>\n");
  }
  
 out:
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, s2));
}
