#ifdef SNORT_RELOAD

#include "appdata_adjuster.h"
#include "sfxhash.h"

#ifdef REG_TEST
#include "reg_test.h"
#endif

//SharedObjectDeleteBegins
#include "reload.h"
#include "preprocessors/session_api.h"
//SharedObjectDeleteEnds
/*SharedObjectAddStarts
#include "sf_dynamic_preprocessor.h"
SharedObjectAddEnds */

///////////////////////////////////////////////////


#define HASH_ROWS 1024
#define IDLE_SPEED 512
#define REGULAR_SPEED 32

struct _appdata_adjuster{
    SFXHASH *h;
    uint32_t protocol_id;
    PreprocMemUsedFunc totalPreprocMemInUse;
    size_t  new_cap;
};

//preproc_memcap may be used in the future for hash optimization
APPDATA_ADJUSTER * ada_init( PreprocMemUsedFunc totalPreprocMemInUse, uint32_t protocol_id, size_t preproc_memcap )
{
    APPDATA_ADJUSTER *a;
    a = calloc(1,sizeof(APPDATA_ADJUSTER));
    if (!a)
        return NULL;

    SFXHASH *h = sfxhash_new(HASH_ROWS, sizeof(void *), 0, 0, 0, NULL, NULL, 0);
    if (!h)
    {
        free(a);
        return NULL;
    }

    a->h = h;
    a->protocol_id = protocol_id;
    a->totalPreprocMemInUse = totalPreprocMemInUse;

    return a;
}

void ada_delete( APPDATA_ADJUSTER *a )
{
    if (!a)
        return;

    sfxhash_delete(a->h);
    free(a);
}

void ada_add( APPDATA_ADJUSTER *a, void *appData, void *scb )
{
    if (!a || !appData || !scb)
        return;

    sfxhash_add(a->h, &appData, scb);
}

void ada_appdata_freed( APPDATA_ADJUSTER *a, void *appData )
{
    if (!a || !appData)
        return;
    sfxhash_remove(a->h, &appData);
}

static void delete_app_data( SFXHASH *h, uint32_t protocol_id )
{
    SFXHASH_NODE *node = sfxhash_lru_node(h);
    if (!node)
        return;

    session_api->set_application_data(node->data,protocol_id,NULL,NULL);
}

bool ada_reload_adjust_func( bool idle, tSfPolicyId raPolicyId, void *userData )
{
    unsigned work_done;
    unsigned speed = idle ? IDLE_SPEED : REGULAR_SPEED;
    if (userData == NULL)
        return false;
    APPDATA_ADJUSTER *a = (APPDATA_ADJUSTER *) userData;

#ifdef REG_TEST
    if (REG_TEST_FLAG_APPDATA_ADJUSTER_RELOAD & getRegTestFlags())
    {
        printf("ada_reload_adjust func newmemcap %zu\n", a->new_cap);
        printf("ada_reload_adjust_func meminuse-before %zu\n", a->totalPreprocMemInUse());
    }
#endif

    for (work_done = 0; work_done<speed && sfxhash_ghead(a->h) && a->new_cap < a->totalPreprocMemInUse(); work_done++)
    {
        delete_app_data(a->h, a->protocol_id);
    }

#ifdef REG_TEST
    if (REG_TEST_FLAG_APPDATA_ADJUSTER_RELOAD & getRegTestFlags())
    {
        printf("ada_reload_adjust_func meminuse-after %zu\n", a->totalPreprocMemInUse());
        printf("ada_reload_adjust_func ghead %d\n", sfxhash_ghead(a->h) != 0);
    }
#endif

    bool done_deleting_sessions = a->new_cap >= a->totalPreprocMemInUse() || !sfxhash_ghead(a->h);
#ifdef REG_TEST
    if (done_deleting_sessions && REG_TEST_FLAG_APPDATA_ADJUSTER_RELOAD & getRegTestFlags())
    {
        printf("ada_reload_adjust_func done 1\n");
    }
#endif
    return done_deleting_sessions;
}

static bool ada_reload_adjust_func_disable( bool idle, tSfPolicyId raPolicyId, void *userData )
{
    APPDATA_ADJUSTER **adaPointer = (APPDATA_ADJUSTER **) userData;
    APPDATA_ADJUSTER *ada = *adaPointer;
    if (ada_reload_adjust_func(idle, raPolicyId, ada))
    {
        ada_delete(ada);
        *adaPointer = NULL;
        return true;
    }
    return false;
}

int ada_reload_adjust_register( APPDATA_ADJUSTER *a, tSfPolicyId policy_id, struct _SnortConfig *snortConfig, const char *raName, size_t new_cap )
{
    if (!a || !snortConfig || !raName)
        return -1;
    a->new_cap = new_cap;
    return ReloadAdjustRegister(snortConfig, raName, policy_id, ada_reload_adjust_func, (void *) a, NULL);
}

void ada_set_new_cap( APPDATA_ADJUSTER *a, size_t new_cap )
{
    if (!a)
        return;
    a->new_cap = new_cap;
}

//in a preprocessor, if you have APPDATA_ADJUSTER *ada = ada_init(....
//then when you call ada_reload_disable(&ada,....
int ada_reload_disable( APPDATA_ADJUSTER **aPointer, struct _SnortConfig *snortConfig, const char *raName, tSfPolicyId policy_id)
{
    if (!aPointer || !(*aPointer)|| !snortConfig || !raName)
        return -1;

    (*aPointer)->new_cap = 0;
    return ReloadAdjustRegister(snortConfig, raName, policy_id, ada_reload_adjust_func_disable, (void *) aPointer, NULL);
}

#endif

