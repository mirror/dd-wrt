#include "compute_outgoing_tuple.h"

#include "mod/common/log.h"
#include "mod/common/rfc6052.h"
#include "mod/common/db/bib/db.h"

/**
 * Ensures @state->entries->bib is computed and valid.
 * (Assuming @state really maps to a databased session, that is.)
 */
static verdict find_bib(struct xlation *state)
{
	int error;

	if (state->entries.bib_set)
		return VERDICT_CONTINUE;

	error = bib_find(state->jool.nat64.bib, &state->in.tuple,
			&state->entries);
	if (error) {
		/*
		 * Bogus ICMP errors might cause this because Filtering skips
		 * them, so it's not critical.
		 */
		log_debug(state, "Session not found. Error code is %d.", error);
		return untranslatable(state, JSTAT_SESSION_NOT_FOUND);
	}

	return VERDICT_CONTINUE;
}

static int xlat_addr64(struct xlation *state, struct ipv4_transport_addr *addr4)
{
	/* The RFC labels this as "(D', d)". */
	struct ipv6_transport_addr *d = &state->in.tuple.dst.addr6;

	addr4->l4 = d->l4;
	return __rfc6052_6to4(&state->jool.globals.pool6.prefix,
			&d->l3, &addr4->l3);
}

static int xlat_addr46(struct xlation *state, struct ipv6_transport_addr *addr6)
{
	/* The RFC labels this as (S, s). */
	struct ipv4_transport_addr *s = &state->in.tuple.src.addr4;

	addr6->l4 = s->l4;
	return __rfc6052_4to6(&state->jool.globals.pool6.prefix,
			&s->l3, &addr6->l3);
}

verdict compute_out_tuple(struct xlation *state)
{
	struct tuple *in;
	struct tuple *out;
	verdict result;

	log_debug(state, "Step 3: Computing the Outgoing Tuple");

	result = find_bib(state);
	if (result != VERDICT_CONTINUE)
		return result;

	in = &state->in.tuple;
	out = &state->out.tuple;

	switch (in->l3_proto) {
	case L3PROTO_IPV6:
		out->l3_proto = L3PROTO_IPV4;
		out->l4_proto = in->l4_proto;
		out->src.addr4 = state->entries.session.src4;
		if (xlat_addr64(state, &out->dst.addr4))
			return untranslatable(state, JSTAT_UNTRANSLATABLE_DST6);

		if (is_3_tuple(out))
			out->dst.addr4.l4 = out->src.addr4.l4;
		break;

	case L3PROTO_IPV4:
		out->l3_proto = L3PROTO_IPV6;
		out->l4_proto = in->l4_proto;
		if (xlat_addr46(state, &out->src.addr6))
			return untranslatable(state, JSTAT_UNTRANSLATABLE_DST4);
		out->dst.addr6 = state->entries.session.src6;

		if (is_3_tuple(out))
			out->src.addr6.l4 = out->dst.addr6.l4;
		break;
	}

	log_tuple(state, out);
	log_debug(state, "Done step 3.");
	return VERDICT_CONTINUE;
}
