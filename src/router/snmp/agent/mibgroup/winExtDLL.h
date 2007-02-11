/*
 * Don't include ourselves twice 
 */
#ifndef _WINEXTDLL_H
#define _WINEXTDLL_H

#ifdef __cplusplus
extern "C" {
#endif

    /*
     * We use 'header_generic' from the util_funcs module,
     *  so make sure this module is included in the agent.
     */
config_require(util_funcs)


    /*
     * Declare our publically-visible functions.
     * Typically, these will include the initialization and shutdown functions,
     *  the main request callback routine and any writeable object methods.
     *
     * Function prototypes are provided for the callback routine ('FindVarMethod')
     *  and writeable object methods ('WriteMethod').
     */
     void     init_winExtDLL(void);

     Netsnmp_Node_Handler var_winExtDLL;
     
#ifdef __cplusplus
}
#endif

#endif                          /* _WINEXTDLL_H */
