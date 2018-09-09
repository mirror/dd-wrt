#ifndef __CVMX_OTRACE_H__
#define __CVMX_OTRACE_H__

/*
 * Octeon Trace logs are recorded in a ring buffer as TVs by the traced cores
 * and consumed by the master. This buffer is allocated/freed on demand.
 * The first int records the head and the 2nd the tail.
 */
#define OCTEON_TRACE_LOG_BUF_LEN	1024
#define OCTEON_TRACE_LOG_NBLOCK(block_name, core)			\
	do {								\
		sprintf((block_name), "otl_queue%d", (core));		\
	} while (0)
#define OCTEON_TRACE_LOG_NEXT(pos)					\
	(((pos) + 1) % OCTEON_TRACE_LOG_BUF_LEN) 

typedef enum {	/* log entry type */
	OTL_ENT_INVALID,
	OTL_ENT_BUF_FULL,
	OTL_ENT_FUNC_ENTER,
	OTL_ENT_FUNC_EXIT,
	OTL_ENT_VAR,
	OTL_ENT_CSR,
	OTL_ENT_COP,
	OTL_ENT_USER,	/* content is user defined */
	OTL_ENT_MAX,
} otrace_log_type_t;

typedef struct {	/* log entry */
	otrace_log_type_t	otl_ent_type;
	union {
		struct {
			uint64_t	ts;
			union {
				struct {
					uint64_t thing1;
					uint64_t thing2;
				} s;
				struct {
					uint64_t this_fn;
					uint64_t call_site;
				} s_fn;
				struct {
					uint64_t ts0;
					uint64_t count;
				} s_full;
				struct {
					uint64_t addr;
					uint64_t value;
				} s_var;
				struct {
					uint32_t reg;
					uint32_t sel;
					uint64_t value;
				} s_cop0;
			} u;
		} s_bltin; 
		uint8_t usrdata[24];
	} otl_ent_u;
#define otl_ent_ts		otl_ent_u.s_bltin.ts
#define otl_ent_thing1		otl_ent_u.s_bltin.u.s.thing1
#define otl_ent_thing2		otl_ent_u.s_bltin.u.s.thing2
#define otl_ent_this_fn		otl_ent_u.s_bltin.u.s_fn.this_fn
#define otl_ent_call_site	otl_ent_u.s_bltin.u.s_fn.call_site
#define otl_ent_ts0		otl_ent_u.s_bltin.u.s_full.ts0
#define otl_ent_count		otl_ent_u.s_bltin.u.s_full.count
#define otl_ent_addr		otl_ent_u.s_bltin.u.s_var.addr
#define otl_ent_var		otl_ent_u.s_bltin.u.s_var.value
#define otl_ent_csr		otl_ent_u.s_bltin.u.s_var.value
#define otl_ent_reg		otl_ent_u.s_bltin.u.s_cop0.reg
#define otl_ent_sel		otl_ent_u.s_bltin.u.s_cop0.sel
#define otl_ent_cop0		otl_ent_u.s_bltin.u.s_cop0.value
#define otl_ent_usrdata		otl_ent_u.usrdata
} otrace_log_entry_t;

typedef struct {	/* per core trace log buffer */
	int			otl_head;
	int			otl_tail;
	otrace_log_entry_t	otl_buf[OCTEON_TRACE_LOG_BUF_LEN];
} otrace_log_que_t;

/*
 * Octeon Trace commands are sent from the master to the stub cores via the
 * command buffer. The command allows a response from the receiving core.
 */
#define OCTEON_TRACE_CMD_BUF_NAME	"otc_bufs"
#define OCTEON_TRACE_CMD_MAX_ARGS	10
#define OCTEON_TRACE_CMD_FUNCALL_MAX_ARGC	(OCTEON_TRACE_CMD_MAX_ARGS - 3)
#define OCTEON_TRACE_CMD_MAX_RVALS	1	/* return values */

typedef enum {	/* commands from the master */
	OTC_CMD_INVALID,
	OTC_FUN_CALL,
	OTC_CMD_MAX,
} otrace_cmd_t;

typedef enum {	/* from the stub in response to a command */
	OTR_SUCCESS,
	OTR_FAIL,
} otrace_ret_t;

typedef enum {	/* errno */
	OTE_INVALID,	/* not used as an errno. */
	OTE_NO_OTLQUE,	/* couldn't find the trace log queue */
	OTE_UNKNOWN_CMD,/* unknown cmd */
	OTE_UNSUPPORTED,/* not supported */
	OTE_MAX,
} otrace_errno_t;

typedef struct {	/* per-core cmd buffer */
	otrace_cmd_t		otc_cmd;
	uint64_t		otc_arg[OCTEON_TRACE_CMD_MAX_ARGS];
	otrace_ret_t		otc_ret;
	uint64_t		otc_rval[OCTEON_TRACE_CMD_MAX_RVALS];
	otrace_log_que_t	*otc_otl;
} otrace_cmd_buf_t;

#endif
