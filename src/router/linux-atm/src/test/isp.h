/* isp.h - Internal Signaling Protocol test generator */

/* Written 1997-2000 by Werner Almesberger, EPFL-ICA */


#ifndef ISP_H
#define ISP_H

#include <atm.h>
#include <linux/atmsvc.h>


/* Field type values */

#define F_VCC		0x00000001
#define F_LISTEN_VCC	0x00000002
#define F_REPLY		0x00000004
#define F_PVC		0x00000008
#define F_LOCAL		0x00000010
#define F_QOS		0x00000020
#define F_SVC		0x00000040
#define F_SAP		0x00000080


typedef enum { vt_none,vt_text,vt_vcc,vt_error,vt_svc,vt_pvc,vt_qos,vt_sap }
  VALUE_TYPE;

typedef struct {
    VALUE_TYPE type;
    union {
	const char *text;
	atm_kptr_t id;
	int num;
	struct sockaddr_atmsvc svc;
	struct sockaddr_atmpvc pvc;
	struct atm_qos qos;
	struct atm_sap sap;
    } u;
} VALUE;

typedef struct _var {
    const char *name;
    VALUE value;
    struct _var *next;
} VAR;


extern int quiet,verbose;
extern VAR *variables;


void yyerror(const char *s);

void print_value(VALUE val);
VAR *create_var(const char *name);
VAR *lookup(const char *name);
void assign(VAR *var,VALUE value);
void check(VALUE a,VALUE b);
VALUE pick(const struct atmsvc_msg *msg,int field);
void store(struct atmsvc_msg *msg,int field,VALUE val);
VALUE eval(VALUE_TYPE type,const char *str);
void cast(VAR *var,VALUE_TYPE type);
VALUE_TYPE type_of(int field);

void send_msg(const struct atmsvc_msg *msg);
void recv_msg(struct atmsvc_msg *msg);
void dump_msg(const char *prefix,const struct atmsvc_msg *msg);

#endif
