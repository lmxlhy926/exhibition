#include "cryptographic_type.h"

// Asymmetric Derivation Operation Parameters
// __________________________________________________________________________________________________________________
// |                                              |                                                                 |
// |        Algorithm                             |       Possible Operation Parameters                             |
// |______________________________________________|_________________________________________________________________|
// |                                              |                                                                 |
// |  VTEE_ALG_DH_DERIVE_SHARED_SECRET             |  VTEE_ATTR_DH_PUBLIC_VALUE : Public key part of the other party. |
// |                                              |  This parameter is mandatory. Possible Operation Parameters     |
// |______________________________________________|_________________________________________________________________|
// |                                              |                                                                 |
// |  VTEE_ALG_ECDH_NIST_P192_DERIVE_SHARED_SECRET |  VTEE_ATTR_ECC_PUBLIC_VALUE_X,                                   |
// |  VTEE_ALG_ECDH_NIST_P224_DERIVE_SHARED_SECRET |  VTEE_ATTR_ECC_PUBLIC_VALUE_Y : Public key part of the other     |
// |  VTEE_ALG_ECDH_NIST_P256_DERIVE_SHARED_SECRET |  party. This parameter is mandatory.                            |
// |  VTEE_ALG_ECDH_NIST_P384_DERIVE_SHARED_SECRET |                                                                 |
// |  VTEE_ALG_ECDH_NIST_P521_DERIVE_SHARED_SECRET |                                                                 |
// |______________________________________________|_________________________________________________________________|

typedef struct
{
    unsigned char *buf;
    size_t length;
} rnd_buf_info;

void print_mpi(mbedtls_mpi *mpi, char* name);

/**
  * VTEE_OperationHandle operation
  * [in] VTEE_Attribute* params
  * uint32_t paramCount
  * VTEE_ObjectHandle derivedKey
  **/
void VTEE_DeriveKey(VTEE_OperationHandle operation, VTEE_Attribute_t* params, uint32_t paramCount, VTEE_ObjectHandle derivedKey);

/**
  * [out] void* randomBuffer
  * uint32_t randomBufferLen
  **/
void VTEE_GenerateRandom(void* randomBuffer, uint32_t randomBufferLen);
