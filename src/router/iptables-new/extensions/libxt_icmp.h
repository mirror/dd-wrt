struct xt_icmp_names {
	const char *name;
	uint8_t type;
	uint8_t code_min, code_max;
};

static void xt_print_icmp_types(const struct xt_icmp_names *icmp_codes,
				unsigned int n_codes)
{
	unsigned int i;

	for (i = 0; i < n_codes; ++i) {
		if (i && icmp_codes[i].type == icmp_codes[i-1].type) {
			if (icmp_codes[i].code_min == icmp_codes[i-1].code_min
			    && (icmp_codes[i].code_max
				== icmp_codes[i-1].code_max))
				printf(" (%s)", icmp_codes[i].name);
			else
				printf("\n   %s", icmp_codes[i].name);
		}
		else
			printf("\n%s", icmp_codes[i].name);
	}
	printf("\n");
}
