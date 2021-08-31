/**
 * @file common.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief common internal definitions for libyang
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_COMMON_H_
#define LY_COMMON_H_

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "context.h"
#include "hash_table.h"
#include "log.h"
#include "set.h"
#include "tree_data.h"

struct ly_ctx;
struct ly_in;
struct lysc_node;

#if __STDC_VERSION__ >= 201112 && !defined __STDC_NO_THREADS__
# define THREAD_LOCAL _Thread_local
#elif defined __GNUC__ || \
    defined __SUNPRO_C || \
    defined __xlC__
# define THREAD_LOCAL __thread
#else
# error "Cannot define THREAD_LOCAL"
#endif

#define GETMACRO1(_1, NAME, ...) NAME
#define GETMACRO2(_1, _2, NAME, ...) NAME
#define GETMACRO3(_1, _2, _3, NAME, ...) NAME
#define GETMACRO4(_1, _2, _3, _4, NAME, ...) NAME
#define GETMACRO5(_1, _2, _3, _4, _5, NAME, ...) NAME
#define GETMACRO6(_1, _2, _3, _4, _5, _6, NAME, ...) NAME

/*
 * If the compiler supports attribute to mark objects as hidden, mark all
 * objects as hidden and export only objects explicitly marked to be part of
 * the public API.
 */
#define API __attribute__((visibility("default")))

/******************************************************************************
 * Logger
 *****************************************************************************/

extern volatile LY_LOG_LEVEL ly_ll;
extern volatile uint32_t ly_log_opts;

struct ly_log_location_s {
    uint64_t line;                   /**< One-time line value being reset after use - replaces whatever is in inputs */
    struct ly_set inputs;            /**< Set of const struct ly_in *in pointers providing the input handler with the line information (LIFO) */
    struct ly_set scnodes;           /**< Set of const struct lysc_node *scnode pointers providing the compiled schema node to generate path (LIFO) */
    struct ly_set dnodes;            /**< Set of const struct lyd_node *dnode pointers providing the data node to generate path (LIFO) */
    struct ly_set paths;             /**< Set of path strings (LIFO) */
};

/**
 * @brief Print a log message and store it into the context (if provided).
 *
 * @param[in] ctx libyang context to store the error record. If not provided, the error is just printed.
 * @param[in] level Log message level (error, warning, etc.)
 * @param[in] no Error type code.
 * @param[in] format Format string to print.
 */
void ly_log(const struct ly_ctx *ctx, LY_LOG_LEVEL level, LY_ERR no, const char *format, ...);

/**
 * @brief Print Validation error and store it into the context (if provided).
 *
 * @param[in] ctx libyang context to store the error record. If not provided, the error is just printed.
 * @param[in] code Validation error code.
 * @param[in] format Format string to print.
 */
void ly_vlog(const struct ly_ctx *ctx, LY_VECODE code, const char *format, ...);

/**
 * @brief Logger's location data setter.
 *
 * @param[in] scnode Compiled schema node.
 * @param[in] dnode Data node.
 * @param[in] path Direct path string to print.
 * @param[in] in Input handler (providing line number)
 * @param[in] line One-time line value to be reset when used.
 * @param[in] reset Flag to indicate if the not set arguments (NULLs) are intended to rewrite the current values or if they
 * are supposed to be ignored and the previous values should be kept.
 */
void ly_log_location(const struct lysc_node *scnode, const struct lyd_node *dnode,
        const char *path, const struct ly_in *in, uint64_t line, ly_bool reset);

/**
 * @brief Revert the specific logger's location data by number of changes made by ::ly_log_location().
 *
 * @param[in] scnode_steps Number of items in ::ly_log_location_s.scnodes to forget.
 * @param[in] dnode_steps Number of items in ::ly_log_location_s.dnodes to forget.
 * @param[in] path_steps Number of path strings in ::ly_log_location_s.paths to forget.
 * @param[in] in_steps Number of input handlers ::ly_log_location_s.inputs to forget.
 */
void ly_log_location_revert(uint32_t scnode_steps, uint32_t dnode_steps, uint32_t path_steps, uint32_t in_steps);

/**
 * @brief Initiate location data for logger, all arguments are set as provided (even NULLs) - overrides the current values.
 *
 * @param[in] SCNODE Compiled schema node.
 * @param[in] DNODE Data node.
 * @param[in] PATH Direct path string to print.
 * @param[in] IN Input handler (providing line number)
 */
#define LOG_LOCINIT(SCNODE, DNODE, PATH, IN) \
    ly_log_location(SCNODE, DNODE, PATH, IN, 0, 1)

/**
 * @brief Update location data for logger, not provided arguments (NULLs) are kept (does not override).
 *
 * @param[in] SCNODE Compiled schema node.
 * @param[in] DNODE Data node.
 * @param[in] PATH Direct path string to print.
 * @param[in] IN Input handler (providing line number)
 */
#define LOG_LOCSET(SCNODE, DNODE, PATH, IN) \
    ly_log_location(SCNODE, DNODE, PATH, IN, 0, 0)

/**
 * @brief Update location data for logger, not provided arguments (NULLs) are kept (does not override).
 *
 * @param[in] SCNODE_STEPS Number of the compiled schema nodes to remove from the stack.
 * @param[in] DNODE_STEPS Number of the data nodes to remove from the stack.
 * @param[in] PATH_STEPS Number of the direct path strings to remove from the stack.
 * @param[in] IN_STEPS Number of the input handlers (providing line number) to remove from the stack.
 */
#define LOG_LOCBACK(SCNODE_STEPS, DNODE_STEPS, PATH_STEPS, IN_STEPS) \
    ly_log_location_revert(SCNODE_STEPS, DNODE_STEPS, PATH_STEPS, IN_STEPS)

#define LOGERR(ctx, errno, str, ...) ly_log(ctx, LY_LLERR, errno, str, ##__VA_ARGS__)
#define LOGWRN(ctx, str, ...) ly_log(ctx, LY_LLWRN, 0, str, ##__VA_ARGS__)
#define LOGVRB(str, ...) ly_log(NULL, LY_LLVRB, 0, str, ##__VA_ARGS__)

#ifdef NDEBUG
#  define LOGDBG(dbg_group, str, ...)
#else
void ly_log_dbg(uint32_t group, const char *format, ...);
#  define LOGDBG(dbg_group, str, ...) ly_log_dbg(dbg_group, str, ##__VA_ARGS__);
#endif

/**
 * Simple EMEM message, it can be safely stored in ::ly_err_item structures without problems when freeing.
 */
#define LY_EMEM_MSG "Memory allocation failed."

#ifdef LOGMEM
/* overwrite shadow definition from tree_edit.h */
#undef LOGMEM
#endif
#define LOGMEM(CTX) LOGERR(CTX, LY_EMEM, "Memory allocation failed (%s()).", __func__)

#define LOGINT(CTX) LOGERR(CTX, LY_EINT, "Internal error (%s:%d).", __FILE__, __LINE__)
#define LOGARG(CTX, ARG) LOGERR(CTX, LY_EINVAL, "Invalid argument %s (%s()).", #ARG, __func__)
#define LOGVAL(CTX, CODE, ...) ly_vlog(CTX, CODE, ##__VA_ARGS__)
#define LOGVAL_LINE(CTX, LINE, CODE, ...) \
    ly_log_location(NULL, NULL, NULL, NULL, LINE, 0); \
    ly_vlog(CTX, CODE, ##__VA_ARGS__)

/**
 * @brief Print Validation error from struct ly_err_item.
 *
 * String ::ly_err_item.msg cannot be used directly because it may contain the % character,
 * which is incorrectly interpreted in this situation as a conversion specification.
 *
 * @param[in] CTX libyang context to store the error record. If not provided, the error is just printed.
 * @param[in] ERRITEM pointer to ly_err_item that contains an error message.
 */
#define LOGVAL_ERRITEM(CTX, ERRITEM) ly_vlog(CTX, ERRITEM->vecode, "%s", ERRITEM->msg)

#define LOGMEM_RET(CTX) LOGMEM(CTX); return LY_EMEM
#define LOGINT_RET(CTX) LOGINT(CTX); return LY_EINT
#define LOGARG_RET(CTX) LOGARG(CTX); return LY_EINVAL

/*
 * Common code to check return value and perform appropriate action.
 */
#define LY_CHECK_GOTO(COND, GOTO) if ((COND)) {goto GOTO;}
#define LY_CHECK_ERR_GOTO(COND, ERR, GOTO) if ((COND)) {ERR; goto GOTO;}
#define LY_CHECK_RET1(RETVAL) {LY_ERR ret__ = RETVAL;if (ret__ != LY_SUCCESS) {return ret__;}}
#define LY_CHECK_RET2(COND, RETVAL) if ((COND)) {return RETVAL;}
#define LY_CHECK_RET(...) GETMACRO2(__VA_ARGS__, LY_CHECK_RET2, LY_CHECK_RET1)(__VA_ARGS__)
#define LY_CHECK_ERR_RET(COND, ERR, RETVAL) if ((COND)) {ERR; return RETVAL;}

#define LY_CHECK_ARG_GOTO1(CTX, ARG, GOTO) if (!(ARG)) {LOGARG(CTX, ARG);goto GOTO;}
#define LY_CHECK_ARG_GOTO2(CTX, ARG1, ARG2, GOTO) LY_CHECK_ARG_GOTO1(CTX, ARG1, GOTO);LY_CHECK_ARG_GOTO1(CTX, ARG2, GOTO)
#define LY_CHECK_ARG_GOTO3(CTX, ARG1, ARG2, ARG3, GOTO) LY_CHECK_ARG_GOTO2(CTX, ARG1, ARG2, GOTO);LY_CHECK_ARG_GOTO1(CTX, ARG3, GOTO)
#define LY_CHECK_ARG_GOTO4(CTX, ARG1, ARG2, ARG3, ARG4, GOTO) LY_CHECK_ARG_GOTO3(CTX, ARG1, ARG2, ARG3, GOTO);\
    LY_CHECK_ARG_GOTO1(CTX, ARG4, GOTO)
#define LY_CHECK_ARG_GOTO(CTX, ...) GETMACRO5(__VA_ARGS__, LY_CHECK_ARG_GOTO4, LY_CHECK_ARG_GOTO3, LY_CHECK_ARG_GOTO2, \
    LY_CHECK_ARG_GOTO1)(CTX, __VA_ARGS__)

#define LY_CHECK_ARG_RET1(CTX, ARG, RETVAL) if (!(ARG)) {LOGARG(CTX, ARG);return RETVAL;}
#define LY_CHECK_ARG_RET2(CTX, ARG1, ARG2, RETVAL) LY_CHECK_ARG_RET1(CTX, ARG1, RETVAL);LY_CHECK_ARG_RET1(CTX, ARG2, RETVAL)
#define LY_CHECK_ARG_RET3(CTX, ARG1, ARG2, ARG3, RETVAL) LY_CHECK_ARG_RET2(CTX, ARG1, ARG2, RETVAL);LY_CHECK_ARG_RET1(CTX, ARG3, RETVAL)
#define LY_CHECK_ARG_RET4(CTX, ARG1, ARG2, ARG3, ARG4, RETVAL) LY_CHECK_ARG_RET3(CTX, ARG1, ARG2, ARG3, RETVAL);\
    LY_CHECK_ARG_RET1(CTX, ARG4, RETVAL)
#define LY_CHECK_ARG_RET5(CTX, ARG1, ARG2, ARG3, ARG4, ARG5, RETVAL) LY_CHECK_ARG_RET4(CTX, ARG1, ARG2, ARG3, ARG4, RETVAL);\
    LY_CHECK_ARG_RET1(CTX, ARG5, RETVAL)
#define LY_CHECK_ARG_RET(CTX, ...) GETMACRO6(__VA_ARGS__, LY_CHECK_ARG_RET5, LY_CHECK_ARG_RET4, LY_CHECK_ARG_RET3, \
    LY_CHECK_ARG_RET2, LY_CHECK_ARG_RET1) (CTX, __VA_ARGS__)

/* count sequence size for LY_VCODE_INCHILDSTMT validation error code */
size_t LY_VCODE_INSTREXP_len(const char *str);
/* default maximum characters to print in LY_VCODE_INCHILDSTMT */
#define LY_VCODE_INSTREXP_MAXLEN 20

#define LY_VCODE_INCHAR         LYVE_SYNTAX, "Invalid character 0x%x."
#define LY_VCODE_INSTREXP       LYVE_SYNTAX, "Invalid character sequence \"%.*s\", expected %s."
#define LY_VCODE_EOF            LYVE_SYNTAX, "Unexpected end-of-input."
#define LY_VCODE_NTERM          LYVE_SYNTAX, "%s not terminated."
#define LY_VCODE_NSUPP          LYVE_SYNTAX, "%s not supported."
#define LY_VCODE_MOD_SUBOMD     LYVE_SYNTAX, "Invalid keyword \"%s\", expected \"module\" or \"submodule\"."
#define LY_VCODE_TRAILING_MOD   LYVE_SYNTAX, "Trailing garbage \"%.*s%s\" after module, expected end-of-input."
#define LY_VCODE_TRAILING_SUBMOD LYVE_SYNTAX, "Trailing garbage \"%.*s%s\" after submodule, expected end-of-input."

#define LY_VCODE_INVAL_MINMAX   LYVE_SEMANTICS, "Invalid combination of min-elements and max-elements: min value %u is bigger than the max value %u."
#define LY_VCODE_NAME_COL       LYVE_SEMANTICS, "Name collision between %s of name \"%s\"."
#define LY_VCODE_NAME2_COL      LYVE_SEMANTICS, "Name collision between %s and %s of name \"%s\"."

#define LY_VCODE_INSTMT         LYVE_SYNTAX_YANG, "Invalid keyword \"%s\"."
#define LY_VCODE_INCHILDSTMT    LYVE_SYNTAX_YANG, "Invalid keyword \"%s\" as a child of \"%s\"."
#define LY_VCODE_INCHILDSTMT2   LYVE_SYNTAX_YANG, "Invalid keyword \"%s\" as a child of \"%s\" - the statement is allowed only in YANG 1.1 modules."
#define LY_VCODE_INCHILDSTMSCOMB LYVE_SYNTAX_YANG, "Invalid combination of keywords \"%s\" and \"%s\" as substatements of \"%s\"."
#define LY_VCODE_DUPSTMT        LYVE_SYNTAX_YANG, "Duplicate keyword \"%s\"."
#define LY_VCODE_DUPIDENT       LYVE_SYNTAX_YANG, "Duplicate identifier \"%s\" of %s statement."
#define LY_VCODE_INVAL          LYVE_SYNTAX_YANG, "Invalid value \"%.*s\" of \"%s\"."
#define LY_VCODE_MISSTMT        LYVE_SYNTAX_YANG, "Missing mandatory keyword \"%s\" as a child of \"%s\"."
#define LY_VCODE_MISSCHILDSTMT  LYVE_SYNTAX_YANG, "Missing %s substatement for %s%s."
#define LY_VCODE_INORD          LYVE_SYNTAX_YANG, "Invalid keyword \"%s\", it cannot appear after \"%s\"."
#define LY_VCODE_OOB            LYVE_SYNTAX_YANG, "Value \"%.*s\" is out of \"%s\" bounds."
#define LY_VCODE_INDEV          LYVE_SYNTAX_YANG, "Deviate \"%s\" does not support keyword \"%s\"."
#define LY_VCODE_INREGEXP       LYVE_SYNTAX_YANG, "Regular expression \"%s\" is not valid (\"%s\": %s)."

#define LY_VCODE_INSUBELEM2     LYVE_SYNTAX_YIN, "Invalid sub-elemnt \"%s\" of \"%s\" element - this sub-element is allowed only in modules with version 1.1 or newer."
#define LY_VCODE_INVAL_YIN      LYVE_SYNTAX_YIN, "Invalid value \"%s\" of \"%s\" attribute in \"%s\" element."
#define LY_VCODE_UNEXP_SUBELEM  LYVE_SYNTAX_YIN, "Unexpected sub-element \"%.*s\" of \"%s\" element."
#define LY_VCODE_INDEV_YIN      LYVE_SYNTAX_YIN, "Deviate of this type doesn't allow \"%s\" as it's sub-element."
#define LY_VCODE_INORDER_YIN    LYVE_SYNTAX_YIN, "Invalid order of %s\'s sub-elements \"%s\" can't appear after \"%s\"."
#define LY_VCODE_OOB_YIN        LYVE_SYNTAX_YIN, "Value \"%s\" of \"%s\" attribute in \"%s\" element is out of bounds."
#define LY_VCODE_INCHILDSTMSCOMB_YIN LYVE_SYNTAX_YIN, "Invalid combination of sub-elemnts \"%s\" and \"%s\" in \"%s\" element."
#define LY_VCODE_DUP_ATTR       LYVE_SYNTAX_YIN, "Duplicit definition of \"%s\" attribute in \"%s\" element."
#define LY_VCODE_UNEXP_ATTR     LYVE_SYNTAX_YIN, "Unexpected attribute \"%.*s\" of \"%s\" element."
#define LY_VCODE_MAND_SUBELEM   LYVE_SYNTAX_YIN, "Missing mandatory sub-element \"%s\" of \"%s\" element."
#define LY_VCODE_FIRT_SUBELEM   LYVE_SYNTAX_YIN, "Sub-element \"%s\" of \"%s\" element must be defined as it's first sub-element."
#define LY_VCODE_SUBELEM_REDEF  LYVE_SYNTAX_YIN, "Redefinition of \"%s\" sub-element in \"%s\" element."

#define LY_VCODE_XP_EOE         LYVE_XPATH, "Unterminated string delimited with %c (%.15s)."
#define LY_VCODE_XP_INEXPR      LYVE_XPATH, "Invalid character '%c'[%u] of expression \'%s\'."
#define LY_VCODE_XP_EOF         LYVE_XPATH, "Unexpected XPath expression end."
#define LY_VCODE_XP_INTOK       LYVE_XPATH, "Unexpected XPath token \"%s\" (\"%.15s\")."
#define LY_VCODE_XP_INTOK2      LYVE_XPATH, "Unexpected XPath token \"%s\" (\"%.15s\"), expected \"%s\"."
#define LY_VCODE_XP_INFUNC      LYVE_XPATH, "Unknown XPath function \"%.*s\"."
#define LY_VCODE_XP_INARGCOUNT  LYVE_XPATH, "Invalid number of arguments (%d) for the XPath function %.*s."
#define LY_VCODE_XP_INARGTYPE   LYVE_XPATH, "Wrong type of argument #%d (%s) for the XPath function %s."
#define LY_VCODE_XP_INCTX       LYVE_XPATH, "Invalid context type %s in %s."
#define LY_VCODE_XP_INOP_1      LYVE_XPATH, "Cannot apply XPath operation %s on %s."
#define LY_VCODE_XP_INOP_2      LYVE_XPATH, "Cannot apply XPath operation %s on %s and %s."
#define LY_VCODE_XP_INMOD       LYVE_XPATH, "Unknown/non-implemented module \"%.*s\"."

#define LY_VCODE_DEV_NOT_PRESENT LYVE_REFERENCE, "Invalid deviation %s \"%s\" property \"%s\" which is not present."

#define LY_VCODE_NOWHEN         LYVE_DATA, "When condition \"%s\" not satisfied."
#define LY_VCODE_NOMAND         LYVE_DATA, "Mandatory node \"%s\" instance does not exist."
#define LY_VCODE_DUP            LYVE_DATA, "Duplicate instance of \"%s\"."
#define LY_VCODE_DUPCASE        LYVE_DATA, "Data for both cases \"%s\" and \"%s\" exist."
#define LY_VCODE_UNEXPNODE      LYVE_DATA, "Unexpected data %s node \"%s\" found."
#define LY_VCODE_NOKEY          LYVE_DATA, "List instance is missing its key \"%s\"."

#define LY_ERRMSG_NOPATTERN /* LYVE_DATA */ "Unsatisfied pattern - \"%.*s\" does not conform to %s\"%s\"."
#define LY_ERRMSG_NOLENGTH /* LYVE_DATA */ "Unsatisfied length - string \"%.*s\" length is not allowed."
#define LY_ERRMSG_NORANGE /* LYVE_DATA */ "Unsatisfied range - value \"%.*s\" is out of the allowed range."

/* RFC 7950 section 15 errors */
#define LY_VCODE_NOUNIQ         LYVE_DATA, "Unique data leaf(s) \"%s\" not satisfied in \"%s\" and \"%s\"."
#define LY_VCODE_NOMAX          LYVE_DATA, "Too many \"%s\" instances."
#define LY_VCODE_NOMIN          LYVE_DATA, "Too few \"%s\" instances."
#define LY_VCODE_NOMUST         LYVE_DATA, "Must condition \"%s\" not satisfied."
#define LY_VCODE_NOMAND_CHOIC   LYVE_DATA, "Mandatory choice \"%s\" data do not exist."

/* RFC 7950 section 15 error messages used in type plugin validation callbacks */
#define LY_ERRMSG_NOLREF_VAL /* LYVE_DATA */ "Invalid leafref value \"%s\" - no target instance \"%s\" with the same value."
#define LY_ERRMSG_NOLREF_INST /* LYVE_DATA */ "Invalid leafref value \"%s\" - no existing target instance \"%s\"."
#define LY_ERRMSG_NOINST /* LYVE_DATA */ "Invalid instance-identifier \"%s\" value - required instance not found."

/******************************************************************************
 * Context
 *****************************************************************************/

/**
 * @brief Context of the YANG schemas
 */
struct ly_ctx {
    struct dict_table dict;           /**< dictionary to effectively store strings used in the context related structures */
    struct ly_set search_paths;       /**< set of directories where to search for schema's imports/includes */
    struct ly_set list;               /**< set of loaded YANG schemas */
    ly_module_imp_clb imp_clb;        /**< Optional callback for retrieving missing included or imported models in a custom way. */
    void *imp_clb_data;               /**< Optional private data for ::ly_ctx.imp_clb */
    uint16_t change_count;            /**< Count of changes of the context, on some changes it could be incremented more times */
    uint16_t flags;                   /**< context settings, see @ref contextoptions. */
    pthread_key_t errlist_key;        /**< key for the thread-specific list of errors related to the context */
    pthread_mutex_t lyb_hash_lock;    /**< lock for storing LYB schema hashes in schema nodes */
};

/**
 * @brief Get the (only) implemented YANG module specified by its name.
 *
 * @param[in] ctx Context where to search.
 * @param[in] name Name of the YANG module to get.
 * @param[in] name_len Optional length of the @p name. If zero, NULL-terminated name is expected.
 * @return The only implemented YANG module revision of the given name in the given context. NULL if there is no
 * implemented module of the given name.
 */
struct lys_module *ly_ctx_get_module_implemented2(const struct ly_ctx *ctx, const char *name, size_t name_len);

/******************************************************************************
 * Generic useful functions.
 *****************************************************************************/

/**
 * @brief Insert string into dictionary.
 *
 * @param[in] CTX libyang context.
 * @param[in] STRING string to store.
 * @param[in] LEN length of the string in WORD to store.
 * @param[in,out] DYNAMIC Set to 1 if @p STRING is dynamically allocated, 0 otherwise.
 * If set to 1, zerocopy version of lydict_insert is used.
 * @param[out] TARGET pointer is set to @p STRING value stored in the dictionary.
 */
#define INSERT_STRING_RET(CTX, STRING, LEN, DYNAMIC, TARGET) \
    if (DYNAMIC) { \
        LY_CHECK_RET(lydict_insert_zc(CTX, (char *)(STRING), &(TARGET))); \
    } else { \
        LY_CHECK_RET(lydict_insert(CTX, LEN ? (STRING) : "", LEN, &(TARGET))); \
    } \
    DYNAMIC = 0

/**
 * @brief Wrapper for realloc() call. The only difference is that if it fails to
 * allocate the requested memory, the original memory is freed as well.
 *
 * @param[in] ptr Memory to reallocate.
 * @param[in] size New size of the memory block.
 *
 * @return Pointer to the new memory, NULL on error.
 */
void *ly_realloc(void *ptr, size_t size);

/**
 * @brief Just like strchr() function except limit the number of examined characters.
 *
 * @param[in] s String to search in.
 * @param[in] c Character to search for.
 * @param[in] len Limit the search to this number of characters in @p s.
 * @return Pointer to first @p c occurence in @p s, NULL if not found in first @p len characters.
 */
char *ly_strnchr(const char *s, int c, size_t len);

/**
 * @brief Compare NULL-terminated @p refstr with @p str_len bytes from @p str.
 *
 * @param[in] refstr NULL-terminated string which must match @p str_len bytes from @p str followed by NULL-byte.
 * @param[in] str String to compare.
 * @param[in] str_len Number of bytes to take into comparison from @p str.
 * @return An integer less than, equal to, or greater than zero if @p refstr matches,
 * respectively, to be less than, to match, or be greater than @p str.
 */
int ly_strncmp(const char *refstr, const char *str, size_t str_len);

/**
 * @brief Similar functionality to strtoul() except number length in the string
 * must be specified and the whole number must be parsed for success.
 *
 * @param[in] nptr Number string.
 * @param[in] len Number string length starting at @p nptr.
 * @param[out] ret Parsed number.
 * @return LY_EDENIED on overflow.
 * @return LY_EVALID on encountering a non-digit character.
 * @return LY_SUCCESS on success.
 */
LY_ERR ly_strntou8(const char *nptr, size_t len, uint8_t *ret);

/**
 * @brief Similar to strlen(3) but accepts NULL and returns 0.
 *
 * @param[in] s String to examine.
 * @return Length of @p s.
 */
static inline size_t
ly_strlen(const char *s)
{
    return s ? strlen(s) : 0;
}

/**
 * @brief Get all possible value prefixes from an YANG value by iteratively returning specific substrings.
 *
 * The function looks for possible prefix ending in a colon at the beginning of @p str_begin.
 * If @p str_begin does not contain the prefix at the beginning, then either:
 * 1. Returns the entire input string if the input string does not contain the prefix at all.
 * 2. Returns a substring before the prefix. The substring is terminated by any character
 *    that is not allowed to be present in prefix (except colon).
 *
 * Examples of inputs and outputs are shown in the table below.
 * Output string @p str_next is used in the next iteration as input parameter @p str_begin.
   @verbatim
   |            INPUT              |                         OUTPUT                        |
   |                               |   iteration 1    |   iteration 2    |   iteration 3   |
   |------------------------------ |------------------|------------------|-----------------|
   | /namespace_prefix:some_string | /                | namespace_prefix | some_string     |
   | namespace_prefix:some_string  | namespace_prefix | some_string      | NULL            |
   | /some_string                  | /some_string     | NULL             | NULL            |
   @endverbatim
 *
 *
 * @param[in] str_begin Begin of the input string.
 * @param[in] str_end Length of the @p str_begin. If set to NULL then the @p str_begin must be NULL-terminated string.
 * @param[out] len Number of bytes (length) of the found prefix/substring starting at @p str_begin.
 * @param[out] is_prefix Type of substring found. Set to True for prefix, otherwise False.
 * @param[out] str_next Remaining string starting after prefix/substring and ending with @p str_end.
 * If the @p is_prefix is set to True then the colon character is skipped.
 * If no string remains, it is set to NULL.
 * @return LY_ERR value.
 */
LY_ERR ly_value_prefix_next(const char *str_begin, const char *str_end, uint32_t *len, ly_bool *is_prefix, const char **str_next);

/**
 * @brief Wrapper around strlen() to handle NULL strings.
 */
#define ly_strlen(STR) (STR ? strlen(STR) : 0)

/**
 * @brief Compile-time strlen() for string contants.
 *
 * Use to avoid magic numbers usage
 */
#define ly_strlen_const(STR) (sizeof STR - 1)

#define ly_sizeofarray(ARRAY) (sizeof ARRAY / sizeof *ARRAY)

/*
 * Numerical bases for use in functions like strtoll() instead of magic numbers
 */
#define LY_BASE_DEC 10  /**< Decimal numeral base */
#define LY_BASE_OCT 8   /**< Octal numeral base */
#define LY_BASE_HEX 16  /**< Hexadecimal numeral base */

/**
 * Maximal length of (needed storage for) a number encoded as a string.
 *
 * Applies not only for standard numbers, but also for YANG's decimal64.
 */
#define LY_NUMBER_MAXLEN 22

/**
 * @brief Get UTF8 code point of the next character in the input string.
 *
 * @param[in,out] input Input string to process, updated according to the processed/read data.
 * @param[out] utf8_char UTF8 code point of the next character.
 * @param[out] bytes_read Number of bytes used to encode the read utf8_char.
 * @return LY_ERR value
 */
LY_ERR ly_getutf8(const char **input, uint32_t *utf8_char, size_t *bytes_read);

/**
 * Store UTF-8 character specified as 4byte integer into the dst buffer.
 *
 * UTF-8 mapping:
 * 00000000 -- 0000007F:    0xxxxxxx
 * 00000080 -- 000007FF:    110xxxxx 10xxxxxx
 * 00000800 -- 0000FFFF:    1110xxxx 10xxxxxx 10xxxxxx
 * 00010000 -- 001FFFFF:    11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 * Includes checking for valid characters (following RFC 7950, sec 9.4)
 *
 * @param[in, out] dst Destination buffer to store the UTF-8 character, must provide enough space (up to 4 bytes) for storing the UTF-8 character.
 * @param[in] value 32b value of the UTF-8 character to store.
 * @param[out] bytes_written Number of bytes written into @p dst (size of the written UTF-8 character).
 * @return LY_SUCCESS on success
 * @return LY_EINVAL in case of invalid UTF-8 @p value to store.
 */
LY_ERR ly_pututf8(char *dst, uint32_t value, size_t *bytes_written);

/**
 * @brief Get number of characters in the @p str, taking multibyte characters into account.
 * @param[in] str String to examine.
 * @param[in] bytes Number of valid bytes that are supposed to be taken into account in @p str.
 * This parameter is useful mainly for non NULL-terminated strings. In case of NULL-terminated
 * string, strlen() can be used.
 * @return Number of characters in (possibly) multibyte characters string.
 */
size_t ly_utf8len(const char *str, size_t bytes);

/**
 * @brief Parse signed integer with possible limitation.
 * @param[in] val_str String value containing signed integer, note that
 * nothing else than whitespaces are expected after the value itself.
 * @param[in] val_len Length of the @p val_str string.
 * @param[in] min Limitation for the value which must not be lower than min.
 * @param[in] max Limitation for the value which must not be higher than max.
 * @param[in] base Numeric base for parsing:
 *        0 - to accept decimal, octal, hexadecimal (e.g. in default value)
 *       10 - to accept only decimal (e.g. data instance value)
 * @param[out] ret Resulting value.
 * @return LY_ERR value:
 * LY_EDENIED - the value breaks the limits,
 * LY_EVALID - string contains invalid value,
 * LY_SUCCESS - successful parsing.
 */
LY_ERR ly_parse_int(const char *val_str, size_t val_len, int64_t min, int64_t max, int base, int64_t *ret);

/**
 * @brief Parse unsigned integer with possible limitation.
 * @param[in] val_str String value containing unsigned integer, note that
 * nothing else than whitespaces are expected after the value itself.
 * @param[in] val_len Length of the @p val_str string.
 * @param[in] max Limitation for the value which must not be higher than max.
 * @param[in] base Numeric base for parsing:
 *        0 - to accept decimal, octal, hexadecimal (e.g. in default value)
 *       10 - to accept only decimal (e.g. data instance value)
 * @param[out] ret Resulting value.
 * @return LY_ERR value:
 * LY_EDENIED - the value breaks the limits,
 * LY_EVALID - string contains invalid value,
 * LY_SUCCESS - successful parsing.
 */
LY_ERR ly_parse_uint(const char *val_str, size_t val_len, uint64_t max, int base, uint64_t *ret);

/**
 * @brief Parse a node-identifier.
 *
 * node-identifier     = [prefix ":"] identifier
 *
 * @param[in, out] id Identifier to parse. When returned, it points to the first character which is not part of the identifier.
 * @param[out] prefix Node's prefix, NULL if there is not any.
 * @param[out] prefix_len Length of the node's prefix, 0 if there is not any.
 * @param[out] name Node's name.
 * @param[out] name_len Length of the node's name.
 * @return LY_ERR value: LY_SUCCESS or LY_EINVAL in case of invalid character in the id.
 */
LY_ERR ly_parse_nodeid(const char **id, const char **prefix, size_t *prefix_len, const char **name, size_t *name_len);

/**
 * @brief parse instance-identifier's predicate, supports key-predicate, leaf-list-predicate and pos rules from YANG ABNF Grammar.
 *
 * @param[in, out] pred Predicate string (including the leading '[') to parse. The string is updated according to what was parsed
 * (even for error case, so it can be used to determine which substring caused failure).
 * @param[in] limit Limiting length of the @p pred. Function expects NULL terminated string which is not overread.
 * The limit value is not checked with each character, so it can be overread and the failure is detected later.
 * @param[in] format Input format of the data containing the @p pred.
 * @param[out] prefix Start of the node-identifier's prefix if any, NULL in case of pos or leaf-list-predicate rules.
 * @param[out] prefix_len Length of the parsed @p prefix.
 * @param[out] id Start of the node-identifier's identifier string, NULL in case of pos rule, "." in case of leaf-list-predicate rule.
 * @param[out] id_len Length of the parsed @p id.
 * @param[out] value Start of the quoted-string (without quotation marks), not NULL in case of success.
 * @param[out] value_len Length of the parsed @p value.
 * @param[out] errmsg Error message string in case of error.
 * @return LY_SUCCESS in case a complete predicate was parsed.
 * @return LY_EVALID in case of invalid predicate form.
 * @return LY_EINVAL in case of reaching @p limit when parsing @p pred.
 */
LY_ERR ly_parse_instance_predicate(const char **pred, size_t limit, LYD_FORMAT format,
        const char **prefix, size_t *prefix_len, const char **id, size_t *id_len,
        const char **value, size_t *value_len, const char **errmsg);

/**
 * @brief mmap(2) wrapper to map input files into memory to unify parsing.
 *
 * The address space is allocate only for reading.
 *
 * @param[in] ctx libyang context for logging
 * @param[in] fd Open file descriptor of a file to map.
 * @param[out] length Allocated size.
 * @param[out] addr Address where the file is mapped.
 * @return LY_ERR value.
 */
LY_ERR ly_mmap(struct ly_ctx *ctx, int fd, size_t *length, void **addr);

/**
 * @brief munmap(2) wrapper to free the memory mapped by ::ly_mmap()
 *
 * @param[in] addr Address where the input file is mapped.
 * @param[in] length Allocated size of the address space.
 * @return LY_ERR value.
 */
LY_ERR ly_munmap(void *addr, size_t length);

/**
 * @brief Concatenate formating string to the @p dest.
 *
 * @param[in, out] dest String to be concatenated by @p format.
 *                 Note that the input string can be reallocated during concatenation.
 * @param[in] format Formating string (as for printf) which is supposed to be added after @p dest.
 * @return LY_SUCCESS or LY_EMEM.
 */
LY_ERR ly_strcat(char **dest, const char *format, ...);

#endif /* LY_COMMON_H_ */
