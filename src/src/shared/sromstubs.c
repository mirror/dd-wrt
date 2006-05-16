/*
 * Should be called bcmsromstubs.c .
 *
 * $Id$
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmsrom.h>

int
srom_var_init(void *sbh, uint bus, void *curmap, osl_t *osh, char **vars, int *count)
{
	return 0;
}

int
srom_read(uint bus, void *curmap, osl_t *osh, uint byteoff, uint nbytes, uint16 *buf)
{
	return 0;
}

int
srom_write(uint bus, void *curmap, osl_t *osh, uint byteoff, uint nbytes, uint16 *buf)
{
	return 0;
}
