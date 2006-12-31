/* dispatch.h - Event dispatcher */

/* Written 1998 by Werner Almesberger, EPFL ICA */


#ifndef DISPATCH_H
#define DISPATCH_H

/*
 * Add a file descriptor to probe for reading in the central dispatcher. If
 * the FD if readable, the callback function is invoked. The dispatcher does
 * not attempt to read itself.
 */

void dsp_fd_add(int fd,void (*callback)(int fd,void *user),void *user);

/*
 * Remove a file descriptor from the central dispatcher. This function can be
 * invoked from within a callback function.
 */

void dsp_fd_remove(int fd);

/*
 * Initialize the dispatcher.
 */

void dsp_init(void);

/*
 * Wait until one of the file descriptors becomes readable and execute the
 * callback function. dsp_poll only handles one event at a time.
 */

void dsp_poll(void);

#endif
