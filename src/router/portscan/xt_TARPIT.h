#pragma once

enum xt_tarpit_target_variant {
	XTTARPIT_TARPIT,
	XTTARPIT_HONEYPOT,
	XTTARPIT_RESET,
};

struct xt_tarpit_tginfo {
	uint8_t variant;
};
