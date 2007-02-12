/*
 * ucdDemoPublic.h 
 */

#ifndef _MIBGROUP_UCDDEMOPUBLIC_H
#define _MIBGROUP_UCDDEMOPUBLIC_H

/*
 * we use header_generic and checkmib from the util_funcs module 
 */

config_require(util_funcs)

    /*
     * Magic number definitions: 
     */
#define   UCDDEMORESETKEYS      1
#define   UCDDEMOPUBLICSTRING   2
#define   UCDDEMOUSERLIST       3
#define   UCDDEMOPASSPHRASE     4
    /*
     * function definitions 
     */
     extern void     init_ucdDemoPublic(void);
     extern FindVarMethod var_ucdDemoPublic;
     WriteMethod     write_ucdDemoResetKeys;
     WriteMethod     write_ucdDemoPublicString;


#endif                          /* _MIBGROUP_UCDDEMOPUBLIC_H */
