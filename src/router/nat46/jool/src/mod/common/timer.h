#ifndef SRC_MOD_NAT64_TIMER_H_
#define SRC_MOD_NAT64_TIMER_H_

/**
 * @file
 * An all-purpose timer used to trigger some of Jool's events. Always runs, as
 * long as Jool is modprobed. At time of writing, this induces session and
 * fragment expiration.
 *
 * Why don't the session and fragment code manage their own timers?
 * Because that's more code and I don't see how it would improve anything.
 */

int jtimer_setup(void);
void jtimer_teardown(void);

#endif /* SRC_MOD_NAT64_TIMER_H_ */
