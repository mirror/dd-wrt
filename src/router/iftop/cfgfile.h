/*
 * cfgfile.h:
 *
 * Copyright (c) 2003 DecisionSoft Ltd.
 *
 */

#ifndef __CFGFILE_H_ /* include guard */
#define __CFGFILE_H_

typedef struct {
    char *name;
    int value;
} config_enumeration_type;

int read_config();

char *config_get_string(const char *directive);
int config_get_bool(const char *directive);
int config_get_int(const char *directive, int *value);
int config_get_float(const char *directive, float *value);
int config_init();



#endif /* __CFGFILE_H_ */
