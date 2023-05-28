/*
     This file is part of libmicrohttpd
     Copyright (C) 2007 Christian Grothoff (and other contributing authors)

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
 * @file minimal_example_empty_ssl.c
 * @brief minimal example for how to use libmicrohttpd
 * @author Christian Grothoff
 */

#include "platform.h"
#include <microhttpd.h>


static enum MHD_Result
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data,
          size_t *upload_data_size,
          void **ptr)
{
  static int aptr;
  struct MHD_Response *response;
  enum MHD_Result ret;

  (void) cls;               /* Unused. Silent compiler warning. */
  (void) url;               /* Unused. Silent compiler warning. */
  (void) version;           /* Unused. Silent compiler warning. */
  (void) upload_data;       /* Unused. Silent compiler warning. */
  (void) upload_data_size;  /* Unused. Silent compiler warning. */

  if (0 != strcmp (method, "GET"))
    return MHD_NO;              /* unexpected method */
  if (&aptr != *ptr)
  {
    /* do never respond on first call */
    *ptr = &aptr;
    return MHD_YES;
  }
  *ptr = NULL;                  /* reset when done */
  response = MHD_create_response_from_buffer (0,
                                              NULL,
                                              MHD_RESPMEM_PERSISTENT);
  ret = MHD_queue_response (connection,
                            MHD_HTTP_NO_CONTENT,
                            response);
  MHD_destroy_response (response);
  return ret;
}


/* test server key */
const char srv_signed_key_pem[] =
  "-----BEGIN PRIVATE KEY-----\n\
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCrm8uH8V0P2Xbl\n\
HCMq6PTIphDcMmXEDiciCAAbfS7rVUyEJBgRpKSb+IXpj6jX1+e0+uncBxO9fesZ\n\
Vyg4ksOA7jITGlzOjBvX6RzxkE9I96R3RCgzUHyYB8ayVSj1s/WmhorVPhKQjCmd\n\
6mi978n//bZKqTpq3OMpDrNC6AWTiHHP5KV9pp3Hsz1iAGK74sSVP3vhYD8IZ5yg\n\
CR99TDDHZfIvOmbqS60kx/UclUf2R4mSv/ZZHaHW7PeUhtUxzwOYqWVj0zLv/Lni\n\
CGO9uXGOgZHFfL8PhQwt6pNT5DaVmqx/uGwpsLiER4P74ngwroSjMwavYNlykuLF\n\
1N8GZZC7AgMBAAECggEAc0F/wR3qUurLX7U2KWuse9aNHFb84mBfCAw3hj7ddFEl\n\
wto7EB50MA0KY4OI8u6fQH4E8zINoAciDzLqYJSxmbZhC1N5YX/Yc3qtZdB2b5tj\n\
anbsSQqVo8YVPVDU4bCsG9vhArdd4JdCnD0DfA3ArZ3JAPwHsB4Ks1icLSOIGz0/\n\
JvOZEryJBdwM6SKbzLMqVOGmYDiY6s7UpJ0rg3cOPqhdg5xv8XZATqXISU0mLBcq\n\
RiS7lHZERASYON2rpznhBiCtikOcr/duQhvZ1uDSGfDzDJil+1hdS3RouS9WZCIe\n\
p3CtvZhPLmv6kFg9YE+AovDwOOwNr0no3H9oJA2FgQKBgQDSWrE/MRMRpFJFBxxC\n\
YckC2v8Y+7sVSMbFNq/0j2eRTql+8AeZBbAoGU4QHUcylCBkv33zDYRY52xNo32E\n\
8mmH2O/pIcYy0LafrVZHdulf+fxybncObidxmmjR9C8aLzwRuIMtABz4simaQcBD\n\
RhZJ1YCqVkfMr/PlbLzvC8V+FwKBgQDQ2MF/Yz/p7QEDHpfKdtx7+yK6i8IM+V8l\n\
d2OuscNkQQywCVqE2vyRZJbjU9y+Om7alNKFPhhBzavdOxNWXGXmlBIlvo3v6M++\n\
fTixza77LxHvbghH0ykplSwGh30vpHtvoxsRS5nRFxmsVK9jNYYT/Aes+J6MXlq7\n\
PYAiZVQs/QKBgFKYY8JhPZCOyfLqsNDr3matoL6pkTLxSYMETyCi8lKe5XS/QOx3\n\
zExia0FujZcxjGqiugymgRH7hI4TpOR/3qoFp2YN6enoA908zYTwDwCtgs9Xyo2y\n\
+O/lZkUSMTCB3X9DyNXxlm6cXjOAn8KKkZPaLlQz3qtjZ0vtX14pbBlvAoGBAKw0\n\
vsCifvYNZhNDa5gXkFBu0MEPMm/uQ+Up37kRfPKyrJqO6+O2iiH81moWIWN93SBB\n\
LKGPhQLlazxdVOGWCLQrDhevW2wiBQKmUFRULF+T/W72xL8sv7k49ndfyvq43ss7\n\
q7sEIo4FRTcTERd179uUqmOXEWze9GOGH5y8/r6lAoGAG2YyqRWF+yxKlgR71b1Q\n\
Zxv53WgXOUwemGRbXxE4g3gHpW5k9zWh4QTkbd0lDD+SQ0DBwZl/x/FWj43jUS+i\n\
a5UojDUx8nYgjiAO7kppMlX3ZaJD1DkwEz+4HW9oPmOFt8smvuTVt0mm6tpmQdRA\n\
yLwgQzGDGVJB6ETVJS7cwWs=\n\
-----END PRIVATE KEY-----";

/* test server CA signed certificates */
const char srv_signed_cert_pem[] =
  "-----BEGIN CERTIFICATE-----\n\
MIIFaTCCA1GgAwIBAgIBATANBgkqhkiG9w0BAQsFADCBgTELMAkGA1UEBhMCUlUx\n\
DzANBgNVBAgMBk1vc2NvdzEPMA0GA1UEBwwGTW9zY293MRswGQYDVQQKDBJ0ZXN0\n\
LWxpYm1pY3JvaHR0cGQxITAfBgkqhkiG9w0BCQEWEm5vYm9keUBleGFtcGxlLm9y\n\
ZzEQMA4GA1UEAwwHdGVzdC1DQTAgFw0yMTA0MDcxNzM2MjFaGA8yMTIxMDMxMzE3\n\
MzYyMVowgYcxCzAJBgNVBAYTAlJVMQ8wDQYDVQQIDAZNb3Njb3cxDzANBgNVBAcM\n\
Bk1vc2NvdzEbMBkGA1UECgwSdGVzdC1saWJtaWNyb2h0dHBkMRQwEgYDVQQDDAt0\n\
ZXN0LXNlcnZlcjEjMCEGA1UdEQwaRE5TOmxvY2FsaG9zdCxJUDoxMjcuMC4wLjEw\n\
ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCrm8uH8V0P2XblHCMq6PTI\n\
phDcMmXEDiciCAAbfS7rVUyEJBgRpKSb+IXpj6jX1+e0+uncBxO9fesZVyg4ksOA\n\
7jITGlzOjBvX6RzxkE9I96R3RCgzUHyYB8ayVSj1s/WmhorVPhKQjCmd6mi978n/\n\
/bZKqTpq3OMpDrNC6AWTiHHP5KV9pp3Hsz1iAGK74sSVP3vhYD8IZ5ygCR99TDDH\n\
ZfIvOmbqS60kx/UclUf2R4mSv/ZZHaHW7PeUhtUxzwOYqWVj0zLv/LniCGO9uXGO\n\
gZHFfL8PhQwt6pNT5DaVmqx/uGwpsLiER4P74ngwroSjMwavYNlykuLF1N8GZZC7\n\
AgMBAAGjgeEwgd4wCwYDVR0PBAQDAgWgMAwGA1UdEwEB/wQCMAAwFgYDVR0lAQH/\n\
BAwwCgYIKwYBBQUHAwEwLAYDVR0RBCUwI4IJbG9jYWxob3N0hwR/AAABhxAAAAAA\n\
AAAAAAAAAAAAAAABMB0GA1UdDgQWBBRifkHd2xWI51NhnH8sL66K8EedpjAoBglg\n\
hkgBhvhCAQ0EGxYZVGVzdCBsaWJtaWNyb2h0dHBkIHNlcnZlcjARBglghkgBhvhC\n\
AQEEBAMCBkAwHwYDVR0jBBgwFoAUWHVDwKVqMcOFNd0arI3/QB3W6SwwDQYJKoZI\n\
hvcNAQELBQADggIBAJoIyNrnwQ+7WcJDaBjjuwSH0ORuK+E3zRI3+nDde08gyfeG\n\
K4QozT2L574WzadTVLSiin9lRShYlp0nr60pmUb9+SKE0O7Cx+rYV0Rfu0KLYsYh\n\
sAkb9J9t1fdIt54fXNcUtvfGPyM2lEI0KxMCGNV2wXDnwzdSNIU6Nk457MntfZdi\n\
r1ISnS6fLd0BIKIGxfCFb10CexhNOSaExgpp1bxZovdYaQWggL0u8eC8j00sJ1C5\n\
Qo4gQ1TQsead6zMs6m19TPLlV7hS+hfXj7yeJ/TTUj69bCjTIMp6HCFnfQbD84BI\n\
HZDKk4Tob9vBRCKbY58kNXHyQ4nxvSCBlKI03VJjvzpsKTI/vW9JBivtnYtMbMl9\n\
ouZal/IVsNqRCeiMTLky62qrFhZr2DHgPG5VcOGQ4y0X4vOgM9n/MMOGWcNBByLX\n\
b5ZaYr7DPCcz9dYZgEbwXj8wnuAzM1sJ2igwTmO/vQsn1G2Q/h/JB471CD1avuuI\n\
awKRqhU2KhYVrwo7ahJkPV9Lm6eoavq2Tu+e1o4qAFhPLMy/6F+bZmK6GfHMvP+L\n\
v+GOQdUJ/vMMus/HB5N3cUZsu9rGnCCVgPW7pkHrp5bRtuVzBT78ISsxkGnOhfT7\n\
6Kp7ApvfEX6/Y/vbDFBC4kyAvEIZ+F8AUkbvZ0+k8j5xlarNd6TQ3slEGi6O\n\
-----END CERTIFICATE-----";


int
main (int argc,
      char *const *argv)
{
  struct MHD_Daemon *d;

  if (argc != 2)
  {
    printf ("%s PORT\n", argv[0]);
    return 1;
  }
  d = MHD_start_daemon (/* MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG, */
    MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG
    | MHD_USE_TLS,
    /* MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG | MHD_USE_POLL, */
    /* MHD_USE_THREAD_PER_CONNECTION | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG | MHD_USE_POLL, */
    /* MHD_USE_THREAD_PER_CONNECTION | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG, */
    atoi (argv[1]),
    NULL, NULL, &ahc_echo, NULL,
    MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
    MHD_OPTION_STRICT_FOR_CLIENT, (int) 1,
    /* Optionally, the gnutls_load_file() can be used to
       load the key and the certificate from file. */
    MHD_OPTION_HTTPS_MEM_KEY, srv_signed_key_pem,
    MHD_OPTION_HTTPS_MEM_CERT, srv_signed_cert_pem,
    MHD_OPTION_END);
  if (d == NULL)
    return 1;
  (void) getc (stdin);
  MHD_stop_daemon (d);
  return 0;
}
