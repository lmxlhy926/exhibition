#include "cryptographic_type.h"

#define BUFSIZE 1024

/**
  * VTEE_OperationHandle operation
  * [inbuf] void* IV
  * uint32_t IVLen
  **/
void VTEE_CipherInit(VTEE_OperationHandle operation, void* IV, uint32_t IVLen);

/**
  * VTEE_OperationHandle operation
  * [inbuf] void* srcData
  * uint32_t srcLen
  * [outbuf] void* destData
  * uint32_t *destLen
  **/
VTEE_Result VTEE_CipherUpdate(VTEE_OperationHandle operation, void* srcData, uint32_t srcLen, void* destData, uint32_t *destLen);

/**
  * VTEE_OperationHandle operation
  * [inbuf] void* srcData
  * uint32_t srcLen
  * [outbufopt] void* destData
  * uint32_t *destLen
  **/
VTEE_Result VTEE_CipherDoFinal(VTEE_OperationHandle operation, void* srcData, uint32_t srcLen, void* destData, uint32_t *destLen);
