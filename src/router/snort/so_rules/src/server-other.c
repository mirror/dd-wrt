//REQUIRES:ber
#include "sf_snort_plugin_api.h"
extern Rule rule37675;
extern Rule ruleCITRIX_METAFRAME_BO;
extern Rule rule26972;
extern Rule rule15148;
extern Rule ruleNOVELL_EVENTSREQUEST;
extern Rule ruleNOVELL_EVENTSREQUEST_FREE;
extern Rule rule41547;
extern Rule rule41548;
extern Rule rule34967;
extern Rule rule20135;
extern Rule rule36153;
extern Rule ruleIMAIL_LDAP;
extern Rule rule15474;
extern Rule rule15968;
extern Rule rule16375;
extern Rule rule34971;
extern Rule rule34972;
extern Rule rule27906;
extern Rule rule17741;
extern Rule rule15973;
extern Rule ruleOPENLDAP_BIND_DOS;
extern Rule rule31361;
extern Rule rule18101;
extern Rule rule13418;
Rule *rules[] = {
    &rule37675,
    &ruleCITRIX_METAFRAME_BO,
    &rule26972,
    &rule15148,
    &ruleNOVELL_EVENTSREQUEST,
    &ruleNOVELL_EVENTSREQUEST_FREE,
    &rule41547,
    &rule41548,
    &rule34967,
    &rule20135,
    &rule36153,
    &ruleIMAIL_LDAP,
    &rule15474,
    &rule15968,
    &rule16375,
    &rule34971,
    &rule34972,
    &rule27906,
    &rule17741,
    &rule15973,
    &ruleOPENLDAP_BIND_DOS,
    &rule31361,
    &rule18101,
    &rule13418,
    NULL
};
