#ifndef POLL2SELECT_H
#define POLL2SELECT_H

/*
 * poll2select() -- function to convert poll() to select()
 * until poll() gets fixed and is usable again
 */

int poll2select(struct pollfd *fds, int fds_used, int timeout);

#endif /* POLL2SELECT_H */
