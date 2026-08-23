// SPDX-License-Identifier: Apache-2.0 AND MIT

/*
 * OQS OpenSSL 3 provider decoders
 *
 * Code strongly inspired by OpenSSL default provider.
 *
 */

#ifndef DECODER_PROVIDER
#    error Macro DECODER_PROVIDER undefined
#endif

#define DECODER_STRUCTURE_type_specific_keypair "type-specific"
#define DECODER_STRUCTURE_type_specific_params  "type-specific"
#define DECODER_STRUCTURE_type_specific         "type-specific"
#define DECODER_STRUCTURE_type_specific_no_pub  "type-specific"
#define DECODER_STRUCTURE_PKCS8                 "pkcs8"
#define DECODER_STRUCTURE_SubjectPublicKeyInfo  "SubjectPublicKeyInfo"
#define DECODER_STRUCTURE_PrivateKeyInfo        "PrivateKeyInfo"

/* Arguments are prefixed with '_' to avoid build breaks on certain platforms */
#define DECODER(_name, _input, _output)                        \
    {                                                          \
        _name, "provider=" DECODER_PROVIDER ",input=" #_input, \
            (oqs_##_input##_to_##_output##_decoder_functions)  \
    }
#define DECODER_w_structure(_name, _input, _structure, _output)              \
    {                                                                        \
        _name,                                                               \
            "provider=" DECODER_PROVIDER ",input=" #_input                   \
            ",structure=" DECODER_STRUCTURE_##_structure,                    \
            (oqs_##_structure##_##_input##_to_##_output##_decoder_functions) \
    }

///// OQS_TEMPLATE_FRAGMENT_MAKE_START
#ifdef OQS_KEM_ENCODERS

#ifdef OQS_ENABLE_KEM_frodokem_640_aes
DECODER_w_structure("frodo640aes", der, PrivateKeyInfo, frodo640aes),
DECODER_w_structure("frodo640aes", der, SubjectPublicKeyInfo, frodo640aes),
DECODER_w_structure("p256_frodo640aes", der, PrivateKeyInfo, p256_frodo640aes),
DECODER_w_structure("p256_frodo640aes", der, SubjectPublicKeyInfo, p256_frodo640aes),DECODER_w_structure("x25519_frodo640aes", der, PrivateKeyInfo, x25519_frodo640aes),
DECODER_w_structure("x25519_frodo640aes", der, SubjectPublicKeyInfo, x25519_frodo640aes),
#endif
#ifdef OQS_ENABLE_KEM_frodokem_640_shake
DECODER_w_structure("frodo640shake", der, PrivateKeyInfo, frodo640shake),
DECODER_w_structure("frodo640shake", der, SubjectPublicKeyInfo, frodo640shake),
DECODER_w_structure("p256_frodo640shake", der, PrivateKeyInfo, p256_frodo640shake),
DECODER_w_structure("p256_frodo640shake", der, SubjectPublicKeyInfo, p256_frodo640shake),DECODER_w_structure("x25519_frodo640shake", der, PrivateKeyInfo, x25519_frodo640shake),
DECODER_w_structure("x25519_frodo640shake", der, SubjectPublicKeyInfo, x25519_frodo640shake),
#endif
#ifdef OQS_ENABLE_KEM_frodokem_976_aes
DECODER_w_structure("frodo976aes", der, PrivateKeyInfo, frodo976aes),
DECODER_w_structure("frodo976aes", der, SubjectPublicKeyInfo, frodo976aes),
DECODER_w_structure("p384_frodo976aes", der, PrivateKeyInfo, p384_frodo976aes),
DECODER_w_structure("p384_frodo976aes", der, SubjectPublicKeyInfo, p384_frodo976aes),DECODER_w_structure("x448_frodo976aes", der, PrivateKeyInfo, x448_frodo976aes),
DECODER_w_structure("x448_frodo976aes", der, SubjectPublicKeyInfo, x448_frodo976aes),
#endif
#ifdef OQS_ENABLE_KEM_frodokem_976_shake
DECODER_w_structure("frodo976shake", der, PrivateKeyInfo, frodo976shake),
DECODER_w_structure("frodo976shake", der, SubjectPublicKeyInfo, frodo976shake),
DECODER_w_structure("p384_frodo976shake", der, PrivateKeyInfo, p384_frodo976shake),
DECODER_w_structure("p384_frodo976shake", der, SubjectPublicKeyInfo, p384_frodo976shake),DECODER_w_structure("x448_frodo976shake", der, PrivateKeyInfo, x448_frodo976shake),
DECODER_w_structure("x448_frodo976shake", der, SubjectPublicKeyInfo, x448_frodo976shake),
#endif
#ifdef OQS_ENABLE_KEM_frodokem_1344_aes
DECODER_w_structure("frodo1344aes", der, PrivateKeyInfo, frodo1344aes),
DECODER_w_structure("frodo1344aes", der, SubjectPublicKeyInfo, frodo1344aes),
DECODER_w_structure("p521_frodo1344aes", der, PrivateKeyInfo, p521_frodo1344aes),
DECODER_w_structure("p521_frodo1344aes", der, SubjectPublicKeyInfo, p521_frodo1344aes),
#endif
#ifdef OQS_ENABLE_KEM_frodokem_1344_shake
DECODER_w_structure("frodo1344shake", der, PrivateKeyInfo, frodo1344shake),
DECODER_w_structure("frodo1344shake", der, SubjectPublicKeyInfo, frodo1344shake),
DECODER_w_structure("p521_frodo1344shake", der, PrivateKeyInfo, p521_frodo1344shake),
DECODER_w_structure("p521_frodo1344shake", der, SubjectPublicKeyInfo, p521_frodo1344shake),
#endif
#ifdef OQS_ENABLE_KEM_kyber_512
DECODER_w_structure("kyber512", der, PrivateKeyInfo, kyber512),
DECODER_w_structure("kyber512", der, SubjectPublicKeyInfo, kyber512),
DECODER_w_structure("p256_kyber512", der, PrivateKeyInfo, p256_kyber512),
DECODER_w_structure("p256_kyber512", der, SubjectPublicKeyInfo, p256_kyber512),DECODER_w_structure("x25519_kyber512", der, PrivateKeyInfo, x25519_kyber512),
DECODER_w_structure("x25519_kyber512", der, SubjectPublicKeyInfo, x25519_kyber512),
#endif
#ifdef OQS_ENABLE_KEM_kyber_768
DECODER_w_structure("kyber768", der, PrivateKeyInfo, kyber768),
DECODER_w_structure("kyber768", der, SubjectPublicKeyInfo, kyber768),
DECODER_w_structure("p384_kyber768", der, PrivateKeyInfo, p384_kyber768),
DECODER_w_structure("p384_kyber768", der, SubjectPublicKeyInfo, p384_kyber768),DECODER_w_structure("x448_kyber768", der, PrivateKeyInfo, x448_kyber768),
DECODER_w_structure("x448_kyber768", der, SubjectPublicKeyInfo, x448_kyber768),DECODER_w_structure("x25519_kyber768", der, PrivateKeyInfo, x25519_kyber768),
DECODER_w_structure("x25519_kyber768", der, SubjectPublicKeyInfo, x25519_kyber768),DECODER_w_structure("p256_kyber768", der, PrivateKeyInfo, p256_kyber768),
DECODER_w_structure("p256_kyber768", der, SubjectPublicKeyInfo, p256_kyber768),
#endif
#ifdef OQS_ENABLE_KEM_kyber_1024
DECODER_w_structure("kyber1024", der, PrivateKeyInfo, kyber1024),
DECODER_w_structure("kyber1024", der, SubjectPublicKeyInfo, kyber1024),
DECODER_w_structure("p521_kyber1024", der, PrivateKeyInfo, p521_kyber1024),
DECODER_w_structure("p521_kyber1024", der, SubjectPublicKeyInfo, p521_kyber1024),
#endif
#ifdef OQS_ENABLE_KEM_ml_kem_512
DECODER_w_structure("mlkem512", der, PrivateKeyInfo, mlkem512),
DECODER_w_structure("mlkem512", der, SubjectPublicKeyInfo, mlkem512),
DECODER_w_structure("p256_mlkem512", der, PrivateKeyInfo, p256_mlkem512),
DECODER_w_structure("p256_mlkem512", der, SubjectPublicKeyInfo, p256_mlkem512),DECODER_w_structure("x25519_mlkem512", der, PrivateKeyInfo, x25519_mlkem512),
DECODER_w_structure("x25519_mlkem512", der, SubjectPublicKeyInfo, x25519_mlkem512),
#endif
#ifdef OQS_ENABLE_KEM_ml_kem_768
DECODER_w_structure("mlkem768", der, PrivateKeyInfo, mlkem768),
DECODER_w_structure("mlkem768", der, SubjectPublicKeyInfo, mlkem768),
DECODER_w_structure("p384_mlkem768", der, PrivateKeyInfo, p384_mlkem768),
DECODER_w_structure("p384_mlkem768", der, SubjectPublicKeyInfo, p384_mlkem768),DECODER_w_structure("x448_mlkem768", der, PrivateKeyInfo, x448_mlkem768),
DECODER_w_structure("x448_mlkem768", der, SubjectPublicKeyInfo, x448_mlkem768),DECODER_w_structure("X25519MLKEM768", der, PrivateKeyInfo, X25519MLKEM768),
DECODER_w_structure("X25519MLKEM768", der, SubjectPublicKeyInfo, X25519MLKEM768),DECODER_w_structure("SecP256r1MLKEM768", der, PrivateKeyInfo, SecP256r1MLKEM768),
DECODER_w_structure("SecP256r1MLKEM768", der, SubjectPublicKeyInfo, SecP256r1MLKEM768),
#endif
#ifdef OQS_ENABLE_KEM_ml_kem_1024
DECODER_w_structure("mlkem1024", der, PrivateKeyInfo, mlkem1024),
DECODER_w_structure("mlkem1024", der, SubjectPublicKeyInfo, mlkem1024),
DECODER_w_structure("p521_mlkem1024", der, PrivateKeyInfo, p521_mlkem1024),
DECODER_w_structure("p521_mlkem1024", der, SubjectPublicKeyInfo, p521_mlkem1024),DECODER_w_structure("p384_mlkem1024", der, PrivateKeyInfo, p384_mlkem1024),
DECODER_w_structure("p384_mlkem1024", der, SubjectPublicKeyInfo, p384_mlkem1024),
#endif
#ifdef OQS_ENABLE_KEM_bike_l1
DECODER_w_structure("bikel1", der, PrivateKeyInfo, bikel1),
DECODER_w_structure("bikel1", der, SubjectPublicKeyInfo, bikel1),
DECODER_w_structure("p256_bikel1", der, PrivateKeyInfo, p256_bikel1),
DECODER_w_structure("p256_bikel1", der, SubjectPublicKeyInfo, p256_bikel1),DECODER_w_structure("x25519_bikel1", der, PrivateKeyInfo, x25519_bikel1),
DECODER_w_structure("x25519_bikel1", der, SubjectPublicKeyInfo, x25519_bikel1),
#endif
#ifdef OQS_ENABLE_KEM_bike_l3
DECODER_w_structure("bikel3", der, PrivateKeyInfo, bikel3),
DECODER_w_structure("bikel3", der, SubjectPublicKeyInfo, bikel3),
DECODER_w_structure("p384_bikel3", der, PrivateKeyInfo, p384_bikel3),
DECODER_w_structure("p384_bikel3", der, SubjectPublicKeyInfo, p384_bikel3),DECODER_w_structure("x448_bikel3", der, PrivateKeyInfo, x448_bikel3),
DECODER_w_structure("x448_bikel3", der, SubjectPublicKeyInfo, x448_bikel3),
#endif
#ifdef OQS_ENABLE_KEM_bike_l5
DECODER_w_structure("bikel5", der, PrivateKeyInfo, bikel5),
DECODER_w_structure("bikel5", der, SubjectPublicKeyInfo, bikel5),
DECODER_w_structure("p521_bikel5", der, PrivateKeyInfo, p521_bikel5),
DECODER_w_structure("p521_bikel5", der, SubjectPublicKeyInfo, p521_bikel5),
#endif
#ifdef OQS_ENABLE_KEM_hqc_128
DECODER_w_structure("hqc128", der, PrivateKeyInfo, hqc128),
DECODER_w_structure("hqc128", der, SubjectPublicKeyInfo, hqc128),
DECODER_w_structure("p256_hqc128", der, PrivateKeyInfo, p256_hqc128),
DECODER_w_structure("p256_hqc128", der, SubjectPublicKeyInfo, p256_hqc128),DECODER_w_structure("x25519_hqc128", der, PrivateKeyInfo, x25519_hqc128),
DECODER_w_structure("x25519_hqc128", der, SubjectPublicKeyInfo, x25519_hqc128),
#endif
#ifdef OQS_ENABLE_KEM_hqc_192
DECODER_w_structure("hqc192", der, PrivateKeyInfo, hqc192),
DECODER_w_structure("hqc192", der, SubjectPublicKeyInfo, hqc192),
DECODER_w_structure("p384_hqc192", der, PrivateKeyInfo, p384_hqc192),
DECODER_w_structure("p384_hqc192", der, SubjectPublicKeyInfo, p384_hqc192),DECODER_w_structure("x448_hqc192", der, PrivateKeyInfo, x448_hqc192),
DECODER_w_structure("x448_hqc192", der, SubjectPublicKeyInfo, x448_hqc192),
#endif
#ifdef OQS_ENABLE_KEM_hqc_256
DECODER_w_structure("hqc256", der, PrivateKeyInfo, hqc256),
DECODER_w_structure("hqc256", der, SubjectPublicKeyInfo, hqc256),
DECODER_w_structure("p521_hqc256", der, PrivateKeyInfo, p521_hqc256),
DECODER_w_structure("p521_hqc256", der, SubjectPublicKeyInfo, p521_hqc256),
#endif

#endif /* OQS_KEM_ENCODERS */


#ifdef OQS_ENABLE_SIG_dilithium_2
DECODER_w_structure("dilithium2", der, PrivateKeyInfo, dilithium2),
DECODER_w_structure("dilithium2", der, SubjectPublicKeyInfo, dilithium2),DECODER_w_structure("p256_dilithium2", der, PrivateKeyInfo, p256_dilithium2),
DECODER_w_structure("p256_dilithium2", der, SubjectPublicKeyInfo, p256_dilithium2),DECODER_w_structure("rsa3072_dilithium2", der, PrivateKeyInfo, rsa3072_dilithium2),
DECODER_w_structure("rsa3072_dilithium2", der, SubjectPublicKeyInfo, rsa3072_dilithium2),
#endif
#ifdef OQS_ENABLE_SIG_dilithium_3
DECODER_w_structure("dilithium3", der, PrivateKeyInfo, dilithium3),
DECODER_w_structure("dilithium3", der, SubjectPublicKeyInfo, dilithium3),DECODER_w_structure("p384_dilithium3", der, PrivateKeyInfo, p384_dilithium3),
DECODER_w_structure("p384_dilithium3", der, SubjectPublicKeyInfo, p384_dilithium3),
#endif
#ifdef OQS_ENABLE_SIG_dilithium_5
DECODER_w_structure("dilithium5", der, PrivateKeyInfo, dilithium5),
DECODER_w_structure("dilithium5", der, SubjectPublicKeyInfo, dilithium5),DECODER_w_structure("p521_dilithium5", der, PrivateKeyInfo, p521_dilithium5),
DECODER_w_structure("p521_dilithium5", der, SubjectPublicKeyInfo, p521_dilithium5),
#endif
#ifdef OQS_ENABLE_SIG_ml_dsa_44
DECODER_w_structure("mldsa44", der, PrivateKeyInfo, mldsa44),
DECODER_w_structure("mldsa44", der, SubjectPublicKeyInfo, mldsa44),DECODER_w_structure("p256_mldsa44", der, PrivateKeyInfo, p256_mldsa44),
DECODER_w_structure("p256_mldsa44", der, SubjectPublicKeyInfo, p256_mldsa44),DECODER_w_structure("rsa3072_mldsa44", der, PrivateKeyInfo, rsa3072_mldsa44),
DECODER_w_structure("rsa3072_mldsa44", der, SubjectPublicKeyInfo, rsa3072_mldsa44),DECODER_w_structure("mldsa44_pss2048", der, PrivateKeyInfo, mldsa44_pss2048),
DECODER_w_structure("mldsa44_pss2048", der, SubjectPublicKeyInfo, mldsa44_pss2048),DECODER_w_structure("mldsa44_rsa2048", der, PrivateKeyInfo, mldsa44_rsa2048),
DECODER_w_structure("mldsa44_rsa2048", der, SubjectPublicKeyInfo, mldsa44_rsa2048),DECODER_w_structure("mldsa44_ed25519", der, PrivateKeyInfo, mldsa44_ed25519),
DECODER_w_structure("mldsa44_ed25519", der, SubjectPublicKeyInfo, mldsa44_ed25519),DECODER_w_structure("mldsa44_p256", der, PrivateKeyInfo, mldsa44_p256),
DECODER_w_structure("mldsa44_p256", der, SubjectPublicKeyInfo, mldsa44_p256),DECODER_w_structure("mldsa44_bp256", der, PrivateKeyInfo, mldsa44_bp256),
DECODER_w_structure("mldsa44_bp256", der, SubjectPublicKeyInfo, mldsa44_bp256),
#endif
#ifdef OQS_ENABLE_SIG_ml_dsa_65
DECODER_w_structure("mldsa65", der, PrivateKeyInfo, mldsa65),
DECODER_w_structure("mldsa65", der, SubjectPublicKeyInfo, mldsa65),DECODER_w_structure("p384_mldsa65", der, PrivateKeyInfo, p384_mldsa65),
DECODER_w_structure("p384_mldsa65", der, SubjectPublicKeyInfo, p384_mldsa65),DECODER_w_structure("mldsa65_pss3072", der, PrivateKeyInfo, mldsa65_pss3072),
DECODER_w_structure("mldsa65_pss3072", der, SubjectPublicKeyInfo, mldsa65_pss3072),DECODER_w_structure("mldsa65_rsa3072", der, PrivateKeyInfo, mldsa65_rsa3072),
DECODER_w_structure("mldsa65_rsa3072", der, SubjectPublicKeyInfo, mldsa65_rsa3072),DECODER_w_structure("mldsa65_p256", der, PrivateKeyInfo, mldsa65_p256),
DECODER_w_structure("mldsa65_p256", der, SubjectPublicKeyInfo, mldsa65_p256),DECODER_w_structure("mldsa65_bp256", der, PrivateKeyInfo, mldsa65_bp256),
DECODER_w_structure("mldsa65_bp256", der, SubjectPublicKeyInfo, mldsa65_bp256),DECODER_w_structure("mldsa65_ed25519", der, PrivateKeyInfo, mldsa65_ed25519),
DECODER_w_structure("mldsa65_ed25519", der, SubjectPublicKeyInfo, mldsa65_ed25519),
#endif
#ifdef OQS_ENABLE_SIG_ml_dsa_87
DECODER_w_structure("mldsa87", der, PrivateKeyInfo, mldsa87),
DECODER_w_structure("mldsa87", der, SubjectPublicKeyInfo, mldsa87),DECODER_w_structure("p521_mldsa87", der, PrivateKeyInfo, p521_mldsa87),
DECODER_w_structure("p521_mldsa87", der, SubjectPublicKeyInfo, p521_mldsa87),DECODER_w_structure("mldsa87_p384", der, PrivateKeyInfo, mldsa87_p384),
DECODER_w_structure("mldsa87_p384", der, SubjectPublicKeyInfo, mldsa87_p384),DECODER_w_structure("mldsa87_bp384", der, PrivateKeyInfo, mldsa87_bp384),
DECODER_w_structure("mldsa87_bp384", der, SubjectPublicKeyInfo, mldsa87_bp384),DECODER_w_structure("mldsa87_ed448", der, PrivateKeyInfo, mldsa87_ed448),
DECODER_w_structure("mldsa87_ed448", der, SubjectPublicKeyInfo, mldsa87_ed448),
#endif
#ifdef OQS_ENABLE_SIG_falcon_512
DECODER_w_structure("falcon512", der, PrivateKeyInfo, falcon512),
DECODER_w_structure("falcon512", der, SubjectPublicKeyInfo, falcon512),DECODER_w_structure("p256_falcon512", der, PrivateKeyInfo, p256_falcon512),
DECODER_w_structure("p256_falcon512", der, SubjectPublicKeyInfo, p256_falcon512),DECODER_w_structure("rsa3072_falcon512", der, PrivateKeyInfo, rsa3072_falcon512),
DECODER_w_structure("rsa3072_falcon512", der, SubjectPublicKeyInfo, rsa3072_falcon512),
#endif
#ifdef OQS_ENABLE_SIG_falcon_padded_512
DECODER_w_structure("falconpadded512", der, PrivateKeyInfo, falconpadded512),
DECODER_w_structure("falconpadded512", der, SubjectPublicKeyInfo, falconpadded512),DECODER_w_structure("p256_falconpadded512", der, PrivateKeyInfo, p256_falconpadded512),
DECODER_w_structure("p256_falconpadded512", der, SubjectPublicKeyInfo, p256_falconpadded512),DECODER_w_structure("rsa3072_falconpadded512", der, PrivateKeyInfo, rsa3072_falconpadded512),
DECODER_w_structure("rsa3072_falconpadded512", der, SubjectPublicKeyInfo, rsa3072_falconpadded512),
#endif
#ifdef OQS_ENABLE_SIG_falcon_1024
DECODER_w_structure("falcon1024", der, PrivateKeyInfo, falcon1024),
DECODER_w_structure("falcon1024", der, SubjectPublicKeyInfo, falcon1024),DECODER_w_structure("p521_falcon1024", der, PrivateKeyInfo, p521_falcon1024),
DECODER_w_structure("p521_falcon1024", der, SubjectPublicKeyInfo, p521_falcon1024),
#endif
#ifdef OQS_ENABLE_SIG_falcon_padded_1024
DECODER_w_structure("falconpadded1024", der, PrivateKeyInfo, falconpadded1024),
DECODER_w_structure("falconpadded1024", der, SubjectPublicKeyInfo, falconpadded1024),DECODER_w_structure("p521_falconpadded1024", der, PrivateKeyInfo, p521_falconpadded1024),
DECODER_w_structure("p521_falconpadded1024", der, SubjectPublicKeyInfo, p521_falconpadded1024),
#endif
#ifdef OQS_ENABLE_SIG_sphincs_sha2_128f_simple
DECODER_w_structure("sphincssha2128fsimple", der, PrivateKeyInfo, sphincssha2128fsimple),
DECODER_w_structure("sphincssha2128fsimple", der, SubjectPublicKeyInfo, sphincssha2128fsimple),DECODER_w_structure("p256_sphincssha2128fsimple", der, PrivateKeyInfo, p256_sphincssha2128fsimple),
DECODER_w_structure("p256_sphincssha2128fsimple", der, SubjectPublicKeyInfo, p256_sphincssha2128fsimple),DECODER_w_structure("rsa3072_sphincssha2128fsimple", der, PrivateKeyInfo, rsa3072_sphincssha2128fsimple),
DECODER_w_structure("rsa3072_sphincssha2128fsimple", der, SubjectPublicKeyInfo, rsa3072_sphincssha2128fsimple),
#endif
#ifdef OQS_ENABLE_SIG_sphincs_sha2_128s_simple
DECODER_w_structure("sphincssha2128ssimple", der, PrivateKeyInfo, sphincssha2128ssimple),
DECODER_w_structure("sphincssha2128ssimple", der, SubjectPublicKeyInfo, sphincssha2128ssimple),DECODER_w_structure("p256_sphincssha2128ssimple", der, PrivateKeyInfo, p256_sphincssha2128ssimple),
DECODER_w_structure("p256_sphincssha2128ssimple", der, SubjectPublicKeyInfo, p256_sphincssha2128ssimple),DECODER_w_structure("rsa3072_sphincssha2128ssimple", der, PrivateKeyInfo, rsa3072_sphincssha2128ssimple),
DECODER_w_structure("rsa3072_sphincssha2128ssimple", der, SubjectPublicKeyInfo, rsa3072_sphincssha2128ssimple),
#endif
#ifdef OQS_ENABLE_SIG_sphincs_sha2_192f_simple
DECODER_w_structure("sphincssha2192fsimple", der, PrivateKeyInfo, sphincssha2192fsimple),
DECODER_w_structure("sphincssha2192fsimple", der, SubjectPublicKeyInfo, sphincssha2192fsimple),DECODER_w_structure("p384_sphincssha2192fsimple", der, PrivateKeyInfo, p384_sphincssha2192fsimple),
DECODER_w_structure("p384_sphincssha2192fsimple", der, SubjectPublicKeyInfo, p384_sphincssha2192fsimple),
#endif
#ifdef OQS_ENABLE_SIG_sphincs_shake_128f_simple
DECODER_w_structure("sphincsshake128fsimple", der, PrivateKeyInfo, sphincsshake128fsimple),
DECODER_w_structure("sphincsshake128fsimple", der, SubjectPublicKeyInfo, sphincsshake128fsimple),DECODER_w_structure("p256_sphincsshake128fsimple", der, PrivateKeyInfo, p256_sphincsshake128fsimple),
DECODER_w_structure("p256_sphincsshake128fsimple", der, SubjectPublicKeyInfo, p256_sphincsshake128fsimple),DECODER_w_structure("rsa3072_sphincsshake128fsimple", der, PrivateKeyInfo, rsa3072_sphincsshake128fsimple),
DECODER_w_structure("rsa3072_sphincsshake128fsimple", der, SubjectPublicKeyInfo, rsa3072_sphincsshake128fsimple),
#endif
#ifdef OQS_ENABLE_SIG_mayo_1
DECODER_w_structure("mayo1", der, PrivateKeyInfo, mayo1),
DECODER_w_structure("mayo1", der, SubjectPublicKeyInfo, mayo1),DECODER_w_structure("p256_mayo1", der, PrivateKeyInfo, p256_mayo1),
DECODER_w_structure("p256_mayo1", der, SubjectPublicKeyInfo, p256_mayo1),
#endif
#ifdef OQS_ENABLE_SIG_mayo_2
DECODER_w_structure("mayo2", der, PrivateKeyInfo, mayo2),
DECODER_w_structure("mayo2", der, SubjectPublicKeyInfo, mayo2),DECODER_w_structure("p256_mayo2", der, PrivateKeyInfo, p256_mayo2),
DECODER_w_structure("p256_mayo2", der, SubjectPublicKeyInfo, p256_mayo2),
#endif
#ifdef OQS_ENABLE_SIG_mayo_3
DECODER_w_structure("mayo3", der, PrivateKeyInfo, mayo3),
DECODER_w_structure("mayo3", der, SubjectPublicKeyInfo, mayo3),DECODER_w_structure("p384_mayo3", der, PrivateKeyInfo, p384_mayo3),
DECODER_w_structure("p384_mayo3", der, SubjectPublicKeyInfo, p384_mayo3),
#endif
#ifdef OQS_ENABLE_SIG_mayo_5
DECODER_w_structure("mayo5", der, PrivateKeyInfo, mayo5),
DECODER_w_structure("mayo5", der, SubjectPublicKeyInfo, mayo5),DECODER_w_structure("p521_mayo5", der, PrivateKeyInfo, p521_mayo5),
DECODER_w_structure("p521_mayo5", der, SubjectPublicKeyInfo, p521_mayo5),
#endif
#ifdef OQS_ENABLE_SIG_cross_rsdp_128_balanced
DECODER_w_structure("CROSSrsdp128balanced", der, PrivateKeyInfo, CROSSrsdp128balanced),
DECODER_w_structure("CROSSrsdp128balanced", der, SubjectPublicKeyInfo, CROSSrsdp128balanced),
#endif
///// OQS_TEMPLATE_FRAGMENT_MAKE_END
