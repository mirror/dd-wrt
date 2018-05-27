#ifdef SNORT_RELOAD

#ifndef __appdata_adjuster__
#define __appdata_adjuster__

#include "sf_types.h"
#include "sfPolicy.h"

typedef struct _appdata_adjuster APPDATA_ADJUSTER;

typedef size_t (*PreprocMemUsedFunc)();

APPDATA_ADJUSTER * ada_init( PreprocMemUsedFunc totalPreprocMemInUse, uint32_t protocol, size_t preproc_memcap );

void ada_delete( APPDATA_ADJUSTER *a );

void ada_add( APPDATA_ADJUSTER *a, void *appData, void *scb );

void ada_appdata_freed( APPDATA_ADJUSTER *a, void *appData );

bool ada_reload_adjust_func( bool idle, tSfPolicyId raPolicyId, void *userData );

struct _SnortConfig;
int ada_reload_adjust_register( APPDATA_ADJUSTER *a, tSfPolicyId policy_id, struct _SnortConfig *snortConfig, const char *raName, size_t new_cap );

void ada_set_new_cap( APPDATA_ADJUSTER *a, size_t new_cap );

int ada_reload_disable( APPDATA_ADJUSTER **aPointer, struct _SnortConfig *snortConfig, const char *raName, tSfPolicyId policy_id);

#endif

#endif
