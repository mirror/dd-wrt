# include <stdio.h> 
struct mime_type_t {
    char *ext;
    char *type;
};

extern struct mime_type_t mime_types[];

