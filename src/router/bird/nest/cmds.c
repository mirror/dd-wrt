/*
 *	BIRD Internet Routing Daemon -- CLI Commands Which Don't Fit Anywhere Else
 *
 *	(c) 2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "nest/bird.h"
#include "nest/cli.h"
#include "conf/conf.h"
#include "nest/cmds.h"
#include "lib/string.h"

void
cmd_show_status(void)
{
  byte tim[TM_DATETIME_BUFFER_SIZE];

  cli_msg(-1000, "BIRD " BIRD_VERSION);
  tm_format_datetime(tim, now);
  cli_msg(-1011, "Router ID is %R", config->router_id);
  cli_msg(-1011, "Current server time is %s", tim);
  tm_format_datetime(tim, boot_time);
  cli_msg(-1011, "Last reboot on %s", tim);
  tm_format_datetime(tim, config->load_time);
  cli_msg(-1011, "Last reconfiguration on %s", tim);
  if (shutting_down)
    cli_msg(13, "Shutdown in progress");
  else if (old_config)
    cli_msg(13, "Reconfiguration in progress");
  else
    cli_msg(13, "Daemon is up and running");
}

void
cmd_show_symbols(struct symbol *sym)
{
  int pos = 0;

  if (sym)
    cli_msg(1010, "%s\t%s", sym->name, cf_symbol_class_name(sym));
  else
    {
      while (sym = cf_walk_symbols(config, sym, &pos))
	cli_msg(-1010, "%s\t%s", sym->name, cf_symbol_class_name(sym));
      cli_msg(0, "");
    }
}
