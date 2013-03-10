#ifndef TTY_H_INCLUDED
#define TTY_H_INCLUDED

int opentty (const char * dev);
void closetty(const char * dev, int fd);
size_t write_all (int fd, const char* buf, size_t count);

#endif /* TTY_H_INCLUDED */
