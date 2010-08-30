#ifndef SEQNUM_H
#define SEQNUM_H 1
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SYSFS_SEQNUM_PATH "/sys/kernel/uevent_seqnum"

#define event_seqnum_t uint64_t

int seqnum_get(event_seqnum_t *);

#endif /* ifndef SEQNUM_H */

