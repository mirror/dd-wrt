#ifndef NFTABLES_NFTABLES_H
#define NFTABLES_NFTABLES_H

#include <stdarg.h>
#include <limits.h>
#include <utils.h>
#include <cache.h>
#include <nftables/libnftables.h>

struct cookie {
	FILE *fp;
	FILE *orig_fp;
	char *buf;
	size_t buflen;
	size_t pos;
};

struct symbol_tables {
	const struct symbol_table	*mark;
	const struct symbol_table	*devgroup;
	const struct symbol_table	*ct_label;
	const struct symbol_table	*realm;
};

struct input_ctx {
	unsigned int flags;
};

static inline bool nft_input_no_dns(const struct input_ctx *ictx)
{
	return ictx->flags & NFT_CTX_INPUT_NO_DNS;
}

static inline bool nft_input_json(const struct input_ctx *ictx)
{
	return ictx->flags & NFT_CTX_INPUT_JSON;
}

struct output_ctx {
	unsigned int flags;
	bool force_newline;
	union {
		FILE *output_fp;
		struct cookie output_cookie;
	};
	union {
		FILE *error_fp;
		struct cookie error_cookie;
	};
	struct symbol_tables tbl;
};

static inline bool nft_output_reversedns(const struct output_ctx *octx)
{
	return octx->flags & NFT_CTX_OUTPUT_REVERSEDNS;
}

static inline bool nft_output_service(const struct output_ctx *octx)
{
	return octx->flags & NFT_CTX_OUTPUT_SERVICE;
}

static inline bool nft_output_stateless(const struct output_ctx *octx)
{
	return octx->flags & NFT_CTX_OUTPUT_STATELESS;
}

static inline bool nft_output_handle(const struct output_ctx *octx)
{
	return octx->flags & NFT_CTX_OUTPUT_HANDLE;
}

static inline bool nft_output_json(const struct output_ctx *octx)
{
	return octx->flags & NFT_CTX_OUTPUT_JSON;
}

static inline bool nft_output_echo(const struct output_ctx *octx)
{
	return octx->flags & NFT_CTX_OUTPUT_ECHO;
}

static inline bool nft_output_guid(const struct output_ctx *octx)
{
	return octx->flags & NFT_CTX_OUTPUT_GUID;
}

static inline bool nft_output_seconds(const struct output_ctx *octx)
{
	return octx->flags & NFT_CTX_OUTPUT_NUMERIC_TIME;
}

static inline bool nft_output_numeric_proto(const struct output_ctx *octx)
{
	return octx->flags & NFT_CTX_OUTPUT_NUMERIC_PROTO;
}

static inline bool nft_output_numeric_prio(const struct output_ctx *octx)
{
	return octx->flags & NFT_CTX_OUTPUT_NUMERIC_PRIO;
}

static inline bool nft_output_numeric_symbol(const struct output_ctx *octx)
{
	return octx->flags & NFT_CTX_OUTPUT_NUMERIC_SYMBOL;
}

static inline bool nft_output_terse(const struct output_ctx *octx)
{
	return octx->flags & NFT_CTX_OUTPUT_TERSE;
}

struct mnl_socket;
struct parser_state;
struct scope;

struct nft_vars {
	const char	*key;
	const char	*value;
};

#define MAX_INCLUDE_DEPTH	16

struct nft_ctx {
	struct mnl_socket	*nf_sock;
	char			**include_paths;
	unsigned int		num_include_paths;
	struct nft_vars		*vars;
	struct {
		const char	*buf;
		struct list_head indesc_list;
	} vars_ctx;
	unsigned int		num_vars;
	unsigned int		parser_max_errors;
	unsigned int		debug_mask;
	struct input_ctx	input;
	struct output_ctx	output;
	bool			check;
	struct nft_cache	cache;
	uint32_t		flags;
	uint32_t		optimize_flags;
	struct parser_state	*state;
	void			*scanner;
	struct scope		*top_scope;
	void			*json_root;
	json_t			*json_echo;
	const char		*stdin_buf;
#if HAVE_FUZZER_BUILD
	int			afl_ctx_stage;
#endif
};

enum nftables_exit_codes {
	NFT_EXIT_SUCCESS	= 0,
	NFT_EXIT_FAILURE	= 1,
	NFT_EXIT_NOMEM		= 2,
	NFT_EXIT_NONL		= 3,
};

struct input_descriptor;
struct location {
	const struct input_descriptor		*indesc;
	union {
		struct {
			unsigned int 		line_offset;
			unsigned int		first_line;
			unsigned int		first_column;
			unsigned int		last_column;
		};
		struct {
			const void		*nle;
		};
	};
};

extern const struct location internal_location;

/**
 * enum input_descriptor_types
 *
 * @INDESC_INVALID:	invalid
 * @INDESC_INTERNAL:	dummy type for internally generated messages
 * @INDESC_BUFFER:	buffer (command line arguments)
 * @INDESC_FILE:	file
 * @INDESC_CLI:		command line interface
 * @INDESC_NETLINK:	received from netlink
 */
enum input_descriptor_types {
	INDESC_INVALID,
	INDESC_INTERNAL,
	INDESC_BUFFER,
	INDESC_FILE,
	INDESC_CLI,
	INDESC_NETLINK,
	INDESC_STDIN,
};

/**
 * struct input_descriptor
 *
 * @location:		location, used for include statements
 * @f:			file descriptor
 * @depth:		include depth of the descriptor
 * @type:		input descriptor type
 * @name:		name describing the input
 * @union:		buffer or file descriptor, depending on type
 * @lineno:		current line number in the input
 * @column:		current column in the input
 * @token_offset:	offset of the current token to the beginning
 * @line_offset:	offset of the current line to the beginning
 */
struct input_descriptor {
	struct list_head		list;
	FILE				*f;
	unsigned int			depth;
	struct location			location;
	enum input_descriptor_types	type;
	const char			*name;
	const char			*data;
	unsigned int			lineno;
	unsigned int			column;
	off_t				token_offset;
	off_t				line_offset;
};

void ct_label_table_init(struct nft_ctx *ctx);
void mark_table_init(struct nft_ctx *ctx);
void realm_table_rt_init(struct nft_ctx *ctx);
void devgroup_table_init(struct nft_ctx *ctx);
void xt_init(void);

void ct_label_table_exit(struct nft_ctx *ctx);
void mark_table_exit(struct nft_ctx *ctx);
void devgroup_table_exit(struct nft_ctx *ctx);
void realm_table_rt_exit(struct nft_ctx *ctx);

int nft_print(struct output_ctx *octx, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));
int nft_gmp_print(struct output_ctx *octx, const char *fmt, ...);

int nft_optimize(struct nft_ctx *nft, struct list_head *cmds);

#define __NFT_OUTPUT_NOTSUPP	UINT_MAX

/* internal marker, not used by the kernel. */
#define NFT_NEVER_TIMEOUT	UINT64_MAX

#endif /* NFTABLES_NFTABLES_H */
