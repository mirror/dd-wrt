#ifndef __AGENTX_CONFIG_H__
#define __AGENTX_CONFIG_H__

#ifdef __cplusplus
extern          "C" {
#endif

    void            agentx_parse_master(const char *token, char *cptr);
    void            agentx_parse_agentx_socket(const char *token,
                                               char *cptr);
    void            init_agentx_config(void);

#ifdef __cplusplus
}
#endif
#endif                          /* __AGENTX_CONFIG_H__ */
