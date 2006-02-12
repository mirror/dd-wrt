#include <stdio.h>
#include "../include/ebtables_u.h"


static void print_help(char **hn)
{
	printf("Supported chain for the broute table:\n");
	printf("%s\n",hn[NF_BR_BROUTING]);
}

static struct
ebt_u_table table =
{
	.name		= "broute",
	.help		= print_help,
};

static void _init(void) __attribute__ ((constructor));
static void _init(void)
{
	register_table(&table);
}
