#include "links.h"

char dummy_val;
volatile char *dummy_ptr = &dummy_val;

void do_not_optimize_here(void *p)
{
	*dummy_ptr = 0;
	/* break ANSI aliasing */
}

#ifdef LEAK_DEBUG
long mem_amount = 0;
long last_mem_amount = -1;
#ifdef LEAK_DEBUG_LIST
struct list_head memory_list = { &memory_list, &memory_list };
#endif
#endif

static inline void force_dump()
{
	fprintf(stderr, "\n\033[1m%s\033[0m\n", "Forcing core dump");
	fflush(stdout);
	fflush(stderr);
	fatal_tty_exit();
	raise(SIGSEGV);
}

void check_memory_leaks()
{
#ifdef LEAK_DEBUG
	if (mem_amount) {
		fprintf(stderr, "\n\033[1mMemory leak by %ld bytes\033[0m\n", mem_amount);
#ifdef LEAK_DEBUG_LIST
		fprintf(stderr, "\nList of blocks: ");
		{
			int r = 0;
			struct alloc_header *ah;
			foreach (ah, memory_list) {
				fprintf(stderr, "%s%p:%lu @ %s:%d", r ? ", ": "", (char *)ah + L_D_S, (unsigned long)ah->size, ah->file, ah->line), r = 1;
				if (ah->comment) fprintf(stderr, ":\"%s\"", ah->comment);
			}
			fprintf(stderr, "\n");
		}
#endif
		force_dump();
	}
#endif
}

void er(int b, unsigned char *m, va_list l)
{
	if (b) fprintf(stderr, "%c", (char)7);
#ifdef HAVE_VPRINTF
	vfprintf(stderr, m, l);
#else
	fprintf(stderr, "%s", m);
#endif
	fprintf(stderr, "\n");
	sleep(1);
}

void error(unsigned char *m, ...)
{
	va_list l;
	va_start(l, m);
	er(1, m, l);
	va_end(l);
}

int errline;
unsigned char *errfile;

unsigned char errbuf[4096];

void int_error(unsigned char *m, ...)
{
	va_list l;
	va_start(l, m);
	sprintf(errbuf, "\033[1mINTERNAL ERROR\033[0m at %s:%d: ", errfile, errline);
	strcat(errbuf, m);
	er(1, errbuf, l);
	force_dump();
	va_end(l);
}

void debug_msg(unsigned char *m, ...)
{
	va_list l;
	va_start(l, m);
	sprintf(errbuf, "DEBUG MESSAGE at %s:%d: ", errfile, errline);
	strcat(errbuf, m);
	er(0, errbuf, l);
	va_end(l);
}

#ifdef LEAK_DEBUG

void *debug_mem_alloc(unsigned char *file, int line, size_t size)
{
	void *p;
#ifdef LEAK_DEBUG
	struct alloc_header *ah;
#endif
	if (!size) return DUMMY;
	if (size > MAXINT) overalloc();
#ifdef LEAK_DEBUG
	mem_amount += size;
	size += L_D_S;
#endif
	if (!(p = xmalloc(size))) {
		error("ERROR: out of memory (malloc returned NULL)");
		fatal_tty_exit();
		exit(RET_FATAL);
		return NULL;
	}
#ifdef LEAK_DEBUG
	ah = p;
	p = (char *)p + L_D_S;
	ah->size = size - L_D_S;
#ifdef LEAK_DEBUG_LIST
	ah->file = file;
	ah->line = line;
	ah->comment = NULL;
	add_to_list(memory_list, ah);
#endif
#endif
	return p;
}

void debug_mem_free(unsigned char *file, int line, void *p)
{
#ifdef LEAK_DEBUG
	struct alloc_header *ah;
#endif
	if (p == DUMMY) return;
	if (!p) {
		errfile = file, errline = line, int_error("mem_free(NULL)");
		return;
	}
#ifdef LEAK_DEBUG
	p = (char *)p - L_D_S;
	ah = p;
#ifdef LEAK_DEBUG_LIST
	del_from_list(ah);
	if (ah->comment) free(ah->comment);
#endif
	mem_amount -= ah->size;
#endif
	xfree(p);
}

void *debug_mem_realloc(unsigned char *file, int line, void *p, size_t size)
{
#ifdef LEAK_DEBUG
	struct alloc_header *ah;
#endif
	if (p == DUMMY) return debug_mem_alloc(file, line, size);
	if (!p) {
		errfile = file, errline = line, int_error("mem_realloc(NULL, %d)", size);
		return NULL;
	}
	if (!size) {
		debug_mem_free(file, line, p);
		return DUMMY;
	}
	if (size > MAXINT) overalloc();
	if (!(p = xrealloc((char *)p - L_D_S, size + L_D_S))) {
		error("ERROR: out of memory (realloc returned NULL)");
		fatal_tty_exit();
		exit(RET_FATAL);
		return NULL;
	}
#ifdef LEAK_DEBUG
	ah = p;
	mem_amount += size - ah->size;
	ah->size = size;
#ifdef LEAK_DEBUG_LIST
	ah->prev->next = ah;
	ah->next->prev = ah;
#endif
#endif
	return (char *)p + L_D_S;
}

void set_mem_comment(void *p, unsigned char *c, int l)
{
#ifdef LEAK_DEBUG_LIST
	struct alloc_header *ah = (struct alloc_header *)((char *)p - L_D_S);
	if (ah->comment) free(ah->comment);
	if ((ah->comment = malloc(l + 1))) memcpy(ah->comment, c, l), ah->comment[l] = 0;
#endif
}

#endif

