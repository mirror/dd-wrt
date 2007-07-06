#include <bmon/config.h>
#include <bmon/utils.h>
#include <bmon/input.h>

struct reader_timing rtiming;

void quit(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	exit(1);
}

int main(int argc, char *argv[])
{
	char *u;
	b_cnt_t t = 100;
	double d;

	for (t = 100; t < 100000000000LL; t += (100*(t/100))) {
		d = cancel_down(t, U_BYTES, &u);
		printf("%.2f %s\n", d, u);
	}

	return 0;
}
