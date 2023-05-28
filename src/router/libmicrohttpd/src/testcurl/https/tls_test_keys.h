/*
     This file is part of libmicrohttpd
     Copyright (C) 2006, 2007, 2008 Christian Grothoff (and other contributing authors)

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

#ifndef MHD_TLS_TEST_KEYS_H
#define MHD_TLS_TEST_KEYS_H

/* Test Certificates */

/* Certificate Authority cert */
const char ca_cert_pem[] =
  "-----BEGIN CERTIFICATE-----\n\
MIIGITCCBAmgAwIBAgIBADANBgkqhkiG9w0BAQsFADCBgTELMAkGA1UEBhMCUlUx\n\
DzANBgNVBAgMBk1vc2NvdzEPMA0GA1UEBwwGTW9zY293MRswGQYDVQQKDBJ0ZXN0\n\
LWxpYm1pY3JvaHR0cGQxITAfBgkqhkiG9w0BCQEWEm5vYm9keUBleGFtcGxlLm9y\n\
ZzEQMA4GA1UEAwwHdGVzdC1DQTAgFw0yMTA0MDcxNzM2MThaGA8yMTIxMDMxNDE3\n\
MzYxOFowgYExCzAJBgNVBAYTAlJVMQ8wDQYDVQQIDAZNb3Njb3cxDzANBgNVBAcM\n\
Bk1vc2NvdzEbMBkGA1UECgwSdGVzdC1saWJtaWNyb2h0dHBkMSEwHwYJKoZIhvcN\n\
AQkBFhJub2JvZHlAZXhhbXBsZS5vcmcxEDAOBgNVBAMMB3Rlc3QtQ0EwggIiMA0G\n\
CSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDdaWupA4qZjCBNkJoJOm5xnCaizl36\n\
ZLUwp4xBL/YfXPWE3LkmAREiVI/YnAb8l6G7CJnz8dTsOJWkNXG6T1KVP5/2RvBI\n\
IaaaufRIAl7hEnj1j9E2hQlV2fxF2ZNhz+nqi0LqKV4LJSpclkXADf2FA9HsVRP/\n\
B7zYh+DP0fSU8V6bsu8XCeRGshroAPrc8rH8lFEEXpNLNIqQr8yKx6SmdB6hfja6\n\
6SQ0++qBhl0aJtn4LHWZohgjBmkIaGFPYIJLgxQ/xyp2Grz2q7lGKJ+zBkBF8iOP\n\
t3x+F1hSCBnr/DGYWmjEm5tYm+7pyuriPddXdCc8+qa2LxMZo3EXxLo5YISpPCyw\n\
Z7V3YAOZTr3m1C24LiYvPehCq1CTIkhhmqtlVJXU7ISD48cx9y+5Pi34wtbTI/gN\n\
x4voyTLAfyavKMmIpxxIRsWldiF2n06HdvCRVdihDQUad10ygTmWf1J/s2ZETAtH\n\
QaSd7MD389t6nQFtTIXigsNKnnDPlrtxt7rOLvLQeR0K04Gzrf/scheOanRAfOXH\n\
KNBFU7YkDFG8rqizlC65rx9qeXFYXQcHZTuqxK7tgZnSgJat3E70VbTSCsEEG7eR\n\
bNX/fChUKAIIpWaiW6HDlKLl6m2y+BzM91umBsKOqTvntMVFBSF9pVYlXK854aIR\n\
q8A2Xujd012seQIDAQABo4GfMIGcMAsGA1UdDwQEAwICpDASBgNVHRMBAf8ECDAG\n\
AQH/AgEBMB0GA1UdDgQWBBRYdUPApWoxw4U13Rqsjf9AHdbpLDATBgNVHSUEDDAK\n\
BggrBgEFBQcDATAkBglghkgBhvhCAQ0EFxYVVGVzdCBsaWJtaWNyb2h0dHBkIENB\n\
MB8GA1UdIwQYMBaAFFh1Q8ClajHDhTXdGqyN/0Ad1uksMA0GCSqGSIb3DQEBCwUA\n\
A4ICAQBvrrcTKVeI1EYnXo4BQD4oCvf9z1fYQmL21EbHwgjg1nmaPkvStgWAc5p1\n\
kKwySrpEMKXfu68X76RccXZyWWIamEjz2OCWYZgjX6d6FpjhLphL8WxXDy5C9eay\n\
ixN7+URz2XQoi22wqR+tCPDhrIzcMPyMkx/6gRgcYeDnaFrkdSeSsKsID4plfcIj\n\
ISWJDvv+IAgrtsG1NVHnGwpAv0od3A8/4/fR6PPyewaU3aydvjZ7Au8O9DGDjlU9\n\
9HdlOkkY6GVJ1pfGZib7cV7lhy0D2kj1g9xZh97YjpoUfppPl9r+6A8gDm0hXlAD\n\
TlzNYlwTb681ZEoSd9PiLEY8HETssHlays2dYXdcNwAEp69iIHz8q1Q98Be9LScl\n\
WEzgaOT9U7lpIw/MWbELoMsC+Ecs1cVWBIuiIq8aSG2kRr1x3S8yVXbAohAXif2s\n\
E6puieM/VJ25iaNhkbLmDkk58QVVmn9NZNv6ETxuSQMp9e0EwbVlj68vzClQ91Y/\n\
nmAiGcLFUEwB9G0szv9+vR+oDW4IkvdFZSUbcICd2cnynnwAD395onqS4hEZO1xM\n\
Gy5ZldbTMTjgn7fChNopz15ChPBnwFIjhm+S0CyiLRQAowfknRVq2IBkj7/5kOWg\n\
4mcxcq76HoQWK/8X/8RFL1eFVAvY7TNHYJ0RS51DMuwCNQictA==\n\
-----END CERTIFICATE-----";


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

/* test server self signed certificates */
const char srv_self_signed_cert_pem[] = "-----BEGIN CERTIFICATE-----\n"
                                        "MIIC+jCCAeSgAwIBAgIES0KCvTALBgkqhkiG9w0BAQUwFzEVMBMGA1UEAxMMdGVz\n"
                                        "dF9jYV9jZXJ0MB4XDTEwMDEwNTAwMDcyNVoXDTQ1MDMxMjAwMDcyNVowFzEVMBMG\n"
                                        "A1UEAxMMdGVzdF9jYV9jZXJ0MIIBHzALBgkqhkiG9w0BAQEDggEOADCCAQkCggEA\n"
                                        "tDEagv3p9OUhUL55jMucxjNK9N5cuozhcnrwDfBSU6oVrqm5kPqO1I7Cggzw68Y5\n"
                                        "jhTcBi4FXmYOZppm1R3MhSJ5JSi/67Q7X4J5rnJLXYGN27qjMpnoGQ/2xmsNG/is\n"
                                        "i+h/2vbtPU+WP9SEJnTfPLLpZ7KqCAk7FUUzKsuLx3/SOKtdkrWxPKwYTgnDEN6D\n"
                                        "JL7tEzCnG5DFc4mQ7YW9PaRdC3rS1T8PvQ3jB2BUnohM0cFvKRuiU35tU7h7CPbL\n"
                                        "4L66VglXoiwqmgcrwI2U968bD0+wRQ5c5bzNoshJOzN6CTMh1IhbklSh/Z6FA/e8\n"
                                        "hj0yVo2tdllXuJGVs3PIEwIDAQABo1UwUzAMBgNVHRMBAf8EAjAAMBMGA1UdJQQM\n"
                                        "MAoGCCsGAQUFBwMBMA8GA1UdDwEB/wQFAwMHIAAwHQYDVR0OBBYEFDfU7pAv9LYn\n"
                                        "n7jb4WHl4+Vgi2FnMAsGCSqGSIb3DQEBBQOCAQEAkaembPQMmv6OOjbIod8zTatr\n"
                                        "x5Bwkwp3TOE1NRyy2OytzFIYRUkNrZYlcmrxcbNNycIK41CNVXbriFCF8gcmIq9y\n"
                                        "vaKZn8Gcy+vGggv+1BP9IAPBGKRwSi0wmq9JoGE8hx+qqTpRSdfbM/cps/09hicO\n"
                                        "0EIR7kWEbvnpMBcMKYOtYE9Gce7rdSMWVAsKc174xn8vW6TxCUvmWFv5DPg5HG1v\n"
                                        "y1SUX73qafRo+W6FN4UC/DHfwRhF8RSKEnVbmgDVCs6GHdKBjU2qRgYyj6nWZqK1\n"
                                        "XFUTWgia+Fl3D9vlsXaFcSZKA0Bq1eojl0B0AfeYAxTFwPWXscKvt/bXZfH8bg==\n"
                                        "-----END CERTIFICATE-----\n";

/* test server key */
const char srv_key_pem[] = "-----BEGIN RSA PRIVATE KEY-----\n"
                           "MIIEpAIBAAKCAQEAtDEagv3p9OUhUL55jMucxjNK9N5cuozhcnrwDfBSU6oVrqm5\n"
                           "kPqO1I7Cggzw68Y5jhTcBi4FXmYOZppm1R3MhSJ5JSi/67Q7X4J5rnJLXYGN27qj\n"
                           "MpnoGQ/2xmsNG/isi+h/2vbtPU+WP9SEJnTfPLLpZ7KqCAk7FUUzKsuLx3/SOKtd\n"
                           "krWxPKwYTgnDEN6DJL7tEzCnG5DFc4mQ7YW9PaRdC3rS1T8PvQ3jB2BUnohM0cFv\n"
                           "KRuiU35tU7h7CPbL4L66VglXoiwqmgcrwI2U968bD0+wRQ5c5bzNoshJOzN6CTMh\n"
                           "1IhbklSh/Z6FA/e8hj0yVo2tdllXuJGVs3PIEwIDAQABAoIBAAEtcg+LFLGtoxjq\n"
                           "b+tFttBJfbRcfdG6ocYqBGmUXF+MgFs573DHX3sHNOQxlaNHtSgIclF1eYgNZFFt\n"
                           "VLIoBFTzfEQXoFosPUDoEuqVMeXLttmD7P2jwL780XJLZ4Xj6GY07npq1iGBcEZf\n"
                           "yCcdoyGkr9jgc5Auyis8DStGg/jfUBC4NBvF0GnuuNPAdYRPKUpKw9EatI+FdMjy\n"
                           "BuroD90fhdkK8EwMEVb9P17bdIc1MCIZFpUE9YHjVdK/oxCUhQ8KRfdbI4JU5Zh3\n"
                           "UtO6Jm2wFuP3VmeVpPvE/C2rxI70pyl6HMSiFGNc0rhJYCQ+yhohWj7nZ67H4vLx\n"
                           "plv5LxkCgYEAz7ewou8oFafDAMNoxaqKudvUg+lxXewdLDKaYBF5ACi9uAPCJ+v7\n"
                           "M5c/fvPFn/XHzo7xaXbtTAH3Z5xzBs+80OsvL+e1Ut4xR+ELRkybknh/s2wQeABk\n"
                           "Kb0vA59ukQGj12LV5phZMaVoXe6KJ7hZnN62d3K6m1wGE/k58i4pPLUCgYEA3hN8\n"
                           "G95zW7g0jVdSr+KUeVmephph9yh8Yb+3I3ojwOIv6d45TopGx8pFZlnBAMZf1ZQx\n"
                           "DIhzJNnaqZy/4w7RNaOGWnPA/5f+MIoHBiLGEEmfHC3lt087Yp9OuwDUHwpETYdV\n"
                           "o+KBCvVh60Et3bZUgF/1k/3YXxn8J5dsmJsjNqcCgYBLflyRa1BrRnTGMz9CEDCp\n"
                           "Si9b3h1Y4Hbd2GppHhCXMTd6yMrpDYhYANGQB3M9Juv+s88j4JhwNoq/uonH4Pqk\n"
                           "B8Y3qAQr4RuSH0WkwDUOsALhqBX4N1QwI1USAQEDbNAqeP5698X7GD3tXcQSmZrg\n"
                           "O8WfdjBCRNjkq4EW9xX/vQKBgQDONtmwJ0iHiu2BseyeVo/4fzfKlgUSNQ4K1rOA\n"
                           "xhIdMeu8Bxa/z7caHsGC4SVPSuYCtbE2Kh6BwapChcPJXCD45fgEViiJLuJiwEj1\n"
                           "caTpyvNsf1IoffJvCe9ZxtMyX549P8ZOgC3Dt0hN5CBrGLwu2Ox5l+YrqT10pi+5\n"
                           "JZX1UQKBgQCrcXrdkkDAc/a4+PxNRpJRLcU4fhv8/lr+UWItE8eUe7bd25bTQfQm\n"
                           "VpNKc/kAJ66PjIED6fy3ADhd2y4naT2a24uAgQ/M494J68qLnGh6K4JU/09uxR2v\n"
                           "1i2q/4FNLdFFk1XP4iNnTHRLZ+NYr2p5Y9RcvQfTjOauz8Ahav0lyg==\n"
                           "-----END RSA PRIVATE KEY-----\n";

#endif
