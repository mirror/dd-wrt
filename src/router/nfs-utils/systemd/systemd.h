#ifndef SYSTEMD_H
#define SYSTEMD_H

char *systemd_escape(char *path, char *suffix);
int systemd_in_initrd(void);

#endif /* SYSTEMD_H */
