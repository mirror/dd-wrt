#ifndef MALLOC_CHECK_H
#define MALLOC_CHECK_H 1

typedef struct Malloc_check_vals
{
 void *ptr;
 size_t sz;
 const char *file;
 unsigned int line;
} Malloc_check_vals;

typedef struct Malloc_check_store
{
 unsigned long      mem_sz;
 unsigned long      mem_num;
 unsigned long      mem_fail_num;
 Malloc_check_vals *mem_vals;
} Malloc_check_store;

#ifndef  MALLOC_CHECK__ATTR_USED
#ifdef __GNUC__
# define MALLOC_CHECK__ATTR_USED() __attribute__((__used__))
#else
# define MALLOC_CHECK__ATTR_USED() /* do nothing */
#endif
#endif

#ifndef  MALLOC_CHECK__ATTR_H
#ifdef __GNUC__
# define MALLOC_CHECK__ATTR_H() __attribute__((__visibility__("hidden")))
#else
# define MALLOC_CHECK__ATTR_H() /* do nothing */
#endif
#endif

#ifndef  MALLOC_CHECK__ATTR_MALLOC
#ifdef __GNUC__
# define MALLOC_CHECK__ATTR_MALLOC() __attribute__ ((__malloc__))
#else
# define MALLOC_CHECK__ATTR_MALLOC() /* do nothing */
#endif
#endif

#ifndef MALLOC_CHECK_SUPPER_SCRUB /* never really call realloc() */
#define MALLOC_CHECK_SUPPER_SCRUB 0
#endif

#ifndef MALLOC_CHECK_STORE
#define MALLOC_CHECK_STORE malloc_check__store
#endif

extern Malloc_check_store MALLOC_CHECK__ATTR_H() MALLOC_CHECK_STORE;

#define MALLOC_CHECK_DECL()                                     \
    Malloc_check_store MALLOC_CHECK_STORE = {0, 0, 0, NULL}

/* NOTE: don't reset fail nums */
#define MALLOC_CHECK_REINIT()                         \
    MALLOC_CHECK_STORE.mem_sz = 0;                   \
    MALLOC_CHECK_STORE.mem_num = 0;                  \
    MALLOC_CHECK_STORE.mem_vals = NULL
#define MALLOC_CHECK_INIT()                           \
    MALLOC_CHECK_STORE.mem_fail_num = 0;             \
    MALLOC_CHECK_REINIT()

#ifndef USE_MALLOC_CHECK
#ifndef VSTR_AUTOCONF_NDEBUG
#  define USE_MALLOC_CHECK 1
# else
#  define USE_MALLOC_CHECK 0
# endif
#endif

#if !(USE_MALLOC_CHECK)
# define MALLOC_CHECK_MEM(x) (1)
# define MALLOC_CHECK_EMPTY() /* nothing */
# define MALLOC_CHECK_DEC() (0)
# define MALLOC_CHECK_FAIL_IN(x) /* nothing */
# define MALLOC_CHECK_SCRUB_PTR(x, y)  /* nothing */

# define malloc_check_malloc(x, F, L)     malloc(x)
# define malloc_check_calloc(x, y, F, L)  calloc(x, y)
# define malloc_check_realloc(x, y, F, L) realloc(x, y)
# define malloc_check_free(x)             free(x)
#else

#include <stdio.h>

# define MALLOC_CHECK_MEM(x)  malloc_check_mem(x)
# define MALLOC_CHECK_EMPTY() malloc_check_empty()
# define MALLOC_CHECK_DEC()                                             \
    (MALLOC_CHECK_STORE.mem_fail_num && !--MALLOC_CHECK_STORE.mem_fail_num)
# define MALLOC_CHECK_FAIL_IN(x) MALLOC_CHECK_STORE.mem_fail_num = (x)
# define MALLOC_CHECK_SCRUB_PTR(x, y)  memset(x, 0xa5, y)

#ifndef MALLOC_CHECK_PRINT
#define MALLOC_CHECK_PRINT 1
#endif

#ifndef SWAP_TYPE
#define SWAP_TYPE(x, y, type) do {              \
      type internal_local_tmp = (x);            \
      (x) = (y);                                \
      (y) = internal_local_tmp;                 \
    } while (FALSE)
#endif

static void malloc_check_alloc(void)
   MALLOC_CHECK__ATTR_USED();
static unsigned int malloc_check_mem(const void *)
   MALLOC_CHECK__ATTR_USED();
static unsigned int malloc_check_sz_mem(const void *, size_t)
   MALLOC_CHECK__ATTR_USED();
static void *malloc_check_malloc(size_t, const char *, unsigned int)
   MALLOC_CHECK__ATTR_MALLOC() MALLOC_CHECK__ATTR_USED();
static void *malloc_check_calloc(size_t, size_t, const char *, unsigned int)
   MALLOC_CHECK__ATTR_MALLOC() MALLOC_CHECK__ATTR_USED();
static void malloc_check_free(void *)
   MALLOC_CHECK__ATTR_USED();
static void *malloc_check_realloc(void *, size_t,
                                  const char *, unsigned int)
   MALLOC_CHECK__ATTR_MALLOC() MALLOC_CHECK__ATTR_USED();
static void malloc_check_empty(void)
   MALLOC_CHECK__ATTR_USED();

static void malloc_check_alloc(void)
{
  size_t sz = MALLOC_CHECK_STORE.mem_sz;
  
  ++MALLOC_CHECK_STORE.mem_num;

  if (!MALLOC_CHECK_STORE.mem_sz)
  {
    sz = 8;
    MALLOC_CHECK_STORE.mem_vals = malloc(sizeof(Malloc_check_vals) * sz);
  }
  else if (MALLOC_CHECK_STORE.mem_num > MALLOC_CHECK_STORE.mem_sz)
  {
    sz *= 2;
    MALLOC_CHECK_STORE.mem_vals = realloc(MALLOC_CHECK_STORE.mem_vals,
                                          sizeof(Malloc_check_vals) * sz);
  }
  ASSERT(MALLOC_CHECK_STORE.mem_num <= sz);
  ASSERT(MALLOC_CHECK_STORE.mem_vals);

  MALLOC_CHECK_STORE.mem_sz = sz;
}

static unsigned int malloc_check_mem(const void *ptr)
{
  unsigned int scan = 0;

  ASSERT(MALLOC_CHECK_STORE.mem_num);
    
  while (MALLOC_CHECK_STORE.mem_vals[scan].ptr &&
         (MALLOC_CHECK_STORE.mem_vals[scan].ptr != ptr))
    ++scan;
  
  ASSERT(MALLOC_CHECK_STORE.mem_vals[scan].ptr);

  return (scan);
}

static unsigned int malloc_check_sz_mem(const void *ptr, size_t sz)
{
  unsigned int scan = malloc_check_mem(ptr);

  ASSERT(MALLOC_CHECK_STORE.mem_vals[scan].sz == sz);

  return (scan);
}

static void *malloc_check_malloc(size_t sz, const char *file, unsigned int line)
{
  void *ret = NULL;

  if (MALLOC_CHECK_DEC())
    return (NULL);

  malloc_check_alloc();

  ASSERT(sz);

  ret = malloc(sz);
  ASSERT_RET(ret, NULL);
  
  MALLOC_CHECK_SCRUB_PTR(ret, sz);

  MALLOC_CHECK_STORE.mem_vals[MALLOC_CHECK_STORE.mem_num - 1].ptr  = ret;
  MALLOC_CHECK_STORE.mem_vals[MALLOC_CHECK_STORE.mem_num - 1].sz   = sz;
  MALLOC_CHECK_STORE.mem_vals[MALLOC_CHECK_STORE.mem_num - 1].file = file;
  MALLOC_CHECK_STORE.mem_vals[MALLOC_CHECK_STORE.mem_num - 1].line = line;

  return (ret);
}

static void *malloc_check_calloc(size_t num, size_t sz,
                                 const char *file, unsigned int line)
{
  size_t real_sz = num * sz;
  void *ret = NULL;
  
  if ((num != 0) && ((real_sz / sz) != num))
    return (NULL);
  if (!(ret = malloc_check_malloc(real_sz, file, line)))
    return (NULL);

  memset(ret, 0, real_sz);
  return (ret);
}

static void malloc_check_free(void *ptr)
{
  if (ptr)
  {
    unsigned int scan = malloc_check_mem(ptr);
    size_t sz = 0;
    
    ASSERT(MALLOC_CHECK_STORE.mem_num > 0);
    --MALLOC_CHECK_STORE.mem_num;

    sz = MALLOC_CHECK_STORE.mem_vals[scan].sz;
    if (scan != MALLOC_CHECK_STORE.mem_num)
    {
      unsigned int num = MALLOC_CHECK_STORE.mem_num;
      Malloc_check_vals *val1 = &MALLOC_CHECK_STORE.mem_vals[scan];
      Malloc_check_vals *val2 = &MALLOC_CHECK_STORE.mem_vals[num];
      
      SWAP_TYPE(val1->ptr,  val2->ptr,  void *);
      SWAP_TYPE(val1->sz,   val2->sz,   size_t);
      SWAP_TYPE(val1->file, val2->file, const char *);
      SWAP_TYPE(val1->line, val2->line, unsigned int);
    }
    MALLOC_CHECK_STORE.mem_vals[MALLOC_CHECK_STORE.mem_num].ptr = NULL;
    MALLOC_CHECK_SCRUB_PTR(ptr, sz);
    free(ptr);
  }
}

static void *malloc_check_realloc(void *ptr, size_t sz,
                                  const char *file, unsigned int line)
{
  void *ret = NULL;
  unsigned int scan = malloc_check_mem(ptr);

  ASSERT(ptr && sz);

  if (MALLOC_CHECK_SUPPER_SCRUB)
  {
    if (!(ret = malloc_check_malloc(sz, file, line)))
      return (NULL);

    if (sz >= MALLOC_CHECK_STORE.mem_vals[scan].sz)
      sz = MALLOC_CHECK_STORE.mem_vals[scan].sz;
    if (sz)
      memcpy(ret, ptr, sz);
    
    malloc_check_free(ptr);
    
    return (ret);
  }

  if (MALLOC_CHECK_DEC())
    return (NULL);

  ret = realloc(ptr, sz);
  ASSERT_RET(ret, NULL);

  /* note we can't scrub ... :( */
  MALLOC_CHECK_STORE.mem_vals[scan].ptr  = ret;
  MALLOC_CHECK_STORE.mem_vals[scan].sz   = sz;
  MALLOC_CHECK_STORE.mem_vals[scan].file = file;
  MALLOC_CHECK_STORE.mem_vals[scan].line = line;

  return (ret);
}

static void malloc_check_empty(void)
{
  if (MALLOC_CHECK_PRINT && MALLOC_CHECK_STORE.mem_num)
  {
    unsigned int scan = 0;

    while (MALLOC_CHECK_STORE.mem_vals[scan].ptr)
    {
      fprintf(stderr, " FAILED MEM CHECK EMPTY: ptr %p, sz %zu, from %u:%s\n",
              MALLOC_CHECK_STORE.mem_vals[scan].ptr,
              MALLOC_CHECK_STORE.mem_vals[scan].sz,
              MALLOC_CHECK_STORE.mem_vals[scan].line,
              MALLOC_CHECK_STORE.mem_vals[scan].file);
      ++scan;
    }
  }
  ASSERT(!MALLOC_CHECK_STORE.mem_num);
}
#endif

#endif
