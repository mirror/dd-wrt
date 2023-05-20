/*
 * HEADER Testing read_config()
 */

static const unsigned char data[] = { 0xff, 0x20, 0xff };
char *path;
FILE *f;
int res;

register_mib_handlers();

res = asprintf(&path, "/tmp/read-config-input-%d", getpid());
OKF(res >= 0, ("asprintf() returned %d", res));
f = fopen(path, "wb");
OKF(f != NULL, ("fopen() %s", f ? "succeeded" : "failed"));
res = fwrite(data, sizeof(data), 1, f);
OKF(res == 1, ("fwrite() %s", res == 1 ? "succeeded" : "failed"));
fclose(f);
read_config(path, read_config_get_handlers("snmp"), 0);
unlink(path);
free(path);
