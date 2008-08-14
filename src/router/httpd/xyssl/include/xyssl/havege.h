/**
 * \file havege.h
 */
#ifndef _HAVEGE_H
#define _HAVEGE_H

#ifdef __cplusplus
extern "C" {
#endif

#define COLLECT_SIZE 1024

/**
 * \brief          HAVEGE state structure
 */
typedef struct
{
    int WALK[8192];
    int pool[COLLECT_SIZE];
    int PT1, PT2, offset;
}
havege_state;

/**
 * \brief          HAVEGE initialization
 *
 * \param hs       HAVEGE state to be initialized
 */
void havege_init( havege_state *hs );

/**
 * \brief          HAVEGE rand function
 *
 * \param rng_st   points to an HAVEGE state
 *
 * \return         A random int
 */
int havege_rand( void *rng_d );

#ifdef __cplusplus
}
#endif

#endif /* havege.h */
