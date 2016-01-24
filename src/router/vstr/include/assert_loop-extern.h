#ifndef ASSERT_LOOP_EXTERN_H
#define ASSERT_LOOP_EXTERN_H

#undef  assert
#undef  ASSERT

#ifdef NDEBUG
#  define assert(x) ((void) 0)
#  define ASSERT(x) ((void) 0)
#  define assert_ret(x, y)   do { if (x) {} else return (y); } while (FALSE)
#  define ASSERT_RET(x, y)   do { if (x) {} else return (y); } while (FALSE)
#  define assert_ret_void(x) do { if (x) {} else return; } while (FALSE)
#  define ASSERT_RET_VOID(x) do { if (x) {} else return; } while (FALSE)
#  define assert_goto(x, y)  do { if (x) {} else goto y; } while (FALSE)
#  define ASSERT_GOTO(x, y)  do { if (x) {} else goto y; } while (FALSE)
#  define assert_no_switch_def() break
#  define ASSERT_NO_SWITCH_DEF() break
#else
# ifdef USE_ASSERT_LOOP
extern void vstr__assert_loop(const char *,
                              const char *, int, const char *);
#  define assert(x) do { \
 if (x) {} else \
  vstr__assert_loop(#x, __FILE__, __LINE__, __func__); \
 } while (FALSE)
#  define ASSERT(x) do { \
 if (x) {} else \
  vstr__assert_loop(#x, __FILE__, __LINE__, __func__); \
 } while (FALSE)

#  define assert_ret(x, y) do { \
 if (x) {} else { \
  vstr__assert_loop(#x, __FILE__, __LINE__, __func__); return (y); } \
 } while (FALSE)
#  define ASSERT_RET(x, y) do { \
 if (x) {} else { \
  vstr__assert_loop(#x, __FILE__, __LINE__, __func__); return (y); } \
 } while (FALSE)
#  define assert_ret_void(x) do { \
 if (x) {} else { \
  vstr__assert_loop(#x, __FILE__, __LINE__, __func__); return; } \
 } while (FALSE)
#  define ASSERT_RET_VOID(x) do { \
 if (x) {} else { \
  vstr__assert_loop(#x, __FILE__, __LINE__, __func__); return; } \
 } while (FALSE)
#  define assert_goto(x, y) do { \
 if (x) {} else { \
  vstr__assert_loop(#x, __FILE__, __LINE__, __func__); return (y); } \
 } while (FALSE)
#  define ASSERT_GOTO(x, y) do { \
 if (x) {} else { \
  vstr__assert_loop(#x, __FILE__, __LINE__, __func__); goto     y; } \
 } while (FALSE)
# else
#  define assert(x) do { \
 if (x) {} else { \
  fprintf(stderr, " -=> assert (%s) failed in (%s) from %d %s.\n", \
          #x , __func__, __LINE__, __FILE__); \
  abort(); } \
 } while (FALSE)
#  define ASSERT(x) do { \
 if (x) {} else { \
  fprintf(stderr, " -=> ASSERT (%s) failed in (%s) from %d %s.\n", \
          #x , __func__, __LINE__, __FILE__); \
  abort(); } \
 } while (FALSE)

#  define assert_ret(x, y) do { \
 if (x) {} else { \
  fprintf(stderr, " -=> assert (%s) failed in (%s) from %d %s.\n", \
          #x , __func__, __LINE__, __FILE__); \
  abort(); } \
 } while (FALSE)
#  define ASSERT_RET(x, y) do { \
 if (x) {} else { \
  fprintf(stderr, " -=> ASSERT (%s) failed in (%s) from %d %s.\n", \
          #x , __func__, __LINE__, __FILE__); \
  abort(); } \
 } while (FALSE)
#  define assert_ret_void(x) do { \
 if (x) {} else { \
  fprintf(stderr, " -=> assert (%s) failed in (%s) from %d %s.\n", \
          #x , __func__, __LINE__, __FILE__); \
  abort(); } \
 } while (FALSE)
#  define ASSERT_RET_VOID(x) do { \
 if (x) {} else { \
  fprintf(stderr, " -=> ASSERT (%s) failed in (%s) from %d %s.\n", \
          #x , __func__, __LINE__, __FILE__); \
  abort(); } \
 } while (FALSE)
#  define assert_goto(x, y) do { \
 if (x) {} else { \
  fprintf(stderr, " -=> assert (%s) failed in (%s) from %d %s.\n", \
          #x , __func__, __LINE__, __FILE__); \
  abort(); } \
 } while (FALSE)
#  define ASSERT_GOTO(x, y) do { \
 if (x) {} else { \
  fprintf(stderr, " -=> ASSERT (%s) failed in (%s) from %d %s.\n", \
          #x , __func__, __LINE__, __FILE__); \
  abort(); goto     y; } \
 } while (FALSE)
# endif
# define assert_no_switch_def() break; default: ASSERT(!"default label")
# define ASSERT_NO_SWITCH_DEF() break; default: ASSERT(!"default label")
#endif

#endif
