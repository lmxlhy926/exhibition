#include "cryptographic_type.h"

/**
  * VTEE_OperationHandle operation
  * [inbuf] void* chunk
  * uint32_t chunkSize
  **/
void VTEE_DigestUpdate(VTEE_OperationHandle operation, void* chunk, uint32_t chunkSize);

/**
  * VTEE_OperationHandle operation
  * [inbuf] void* chunk
  * uint32_t chunkLen
  * [outbuf] void* hash
  * uint32_t *hashLen
  **/
VTEE_Result VTEE_DigestDoFinal(VTEE_OperationHandle operation, void* chunk, uint32_t chunkLen, void* hash, uint32_t *hashLen);
