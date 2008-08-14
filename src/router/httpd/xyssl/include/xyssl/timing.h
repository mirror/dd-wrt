/**
 * \file timing.h
 */
#ifndef _TIMING_H
#define _TIMING_H

#ifdef __cplusplus
extern "C" {
#endif

extern int alarmed;

/**
 * \brief          timer structure
 */
struct hr_time
{
    unsigned char opaque[32];
};

/**
 * \brief          Return the CPU cycle counter value
 */
unsigned long hardclock( void );

/**
 * \brief          Return the elapsed time in milliseconds
 *
 * \param val      points to a timer structure
 * \param reset    if set to 1, the timer is restarted
 */
unsigned long set_timer( struct hr_time *val, int reset );

/**
 * \brief          Setup an alarm clock
 *
 * \param seconds  delay before the "alarmed" flag is set
 */
void set_alarm( int seconds );

#ifdef __cplusplus
}
#endif

#endif /* timing.h */
