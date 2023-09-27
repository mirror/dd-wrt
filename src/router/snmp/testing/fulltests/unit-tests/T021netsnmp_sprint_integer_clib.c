/* HEADER Test sprint_integer() */
/* See also https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=59491. */

static const struct {
    long val;
    const char *hint;
    const char *result;
} test[] = {
    { 19, "b", "INTEGER: 00000000000000000000000000010011" },
    { 19, "d", "INTEGER: 19" },
    { 19, "o", "INTEGER: 23" },
    { 19, "x", "INTEGER: 13" },
    { 19, "f", NULL },
    { 19, "s", NULL },
    { 19, "p", NULL },
};
char *buf;
size_t buf_len;
char mibdir[PATH_MAX];
netsnmp_variable_list variable;
long input_value;
int i, ret;

snprintf(mibdir, sizeof(mibdir), "%s/%s", ABS_SRCDIR, "mibs");
netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIBDIRS, mibdir);
netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT, TRUE);

init_snmp("T021");

buf_len = 100;
buf = malloc(buf_len);
memset(&variable, 0, sizeof(variable));
variable.type = ASN_INTEGER;
variable.val.integer = &input_value;
variable.val_len = sizeof(variable.val.integer);

for (i = 0; i < sizeof(test) / sizeof(test[0]); i++) {
    input_value = test[i].val;

    ret = snprint_integer(buf, buf_len, &variable, /*enums=*/NULL,
                          test[i].hint, /*units=*/NULL);
    if (test[i].result) {
        OKF(ret > 0, ("[%d] sprint_integer() returned %d; expected > 0", i, ret));
        if (ret > 0)
            OKF(strcmp(buf, test[i].result) == 0,
                ("%s <> %s", buf, test[i].result));
    } else {
        OKF(ret < 0, ("[%d] sprint_integer() failed", i));
    }
}

free(buf);
snmp_shutdown("T021");
