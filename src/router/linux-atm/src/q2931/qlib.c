/* qlib.c - run-time library */
 
/* Written 1995-1997 by Werner Almesberger, EPFL-LRC */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef DUMP_MODE
static int q_dump = 0;
#else
int q_dump = 0;
#endif

#ifndef STANDALONE
#define DUMP qd_dump
#else
#define DUMP printf

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "qlib.h"
#include "op.h"


int debug = 0;


void PREFIX(report)(int severity,const char *msg,...)
{
    va_list ap;

    if (!debug && severity > Q_ERROR) return;
    va_start(ap,msg);
    vprintf(msg,ap);
    printf("\n");
    va_end(ap);
    if (severity == Q_FATAL) exit(1);
}

#endif


#define LENGTH_STACK 10
#define LENGTH_R_STACK 5


typedef struct {
    int pos,size;
    unsigned char *start;
} LEN_BUF;

typedef struct _rstack {
    int *pc;
    int sp;
    unsigned char *pos;
    unsigned char *end;
    struct _rstack *next;
} RSTACK;


static int q_test(unsigned char *table,int pos)
{
    return !!(table[pos >> 3] & (1 << (pos & 7)));
}


static void q_set(unsigned char *table,int pos)
{
    table[pos >> 3] |= 1 << (pos & 7);
}


static void q_clear(unsigned char *table,int pos)
{
    table[pos >> 3] &= ~(1 << (pos & 7));
}


/* slightly ugly */


static void q_put(unsigned char *table,int pos,int size,unsigned long value)
{
    int end;
 
    PREFIX(report)(Q_DEBUG,"put %d %d %ld",pos,size,value);
    end = pos+size;
    if (((pos | size) & 7) && ((pos ^ (end-1)) & ~7))
	PREFIX(report)(Q_FATAL,"unsupported alignment (put %d,%d)",pos,size);
    if (size <= 8) {
	unsigned char *here;
	int shift;

	here = &table[pos >> 3];
	shift = pos & 7;
	*here = (*here & ~(((1 << size)-1) << shift)) | value << shift;
    }
    else {
	table = table+end/8-1;
	while (size > 0) {
	    *table-- = value;
	    value >>= 8;
	    size -= 8;
        }
    }
}


static unsigned long q_get(unsigned char *table,int pos,int size)
{
    unsigned long value;
    int end;
 
    PREFIX(report)(Q_DEBUG,"get %d %d ...",pos,size);
    end = pos+size;
    if (((pos | size) & 7) && ((pos ^ (end-1)) & ~7))
	PREFIX(report)(Q_FATAL,"unsupported alignment (get %d,%d)",pos,size);
    if (size <= 8) value = (table[pos >> 3] >> (pos & 7)) & ((1 << size)-1);
    else {
	table += pos >> 3;
	value = 0;
	while (size > 0) {
	    value = (value << 8) | *table++;
	    size -= 8;
        }
    }
    PREFIX(report)(Q_DEBUG,"  %ld",value);
    return value;
}


static void q_copy(unsigned char *src,int pos,unsigned char *dst,int size)
{
    src += pos >> 3;
    pos &= 7;
    if (pos+size <= 8) *dst |= *src & (((1 << size)-1) << pos);
    else {
	if (pos) {
	    *dst++ |= *src++ & (0xff << pos);
	    size -= 8-pos;
	}
	while (size >= 8) {
	    *dst++ = *src++;
	    size -= 8;
	}
	if (size > 0) *dst |= *src & ((1 << size)-1);
    }
}


void PREFIX(start)(void)
{
    q_init_global();
}


static int q_init(Q_DSC *dsc)
{
    size_t bytes;
    int i;

    /* initialize verything in case anything goes wrong during allocations. */
    dsc->errors = NULL;
    dsc->field_present = NULL;
    dsc->group_present = NULL;
    dsc->data = NULL;
    dsc->required = NULL;
    dsc->length = NULL;
    dsc->field_map = NULL;
    dsc->group_map = NULL;
    dsc->data = malloc((size_t) Q_DATA_BYTES);
    dsc->error = 1;
    if (!dsc->data) {
	perror("out of memory");
	return -1;
    }
    memcpy(dsc->data,q_initial,Q_DATA_BYTES);
    bytes = (Q_FIELDS+7) >> 3;
    dsc->required = malloc(bytes);
    if (!dsc->required) {
	perror("out of memory");
	return -1;
    }
    memset(dsc->required,0,bytes);
    dsc->field_present = malloc(bytes);
    if (!dsc->field_present) {
	perror("out of memory");
	return -1;
    }
    memset(dsc->field_present,0,bytes);
    bytes = (Q_GROUPS+(sizeof(unsigned long)*8-1)) >> 3;
    dsc->group_present = malloc(bytes);
    if (!dsc->group_present) {
	perror("out of memory");
	return -1;
    }
    memset(dsc->group_present,0,bytes);
    if (!Q_VARLEN_FIELDS) dsc->length = NULL;
    else {
	dsc->length = malloc(sizeof(int)*Q_VARLEN_FIELDS);
	if (!dsc->length) {
	    perror("out of memory");
	    return -1;
	}
	memset(dsc->length,0,sizeof(int)*Q_VARLEN_FIELDS);
    }
    dsc->field_map = malloc(sizeof(int)*Q_FIELDS);
    if (!dsc->field_map) {
	perror("out of memory");
	return -1;
    }
    for (i = 0; i < Q_FIELDS; i++) dsc->field_map[i] = i;
    dsc->group_map = malloc(sizeof(int)*Q_GROUPS);
    if (!dsc->group_map) {
	perror("out of memory");
	return -1;
    }
    for (i = 0; i < Q_GROUPS; i++) dsc->group_map[i] = i;
    dsc->error = 0;
    return 0;
}


#ifndef DUMP_MODE

static void use_group(Q_DSC *dsc,int group)
{
    int *scan;

    while (group != -1) {
	q_set((unsigned char *) dsc->group_present,group);
	for (scan = groups[group].required; scan && *scan != -1; scan++)
	    q_set(dsc->required,*scan);
	group = groups[group].parent;
    }
}


void q_assign(Q_DSC *dsc,int field,unsigned long value)
{
    int *walk;

    if (field < 0 || field >= Q_FIELDS)
	PREFIX(report)(Q_FATAL,"invalid field value (%d)",field);
    field = dsc->field_map[field];
    if (!fields[field].values) {
	if (q_test(dsc->field_present,field)) /* probably an error ... */
	    PREFIX(report)(Q_ERROR,"changing field %d",field);
	q_set(dsc->field_present,field);
	q_put(dsc->data,fields[field].pos,fields[field].size,value);
	use_group(dsc,fields[field].parent);
    }
    else {
	if (q_test(dsc->field_present,field))
	    PREFIX(report)(Q_FATAL,"can't change field %d",field);
	q_set(dsc->field_present,field);
	q_put(dsc->data,fields[field].pos,fields[field].size,value);
	for (walk = fields[field].values; walk[1] != -1; walk += 2)
	    if (*walk == value || *walk == -2) {
		use_group(dsc,walk[1]);
		return;
	    }
	PREFIX(report)(Q_ERROR,"invalid value (%d in field %d)",value,field);
	dsc->error = 1;
    }
}


void q_write(Q_DSC *dsc,int field,const void *buf,int size)
{
    if (field < 0 || field >= Q_FIELDS)
	PREFIX(report)(Q_FATAL,"invalid field value (%d)",field);
    field = dsc->field_map[field];
    if (fields[field].pos & 7)
	PREFIX(report)(Q_FATAL,"invalid use of q_write (%d)",field);
    if (fields[field].actual >= 0) {
	if (size > fields[field].size/8) {
	    PREFIX(report)(Q_ERROR,"%d bytes too big for %d byte field %d",
	      size,fields[field].size/8,field);
	    dsc->error = 1;
	    return;
	}
	dsc->length[fields[field].actual] = size;
    }
    else if ((fields[field].pos | fields[field].size) & 7)
	    PREFIX(report)(Q_FATAL,"field %d is neither var-len nor "
	      "well-shaped",field);
    memcpy(dsc->data+(fields[field].pos/8),buf,(size_t) size);
    q_set(dsc->field_present,field);
    use_group(dsc,fields[field].parent);
}


int q_present(const Q_DSC *dsc,int field)
{
    if (field < 0) {
	if (field < -Q_GROUPS)
	    PREFIX(report)(Q_FATAL,"invalid group number (%d)",field);
	if (!dsc->group_present) return 0;
	field = dsc->group_map[-field-1];
	return q_test((unsigned char *) dsc->group_present,field);
    }
    else {
	if (field >= Q_FIELDS)
	    PREFIX(report)(Q_FATAL,"invalid field number (%d)",field);
	if (!dsc->field_present) return 0;
	field = dsc->field_map[field];
	if (q_test(dsc->field_present,field)) return 1;
	return q_test((unsigned char *) dsc->group_present,
	  fields[field].parent);
    }
}


unsigned long q_fetch(const Q_DSC *dsc,int field)
{
    if (field < 0 || field >= Q_FIELDS)
	PREFIX(report)(Q_FATAL,"invalid field value (%d)",field);
    field = dsc->field_map[field];
    return q_get(dsc->data,fields[field].pos,fields[field].size);
}


int q_length(const Q_DSC *dsc,int field)
{
    if (field < 0 || field >= Q_FIELDS)
	PREFIX(report)(Q_FATAL,"invalid field value (%d)",field);
    field = dsc->field_map[field];
    if (fields[field].pos & 7)
	PREFIX(report)(Q_FATAL,"invalid use of q_length (%d)",field);
    if (fields[field].actual < 0)
	PREFIX(report)(Q_FATAL,"field %d is not var-len",field);
    return dsc->length[fields[field].actual];
}


int q_read(Q_DSC *dsc,int field,void *buf,int size)
{
    int len;

    len = 0; /* for gcc */
    if (field < 0 || field >= Q_FIELDS)
	PREFIX(report)(Q_FATAL,"invalid field value (%d)",field);
    field = dsc->field_map[field];
    if (fields[field].pos & 7)
	PREFIX(report)(Q_FATAL,"invalid use of q_read (%d)",field);
    if (fields[field].actual >= 0) len = dsc->length[fields[field].actual];
    else if (!(fields[field].size & 7)) len = fields[field].size >> 3;
	else PREFIX(report)(Q_FATAL,"field %d is not byte-sized (%d bits)",
	      field,fields[field].size);
    if (size < len) {
	PREFIX(report)(Q_ERROR,"%d bytes too big for %d byte buffer (field "
	  "%d)",len,size,field);
	dsc->error = 1;
	return -1;
    }
    memcpy(buf,dsc->data+(fields[field].pos/8),len);
    return len;
}


void q_instance(Q_DSC *dsc,int group)
{
    int i;

    if (group >= 0 || group < -Q_GROUPS)
	PREFIX(report)(Q_FATAL,"invalid group number (%d)",group);
    if (groups[-group-1].start == -1)
	PREFIX(report)(Q_FATAL,"group %d is unique",group);
    for (i = 0; i < groups[-group-1].length; i++)
	dsc->field_map[groups[-group-1].start+i] = groups[-group-1].start+i+
	  groups[-group-1].offset;
}


static int q_compose(Q_DSC *dsc,unsigned char *buf,int size)
{
    LEN_BUF stack[LENGTH_STACK];
    unsigned char *pos;
    int *pc;
    int j,i,sp;

    for (i = 0; i < Q_FIELDS; i++)
	if (q_test(dsc->required,i) && !q_test(dsc->field_present,i))
	    PREFIX(report)(Q_ERROR,"required field %d is missing",i);
    memset(buf,0,(size_t) size);
    if (q_dump)
	for (j = 0; j < 100; j += 20) {
	    fprintf(stderr,"%3d:",j);
	    for (i = 0; i < 20; i++) fprintf(stderr," %02X",dsc->data[i+j]);
	    putc('\n',stderr);
	}
    pos = buf;
    pc = construct; 
    sp = 0;
    while (1) {
	PREFIX(report)(Q_DEBUG,"%d(%d):",pc-construct,pos-buf);
	switch (*pc++) {
	    case OP_COPY:
		if (size < *pc) {
		    PREFIX(report)(Q_ERROR,"not enough space (%d < %d)",size,
		      *pc);
		    dsc->error = 1;
		    return -1;
		}
		PREFIX(report)(Q_DEBUG,"copy %d %d %d",pc[1],pos-buf,pc[2]);
		q_copy(dsc->data,pc[1],pos,pc[2]);
		if (q_dump) {
		    for (i = 0; i < 50; i++) fprintf(stderr,"%02X ",buf[i]);
		    putc('\n',stderr);
		}
		pos += *pc;
		size -= *pc;
		pc += 3;
		break;
	    case OP_COPYVAR:
		if (size < dsc->length[*pc]) {
		    PREFIX(report)(Q_ERROR,"not enough space (%d < %d)",size,
		      dsc->length[*pc]);
		    dsc->error = 1;
		    return -1;
		}
		memcpy(pos,dsc->data+pc[1]/8,dsc->length[*pc]);
		pos += dsc->length[*pc];
		size -= dsc->length[*pc];
		pc += 3;
		break;
	    case OP_BEGIN_LEN:
		if (size < *pc) {
		    PREFIX(report)(Q_ERROR,"not enough space (%d < %d)",size,
		      *pc);
		    dsc->error = 1;
		    return -1;
		}
		if (sp == LENGTH_STACK) {
		    PREFIX(report)(Q_ERROR,"length stack overflow");
		    dsc->error = 1;
		    return -1;
		}
		stack[sp].pos = pc[1]; /* not used */
		stack[sp].size = pc[2];
		stack[sp].start = pos;
		pos += *pc; /* allocate length here */
		size -= *pc;
		sp++;
		pc += 3;
		break;
	    case OP_END_LEN:
		if (!sp--) PREFIX(report)(Q_FATAL,"length stack underflow");
		q_put(stack[sp].start,0,stack[sp].size,
		  (size_t) ((pos-stack[sp].start)-((stack[sp].size+7) >> 3)));
		break;
	    case OP_IFGROUP:
		if (q_test((unsigned char *) dsc->group_present,*pc++)) pc++;
		else pc += *pc+1;
		break;
#if 0
	    case OP_CASE:
		{
		    int len;

		    for (len = *pc++; len; len--)
			if (!q_test((unsigned char *) dsc->group_present,*pc++))
			    pc++;
			else {
			    pc += *pc+1;
			    break;
			}
		    if (!len)
			PREFIX(report)(Q_FATAL,"multi failed (pc %d)",
			  pc-construct);
		}
		break;
#endif
	    case OP_JUMP:
		pc += *pc+1;
		break;
	    case OP_END:
		return pos-buf;
	    default:
		PREFIX(report)(Q_FATAL,"unrecognized opcode %d",pc[-1]);
		return -1; /* for gcc */
	}
    }
}

#endif


static const char *q_err_msg[] = { "???","not enough space (%d left)",
  "case failed (value 0x%x)","application-specific error (code %d)" };


/*
 * Rather messy ... too bad C doesn't have function-local functions ...
 */

static void handle_error(Q_DSC *dsc,int size,unsigned char *buf,
  unsigned char **stack,RSTACK *r_stack,int **pc,int *sp,int *rp,
  unsigned char **pos,unsigned char **end,Q_ERR_TYPE type,int value)
{
    Q_ERR_DSC *error,**last;

    PREFIX(report)(Q_ERROR,q_err_msg[type],value);
    PREFIX(report)(Q_ERROR,"[ PC=%d SP=%d RP=%d, pos=%d end=%d ]",*pc-parse,
      *sp,*rp,*pos-buf,*end-buf);
    error = malloc(sizeof(Q_ERR_DSC));
    if (!error) {
	perror("out of memory");
	exit(0);
    }
    error->type = type;
    for (last = &dsc->errors; *last; last = &(*last)->next);
    *last = error;
    error->next = NULL;
    if (!pc) return;
    error->pc = *pc-parse;
    error->offset = *pos-buf;
    error->value = value;
    if (*rp) {
	(*rp)--;
	error->id = r_stack[*rp].pc[0];
	error->start = r_stack[*rp].pos-buf;
	*sp = r_stack[*rp].sp;
	*pos = r_stack[*rp].end;
	error->length = *pos-r_stack[*rp].pos;
	error->group = r_stack[*rp].pc[1];
	*pc = parse+r_stack[*rp].pc[2];
    }
    else {
	error->id = 0;
	error->start = 0;
	error->length = size;
	error->group = 0;
	*sp = 0;
	*pc = parse+sizeof(parse)/sizeof(*parse)-1;
	*pos = buf+size;
    }
}


#define ERROR(type,value) \
    handle_error(dsc,size,buf,stack,r_stack,&pc,&sp,&rp,&pos,&end,type,value)


static int _q_parse(Q_DSC *dsc,unsigned char *buf,int size)
{
    RSTACK r_stack[LENGTH_R_STACK];
    unsigned char *stack[LENGTH_STACK];
    unsigned char *pos,*end;
    int *pc;
    int i,sp,rp;

    end = buf+size;
    pos = buf;
    pc = parse; 
    sp = rp = 0;
    while (1) {
	PREFIX(report)(Q_DEBUG,"%d(%d):",pc-parse,pos-buf);
	switch (*pc++) {
#ifdef DUMP_MODE
	    case OP_DUMP:
		{
		    unsigned long value;
		    int len;

		    for (i = dump_fields[*pc].level; i; i--) DUMP("  ");
		    DUMP("%s =",dump_fields[*pc++].name);
		    len = *pc == OP_COPYVAR ? (end-pos)*8 : pc[3];
		    if (len <= 32) {
			const SYM_NAME *sym;

			value = q_get(pos,pc[2] & 7,len);
			if (!(sym = dump_fields[pc[-1]].sym))
			    DUMP(" %ld (0x%lx)\n",value,value);
			else {
			    while (sym->name)
				if (sym->value == value) break;
				else sym++;
			    if (sym->name) DUMP(" %s\n",sym->name);
			    else DUMP(" %ld (0x%lx)\n",value,value);
			}
		    }
		    else {
			for (i = 0; i < len/8; i++)
			    DUMP(" %02x",pos[i]);
			DUMP("\n");
		    }
		}
		break;
#endif
	    case OP_COPY:
		if (pos+*pc > end) {
		    ERROR(qet_space,end-pos);
		    continue;
		}
		PREFIX(report)(Q_DEBUG,"copy %d %d %d",pc[1],pos-buf,pc[2]);
		q_copy(pos,pc[1] & 7,dsc->data+(pc[1] >> 3),pc[2]);
		if (q_dump) {
		    for (i = 0; i < 20; i++)
			fprintf(stderr,"%02X ",dsc->data[i]);
		    putc('\n',stderr);
		}
		pos += *pc;
		pc += 3;
		break;
	    case OP_COPYVAR:
		{
		    int len;

		    len = end-pos;
		    if (len > pc[2]) len = pc[2];
		    memcpy(dsc->data+pc[1]/8,pos,(size_t) len);
		    PREFIX(report)(Q_DEBUG,"len %d for %d",len,*pc);
		    dsc->length[*pc] = len;
		    pos += len;
		    pc += 3;
		    break;
		}
	    case OP_BEGIN_LEN:
		if (pos+*pc > end) {
		    ERROR(qet_space,end-pos);
		    continue;
		}
		if (sp == LENGTH_STACK)
		    PREFIX(report)(Q_FATAL,"length stack overflow");
		stack[sp] = end;
		end = pos+q_get(pos,pc[1] & 7,pc[2])+*pc;
		if (end > stack[sp])
		    PREFIX(report)(Q_FATAL,"length has grown");
		pos += *pc;
		sp++;
		pc += 3;
		break;
	    case OP_BEGIN_REC:
		PREFIX(report)(Q_DEBUG,"begin_rec pc %d sp %d pos %d end %d",
		  pc-parse,sp,pos-buf,end-buf);
		if (rp == LENGTH_R_STACK)
		    PREFIX(report)(Q_FATAL,"recovery stack overflow");
		r_stack[rp].pc = pc;
		r_stack[rp].sp = sp;
		r_stack[rp].pos = pos;
		r_stack[rp].end = end;
		rp++;
		pc += 3;
		break;
	    case OP_END_LEN:
		if (!sp--) PREFIX(report)(Q_FATAL,"length stack underflow");
		end = stack[sp];
		break;
	    case OP_END_REC:
		PREFIX(report)(Q_DEBUG,"end_rec");
		if (!rp--) PREFIX(report)(Q_FATAL,"recovery stack underflow");
		break;
	    case OP_CASE:
		{
		    int len,value,group;

		    if (pos+*pc > end) {
			ERROR(qet_space,end-pos);
			continue;
		    }
		    value = q_get(pos,pc[1] & 7,pc[2]);
		    pos += *pc;
		    pc += 3;
		    for (len = *pc++; len; len--)
			if (*pc != value && *pc != -1) pc += 3;
			else {
			    pc++;
			    if (*pc != -1 && q_test((unsigned char *)
			      dsc->group_present,*pc)) {
				pc += 2;
				continue;
			    }
			    for (group = *pc++; group != -1; group =
			      groups[group].parent)
				q_set((unsigned char *) dsc->group_present,
				  group);
			    pc += *pc+1;
			    break;
			}
		    if (!len) {
			ERROR(qet_case,value);
			continue;
		    }
		}
		break;
	    case OP_JUMP:
		pc += *pc+1;
		break;
	    case OP_IFEND:
		PREFIX(report)(Q_DEBUG,"ifend - %d/%d",pos-buf,end-buf);
		if (pos == end) pc += *pc;
		pc++;
		break;
	    case OP_ABORT:
		ERROR(qet_abort,*pc);
		continue;
	    case OP_END:
		return dsc->errors ? -1 : 0;
	    default:
		PREFIX(report)(Q_FATAL,"unrecognized opcode %d",pc[-1]);
		return -1; /* for gcc */
	}
    }
}


#define GROUP_STACK_SIZE 100 /* make sure it's big enough ... */


static void fixups(Q_DSC *dsc)
{
    int gs[GROUP_STACK_SIZE];
    Q_ERR_DSC *walk;
    int gp,i,j;

    for (walk = dsc->errors; walk; walk = walk->next) {
	gs[gp = 0] = walk->group;
	q_clear((unsigned char *) dsc->group_present,walk->group);
	for (i = walk->group+1; i < Q_GROUPS; i++) {
	    while (groups[i].parent != gs[gp])
		if (!gp--) break;
	    if (gp < 0) break;
	    gs[++gp] = i;
	    q_clear((unsigned char *) dsc->group_present,i);
	}
	for (j = 0; j < Q_FIELDS; j++)
	     if (fields[j].parent == walk->group) break;
	while (j < Q_FIELDS && fields[j].parent < i) {
	    q_clear(dsc->field_present,j);
	    j++;
	}
    }
}


static int q_parse(Q_DSC *dsc,unsigned char *buf,int size)
{
    int error;
    int j,i;
    int *p;

    if (q_dump)
	for (j = 0; j < 100; j += 20) {
	    fprintf(stderr,"%3d:",j);
	    for (i = 0; i < 20; i++) fprintf(stderr," %02X",dsc->data[i+j]);
	    putc('\n',stderr);
	}
    error = _q_parse(dsc,buf,size);
    if (error) {
	fixups(dsc);
	return error;
    }
    if (q_dump) {
	putc('\n',stderr);
	for (j = 0; j < 100; j += 20) {
	    fprintf(stderr,"%3d:",j);
	    for (i = 0; i < 20; i++) fprintf(stderr," %02X",dsc->data[i+j]);
	    putc('\n',stderr);
	}
	for (i = 0; i < Q_GROUPS; i++)
	    if (q_test((unsigned char *) dsc->group_present,i))
		for (p = groups[i].required; p && *p != -1; p++)
		    fprintf(stderr,"%d: %ld / 0x%lx\n",*p,
		      q_get(dsc->data,fields[*p].pos,fields[*p].size),
		      q_get(dsc->data,fields[*p].pos,fields[*p].size));
    }
    return 0;
}


int PREFIX(open)(Q_DSC *dsc,void *buf,int size)
{
    int error;

    dsc->buffer = NULL;
    error = q_init(dsc);
    if (error) {
	handle_error(dsc,size,buf,NULL,NULL,NULL,NULL,NULL,NULL,NULL,qet_init,
	  0);
	return error;
    }
    return q_parse(dsc,buf,size);
}


#ifndef DUMP_MODE

int q_create(Q_DSC *dsc,void *buf,int size)
{
   dsc->buffer = buf;
   dsc->buf_size = size;
   return q_init(dsc);
}

#endif


int PREFIX(close)(Q_DSC *dsc)
{
    int size;

    size = 0; /* for gcc */
#ifndef DUMP_MODE
    if (dsc->buffer && !dsc->error)
	size = q_compose(dsc,dsc->buffer,dsc->buf_size);
#endif
    if (dsc->data) free(dsc->data);
    if (dsc->required) free(dsc->required);
    if (dsc->field_present) free(dsc->field_present);
    if (dsc->group_present) free(dsc->group_present);
    if (dsc->length) free(dsc->length);
    if (dsc->field_map) free(dsc->field_map);
    if (dsc->group_map) free(dsc->group_map);
    while (dsc->errors) {
	Q_ERR_DSC *next;

	next = dsc->errors->next;
	free(dsc->errors);
	dsc->errors = next;
    }
    return dsc->error ? -1 : dsc->buffer ? size : 0;
}


#ifdef STANDALONE

int main(int argc,const char **argv)
{
    unsigned char msg[5000]; /* should be large enough for that */
    Q_DSC dsc;
    int len,c;

    debug = argc != 1;
    len = 0;
    while (scanf("%x",&c) == 1) msg[len++] = c;
    qd_start();
    qd_open(&dsc,msg,len);
    qd_close(&dsc);
    return 0;
}

#endif
