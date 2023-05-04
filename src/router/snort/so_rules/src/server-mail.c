//REQUIRES:base64-decode
#include "sf_snort_plugin_api.h"
extern Rule rule13718;
extern Rule ruleEXCHANGE_BASE64_DECODE;
extern Rule rule15329;
extern Rule rule42438;
extern Rule rule15301;
extern Rule rule17693;
extern Rule rule13921;
Rule *rules[] = {
    &rule13718,
    &ruleEXCHANGE_BASE64_DECODE,
    &rule15329,
    &rule42438,
    &rule15301,
    &rule17693,
    &rule13921,
    NULL
};
