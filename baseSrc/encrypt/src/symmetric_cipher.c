#include "symmetric_cipher.h"
#include "cryptographic_generic.h"

#include <mbedtls/aes.h>

static const unsigned char ad[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13
};

/**
  * VTEE_OperationHandle operation
  * [inbuf] void* IV
  * uint32_t IVLen
  **/
void VTEE_CipherInit(VTEE_OperationHandle operation, void* IV, uint32_t IVLen)
{
    operation->aes_iv = (uint8_t *) malloc(IVLen);
    memcpy(operation->aes_iv, IV, IVLen);
    operation->aes_iv_len = IVLen;
}

/**
  * VTEE_OperationHandle operation
  * [inbuf] void* srcData
  * uint32_t srcLen
  * [outbuf] void* destData
  * uint32_t *destLen
  **/
VTEE_Result VTEE_CipherUpdate(VTEE_OperationHandle operation, void* srcData, uint32_t srcLen, void* destData, uint32_t *destLen)
{
    if (operation->operationInfo.algorithm == AES_CTR)
    {
        int keysize;
        size_t nc_offset = 0;
        unsigned char stream_block[16];

        for (keysize = 128; keysize <= 256; keysize += 64) {
            int ret = mbedtls_aes_crypt_ctr(operation->aes_ctx, srcLen, &nc_offset, operation->aes_iv, stream_block, srcData, destData);
            *destLen = srcLen;
            return ret;
        }
    }

    if (operation->operationInfo.mode == VTEE_MODE_ENCRYPT)
    {
        if (operation->operationInfo.algorithm == AES_ECB) {
            mbedtls_aes_crypt_ecb(operation->aes_ctx, MBEDTLS_AES_ENCRYPT, srcData, destData);
        } else if (operation->operationInfo.algorithm == AES_CBC) {
            mbedtls_aes_crypt_cbc(operation->aes_ctx, MBEDTLS_AES_ENCRYPT, srcLen, operation->aes_iv, srcData, destData);
        } else if (operation->operationInfo.algorithm == AES_CCM) {
            unsigned char tag_output[16];
            mbedtls_ccm_encrypt_and_tag(operation->ccm_context, srcLen, operation->aes_iv, sizeof(ad), ad, 0, srcData, destData, tag_output, 16);
        } else if (operation->operationInfo.algorithm == AES_GCM) {
            unsigned char tag_output[16];
            size_t tag_len = 16;
            memset(tag_output, 0x00, 16);
            mbedtls_gcm_crypt_and_tag(operation->gcm_context, MBEDTLS_GCM_ENCRYPT, srcLen, operation->aes_iv, operation->aes_iv_len,
                                      srcData, srcLen, srcData, destData, tag_len, tag_output);
        }
    }
    else if (operation->operationInfo.mode == VTEE_MODE_DECRYPT)
    {
        if (operation->operationInfo.algorithm == AES_ECB) {
            mbedtls_aes_crypt_ecb(operation->aes_ctx, MBEDTLS_AES_DECRYPT, srcData, destData);
        } else if (operation->operationInfo.algorithm == AES_CBC) {
            mbedtls_aes_crypt_cbc(operation->aes_ctx, MBEDTLS_AES_DECRYPT, srcLen, operation->aes_iv, srcData, destData);
        } else if (operation->operationInfo.algorithm == AES_CCM) {
            unsigned char tag_output[16];
            mbedtls_ccm_auth_decrypt(operation->ccm_context, srcLen, operation->aes_iv, operation->aes_iv_len, ad, sizeof(ad), srcData, srcData, tag_output, 16);
        } else if (operation->operationInfo.algorithm == AES_GCM) {
            unsigned char tag_output[16];
            size_t tag_len = 16;
            memset(tag_output, 0x00, 16);
            mbedtls_gcm_crypt_and_tag(operation->gcm_context, MBEDTLS_GCM_DECRYPT, srcLen, operation->aes_iv, operation->aes_iv_len,
                                      srcData, srcLen, srcData, destData, tag_len, tag_output);
        }
    }
    else
    {
        return VTEE_ERROR_BAD_PARAMETERS;
    }
    return VTEE_SUCCESS;
}

/**
  * VTEE_OperationHandle operation
  * [inbuf] void* srcData
  * uint32_t srcLen
  * [outbufopt] void* destData
  * uint32_t *destLen
  **/
VTEE_Result VTEE_CipherDoFinal(VTEE_OperationHandle operation, void* srcData, uint32_t srcLen, void* destData, uint32_t *destLen)
{
    if (operation->operationInfo.algorithm == AES_CTR)
    {
        int keysize;
        size_t nc_offset = 0;
        unsigned char stream_block[16];

        for (keysize = 128; keysize <= 256; keysize += 64) {
            int ret = mbedtls_aes_crypt_ctr(operation->aes_ctx, srcLen, &nc_offset, operation->aes_iv, stream_block, srcData, destData);
            *destLen = srcLen;
            return ret;
        }
    }

    if (operation->operationInfo.mode == VTEE_MODE_ENCRYPT)
    {
        if (operation->operationInfo.algorithm == AES_ECB) {
            mbedtls_aes_crypt_ecb(operation->aes_ctx, MBEDTLS_AES_ENCRYPT, srcData, destData);
        } else if (operation->operationInfo.algorithm == AES_CBC) {
            mbedtls_aes_crypt_cbc(operation->aes_ctx, MBEDTLS_AES_ENCRYPT, srcLen, operation->aes_iv, srcData, destData);
        } else if (operation->operationInfo.algorithm == AES_CCM) {
            unsigned char tag[16];
            mbedtls_ccm_encrypt_and_tag(operation->ccm_context, srcLen, operation->aes_iv, sizeof(ad), ad, 0, srcData, destData, tag, 16);
        } else if (operation->operationInfo.algorithm == AES_GCM) {
            unsigned char tag_output[16];
            size_t tag_len = 16;
            memset(tag_output, 0x00, 16);
            mbedtls_gcm_crypt_and_tag(operation->gcm_context, MBEDTLS_GCM_ENCRYPT, srcLen, operation->aes_iv, operation->aes_iv_len,
                                      srcData, srcLen, srcData, destData, tag_len, tag_output);
        }
    }
    else if (operation->operationInfo.mode == VTEE_MODE_DECRYPT)
    {
        if (operation->operationInfo.algorithm == AES_ECB) {
            mbedtls_aes_crypt_ecb(operation->aes_ctx, MBEDTLS_AES_DECRYPT, srcData, destData);
        } else if (operation->operationInfo.algorithm == AES_CBC) {
            mbedtls_aes_crypt_cbc(operation->aes_ctx, MBEDTLS_AES_DECRYPT, srcLen, operation->aes_iv, srcData, destData);
        } else if (operation->operationInfo.algorithm == AES_CCM) {
            unsigned char tag_output[16];
            mbedtls_ccm_auth_decrypt(operation->ccm_context, srcLen, operation->aes_iv, operation->aes_iv_len, ad, sizeof(ad), srcData, srcData, tag_output, 16);
        } else if (operation->operationInfo.algorithm == AES_GCM) {
            unsigned char tag_output[16];
            size_t tag_len = 16;
            memset(tag_output, 0x00, 16);
            mbedtls_gcm_crypt_and_tag(operation->gcm_context, MBEDTLS_GCM_DECRYPT, srcLen, operation->aes_iv, operation->aes_iv_len,
                                      srcData, srcLen, srcData, destData, tag_len, tag_output);
        }
    }
    else
    {
        return VTEE_ERROR_BAD_PARAMETERS;
    }
    return VTEE_SUCCESS;
}
