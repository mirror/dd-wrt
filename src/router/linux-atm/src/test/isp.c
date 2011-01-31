/* isp.c - Internal Signaling Protocol test generator */

/* Written 1997-2000 by Werner Almesberger, EPFL-ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#include <atm.h>
#include <atmd.h>
#include <linux/atmsvc.h>

#include "isp.h"


extern int yyparse(void);

int quiet = 0;
int verbose = 0;
VAR *variables = NULL;

static int sock;


void send_msg(const struct atmsvc_msg *msg)
{
    int wrote;

    wrote = write(sock,msg,sizeof(*msg));
    if (wrote == sizeof(*msg)) return;
    if (wrote < 0) perror("write");
    else fprintf(stderr,"bad write: %d != %d\n",wrote,sizeof(*msg));
    exit(1);
}


void recv_msg(struct atmsvc_msg *msg)
{
    int got;

    got = read(sock,msg,sizeof(*msg));
    if (got == sizeof(*msg)) return;
    if (got < 0) perror("read");
    else fprintf(stderr,"bad read: %d != %d\n",got,sizeof(*msg));
    exit(1);
}


static struct errno_table {
    const char *name;
    int value;
} table[] = {
    { "EPERM", EPERM },
    { "ENOENT", ENOENT },
    { "ESRCH", ESRCH },
    { "EINTR", EINTR },
    { "EIO", EIO },
    { "ENXIO", ENXIO },
    { "E2BIG", E2BIG },
    { "ENOEXEC", ENOEXEC },
    { "EBADF", EBADF },
    { "ECHILD", ECHILD },
    { "EAGAIN", EAGAIN },
    { "ENOMEM", ENOMEM },
    { "EACCES", EACCES },
    { "EFAULT", EFAULT },
    { "ENOTBLK", ENOTBLK },
    { "EBUSY", EBUSY },
    { "EEXIST", EEXIST },
    { "EXDEV", EXDEV },
    { "ENODEV", ENODEV },
    { "ENOTDIR", ENOTDIR },
    { "EISDIR", EISDIR },
    { "EINVAL", EINVAL },
    { "ENFILE", ENFILE },
    { "EMFILE", EMFILE },
    { "ENOTTY", ENOTTY },
    { "ETXTBSY", ETXTBSY },
    { "EFBIG", EFBIG },
    { "ENOSPC", ENOSPC },
    { "ESPIPE", ESPIPE },
    { "EROFS", EROFS },
    { "EMLINK", EMLINK },
    { "EPIPE", EPIPE },
    { "EDOM", EDOM },
    { "ERANGE", ERANGE },
    { NULL, 0 }
};


static const char *errno2str(int code)
{
    static char buf[30]; /* probably large enough :) */
    const struct errno_table *walk;

    for (walk = table; walk->name; walk++) {
	if (walk->value == code) return walk->name;
	if (walk->value == -code) {
	    sprintf(buf,"-%s",walk->name);
	    return buf;
	}
    }
    sprintf(buf,"%d (0x%x)",code,code);
    return buf;
}


/* Synchronized with include/linux/atmsvc.h:enum atmsvc_msg_type */

static struct {
    const char *name;
    int fields;
} types[] = {
    { "<invalid>",	0 },
    { "bind",		F_VCC | F_SVC | F_SAP },
    { "connect",	F_VCC | F_PVC | F_LOCAL | F_QOS | F_SVC | F_SAP },
    { "accept",		F_VCC | F_LISTEN_VCC },
    { "reject",		F_VCC | F_LISTEN_VCC | F_REPLY },
    { "listen",		F_VCC | F_QOS | F_SVC | F_SAP },
    { "okay",		F_VCC | F_PVC | F_LOCAL | F_QOS | F_SVC | F_SAP },
    { "error",		F_VCC | F_REPLY },
    { "indicate",	F_LISTEN_VCC | F_PVC | F_LOCAL | F_QOS | F_SVC | F_SAP},
    { "close",		F_VCC | F_REPLY },
    { "itf_notify",	F_PVC },
    { "modify",		F_VCC | F_REPLY | F_QOS },
    { "identify",	F_VCC | F_LISTEN_VCC | F_PVC },
    { "terminate",	0 }};

#define MSG_TYPES (sizeof(types)/sizeof(*types))


void print_value(VALUE val)
{
    char buf[1000]; /* bigger than any MAX_ATM_*_LEN */

    switch (val.type) {
	case vt_text:
	    printf("\"%s\"",val.u.text);
	    return;
	case vt_vcc:
	    printf("%s",kptr_print(&val.u.id));
	    return;
	case vt_error:
	    printf("%s",errno2str(val.u.num));
	    return;
	case vt_pvc:
	    if (atm2text(buf,sizeof(buf),(struct sockaddr *) &val.u.pvc,
	      A2T_PRETTY | A2T_NAME) < 0) strcpy(buf,"<invalid>");
	    printf("%s",buf);
	    return;
	case vt_svc:
	    if (atm2text(buf,sizeof(buf),(struct sockaddr *) &val.u.svc,
	      A2T_PRETTY | A2T_NAME) < 0) strcpy(buf,"<invalid>");
	    printf("%s",buf);
	    return;
	case vt_qos:
	    if (qos2text(buf,sizeof(buf),&val.u.qos,0) < 0)
		strcpy(buf,"<invalid>");
	    printf("%s",buf);
	    return;
	case vt_sap:
	    if (sap2text(buf,sizeof(buf),&val.u.sap,S2T_NAME) < 0)
		strcpy(buf,"<invalid>");
	    printf("%s",buf);
	    return;
	default:
	    fprintf(stderr,"\ninvalid value type %d\n",val.type);
	    exit(1);
    }
}


#define FIELD(FLD,MSG) \
  if (fields & FLD) { \
    printf("%s",MSG); \
    print_value(pick(msg,FLD)); \
    putchar('\n'); \
  }


void dump_msg(const char *prefix,const struct atmsvc_msg *msg)
{
    int fields;

    if (msg->type >= MSG_TYPES) {
	printf("%s: unknown message type %d\n",prefix,msg->type);
	return;
    }
    fields = types[msg->type].fields;
    printf("%s: %s\n",prefix,types[msg->type].name);
    FIELD(F_VCC,	"  vcc        = ");
    FIELD(F_LISTEN_VCC, "  listen_vcc = ");
    FIELD(F_REPLY,	"  reply      = ");
    FIELD(F_PVC,	"  pvc        = ");
    FIELD(F_LOCAL,	"  local      = ");
    FIELD(F_QOS,	"  qos        = ");
    FIELD(F_SVC,	"  svc        = ");
    FIELD(F_SAP,	"  sap        = ");
}


#undef FIELD


VAR *create_var(const char *name)
{
    VAR *var,**walk;

    var = malloc(sizeof(VAR));
    if (!var) {
	perror("malloc");
	exit(1);
    }
    var->name = name; /* strdup'ed */
    var->value.type = vt_none;
    for (walk = &variables; *walk; walk = &(*walk)->next)
	if (strcmp(name,(*walk)->name) > 0) break;
    var->next = *walk;
    *walk = var;
    return var;
}


VAR *lookup(const char *name)
{
    VAR *var;

    for (var = variables; var; var = var->next)
	if (!strcmp(var->name,name)) break;
    return var;
}


static void destroy(VALUE value)
{
    if (value.type == vt_text) free((char *) value.u.text);
}


void assign(VAR *var,VALUE value)
{
    destroy(var->value);
    var->value = value;
}


static int str2errno(const char *str)
{
    const struct errno_table *walk;

    for (walk = table; walk->name; walk++)
	if (!strcmp(walk->name,str)) break;
    return walk->value;
}


static VALUE convert(VALUE in,VALUE_TYPE type)
{
    VALUE out;
    char *end;

    if (in.type == type) {
	if (type == vt_text) {
	    in.u.text = strdup(in.u.text);
	    if (!in.u.text) {
		perror("strdup");
		exit(1);
	    }
	}
	return in;
    }
    if (in.type != vt_text) yyerror("type conflict");
    out.type = type;
    switch (type) {
	case vt_vcc:
	    memset(&out.u.id,0,sizeof(out.u.id));
	    *(unsigned long *) &out.u.id = strtoul(in.u.text,&end,0);
	    if (*end) yyerror("invalid number");
	    break;
	case vt_error:
	    out.u.num = strtoul(in.u.text,&end,0);
	    if (*end) {
		out.u.num = str2errno(*in.u.text == '-' ? in.u.text+1 :
		  in.u.text);
		if (!out.u.num) yyerror("invalid error code");
		if (*in.u.text == '-') out.u.num = -out.u.num;
	    }
	    break;
	case vt_svc:
	    if (text2atm(in.u.text,(struct sockaddr *) &out.u.svc,
	      sizeof(out.u.svc), T2A_SVC | T2A_NAME) < 0)
		yyerror("invalid SVC address");
	    break;
	case vt_pvc:
	    if (text2atm(in.u.text,(struct sockaddr *) &out.u.pvc,
	      sizeof(out.u.pvc),T2A_PVC | T2A_NNI | T2A_NAME) < 0)
		yyerror("invalid PVC address");
	    break;
	case vt_qos:
	    if (text2qos(in.u.text,&out.u.qos,0) < 0) yyerror("invalid QOS");
	    break;
	case vt_sap:
	    if (text2sap(in.u.text,&out.u.sap,T2S_NAME) < 0)
		yyerror("invalid SAP address");
	    break;
	default:
	    fprintf(stderr,"unexpected conversion type %d\n",type);
	    exit(1);
    }
    return out;
}


void check(VALUE a,VALUE b)
{
    if (a.type == vt_text) a = convert(a,b.type);
    if (b.type == vt_text) b = convert(b,a.type);
    if (a.type != b.type) yyerror("type conflict");
    switch (a.type) {
	case vt_vcc:
	case vt_error:
	    if (kptr_eq(&a.u.id,&b.u.id)) return;
	    break;
	case vt_svc:
	    if (atm_equal((struct sockaddr *) &a.u.svc,
	      (struct sockaddr *) &b.u.svc,0,0)) return;
	    break;
	case vt_pvc:
	    if (atm_equal((struct sockaddr *) &a.u.pvc,
	      (struct sockaddr *) &b.u.pvc,0,0)) return;
	    break;
	case vt_qos:
	    if (qos_equal(&a.u.qos,&b.u.qos)) return;
	    break;
	case vt_sap:
	    if (sap_equal(&a.u.sap,&b.u.sap,0)) return;
	    break;
	default:
	    fprintf(stderr,"unexpected conversion type %d\n",a.type);
	    exit(1);
    }
    printf("Expected ");
    print_value(b);
    printf(",\nbut message contains ");
    print_value(a);
    printf("\n");
    exit(1);
}


#define COPY_MSG_VAL(V) \
  switch (field) { \
    case F_VCC:		_COPY(V.u.id,msg->vcc); break; \
    case F_LISTEN_VCC:	_COPY(V.u.id,msg->listen_vcc); break; \
    case F_REPLY:	_COPY(V.u.num,msg->reply); break; \
    case F_PVC:		_COPY(V.u.pvc,msg->pvc); break; \
    case F_LOCAL:	_COPY(V.u.svc,msg->local); break; \
    case F_QOS:		_COPY(V.u.qos,msg->qos); break; \
    case F_SVC:		_COPY(V.u.svc,msg->svc); break; \
    case F_SAP:		_COPY(V.u.sap,msg->sap); break; \
    default:		fprintf(stderr,"unexpected field type 0x%x\n",field); \
			exit(1); \
  }



VALUE pick(const struct atmsvc_msg *msg,int field)
{
    VALUE out;

    if (msg->type >= MSG_TYPES) {
	fprintf(stderr,"invalid message type %d",msg->type);
	exit(1);
    }
    if (!(types[msg->type].fields & field)) yyerror("no such field in message");
    out.type = type_of(field);
#define _COPY(V,M) V = M
COPY_MSG_VAL(out)
#undef _COPY
    return out;
}


void store(struct atmsvc_msg *msg,int field,VALUE val)
{
    if (msg->type >= MSG_TYPES) {
	fprintf(stderr,"invalid message type %d",msg->type);
	exit(1);
    }
    if (!(types[msg->type].fields & field)) yyerror("no such field in message");
    if (val.type != type_of(field)) yyerror("type conflict");
#define _COPY(V,M) M = V
COPY_MSG_VAL(val)
#undef _COPY
}


#undef COPY_MSG_VAL


VALUE eval(VALUE_TYPE type,const char *str)
{
    VALUE a,b;

    a.type = vt_text;
    a.u.text = strdup(str);
    if (!a.u.text) {
	perror("strdup");
	exit(1);
    }
    b = convert(a,type);
    destroy(a);
    return b;
}


void cast(VAR *var,VALUE_TYPE type)
{
    VALUE old;

    if (var->value.type == type) return;
    old = var->value;
    var->value = convert(var->value,type);
    destroy(old);
}


VALUE_TYPE type_of(int field)
{
    switch (field) {
	case F_VCC:
	case F_LISTEN_VCC:
	    return vt_vcc;
	case F_REPLY:
	    return vt_error;
	case F_PVC:
	    return vt_pvc;
	case F_LOCAL:
	case F_SVC:
	    return vt_svc;
	case F_QOS:
	    return vt_qos;
	case F_SAP:
	    return vt_sap;
	default:
	    fprintf(stderr,"unexpected field type 0x%x\n",field);
	    exit(1);
    }
}


int main(int argc,char **argv)
{
    const char *name;

    name = *argv;
    if (argc > 2 && !strcmp(argv[1],"-q")) {
	quiet = 1;
	argc--;
	argv++;
    }
    if (argc > 2 && !strcmp(argv[1],"-v")) {
	verbose = 1;
	argc--;
	argv++;
    }
    if (argc != 2 || (quiet && verbose)) {
	fprintf(stderr,"usage: %s [-q | -v] socket\n",name);
	return 1;
    }
    sock = un_attach(argv[1]);
    if (sock < 0) {
	perror(argv[1]);
	return 1;
    }
    return yyparse();
}
