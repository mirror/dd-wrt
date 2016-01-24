#define VSTR_COMPILE_INCLUDE 1
#include <vstr.h>

int main(int argc, char *argv[])
{
	Vstr_base *s1 = NULL;
	unsigned int scan = 1;
	unsigned int ns_tot = 0;

	if (!vstr_init()) abort();

	if (!(s1 = vstr_make_base(NULL))) abort();

	ns_tot = vstr_add_netstr_beg(s1, s1->len);
	while (scan < (unsigned)argc)
	{
		unsigned int ns = vstr_add_netstr_beg(s1, s1->len);
		vstr_add_cstr_ptr(s1, s1->len, argv[scan]);
		vstr_add_netstr_end(s1, ns, s1->len);

		++scan;
	}
	vstr_add_netstr_end(s1, ns_tot, s1->len);
	vstr_add_cstr_ptr(s1, s1->len, "\n");


	if (s1->conf->malloc_bad)
		exit (EXIT_FAILURE);

	while (s1->len && vstr_sc_write_fd(s1, 1, s1->len, 1, NULL))
	{ /* nothing */ }

	vstr_exit();

	exit (EXIT_SUCCESS);
}

