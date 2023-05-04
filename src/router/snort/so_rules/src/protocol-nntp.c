#include "sf_snort_plugin_api.h"
extern Rule ruleNNTP_XHDR_BO;
Rule *rules[] = {
    &ruleNNTP_XHDR_BO,
    NULL
};
