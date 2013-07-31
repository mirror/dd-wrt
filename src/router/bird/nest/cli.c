/*
 *	BIRD Internet Routing Daemon -- Command-Line Interface
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Command line interface
 *
 * This module takes care of the BIRD's command-line interface (CLI).
 * The CLI exists to provide a way to control BIRD remotely and to inspect
 * its status. It uses a very simple textual protocol over a stream
 * connection provided by the platform dependent code (on UNIX systems,
 * it's a UNIX domain socket).
 *
 * Each session of the CLI consists of a sequence of request and replies,
 * slightly resembling the FTP and SMTP protocols.
 * Requests are commands encoded as a single line of text, replies are
 * sequences of lines starting with a four-digit code followed by either
 * a space (if it's the last line of the reply) or a minus sign (when the
 * reply is going to continue with the next line), the rest of the line
 * contains a textual message semantics of which depends on the numeric
 * code. If a reply line has the same code as the previous one and it's
 * a continuation line, the whole prefix can be replaced by a single
 * white space character.
 *
 * Reply codes starting with 0 stand for `action successfully completed' messages,
 * 1 means `table entry', 8 `runtime error' and 9 `syntax error'.
 *
 * Each CLI session is internally represented by a &cli structure and a
 * resource pool containing all resources associated with the connection,
 * so that it can be easily freed whenever the connection gets closed, not depending
 * on the current state of command processing.
 *
 * The CLI commands are declared as a part of the configuration grammar
 * by using the |CF_CLI| macro. When a command is received, it is processed
 * by the same lexical analyzer and parser as used for the configuration, but
 * it's switched to a special mode by prepending a fake token to the text,
 * so that it uses only the CLI command rules. Then the parser invokes
 * an execution routine corresponding to the command, which either constructs
 * the whole reply and returns it back or (in case it expects the reply will be long)
 * it prints a partial reply and asks the CLI module (using the @cont hook)
 * to call it again when the output is transferred to the user.
 *
 * The @this_cli variable points to a &cli structure of the session being
 * currently parsed, but it's of course available only in command handlers
 * not entered using the @cont hook.
 *
 * TX buffer management works as follows: At cli.tx_buf there is a
 * list of TX buffers (struct cli_out), cli.tx_write is the buffer
 * currently used by the producer (cli_printf(), cli_alloc_out()) and
 * cli.tx_pos is the buffer currently used by the consumer
 * (cli_write(), in system dependent code). The producer uses
 * cli_out.wpos ptr as the current write position and the consumer
 * uses cli_out.outpos ptr as the current read position. When the
 * producer produces something, it calls cli_write_trigger(). If there
 * is not enough space in the current buffer, the producer allocates
 * the new one. When the consumer processes everything in the buffer
 * queue, it calls cli_written(), tha frees all buffers (except the
 * first one) and schedules cli.event .
 * 
 */

#include "nest/bird.h"
#include "nest/cli.h"
#include "conf/conf.h"
#include "lib/string.h"

pool *cli_pool;

static byte *
cli_alloc_out(cli *c, int size)
{
  struct cli_out *o;

  if (!(o = c->tx_write) || o->wpos + size > o->end)
    {
      if (!o && c->tx_buf)
	o = c->tx_buf;
      else
	{
	  o = mb_alloc(c->pool, sizeof(struct cli_out) + CLI_TX_BUF_SIZE);
	  if (c->tx_write)
	    c->tx_write->next = o;
	  else
	    c->tx_buf = o;
	  o->wpos = o->outpos = o->buf;
	  o->end = o->buf + CLI_TX_BUF_SIZE;
	}
      c->tx_write = o;
      if (!c->tx_pos)
	c->tx_pos = o;
      o->next = NULL;
    }
  o->wpos += size;
  return o->wpos - size;
}

/**
 * cli_printf - send reply to a CLI connection
 * @c: CLI connection
 * @code: numeric code of the reply, negative for continuation lines
 * @msg: a printf()-like formatting string.
 *
 * This function send a single line of reply to a given CLI connection.
 * In works in all aspects like bsprintf() except that it automatically
 * prepends the reply line prefix.
 *
 * Please note that if the connection can be already busy sending some
 * data in which case cli_printf() stores the output to a temporary buffer,
 * so please avoid sending a large batch of replies without waiting
 * for the buffers to be flushed.
 *
 * If you want to write to the current CLI output, you can use the cli_msg()
 * macro instead.
 */
void
cli_printf(cli *c, int code, char *msg, ...)
{
  va_list args;
  byte buf[CLI_LINE_SIZE];
  int cd = code;
  int errcode;
  int size, cnt;

  if (cd < 0)
    {
      cd = -cd;
      if (cd == c->last_reply)
	size = bsprintf(buf, " ");
      else
	size = bsprintf(buf, "%04d-", cd);
      errcode = -8000;
    }
  else if (cd == CLI_ASYNC_CODE)
    {
      size = 1; buf[0] = '+'; 
      errcode = cd;
    }
  else
    {
      size = bsprintf(buf, "%04d ", cd);
      errcode = 8000;
    }

  c->last_reply = cd;
  va_start(args, msg);
  cnt = bvsnprintf(buf+size, sizeof(buf)-size-1, msg, args);
  va_end(args);
  if (cnt < 0)
    {
      cli_printf(c, errcode, "<line overflow>");
      return;
    }
  size += cnt;
  buf[size++] = '\n';
  memcpy(cli_alloc_out(c, size), buf, size);
}

static void
cli_copy_message(cli *c)
{
  byte *p, *q;
  unsigned int cnt = 2;

  if (c->ring_overflow)
    {
      byte buf[64];
      int n = bsprintf(buf, "<%d messages lost>\n", c->ring_overflow);
      c->ring_overflow = 0;
      memcpy(cli_alloc_out(c, n), buf, n);
    }
  p = c->ring_read;
  while (*p)
    {
      cnt++;
      p++;
      if (p == c->ring_end)
	p = c->ring_buf;
      ASSERT(p != c->ring_write);
    }
  c->async_msg_size += cnt;
  q = cli_alloc_out(c, cnt);
  *q++ = '+';
  p = c->ring_read;
  do
    {
      *q = *p++;
      if (p == c->ring_end)
	p = c->ring_buf;
    }
  while (*q++);
  c->ring_read = p;
  q[-1] = '\n';
}

static void
cli_hello(cli *c)
{
  cli_printf(c, 1, "BIRD " BIRD_VERSION " ready.");
  c->cont = NULL;
}

static void
cli_free_out(cli *c)
{
  struct cli_out *o, *p;

  if (o = c->tx_buf)
    {
      o->wpos = o->outpos = o->buf;
      while (p = o->next)
	{
	  o->next = p->next;
	  mb_free(p);
	}
    }
  c->tx_write = c->tx_pos = NULL;
  c->async_msg_size = 0;
}

void
cli_written(cli *c)
{
  cli_free_out(c);
  ev_schedule(c->event);
}


static byte *cli_rh_pos;
static unsigned int cli_rh_len;
static int cli_rh_trick_flag;
struct cli *this_cli;

static int
cli_cmd_read_hook(byte *buf, unsigned int max, UNUSED int fd)
{
  if (!cli_rh_trick_flag)
    {
      cli_rh_trick_flag = 1;
      buf[0] = '!';
      return 1;
    }
  if (max > cli_rh_len)
    max = cli_rh_len;
  memcpy(buf, cli_rh_pos, max);
  cli_rh_pos += max;
  cli_rh_len -= max;
  return max;
}

static void
cli_command(struct cli *c)
{
  struct config f;
  int res;

  if (config->cli_debug > 1)
    log(L_TRACE "CLI: %s", c->rx_buf);
  bzero(&f, sizeof(f));
  f.mem = c->parser_pool;
  cf_read_hook = cli_cmd_read_hook;
  cli_rh_pos = c->rx_buf;
  cli_rh_len = strlen(c->rx_buf);
  cli_rh_trick_flag = 0;
  this_cli = c;
  lp_flush(c->parser_pool);
  res = cli_parse(&f);
  if (!res)
    cli_printf(c, 9001, f.err_msg);
}

static void
cli_event(void *data)
{
  cli *c = data;
  int err;

  while (c->ring_read != c->ring_write &&
      c->async_msg_size < CLI_MAX_ASYNC_QUEUE)
    cli_copy_message(c);

  if (c->tx_pos)
    ;
  else if (c->cont)
    c->cont(c);
  else
    {
      err = cli_get_command(c);
      if (!err)
	return;
      if (err < 0)
	cli_printf(c, 9000, "Command too long");
      else
	cli_command(c);
    }

  cli_write_trigger(c);
}

cli *
cli_new(void *priv)
{
  pool *p = rp_new(cli_pool, "CLI");
  cli *c = mb_alloc(p, sizeof(cli));

  bzero(c, sizeof(cli));
  c->pool = p;
  c->priv = priv;
  c->event = ev_new(p);
  c->event->hook = cli_event;
  c->event->data = c;
  c->cont = cli_hello;
  c->parser_pool = lp_new(c->pool, 4096);
  c->rx_buf = mb_alloc(c->pool, CLI_RX_BUF_SIZE);
  ev_schedule(c->event);
  return c;
}

void
cli_kick(cli *c)
{
  if (!c->cont && !c->tx_pos)
    ev_schedule(c->event);
}

static list cli_log_hooks;
static int cli_log_inited;

void
cli_set_log_echo(cli *c, unsigned int mask, unsigned int size)
{
  if (c->ring_buf)
    {
      mb_free(c->ring_buf);
      c->ring_buf = c->ring_end = c->ring_read = c->ring_write = NULL;
      rem_node(&c->n);
    }
  c->log_mask = mask;
  if (mask && size)
    {
      c->ring_buf = mb_alloc(c->pool, size);
      c->ring_end = c->ring_buf + size;
      c->ring_read = c->ring_write = c->ring_buf;
      add_tail(&cli_log_hooks, &c->n);
      c->log_threshold = size / 8;
    }
  c->ring_overflow = 0;
}

void
cli_echo(unsigned int class, byte *msg)
{
  unsigned len, free, i, l;
  cli *c;
  byte *m;

  if (!cli_log_inited || EMPTY_LIST(cli_log_hooks))
    return;
  len = strlen(msg) + 1;
  WALK_LIST(c, cli_log_hooks)
    {
      if (!(c->log_mask & (1 << class)))
	continue;
      if (c->ring_read <= c->ring_write)
	free = (c->ring_end - c->ring_buf) - (c->ring_write - c->ring_read + 1);
      else
	free = c->ring_read - c->ring_write - 1;
      if ((len > free) ||
	  (free < c->log_threshold && class < (unsigned) L_INFO[0]))
	{
	  c->ring_overflow++;
	  continue;
	}
      if (c->ring_read == c->ring_write)
	ev_schedule(c->event);
      m = msg;
      l = len;
      while (l)
	{
	  if (c->ring_read <= c->ring_write)
	    i = c->ring_end - c->ring_write;
	  else
	    i = c->ring_read - c->ring_write;
	  if (i > l)
	    i = l;
	  memcpy(c->ring_write, m, i);
	  m += i;
	  l -= i;
	  c->ring_write += i;
	  if (c->ring_write == c->ring_end)
	    c->ring_write = c->ring_buf;
	}
    }
}

/* Hack for scheduled undo notification */
extern cli *cmd_reconfig_stored_cli;

void
cli_free(cli *c)
{
  cli_set_log_echo(c, 0, 0);
  if (c->cleanup)
    c->cleanup(c);
  if (c == cmd_reconfig_stored_cli)
    cmd_reconfig_stored_cli = NULL;
  rfree(c->pool);
}

/**
 * cli_init - initialize the CLI module
 *
 * This function is called during BIRD startup to initialize
 * the internal data structures of the CLI module.
 */
void
cli_init(void)
{
  cli_pool = rp_new(&root_pool, "CLI");
  init_list(&cli_log_hooks);
  cli_log_inited = 1;
}
