#include <string.h>
#include "include/ebtables_u.h"

static struct ebt_u_replace replace;
void ebt_early_init_once();

int pseudomain(int argc, char *argv[])
{
	ebt_silent = 0;
	ebt_early_init_once();
	strcpy(replace.name, "filter");
	do_command(argc, argv, EXEC_STYLE_PRG, &replace);
	return 0;
}

int main(int argc, char *argv[])
{
 	_802_3_init();
	_nat_init();
	_arp_init();
	_arpreply_init();
	_ip_init();
	_standard_init();
	_log_init();
	_redirect_init();
	_vlan_init();
	_mark_m_init();
	_mark_init();
	_pkttype_init();
	_stp_init();
	_among_init();
	_limit_init();
	_ulog_init();
	_t_filter_init();
	_t_nat_init();
	_t_broute_init();

	pseudomain(argc, argv);
	return 0;
}
