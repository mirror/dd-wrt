
#define RTC_DENOMINATOR 100
#define RTC_PERIOD 110000000 / RTC_DENOMINATOR

#define MACRO_START do {
#define MACRO_END \
	}         \
	while (0)
static unsigned int clock_period;

#define _clock_initialize(_period_)    \
	MACRO_START                    \
	asm volatile("mtc0 $0,$9\n"    \
		     "nop; nop; nop\n" \
		     "mtc0 %0,$11\n"   \
		     "nop; nop; nop\n" \
		     :                 \
		     : "r"(_period_)); \
	clock_period = _period_;       \
	MACRO_END

#define clock_reset(_vector_, _period_) \
	MACRO_START                     \
	asm volatile("mtc0 $0,$9\n"     \
		     "nop; nop; nop\n"  \
		     "mtc0 %0,$11\n"    \
		     "nop; nop; nop\n"  \
		     :                  \
		     : "r"(_period_));  \
	MACRO_END

#define clock_read(_pvalue_)                           \
	MACRO_START                                    \
	register unsigned int result;                  \
	asm volatile("mfc0   %0,$9\n" : "=r"(result)); \
	*(_pvalue_) = result;                          \
	MACRO_END

/*
 * udelay implementation based on cpu cycle counter
 */
static void udelay(int us)
{
	unsigned int val1, val2;
	int diff;
	long usticks;
	long ticks;

	// Calculate the number of counter register ticks per microsecond.

	usticks = (RTC_PERIOD * RTC_DENOMINATOR) / 1000000;

	// Make sure that the value is not zero. This will only happen if the
	// CPU is running at < 2MHz.
	if (usticks == 0)
		usticks = 1;

	while (us > 0) {
		int us1 = us;

		// Wait in bursts of less than 10000us to avoid any overflow
		// problems in the multiply.
		if (us1 > 10000)
			us1 = 10000;

		us -= us1;

		ticks = us1 * usticks;

		clock_read(&val1);
		while (ticks > 0) {
			do {
				clock_read(&val2);
			} while (val1 == val2);
			diff = val2 - val1;
			if (diff < 0)
				diff += RTC_PERIOD;
			ticks -= diff;
			val1 = val2;
		}
	}
}
void msleep(int msec)
{
	udelay(msec * 1000);
}
void clock_initialize(void)
{
	_clock_initialize(RTC_PERIOD);
}
unsigned int get_clock_period(void)
{
	return clock_period;
}