/* minimal stand-in for Mbed TLS PSA, needed by p256-m.c; implemented in eccert.c */
#include <stddef.h>
#include <stdint.h>
int psa_generate_random(uint8_t *out, size_t n);
