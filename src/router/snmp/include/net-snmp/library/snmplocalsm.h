/*
 * Header file for Local Security Model support
 */

#ifndef SNMPLOCALSM_H
#define SNMPLOCALSM_H

#ifdef __cplusplus
extern          "C" {
#endif

    int             localsm_rgenerate_out_msg(struct
                                          snmp_secmod_outgoing_params *);
    int             localsm_process_in_msg(struct snmp_secmod_incoming_params
                                       *);
    void            init_usm(void);

#ifdef __cplusplus
}
#endif
#endif                          /* SNMPLOCALSM_H */
