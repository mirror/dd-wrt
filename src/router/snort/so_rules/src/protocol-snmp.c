//REQUIRES:ber
#include "sf_snort_plugin_api.h"
extern Rule rule17632;
extern Rule rule17699;
Rule *rules[] = {
    &rule17632,
    &rule17699,
    NULL
};
