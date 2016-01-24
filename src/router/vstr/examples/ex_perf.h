#ifndef EX_PERF_H
#define EX_PERF_H 1

#include <sys/time.h> /* gettimeofday */
#include <malloc.h> /* mallinfo */

#define TST_HDR_BEG() \
  vstr_add_fmt(out, out->len, \
               "+${rep_chr:%c%zu}" \
               "+${rep_chr:%c%zu}" \
               "+${rep_chr:%c%zu}+\n" \
               "| %34s | %15s | %16s |\n" \
               "+${rep_chr:%c%zu}" \
               "+${rep_chr:%c%zu}" \
               "+${rep_chr:%c%zu}+\n", \
               '=', 18 + 18, '=', 17, '=', 18, \
               "Name", "Speed", "Memory", \
               '-', 18 + 18, '-', 17, '-', 18)

#define TST_HDR_END() \
  vstr_add_fmt(out, out->len, \
               "+${rep_chr:%c%zu}" \
               "+${rep_chr:%c%zu}" \
               "+${rep_chr:%c%zu}+\n", \
               '-', 18 + 18, '-', 17, '-', 18)

#define TST_BEG(x, y) do { \
		struct mallinfo mal_beg; \
		struct mallinfo mal_end; \
		struct timeval tv_beg; \
		struct timeval tv_end; \
		unsigned int test_for = (y); \
                unsigned int tst_count = 0; \
                 \
                mal_beg = mallinfo(); /* warning */ \
		gettimeofday(&tv_beg, NULL); \
		 \
		while (tst_count < test_for) \
		{ \
                  unsigned char buf_out[x];     \
                  buf_out[0] = tv_beg.tv_usec


#define TST__END() \
                        ++tst_count; \
		} \
                 \
		gettimeofday(&tv_end, NULL); \
                if (tv_end.tv_usec < tv_beg.tv_usec) \
                { \
                  tv_end.tv_usec += 1000 * 1000; \
                  tv_end.tv_sec  -= 1; \
                } \
                mal_end = mallinfo(); /* warning */

#define TST_END(name) TST__END() \
		vstr_add_fmt(out, out->len, \
                             "| %-34s | %'8lu.%06lu | %'16lu |\n", \
                             (name), \
		             tv_end.tv_sec  - tv_beg.tv_sec, \
                             tv_end.tv_usec - tv_beg.tv_usec, \
                             (unsigned long) \
                             (mal_end.uordblks - mal_beg.uordblks) + \
                             (mal_end.hblkhd - mal_beg.hblkhd)); \
	} while (0)

#define TST_CALC_END(name) TST__END() \
  vstr_add_fmt(out, out->len, \
               "%s %u %u %u %u %lu.%06lu %ld %ld\n", (name), \
               len, num, sz, extra, \
               tv_end.tv_sec  - tv_beg.tv_sec, \
               tv_end.tv_usec - tv_beg.tv_usec, \
               ((long)mal_end.uordblks - (long)mal_beg.uordblks) + \
               ((long)mal_end.hblkhd - (long)mal_beg.hblkhd), \
               ((long)mal_end.fordblks - (long)mal_beg.fordblks)); \
	} while (0)



#endif

