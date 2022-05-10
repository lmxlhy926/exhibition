#include "cryptographic_type.h"

struct __VTEE_OperationHandle {
        VTEE_OperationInfo_t                     operationInfo ;
        unsigned  int                           operationState;
        // [31]: handle         [1: OPENED]             TEEI_OPERATION_STATE_OPENED
        // [02]: final_func     [1: CALLED]             TEEI_OPERATION_STATE_CALLFE
        // [01]: init_func      [1: CALLED]             TEEI_OPERATION_STATE_CALLIE
        // [00]: state          [1: ACTIVE]             TEEI_OPERATION_STATE_ACTIVE

        mbedtls_md_context_t                    *md_context;
        /////////////////////////////////////////
        mbedtls_aes_context                     *aes_ctx;
        uint8_t                                 *aes_iv;
        uint32_t                                aes_iv_len;
        /////////////////////////////////////////
        mbedtls_gcm_context                     *gcm_context;
        /////////////////////////////////////////
        mbedtls_ccm_context                     *ccm_context;
        /////////////////////////////////////////
        mbedtls_rsa_context                     *rsa_context;
        /////////////////////////////////////////
        mbedtls_ecdsa_context                   *ecdsa_context;
        /////////////////////////////////////////
        mbedtls_ecdh_context                    *ecdh_context;
        /////////////////////////////////////////
        unsigned char                           reserve[128];

};

struct __VTEE_ObjectHandle {
        uint32_t flags;
        uint8_t *buffer;
        uint32_t buffer_len;
};

// The parameter algorithm MUST be one of the constants defined in section 6.10.1. cryptographic_type.h LINE 4 - 75
// The parameter mode MUST be one of the constants defined in section 6.1.1. It MUST be compatible with the algorithm as defined by Table 6-4.
// The parameter maxKeySize MUST be a valid value as defined in Table 5-9 for the algorithm.

// Table 5-9: VTEE_AllocateTransientObject Object Types and Key Sizes
// Object Type   | Possible Key Sizes
// VTEE_TYPE_AES  | 128, 192, or 256 bits
// VTEE_TYPE_DES  | Always 64 bits including the parity bits. This gives an effective key size of 56 bits
// VTEE_TYPE_DES3 | 128 or 192 bits including the parity bits. This gives effective key sizes of 112 or 168 bits
// VTEE_TYPE_RSA_PUBLIC_KEY | The number of bits in the modulus. 256, 512, 768, 1024, 1536 and 2048 bit keys MUST be supported. Support for other key sizes including bigger key sizes is implementation-dependent. Minimum key size is 256 bits.
// VTEE_TYPE_RSA_KEYPAIR    | Same as for RSA public key size.
// VTEE_TYPE_DSA_PUBLIC_KEY | Depends on Algorithm:
//                           ALG_DSA_SHA1: Between 512 and 1024 bits, multiple of 64 bits
//                           ALG_DSA_SHA224: 2048 bits
//                           ALG_DSA_SHA256: 2048 or 3072 bits
// VTEE_TYPE_DSA_KEYPAIR      | Same as for DSA public key size.
// VTEE_TYPE_DH_KEYPAIR       | From 256 to 2048 bits
// VTEE_TYPE_ECDSA_PUBLIC_KEY | Conditional: If ECC is supported, then all the curve sizes defined in Table 6-14 MUST be supported.
// VTEE_TYPE_ECDSA_KEYPAIR    | Conditional: If ECC is supported, then MUST be same value as for ECDSA public key size.
// VTEE_TYPE_ECDH_PUBLIC_KEY  | Conditional: If ECC is supported, then all the curve sizes defined in Table 6-14 MUST be supported.
// VTEE_TYPE_ECDH_KEYPAIR     | Conditional: If ECC is supported, then MUST be same value as for ECH public key size.
// VTEE_TYPE_GENERIC_SECRET   | Multiple of 8 bits, up to 4096 bits. This type is intended for secret data that is not directly used as a key in a cryptographic operation, but participates in a key derivation.
// VTEE_TYPE_DATA             | 0 – All data is in the associated data stream.

VTEE_Result VTEE_AllocateOperation(VTEE_OperationHandle* operation, uint32_t algorithm, uint32_t mode, uint32_t maxKeySize);

void VTEE_FreeOperation(VTEE_OperationHandle operation);

/**
  * VTEE_OperationHandle operation [IN]
  * VTEE_OperationInfo* operationInfo [OUT]
  **/
// void VTEE_GetOperationInfo(VTEE_OperationHandle operation, VTEE_OperationInfo* operationInfo);

/**
  * VTEE_OperationHandle operation
  * VTEE_OperationInfoMultiple* operationInfoMultiple [outbuf]
  * uint32_t* operationSize
  **/
// VTEE_Result VTEE_GetOperationInfoMultiple(VTEE_OperationHandle operation, VTEE_OperationInfoMultiple* operationInfoMultiple, uint32_t* operationSize);

void VTEE_ResetOperation(VTEE_OperationHandle operation);

VTEE_Result VTEE_SetOperationKey(VTEE_OperationHandle operation, VTEE_ObjectHandle key);

/**
  * Public Key Allowed Modes
  **/
// ________________________________________________________________________________
// |                                       |                                      |
// |          Key Type                     |     Allowed Operation Modes          |
// |_______________________________________|______________________________________|
// |                                       |                                      |
// |  VTEE_TYPE_RSA_PUBLIC_KEY              |  VTEE_MODE_VERIFY or VTEE_MODE_ENCRYPT |
// |_______________________________________|______________________________________|
// |                                       |                                      |
// |  VTEE_TYPE_DSA_PUBLIC_KEY              |  VTEE_MODE_VERIFY                     |
// |_______________________________________|______________________________________|
// |                                       |                                      |
// |  VTEE_TYPE_ECDSA_PUBLIC_KEY (optional) |  VTEE_MODE_VERIFY                     |
// |_______________________________________|______________________________________|
// |                                       |                                      |
// |  VTEE_TYPE_ECDH_PUBLIC_KEY (optional)  |  VTEE_MODE_DERIVE                     |
// |_______________________________________|______________________________________|


/**
  * Key-Pair Parts for Operation Modes
  **/
// ________________________________________________________________________________
// |                                       |                                      |
// |          Operation Mode               |     Key Parts Used                   |
// |_______________________________________|______________________________________|
// |                                       |                                      |
// |  VTEE_MODE_VERIFY                      |  Public                              |
// |_______________________________________|______________________________________|
// |                                       |                                      |
// |  VTEE_MODE_SIGN                        |  Private                             |
// |_______________________________________|______________________________________|
// |                                       |                                      |
// |  VTEE_MODE_ENCRYPT                     |  Public                              |
// |_______________________________________|______________________________________|
// |                                       |                                      |
// |  VTEE_MODE_DECRYPT                     |  Private                             |
// |_______________________________________|______________________________________|
// |                                       |                                      |
// |  VTEE_MODE_DERIVE                      |  Public and Private                  |
// |_______________________________________|______________________________________|

// VTEE_Result VTEE_SetOperationKey2(VTEE_OperationHandle operation, VTEE_ObjectHandle key1, VTEE_ObjectHandle key2);

void VTEE_CopyOperation(VTEE_OperationHandle dstOperation, VTEE_OperationHandle srcOperation);

void print_hex(const char *title, const unsigned char buf[], size_t len);
