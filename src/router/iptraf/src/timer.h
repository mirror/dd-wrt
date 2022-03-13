#ifndef IPTRAF_NG_TIMER_H
#define IPTRAF_NG_TIMER_H

void printelapsedtime(time_t elapsed, int x, WINDOW *win);
bool time_after(struct timespec const *a, struct timespec const *b);
void time_add_msecs(struct timespec *time, unsigned int msecs);

#endif	/* IPTRAF_NG_TIMER_H */
