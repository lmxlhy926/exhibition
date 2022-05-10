#include <stdint.h>
#include <mbedtls/md.h>
#include <mbedtls/md5.h>
#include <mbedtls/aes.h>
#include <mbedtls/ccm.h>
#include <mbedtls/gcm.h>
#include <mbedtls/rsa.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>
#include <mbedtls/entropy.h>
#include <mbedtls/platform.h>
#include <mbedtls/ctr_drbg.h>

#include <string.h>
#include <strings.h>

#ifndef _CRYPTOGRAPHIC_TYPE_H_
#define _CRYPTOGRAPHIC_TYPE_H_


typedef uint32_t VTEE_Result;
typedef uint32_t VTEE_BigInt;

/* Error code*/
#define VTEE_ERROR_INTERNAL                   0x00000001
#define VTEE_ERROR_RESOURCE_LIMIT             0x00000002
#define VTEE_ERROR_DEVICE_BUSY                0x00000003
#define VTEE_ERROR_PANIC                      0xFFFFAAAA
#define VTEE_SUCCESS                          0x00000000
#define VTEE_ERROR_GENERIC                    0xFFFF0000
#define VTEE_ERROR_ACCESS_DENIED              0xFFFF0001
#define VTEE_ERROR_CANCEL                     0xFFFF0002
#define VTEE_ERROR_ACCESS_CONFLICT            0xFFFF0003
#define VTEE_ERROR_EXCESS_DATA                0xFFFF0004
#define VTEE_ERROR_BAD_FORMAT                 0xFFFF0005
#define VTEE_ERROR_BAD_PARAMETERS             0xFFFF0006
#define VTEE_ERROR_BAD_STATE                  0xFFFF0007
#define VTEE_ERROR_ITEM_NOT_FOUND             0xFFFF0008
#define VTEE_ERROR_NOT_IMPLEMENTED            0xFFFF0009
#define VTEE_ERROR_NOT_SUPPORTED              0xFFFF000A
#define VTEE_ERROR_NO_DATA                    0xFFFF000B
#define VTEE_ERROR_OUT_OF_MEMORY              0xFFFF000C
#define VTEE_ERROR_BUSY                       0xFFFF000D
#define VTEE_ERROR_COMMUNICATION              0xFFFF000E
#define VTEE_ERROR_SECURITY                   0xFFFF000F
#define VTEE_ERROR_SHORT_BUFFER               0xFFFF0010
#define VTEE_PENDING                          0xFFFF2000
#define VTEE_ERROR_TIMEOUT                    0xFFFF3001
#define VTEE_ERROR_OVERFLOW                   0xFFFF300F
#define VTEE_ERROR_TARGET_DEAD                0xFFFF3024
#define VTEE_ERROR_STORAGE_NO_SPACE           0xFFFF3041
#define VTEE_ERROR_MAC_INVALID                0xFFFF3071
#define VTEE_ERROR_SIGNATURE_INVALID          0xFFFF3072
#define VTEE_ERROR_TIME_NOT_SET               0xFFFF5000
#define VTEE_ERROR_TIME_NEEDS_RESET           0xFFFF5001

// List of Algorithm Identifiers
#define AES_ECB 0x10000010
#define AES_CBC 0x10000110
#define AES_CTR 0x10000210
#define AES_CCM 0x40000710
#define AES_GCM 0x40000810

#define VTEE_ALG_DES_ECB_NOPAD 0x10000011
#define VTEE_ALG_DES_CBC_NOPAD 0x10000111
#define VTEE_ALG_DES_CBC_MAC_NOPAD 0x30000111
#define VTEE_ALG_DES_CBC_MAC_PKCS5 0x30000511

#define VTEE_ALG_DES3_ECB_NOPAD 0x10000013
#define VTEE_ALG_DES3_CBC_NOPAD 0x10000113
#define VTEE_ALG_DES3_CBC_MAC_NOPAD 0x30000113
#define VTEE_ALG_DES3_CBC_MAC_PKCS5 0x30000513

#define VTEE_ALG_RSASSA_PKCS1_V1_5_MD5 0x70001830
#define VTEE_ALG_RSASSA_PKCS1_V1_5_SHA1 0x70002830
#define VTEE_ALG_RSASSA_PKCS1_V1_5_SHA224 0x70003830
#define VTEE_ALG_RSASSA_PKCS1_V1_5_SHA256 0x70004830
#define VTEE_ALG_RSASSA_PKCS1_V1_5_SHA384 0x70005830
#define VTEE_ALG_RSASSA_PKCS1_V1_5_SHA512 0x70006830
#define VTEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA1 0x70212930
#define VTEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA224 0x70313930
#define VTEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA256 0x70414930
#define VTEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA384 0x70515930
#define VTEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA512 0x70616930

#define VTEE_ALG_RSAES_PKCS1_V1_5 0x60000130
#define VTEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA1 0x60210230
#define VTEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA224 0x60310230
#define VTEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA256 0x60410230
#define VTEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA384 0x60510230
#define VTEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA512 0x60610230

#define VTEE_ALG_RSA_NOPAD 0x60000030
#define VTEE_ALG_DSA_SHA1 0x70002131
#define VTEE_ALG_DSA_SHA224 0x70003131
#define VTEE_ALG_DSA_SHA256 0x70004131

#define VTEE_ALG_DH_DERIVE_SHARED_SECRET 0x80000032

#define VTEE_ALG_MD5 0x50000001
#define VTEE_ALG_SHA1 0x50000002
#define VTEE_ALG_SHA224 0x50000003
#define VTEE_ALG_SHA256 0x50000004
#define VTEE_ALG_SHA384 0x50000005
#define VTEE_ALG_SHA512 0x50000006

#define VTEE_ALG_HMAC_MD5 0x30000001
#define VTEE_ALG_HMAC_SHA1 0x30000002
#define VTEE_ALG_HMAC_SHA224 0x30000003
#define VTEE_ALG_HMAC_SHA256 0x30000004
#define VTEE_ALG_HMAC_SHA384 0x30000005
#define VTEE_ALG_HMAC_SHA512 0x30000006

#define VTEE_ALG_ECDSA_P192 0x70001042
#define VTEE_ALG_ECDSA_P224 0x70002042
#define VTEE_ALG_ECDSA_P256 0x70003042
#define VTEE_ALG_ECDSA_P384 0x70004042
#define VTEE_ALG_ECDSA_P521 0x70005042

#define VTEE_ALG_ECDSA_SECP256K1 0x70006001

#define VTEE_ALG_ECDH_P192 0x80001042
#define VTEE_ALG_ECDH_P224 0x80002042
#define VTEE_ALG_ECDH_P256 0x80003042
#define VTEE_ALG_ECDH_P384 0x80004042
#define VTEE_ALG_ECDH_P521 0x80005042

// List of Object Types
#define VTEE_TYPE_AES 0xA0000010
#define VTEE_TYPE_DES 0xA0000011
#define VTEE_TYPE_DES3 0xA0000013
#define VTEE_TYPE_HMAC_MD5 0xA0000001
#define VTEE_TYPE_HMAC_SHA1 0xA0000002
#define VTEE_TYPE_HMAC_SHA224 0xA0000003
#define VTEE_TYPE_HMAC_SHA256 0xA0000004
#define VTEE_TYPE_HMAC_SHA384 0xA0000005
#define VTEE_TYPE_HMAC_SHA512 0xA0000006
#define VTEE_TYPE_RSA_PUBLIC_KEY 0xA0000030
#define VTEE_TYPE_RSA_KEYPAIR 0xA1000030
#define VTEE_TYPE_DSA_PUBLIC_KEY 0xA0000031
#define VTEE_TYPE_DSA_KEYPAIR 0xA1000031
#define VTEE_TYPE_DH_KEYPAIR 0xA1000032
#define VTEE_TYPE_ECDSA_PUBLIC_KEY 0xA0000041
#define VTEE_TYPE_ECDSA_KEYPAIR 0xA1000041
#define VTEE_TYPE_ECDH_PUBLIC_KEY 0xA0000042
#define VTEE_TYPE_ECDH_KEYPAIR 0xA1000042
#define VTEE_TYPE_GENERIC_SECRET 0xA0000000
#define VTEE_TYPE_CORRUPTED_OBJECT 0xA00000BE
#define VTEE_TYPE_DATA 0xA00000BF

// ECC CURVE
#define VTEE_ECC_CURVE_NIST_P192 0x00000001
#define VTEE_ECC_CURVE_NIST_P224 0x00000002
#define VTEE_ECC_CURVE_NIST_P256 0x00000003
#define VTEE_ECC_CURVE_NIST_P384 0x00000004
#define VTEE_ECC_CURVE_NIST_P521 0x00000005

// Object or Operation Attributes
// __________________________________________________________________________________________________________________________________________________________
// |                               |            |            |      |                     |                                                                 |
// | Name                          | Value      | Protection | Type | Format (Table 6-16) | Comment                                                         |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_SECRET_VALUE         | 0xC0000000 | Protected  |  Ref |       binary        | Used for all secret keys for symmetric ciphers, MACs, and HMACs |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_RSA_MODULUS          | 0xD0000130 | Public     |  Ref |       bignum        |                                                                 |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_RSA_PUBLIC_EXPONENT  | 0xD0000230 | Public     |  Ref |       bignum        |                                                                 |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_RSA_PRIVATE_EXPONENT | 0xC0000330 | Protected  |  Ref |       bignum        |                                                                 |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_RSA_PRIME1           | 0xC0000430 | Protected  |  Ref |       bignum        | Usually referred to as p.                                       |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_RSA_PRIME2           | 0xC0000530 | Protected  |  Ref |       bignum        | q                                                               |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_RSA_EXPONENT1        | 0xC0000630 | Protected  |  Ref |       bignum        | dp                                                              |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_RSA_EXPONENT2        | 0xC0000730 | Protected  |  Ref |       bignum        | dq                                                              |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_RSA_COEFFICIENT      | 0xC0000830 | Protected  |  Ref |       bignum        | iq                                                              |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_DSA_PRIME            | 0xD0001031 | Public     |  Ref |       bignum        | p                                                               |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_DSA_SUBPRIME         | 0xD0001131 | Public     |  Ref |       bignum        | q                                                               |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_DSA_BASE             | 0xD0001231 | Public     |  Ref |       bignum        | g                                                               |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_DSA_PUBLIC_VALUE     | 0xD0000131 | Public     |  Ref |       bignum        | y                                                               |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_DSA_PRIVATE_VALUE    | 0xC0000231 | Protected  |  Ref |       bignum        | x                                                               |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_DH_PRIME             | 0xD0001032 | Public     |  Ref |       bignum        | p                                                               |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_DH_SUBPRIME          | 0xD0001132 | Public     |  Ref |       bignum        | q                                                               |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_DH_BASE              | 0xD0001232 | Public     |  Ref |       bignum        | g                                                               |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_DH_X_BITS            | 0xF0001332 | Public     | Value|       int           | l                                                               |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_DH_PUBLIC_VALUE      | 0xD0000132 | Public     |  Ref |       bignum        | y                                                               |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_DH_PRIVATE_VALUE     | 0xC0000232 | Protected  |  Ref |       bignum        | x                                                               |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_RSA_OAEP_LABEL       | 0xD0000930 | Public     |  Ref |       binary        |                                                                 |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_RSA_PSS_SALT_LENGTH  | 0xF0000A30 | Public     | Value|       int           |                                                                 |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_ECC_PUBLIC_VALUE_X   | 0xD0000141 | Public     | Ref  |       bignum        |                                                                 |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_ECC_PUBLIC_VALUE_Y   | 0xD0000241 | Public     | Ref  |       bignum        |                                                                 |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_ECC_PRIVATE_VALUE    | 0xC0000341 | Protected  | Ref  |       bignum        | d                                                               |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|
// |                               |            |            |      |                     |                                                                 |
// | VTEE_ATTR_ECC_CURVE            | 0xF0000441 | Public     | Value|        int          | Value from ECC CURVE                                            |
// |_______________________________|____________|____________|______|_____________________|_________________________________________________________________|

typedef enum VTEE_OperationMode {
    // Encryption mode
    VTEE_MODE_ENCRYPT = 0,
    // Decryption mode
    VTEE_MODE_DECRYPT = 1,
    // Signature generation mode
    VTEE_MODE_SIGN = 2,
    // Signature verification mode
    VTEE_MODE_VERIFY = 3,
    // MAC mode
    VTEE_MODE_MAC = 4,
    // Digest mode
    VTEE_MODE_DIGEST = 5,
    // Key derivation mode
    VTEE_MODE_DERIVE = 6
} VTEE_OperationMode_t;

typedef struct VTEE_OperationInfo {
    uint32_t algorithm;
    uint32_t operationClass;
    uint32_t mode;
    uint32_t digestLength;
    uint32_t maxKeySize;
    uint32_t keySize;
    uint32_t requiredKeyUsage;
    uint32_t handleState;
} VTEE_OperationInfo_t;

typedef struct VTEE_OperationInfoKey {
    uint32_t keySize;
    uint32_t requiredKeyUsage;
} VTEE_OperationInfoKey_t;

// typedef struct {
//     uint32_t algorithm;
//     uint32_t operationClass;
//     uint32_t mode;
//     uint32_t digestLength;
//     uint32_t maxKeySize;
//     uint32_t handleState;
//     uint32_t operationState;
//     uint32_t numberOfKeys;
//     VTEE_OperationInfoKey keyInformation[];
// } VTEE_OperationInfoMultiple;

typedef struct ref
{
    void* buffer;
    uint32_t length;
} ref_t;

typedef struct value
{
    uint32_t a, b;
} value_t;

typedef struct VTEE_Attribute {
    uint32_t attributeID;
    union
    {
        ref_t ref;
        value_t value;
    } content;
} VTEE_Attribute_t;

/*
 * Operation Handle Define.
 */

#define MAX_KEYCNT                                      (8)

typedef struct __VTEE_OperationHandle* VTEE_OperationHandle;

typedef struct __VTEE_ObjectHandle* VTEE_ObjectHandle;

#endif /* _CRYPTOGRAPHIC_TYPE_H_ */
