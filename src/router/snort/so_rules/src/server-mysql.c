#include "sf_snort_plugin_api.h"
extern Rule ruleMYSQL_COM_TABLE_DUMP;
Rule *rules[] = {
    &ruleMYSQL_COM_TABLE_DUMP,
    NULL
};
