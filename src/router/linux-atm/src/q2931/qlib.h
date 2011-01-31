/* qlib.h - run-time library */
 
/* Written 1995-1997 by Werner Almesberger, EPFL-LRC */
 
 
#ifndef QLIB_H
#define QLIB_H

#ifdef DUMP_MODE
#define PREFIX(x) qd_##x
#else
#define PREFIX(x) q_##x
#endif


/*
 * Severity codes used by the run-time library. Surprisingly, they happen to
 * have the same numerical values as their corresponding atmsigd counterparts.
 */

#define Q_DEBUG		3
#define Q_ERROR		0
#define Q_FATAL		-1


#ifndef DUMP_MODE
extern int q_dump;
#endif
extern void q_report(int severity,const char *msg,...);

#ifdef DUMP_MODE
#ifndef STANDALONE
extern void qd_dump(const char *fmt,...);
#endif
extern void qd_report(int severity,const char *msg,...);
#endif


typedef enum {
    qet_catch_zero,
    qet_space,	/* length of message area exceeds length of surroundings */
    qet_case,	/* case value not found */
    qet_abort,	/* user abort */
    qet_init	/* fatal initialization error. DO NOT PROCEED ! */
} Q_ERR_TYPE;

typedef struct _q_err_dsc {
    Q_ERR_TYPE type; /* error type code */
    int pc; /* PC when error was discovered */
    int offset; /* offset into message when error was discovered */
    int value; /* additional value (optional) */
    int id; /* user-assigned id */
    int start; /* recovery area */
    int length;
    int group; /* group that failed (for fixups) */
    struct _q_err_dsc *next;
} Q_ERR_DSC;

typedef struct {
    unsigned char *data;
    unsigned char *required;
    unsigned char *field_present;
    unsigned long *group_present;
    int *length;
    int *field_map;
    int *group_map; /* @@@ useless overhead until we allow nested structures */
    void *buffer;
    int buf_size;
    int error;
    Q_ERR_DSC *errors;
} Q_DSC;
 

/*
 * *_START intializes global data structures and needs to be invoked before any
 * other function of qlib.
 */

void PREFIX(start)(void);

/*
 * *_OPEN opens an existing Q.2931 message (pointed to be BUF and of size
 * SIZE), parses its contents, and initializes the descriptor for further use
 * by Q_PRESENT, etc. *_OPEN returns zero on success, -1 if any problem has
 * been discovered with the message. In the latter case, the error list on
 * DSC.errors should be examined. Note that parts of the message may still be
 * valid and can be accessed with the usual functions. (Actually, because
 * invalid or unrecognized fields will simply be treated as absent, code that
 * tests for presence of mandatory elements does not need to examine the
 * error list. Not that, however, an error report of type QET_INIT carries
 * incomplete information and must not be examined beyond the TYPE element.
 * Also, _all_ calls to Q_PRESENT will return zero in this case.) The
 * descriptor must be closed with *_CLOSE even if *_OPEN fails.
 */

int PREFIX(open)(Q_DSC *dsc,void *buf,int size);

/*
 * Q_CREATE initializes the descriptor DSC points to. It also registers the
 * message area (to be used by Q_CLOSE) starting at BUF and of size SIZE in the
 * descriptor. Q_CREATE returns zero on success, -1 on failure. *_CLOSE does
 * not need to be called if Q_CREATE fails.
 */

int q_create(Q_DSC *dsc,void *buf,int size);

/*
 * Deallocates resources from the specified descriptor. If the descriptor was
 * initialized by Q_CREATE and if no error occurred, the message is composed
 * and *_CLOSE returns its size in bytes. In all other cases, *_CLOSE returns
 * zero on success, -1 on failure. Note that *_CLOSE also clears the error
 * list.
 */

int PREFIX(close)(Q_DSC *dsc);

/*
 * Q_ASSIGN sets the specified field to VALUE. Q_ASSIGN can only be used with
 * fixed-length fields of a size less than sizeof(unsigned long)*8 bits. Note
 * that the values of fields with associated groups can't be changed.
 */

void q_assign(Q_DSC *dsc,int field,unsigned long value);

/*
 * Like Q_ASSIGN, but for all fields (without associated groups) whose size is
 * a multiple of one byte.
 */

void q_write(Q_DSC *dsc,int field,const void *buf,int size);

/*
 * Checks the presence of the specified field (QF_*) or group (QG_*). Returns
 * one if the field or group is present, zero otherwise.
 */

int q_present(const Q_DSC *dsc,int field);

/*
 * Returns the value of the specified fields. Be warned that Q_FETCH does not
 * check if the fields is present or if its content fits in an unsigned long.
 */

unsigned long q_fetch(const Q_DSC *dsc,int field);

/*
 * Like Q_FETCH, but for byte-aligned fields of arbitrary size (as long as it's
 * a multiple of one byte). BUF points to a buffer and SIZE is its size. Q_READ
 * returns how many bytes were written to that buffer.
 */

int q_read(Q_DSC *dsc,int field,void *buf,int size);

/*
 * Returns the size (in bytes) of the specified variable-length field.
 */

int q_length(const Q_DSC *dsc,int field);

/*
 * Enables all instances which are controlled by the specified group.
 * It is an error to use a group that controls no instances.
 */

void q_instance(Q_DSC *dsc,int group);

#endif
