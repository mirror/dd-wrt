/* This is a slowcat program, you can limit the number of bytes written and
 * how often they are written.
 * Does stdin if no args are given */

#define EX_UTILS_NO_USE_LIMIT  1
#define EX_UTILS_NO_USE_GET    1
#define EX_UTILS_NO_USE_PUT    1
#define EX_UTILS_NO_USE_PUTALL 1
#include "ex_utils.h"
#include "opt.h"

#include <timer_q.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#define EX_SLOWCAT_READ_BYTES  80
#define EX_SLOWCAT_WRITE_BYTES 80
#define EX_SLOWCAT_WRITE_WAIT_SEC 1
#define EX_SLOWCAT_WRITE_WAIT_USEC 0 /* 500000 */

typedef struct Ex_slowcat_vars
{
 unsigned int opt_read_bytes;
 unsigned int opt_write_bytes;
 
 unsigned int opt_write_wait_sec;
 unsigned int opt_write_wait_usec;

 int argc;
 char **argv;
 Vstr_base *s1;
 int arg_count;
 Timer_q_base *base;
 Timer_q_node *node;
 int fd;

 unsigned int finished_reading_data : 1;
 unsigned int finished_reading_file : 1;
 unsigned int use_mmap : 1;
} Ex_slowcat_vars;

#undef MIN_T
#define MIN_T(x, y, z) ((((z) (x)) < ((z) (y))) ? ((z) (x)) : ((z) (y)))

static void ex_slowcat_nxt_timer_func(Ex_slowcat_vars *v)
{
  struct timeval tv[1];

  gettimeofday(tv, NULL);
  TIMER_Q_TIMEVAL_ADD_SECS(tv, v->opt_write_wait_sec, v->opt_write_wait_usec);
  
  v->node = timer_q_add_node(v->base, v, tv, TIMER_Q_FLAG_NODE_DEFAULT);
  if (!v->node)
    errno = ENOMEM, err(EXIT_FAILURE, "timer_q_add_node");
}

static void ex_slowcat_timer_func(int type, void *data)
{
  Ex_slowcat_vars *v = data;
  size_t len = 0;
  int blocks_read = FALSE;
  
  if (type == TIMER_Q_TYPE_CALL_DEL)
    return;

  if (!v->finished_reading_data && (v->s1->len < v->opt_write_bytes))
  {
    if (!v->argc && !v->arg_count && v->finished_reading_file)
    {
      v->finished_reading_file = FALSE;
      v->fd = STDIN_FILENO;
      io_fd_set_o_nonblock(STDIN_FILENO);
    }
    else
    {
      if (v->finished_reading_file)
      {
        ASSERT(v->arg_count < v->argc);
        
        v->finished_reading_file = FALSE;
        
        v->fd = io_open(v->argv[v->arg_count]);
        
        ++v->arg_count;
        
        if (v->use_mmap &&
            vstr_sc_mmap_fd(v->s1, v->s1->len, v->fd, 0, 0, NULL))
        {
          if (v->arg_count >= v->argc)
            v->finished_reading_data = TRUE;
          v->finished_reading_file = TRUE;
        }
        else
          io_fd_set_o_nonblock(v->fd);
      }
    }

    if (!v->finished_reading_file)
    {
      unsigned int ern = 0;
      
      vstr_sc_read_len_fd(v->s1, v->s1->len, v->fd,
                          v->opt_read_bytes, &ern);
      
      if (ern == VSTR_TYPE_SC_READ_FD_ERR_EOF)
      {
        if (close(v->fd) == -1)
          err(EXIT_FAILURE, "close");
        
        if (v->arg_count >= v->argc)
          v->finished_reading_data = TRUE;
        v->finished_reading_file = TRUE;
      }
      else if (ern)
      {
        if ((ern == VSTR_TYPE_SC_READ_FD_ERR_READ_ERRNO) && (errno == EAGAIN))
          blocks_read = TRUE;
        else
          err(EXIT_FAILURE, "read");
      }
    }
  }

  len = MIN_T(v->opt_write_bytes, v->s1->len, size_t);
  /* do a write of the right ammount */
  if (!vstr_sc_write_fd(v->s1, 1, len, STDOUT_FILENO, NULL))
  {
    if (errno != EAGAIN)
      err(EXIT_FAILURE, "write");
    
    if ((v->s1->len > EX_MAX_W_DATA_INCORE) ||
        (v->s1->len && v->finished_reading_data))
      io_block(-1, STDOUT_FILENO);
    else if (blocks_read)
      io_block(v->fd, STDOUT_FILENO);
  }
  else if (!v->s1->len && blocks_read)
    io_block(v->fd, -1);
  
  if (v->finished_reading_data && !v->s1->len)
    return;

  if (type == TIMER_Q_TYPE_CALL_RUN_ALL)
    return;

  ex_slowcat_nxt_timer_func(v);
}

static int ex_slowcat_init_cmd_line(Ex_slowcat_vars *v)
{
  char optchar = 0;
  const char *program_name = NULL;
  struct option long_options[] =
    {
     {"bytes", required_argument, NULL, 'b'},
     {"read-bytes", required_argument, NULL, 'r'},
     {"write-bytes", required_argument, NULL, 'w'},
     {"help", no_argument, NULL, 'h'},
     {"mmap", optional_argument, NULL, 1},
     {"seconds", required_argument, NULL, 's'},
     {"useconds", required_argument, NULL, 'u'},
     {"version", no_argument, NULL, 'V'},
     {NULL, 0, NULL, 0}
    };
  FILE *help_stdout = NULL;

  help_stdout = stdout;

  program_name = opt_program_name(v->argv[0], "slowcat");

  while ((optchar = getopt_long(v->argc, v->argv, "b:hs:u:vHV",
                                long_options, NULL)) != EOF)
    switch (optchar)
    {
      case 'b':
        v->opt_read_bytes = v->opt_write_bytes = atoi(optarg);
        break;
      case 'r':
        v->opt_read_bytes  = atoi(optarg);
        break;
      case 'w':
        v->opt_write_bytes = atoi(optarg);
        break;

      case '?':
        fprintf(stderr, " The option -- %c -- is not valid.\n", optchar);
        help_stdout = stderr;
      case 'H':
      case 'h':
        fprintf(help_stdout, "\n Format: %s [-bhsuvHV] [files]\n"
                " --bytes -b        - Number of bytes to read/write at once.\n"
                " --write-bytes -w  - Number of bytes to write at once.\n"
                " --read-bytes -r   - Number of bytes to read at once.\n"
                " --help -h         - Print this message.\n"
                " --mmap            - Toggle use of mmap() to load files.\n"
                " --seconds -s      - Number of seconds to wait between write calls.\n"
                " --useconds -u     - Number of micro seconds to wait between write calls.\n"
                " --version -v      - Print the version string.\n",
                program_name);
        if (optchar == '?')
          exit (EXIT_FAILURE);
        else
          exit (EXIT_SUCCESS);
        
      case 's':
        v->opt_write_wait_sec = atoi(optarg);
        break;

      case 'u':
        v->opt_write_wait_usec = atoi(optarg);
        break;

      case   1: OPT_TOGGLE_ARG(v->use_mmap); break;
        
      case 'v':
      case 'V':
        printf(" %s is version 0.7.1\n", program_name);
        exit (EXIT_SUCCESS);
    }

  v->argc -= optind;
  v->argv += optind;

  return (optind);
}

int main(int argc, char *argv[])
{ /* This is "slowcat" */
  Vstr_base *s1 = ex_init(NULL);
  Ex_slowcat_vars v;
  struct timeval s_tv;
  const struct timeval *tv = NULL;

  /* init stuff... */
  v.opt_read_bytes      = EX_SLOWCAT_READ_BYTES;
  v.opt_write_bytes     = EX_SLOWCAT_WRITE_BYTES;
  v.opt_write_wait_sec  = EX_SLOWCAT_WRITE_WAIT_SEC;
  v.opt_write_wait_usec = EX_SLOWCAT_WRITE_WAIT_USEC;

  v.argc = argc;
  v.argv = argv;
  v.s1 = s1;
  v.arg_count = 0;
  v.base = NULL;
  v.node = NULL;
  v.finished_reading_data = FALSE;
  v.finished_reading_file = TRUE;
  v.use_mmap = FALSE;

  /* setup code... */
  ex_slowcat_init_cmd_line(&v);

  v.base = timer_q_add_base(ex_slowcat_timer_func, TIMER_Q_FLAG_BASE_DEFAULT);
  if (!v.base)
    errno = ENOMEM, err(EXIT_FAILURE, "timer_q_add_base");

  ex_slowcat_nxt_timer_func(&v);

  io_fd_set_o_nonblock(STDOUT_FILENO);

  while ((tv = timer_q_first_timeval()))
  {
    long wait_period = 0;
    
    gettimeofday(&s_tv, NULL);
    
    wait_period = timer_q_timeval_diff_usecs(tv, &s_tv);
    if (wait_period > 0)
      usleep(wait_period);
    
    gettimeofday(&s_tv, NULL);
  
    timer_q_run_norm(&s_tv);
  }
  
  timer_q_del_base(v.base);
  
  exit(ex_exit(s1, NULL));
}
