#ifndef IPTRAF_NG_INSTANCES_H
#define IPTRAF_NG_INSTANCES_H

/***

instances.h - header file for instances.c

***/

#ifndef MAIN_MODULE
extern int is_first_instance;
#endif

void mark_facility(char *tagfile, char *facility, char *ifptr);
void unmark_facility(char *tagfile, char *ifptr);
int facility_active(char *tagfile, char *ifptr);
int adjust_instance_count(char *countfile, int inc);
int get_instance_count(char *countfile);
int is_last_instance(void);
int first_active_facility(void);

#endif	/* IPTRAF_NG_INSTANCES_H */
