#ifndef __H2_PAF_H__
#define __H2_PAF_H__

#include "sfPolicy.h"
#include "sf_types.h"

int h2_paf_register_port(struct _SnortConfig *sc, uint16_t port, bool client, bool server, tSfPolicyId pid, bool auto_on);
int h2_paf_register_service(struct _SnortConfig *, uint16_t service, bool client, bool server, tSfPolicyId pid, bool auto_on);

#endif
