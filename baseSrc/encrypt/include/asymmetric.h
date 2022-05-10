#include "cryptographic_type.h"

#define KEY_SIZE 2048
#define EXPONENT 65537
#define pers "Generate"

/**
  * VTEE_OperationHandle operation
  * [in] VTEE_Attribute* params
  * uint32_t paramCount
  * [inbuf] void* srcData
  * uint32_t srcLen
  * [outbuf] void* destData
  * uint32_t *destLen
  **/
VTEE_Result VTEE_AsymmetricEncrypt(VTEE_OperationHandle operation, VTEE_Attribute_t* params, uint32_t paramCount, void* srcData, uint32_t srcLen, void* destData, uint32_t *destLen);

/**
  * VTEE_OperationHandle operation
  * [in] VTEE_Attribute* params
  * uint32_t paramCount
  * [inbuf] void* srcData
  * uint32_t srcLen
  * [outbuf] void* destData
  * uint32_t *destLen
  **/
VTEE_Result VTEE_AsymmetricDecrypt(VTEE_OperationHandle operation, VTEE_Attribute_t* params, uint32_t paramCount, void* srcData, uint32_t srcLen, void* destData, uint32_t *destLen);

/**
  * VTEE_OperationHandle operation
  * [in] VTEE_Attribute* params
  * uint32_t paramCount
  * [inbuf] void* digest
  * uint32_t digestLen
  * [outbuf] void* signature
  * uint32_t *signatureLen
  **/
VTEE_Result VTEE_AsymmetricSignDigest(VTEE_OperationHandle operation, VTEE_Attribute_t* params, uint32_t paramCount, void* digest, uint32_t digestLen, void* signature, uint32_t *signatureLen);

/**
  * VTEE_OperationHandle operation
  * [in] VTEE_Attribute* params
  * uint32_t paramCount
  * [inbuf] void* digest
  * uint32_t digestLen
  * [inbuf] void* signature
  * uint32_t *signatureLen
  **/
VTEE_Result VTEE_AsymmetricVerifyDigest(VTEE_OperationHandle operation, VTEE_Attribute_t* params, uint32_t paramCount, void* digest, uint32_t digestLen, void* signature, uint32_t signatureLen);

VTEE_Result VTEE_GenerateKey(VTEE_OperationHandle operation);

VTEE_Result VTEE_ImportKey(VTEE_OperationHandle operation, VTEE_Attribute_t* params, uint32_t paramCount);
VTEE_Result VTEE_ExportPubKey(VTEE_OperationHandle operation, VTEE_Attribute_t* params, uint32_t paramCount);
VTEE_Result VTEE_SaveKeyPair(VTEE_OperationHandle operation, uint8_t *file_path, uint32_t file_path_len, uint8_t *file_name, uint32_t *file_name_len);
VTEE_Result VTEE_prepairKeyPair(VTEE_OperationHandle operation,  uint8_t *file_path, uint32_t file_path_len, uint8_t *file_name, uint32_t file_name_len);
