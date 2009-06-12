#ifndef __NIOS_A_OUT_H__
#define __NIOS_A_OUT_H__

#define SPARC_PGSIZE    0x1000        /* Thanks to the sun4 architecture... */
#define SEGMENT_SIZE    SPARC_PGSIZE  /* whee... */

struct exec {
	unsigned char a_dynamic:1;      /* A __DYNAMIC is in this image */
	unsigned char a_toolversion:7;
	unsigned char a_machtype;
	unsigned short a_info;
	unsigned long a_text;		/* length of text, in bytes */
	unsigned long a_data;		/* length of data, in bytes */
	unsigned long a_bss;		/* length of bss, in bytes */
	unsigned long a_syms;		/* length of symbol table, in bytes */
	unsigned long a_entry;		/* where program begins */
	unsigned long a_trsize;
	unsigned long a_drsize;
};

/* Where in the file does the text information begin? */
#define N_TXTOFF(x)     (N_MAGIC(x) == ZMAGIC ? 0 : sizeof (struct exec))

/* Where do the Symbols start? */
#define N_SYMOFF(x)     (N_TXTOFF(x) + (x).a_text +   \
                         (x).a_data + (x).a_trsize +  \
                         (x).a_drsize)

/* Where does text segment go in memory after being loaded? */
#define N_TXTADDR(x)    (((N_MAGIC(x) == ZMAGIC) &&        \
	                 ((x).a_entry < SPARC_PGSIZE)) ?   \
                          0 : SPARC_PGSIZE)

/* And same for the data segment.. */
#define N_DATADDR(x) (N_MAGIC(x)==OMAGIC ?         \
                      (N_TXTADDR(x) + (x).a_text)  \
                       : (_N_SEGMENT_ROUND (_N_TXTENDADDR(x))))

#define N_TRSIZE(a)	((a).a_trsize)
#define N_DRSIZE(a)	((a).a_drsize)
#define N_SYMSIZE(a)	((a).a_syms)

#ifdef __KERNEL__

#define STACK_TOP	TASK_SIZE

#endif

#endif /* __NIOS_A_OUT_H__ */
