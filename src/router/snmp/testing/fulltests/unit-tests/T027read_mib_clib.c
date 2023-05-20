/*
 * HEADER Testing read_mib()
 *
 * See also https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=37187.
 */

static const char mib[226] =
    "qu::=Opaque(SIZE(SI4-\n"
    "--\n"
    "--\n"
    "--\n"
    "--\n"
    "--\n"
    "--\n"
    "--\n"
    "OCTETSTRING\n"
    "----\n"
    "--\n"
    "--\n"
    "---\n"
    "--\n"
    "--\n"
    "-68--------------------1844674407370954460124----DD-IIIIIIIIIII0NZe9223300000446069494I32768g-I2768g-I3gg-I4g-55I420RPRISEOONOAO:\x001\0\0---::=\0||231\0\0\0\x001=O";
char *path;
FILE *f;
int res;

res = asprintf(&path, "/tmp/read-mib-input-%d", getpid());
OKF(res >= 0, ("asprintf() returned %d", res));
f = fopen(path, "wb");
OKF(f != NULL, ("fopen() %s", f ? "succeeded" : "failed"));
res = fwrite(mib, sizeof(mib), 1, f);
OKF(res == 1, ("fwrite() %s", res == 1 ? "succeeded" : "failed"));
fclose(f);
read_mib(path);
unlink(path);
free(path);
