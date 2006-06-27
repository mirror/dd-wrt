#ifndef _IPT_CHILDLEVEL_H
#define _IPT_CHILDLEVEL_H

typedef char *(*proc_ipt_search) (int, char);

struct ipt_childlevel_info {
    int childlevel; 
    char invert;
};

#endif /* _IPT_CHILDLEVEL_H */
