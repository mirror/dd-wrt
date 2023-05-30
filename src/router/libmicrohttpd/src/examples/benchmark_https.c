/*
     This file is part of libmicrohttpd
     Copyright (C) 2007, 2013 Christian Grothoff (and other contributing authors)

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/**
 * @file benchmark_https.c
 * @brief minimal code to benchmark MHD GET performance with HTTPS
 * @author Christian Grothoff
 */

#include "platform.h"
#include <microhttpd.h>

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif /* HAVE_INTTYPES_H */
#ifndef PRIu64
#define PRIu64  "llu"
#endif /* ! PRIu64 */

#if defined(MHD_CPU_COUNT) && (MHD_CPU_COUNT + 0) < 2
#undef MHD_CPU_COUNT
#endif
#if ! defined(MHD_CPU_COUNT)
#define MHD_CPU_COUNT 2
#endif

#define PAGE \
  "<html><head><title>libmicrohttpd demo</title></head><body>libmicrohttpd demo</body></html>"


#define SMALL (1024 * 128)

/**
 * Number of threads to run in the thread pool.  Should (roughly) match
 * the number of cores on your system.
 */
#define NUMBER_OF_THREADS MHD_CPU_COUNT

static unsigned int small_deltas[SMALL];

static struct MHD_Response *response;


/**
 * Signature of the callback used by MHD to notify the
 * application about completed requests.
 *
 * @param cls client-defined closure
 * @param connection connection handle
 * @param con_cls value as set by the last call to
 *        the MHD_AccessHandlerCallback
 * @param toe reason for request termination
 * @see MHD_OPTION_NOTIFY_COMPLETED
 */
static void
completed_callback (void *cls,
                    struct MHD_Connection *connection,
                    void **con_cls,
                    enum MHD_RequestTerminationCode toe)
{
  struct timeval *tv = *con_cls;
  struct timeval tve;
  uint64_t delta;
  (void) cls;         /* Unused. Silent compiler warning. */
  (void) connection;  /* Unused. Silent compiler warning. */
  (void) toe;         /* Unused. Silent compiler warning. */

  if (NULL == tv)
    return;
  gettimeofday (&tve, NULL);

  delta = 0;
  if (tve.tv_usec >= tv->tv_usec)
    delta += (tve.tv_sec - tv->tv_sec) * 1000000LL
             + (tve.tv_usec - tv->tv_usec);
  else
    delta += (tve.tv_sec - tv->tv_sec) * 1000000LL
             - tv->tv_usec + tve.tv_usec;
  if (delta < SMALL)
    small_deltas[delta]++;
  else
    fprintf (stdout, "D: %" PRIu64 " 1\n", delta);
  free (tv);
}


static void *
uri_logger_cb (void *cls,
               const char *uri)
{
  struct timeval *tv = malloc (sizeof (struct timeval));
  (void) cls; /* Unused. Silent compiler warning. */
  (void) uri; /* Unused. Silent compiler warning. */

  if (NULL != tv)
    gettimeofday (tv, NULL);
  return tv;
}


static enum MHD_Result
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size, void **ptr)
{
  (void) cls;               /* Unused. Silent compiler warning. */
  (void) url;               /* Unused. Silent compiler warning. */
  (void) version;           /* Unused. Silent compiler warning. */
  (void) upload_data;       /* Unused. Silent compiler warning. */
  (void) upload_data_size;  /* Unused. Silent compiler warning. */
  (void) ptr;               /* Unused. Silent compiler warning. */

  if (0 != strcmp (method, "GET"))
    return MHD_NO;              /* unexpected method */
  return MHD_queue_response (connection, MHD_HTTP_OK, response);
}


/* test server key */
const char srv_signed_key_pem[] =
  "-----BEGIN PRIVATE KEY-----\n\
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCff7amw9zNSE+h\n\
rOMhBrzbbsJluUP3gmd8nOKY5MUimoPkxmAXfp2L0il+MPZT/ZEmo11q0k6J2jfG\n\
UBQ+oZW9ahNZ9gCDjbYlBblo/mqTai+LdeLO3qk53d0zrZKXvCO6sA3uKpG2WR+g\n\
+sNKxfYpIHCpanqBU6O+degIV/+WKy3nQ2Fwp7K5HUNj1u0pg0QQ18yf68LTnKFU\n\
HFjZmmaaopWki5wKSBieHivzQy6w+04HSTogHHRK/y/UcoJNSG7xnHmoPPo1vLT8\n\
CMRIYnSSgU3wJ43XBJ80WxrC2dcoZjV2XZz+XdQwCD4ZrC1ihykcAmiQA+sauNm7\n\
dztOMkGzAgMBAAECggEAIbKDzlvXDG/YkxnJqrKXt+yAmak4mNQuNP+YSCEdHSBz\n\
+SOILa6MbnvqVETX5grOXdFp7SWdfjZiTj2g6VKOJkSA7iKxHRoVf2DkOTB3J8np\n\
XZd8YaRdMGKVV1O2guQ20Dxd1RGdU18k9YfFNsj4Jtw5sTFTzHr1P0n9ybV9xCXp\n\
znSxVfRg8U6TcMHoRDJR9EMKQMO4W3OQEmreEPoGt2/+kMuiHjclxLtbwDxKXTLP\n\
pD0gdg3ibvlufk/ccKl/yAglDmd0dfW22oS7NgvRKUve7tzDxY1Q6O5v8BCnLFSW\n\
D+z4hS1PzooYRXRkM0xYudvPkryPyu+1kEpw3fNsoQKBgQDRfXJo82XQvlX8WPdZ\n\
Ts3PfBKKMVu3Wf8J3SYpuvYT816qR3ot6e4Ivv5ZCQkdDwzzBKe2jAv6JddMJIhx\n\
pkGHc0KKOodd9HoBewOd8Td++hapJAGaGblhL5beIidLKjXDjLqtgoHRGlv5Cojo\n\
zHa7Viel1eOPPcBumhp83oJ+mQKBgQDC6PmdETZdrW3QPm7ZXxRzF1vvpC55wmPg\n\
pRfTRM059jzRzAk0QiBgVp3yk2a6Ob3mB2MLfQVDgzGf37h2oO07s5nspSFZTFnM\n\
KgSjFy0xVOAVDLe+0VpbmLp1YUTYvdCNowaoTE7++5rpePUDu3BjAifx07/yaSB+\n\
W+YPOfOuKwKBgQCGK6g5G5qcJSuBIaHZ6yTZvIdLRu2M8vDral5k3793a6m3uWvB\n\
OFAh/eF9ONJDcD5E7zhTLEMHhXDs7YEN+QODMwjs6yuDu27gv97DK5j1lEsrLUpx\n\
XgRjAE3KG2m7NF+WzO1K74khWZaKXHrvTvTEaxudlO3X8h7rN3u7ee9uEQKBgQC2\n\
wI1zeTUZhsiFTlTPWfgppchdHPs6zUqq0wFQ5Zzr8Pa72+zxY+NJkU2NqinTCNsG\n\
ePykQ/gQgk2gUrt595AYv2De40IuoYk9BlTMuql0LNniwsbykwd/BOgnsSlFdEy8\n\
0RQn70zOhgmNSg2qDzDklJvxghLi7zE5aV9//V1/ewKBgFRHHZN1a8q/v8AAOeoB\n\
ROuXfgDDpxNNUKbzLL5MO5odgZGi61PBZlxffrSOqyZoJkzawXycNtoBP47tcVzT\n\
QPq5ZOB3kjHTcN7dRLmPWjji9h4O3eHCX67XaPVMSWiMuNtOZIg2an06+jxGFhLE\n\
qdJNJ1DkyUc9dN2cliX4R+rG\n\
-----END PRIVATE KEY-----";

/* test server CA signed certificates */
const char srv_signed_cert_pem[] =
  "-----BEGIN CERTIFICATE-----\n\
MIIFSzCCAzOgAwIBAgIBBDANBgkqhkiG9w0BAQsFADCBgTELMAkGA1UEBhMCUlUx\n\
DzANBgNVBAgMBk1vc2NvdzEPMA0GA1UEBwwGTW9zY293MRswGQYDVQQKDBJ0ZXN0\n\
LWxpYm1pY3JvaHR0cGQxITAfBgkqhkiG9w0BCQEWEm5vYm9keUBleGFtcGxlLm9y\n\
ZzEQMA4GA1UEAwwHdGVzdC1DQTAgFw0yMjA0MjAxODQzMDJaGA8yMTIyMDMyNjE4\n\
NDMwMlowZTELMAkGA1UEBhMCUlUxDzANBgNVBAgMBk1vc2NvdzEPMA0GA1UEBwwG\n\
TW9zY293MRswGQYDVQQKDBJ0ZXN0LWxpYm1pY3JvaHR0cGQxFzAVBgNVBAMMDnRl\n\
c3QtbWhkc2VydmVyMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAn3+2\n\
psPczUhPoazjIQa8227CZblD94JnfJzimOTFIpqD5MZgF36di9IpfjD2U/2RJqNd\n\
atJOido3xlAUPqGVvWoTWfYAg422JQW5aP5qk2ovi3Xizt6pOd3dM62Sl7wjurAN\n\
7iqRtlkfoPrDSsX2KSBwqWp6gVOjvnXoCFf/list50NhcKeyuR1DY9btKYNEENfM\n\
n+vC05yhVBxY2ZpmmqKVpIucCkgYnh4r80MusPtOB0k6IBx0Sv8v1HKCTUhu8Zx5\n\
qDz6Nby0/AjESGJ0koFN8CeN1wSfNFsawtnXKGY1dl2c/l3UMAg+GawtYocpHAJo\n\
kAPrGrjZu3c7TjJBswIDAQABo4HmMIHjMAsGA1UdDwQEAwIFoDAMBgNVHRMBAf8E\n\
AjAAMBYGA1UdJQEB/wQMMAoGCCsGAQUFBwMBMDEGA1UdEQQqMCiCDnRlc3QtbWhk\n\
c2VydmVyhwR/AAABhxAAAAAAAAAAAAAAAAAAAAABMB0GA1UdDgQWBBQ57Z06WJae\n\
8fJIHId4QGx/HsRgDDAoBglghkgBhvhCAQ0EGxYZVGVzdCBsaWJtaWNyb2h0dHBk\n\
IHNlcnZlcjARBglghkgBhvhCAQEEBAMCBkAwHwYDVR0jBBgwFoAUWHVDwKVqMcOF\n\
Nd0arI3/QB3W6SwwDQYJKoZIhvcNAQELBQADggIBAI7Lggm/XzpugV93H5+KV48x\n\
X+Ct8unNmPCSzCaI5hAHGeBBJpvD0KME5oiJ5p2wfCtK5Dt9zzf0S0xYdRKqU8+N\n\
aKIvPoU1hFixXLwTte1qOp6TviGvA9Xn2Fc4n36dLt6e9aiqDnqPbJgBwcVO82ll\n\
HJxVr3WbrAcQTB3irFUMqgAke/Cva9Bw79VZgX4ghb5EnejDzuyup4pHGzV10Myv\n\
hdg+VWZbAxpCe0S4eKmstZC7mWsFCLeoRTf/9Pk1kQ6+azbTuV/9QOBNfFi8QNyb\n\
18jUjmm8sc2HKo8miCGqb2sFqaGD918hfkWmR+fFkzQ3DZQrT+eYbKq2un3k0pMy\n\
UySy8SRn1eadfab+GwBVb68I9TrPRMrJsIzysNXMX4iKYl2fFE/RSNnaHtPw0C8y\n\
B7memyxPRl+H2xg6UjpoKYh3+8e44/XKm0rNIzXjrwA8f8gnw2TbqmMDkj1YqGnC\n\
SCj5A27zUzaf2pT/YsnQXIWOJjVvbEI+YKj34wKWyTrXA093y8YI8T3mal7Kr9YM\n\
WiIyPts0/aVeziM0Gunglz+8Rj1VesL52FTurobqusPgM/AME82+qb/qnxuPaCKj\n\
OT1qAbIblaRuWqCsid8BzP7ZQiAnAWgMRSUg1gzDwSwRhrYQRRWAyn/Qipzec+27\n\
/w0gW9EVWzFhsFeGEssi\n\
-----END CERTIFICATE-----";


int
main (int argc, char *const *argv)
{
  struct MHD_Daemon *d;
  unsigned int i;

  if (argc != 2)
  {
    printf ("%s PORT\n", argv[0]);
    return 1;
  }
  response = MHD_create_response_from_buffer (strlen (PAGE),
                                              (void *) PAGE,
                                              MHD_RESPMEM_PERSISTENT);
  d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_TLS
#ifdef EPOLL_SUPPORT
                        | MHD_USE_EPOLL | MHD_USE_TURBO
#endif
                        ,
                        atoi (argv[1]),
                        NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
                        MHD_OPTION_THREAD_POOL_SIZE, (unsigned
                                                      int) NUMBER_OF_THREADS,
                        MHD_OPTION_URI_LOG_CALLBACK, &uri_logger_cb, NULL,
                        MHD_OPTION_NOTIFY_COMPLETED, &completed_callback, NULL,
                        MHD_OPTION_CONNECTION_LIMIT, (unsigned int) 1000,
                        /* Optionally, the gnutls_load_file() can be used to
                           load the key and the certificate from file. */
                        MHD_OPTION_HTTPS_MEM_KEY, srv_signed_key_pem,
                        MHD_OPTION_HTTPS_MEM_CERT, srv_signed_cert_pem,
                        MHD_OPTION_END);
  if (d == NULL)
    return 1;
  (void) getc (stdin);
  MHD_stop_daemon (d);
  MHD_destroy_response (response);
  for (i = 0; i < SMALL; i++)
    if (0 != small_deltas[i])
      fprintf (stdout, "D: %u %u\n", i, small_deltas[i]);
  return 0;
}
