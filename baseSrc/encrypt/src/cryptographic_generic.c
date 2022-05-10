#include "cryptographic_generic.h"
#include "asymmetric.h"

void print_hex(const char *title, const unsigned char buf[], size_t len)
{
    printf("%s: ", title);

    for (size_t i = 0; i < len; i++)
        printf("%02x", buf[i]);

    printf("\r\n");
}

int init_message_digest(mbedtls_md_context_t *context, mbedtls_md_type_t type)
{
    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(type);
    if (md_info == NULL)
    {
        printf("not supported message digest algorithm \r\n");
        return 1;
    }
    mbedtls_md_init(context);
    return mbedtls_md_init_ctx(context, md_info);
}

VTEE_Result VTEE_AllocateOperation(VTEE_OperationHandle* operation, uint32_t algorithm, uint32_t mode, uint32_t maxKeySize)
{
    if (operation == NULL) {
        return VTEE_ERROR_BAD_PARAMETERS;
    }

    int ret = 0;

    // init operation
    *operation = (VTEE_OperationHandle) malloc(sizeof(struct __VTEE_OperationHandle));
    if (*operation == NULL) {
        printf("VTEE_AllocateOperation +| Allocate Operation Handle Failed !!!");
        return VTEE_ERROR_OUT_OF_MEMORY;
    }
    memset(*operation, 0, sizeof(struct __VTEE_OperationHandle));

    if (VTEE_MODE_ENCRYPT == mode || VTEE_MODE_DECRYPT == mode) {
        if (algorithm == AES_ECB || algorithm == AES_CBC || algorithm == AES_CTR) {
             mbedtls_aes_context ctx;
             mbedtls_aes_init(&ctx);
             (*operation)->aes_ctx = (mbedtls_aes_context *) malloc (sizeof(mbedtls_aes_context));
             memcpy((*operation)->aes_ctx, &ctx, sizeof(mbedtls_aes_context));
         } else if (algorithm == AES_CCM) {
             mbedtls_ccm_context ctx;
             mbedtls_ccm_init(&ctx);
             (*operation)->ccm_context = (mbedtls_ccm_context *) malloc (sizeof(mbedtls_ccm_context));
             memcpy((*operation)->ccm_context, &ctx, sizeof(mbedtls_ccm_context));
         } else if (algorithm == AES_GCM) {
             mbedtls_gcm_context ctx;
             mbedtls_gcm_init(&ctx);
             (*operation)->gcm_context = (mbedtls_gcm_context *) malloc (sizeof(mbedtls_gcm_context));
             memcpy((*operation)->gcm_context, &ctx, sizeof(mbedtls_gcm_context));
         } else if (algorithm == VTEE_ALG_RSASSA_PKCS1_V1_5_SHA256 ||
             algorithm == VTEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA256) {
             mbedtls_rsa_context rsa;
             if (algorithm == VTEE_ALG_RSASSA_PKCS1_V1_5_SHA256) {
                 mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
             } else {
                 mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V21, 0);
             }
             (*operation)->rsa_context = (mbedtls_rsa_context *) malloc (sizeof(mbedtls_rsa_context));
             memcpy((*operation)->rsa_context, &rsa, sizeof(mbedtls_rsa_context));
         } else {
             return VTEE_ERROR_BAD_PARAMETERS;
         }
    } else if (VTEE_MODE_SIGN == mode || VTEE_MODE_VERIFY == mode) {
        if (algorithm == VTEE_ALG_ECDSA_P256) {
            mbedtls_ecdsa_context ecdsa;
            mbedtls_ecdsa_init(&ecdsa);
            mbedtls_ecp_group_load(&ecdsa.grp, MBEDTLS_ECP_DP_SECP256R1);
            (*operation)->ecdsa_context = (mbedtls_ecdsa_context *) malloc (sizeof(mbedtls_ecdsa_context));
            memcpy((*operation)->ecdsa_context, &ecdsa, sizeof(mbedtls_ecdsa_context));
        }else if (algorithm == VTEE_ALG_ECDSA_SECP256K1) {
            mbedtls_ecdsa_context ecdsa;
            mbedtls_ecdsa_init(&ecdsa);
            mbedtls_ecp_group_load(&ecdsa.grp, MBEDTLS_ECP_DP_SECP256K1);
            (*operation)->ecdsa_context = (mbedtls_ecdsa_context *) malloc (sizeof(mbedtls_ecdsa_context));
            memcpy((*operation)->ecdsa_context, &ecdsa, sizeof(mbedtls_ecdsa_context));
        }else if (algorithm == VTEE_ALG_RSASSA_PKCS1_V1_5_SHA256 ||
            algorithm == VTEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA256) {
            mbedtls_rsa_context rsa;
            if (algorithm == VTEE_ALG_RSASSA_PKCS1_V1_5_SHA256) {
                mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
            } else {
                mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V21, 0);
            }
            (*operation)->rsa_context = (mbedtls_rsa_context *) malloc (sizeof(mbedtls_rsa_context));
            memcpy((*operation)->rsa_context, &rsa, sizeof(mbedtls_rsa_context));
        } else {
            return VTEE_ERROR_BAD_PARAMETERS;
        }
    } else if (VTEE_MODE_DIGEST == mode) {
        if (algorithm != VTEE_ALG_MD5 &&
            algorithm != VTEE_ALG_SHA1 &&
            algorithm != VTEE_ALG_SHA224 &&
            algorithm != VTEE_ALG_SHA256 &&
            algorithm != VTEE_ALG_SHA384 &&
            algorithm != VTEE_ALG_SHA512) {
            return VTEE_ERROR_BAD_PARAMETERS;
        }
        mbedtls_md_context_t ctx;
        switch (algorithm) {
            case VTEE_ALG_MD5:
                ret = init_message_digest(&ctx, MBEDTLS_MD_MD5);
                (*operation)->operationInfo.digestLength = 16;
                break;
            case VTEE_ALG_SHA1:
                ret = init_message_digest(&ctx, MBEDTLS_MD_SHA1);
                (*operation)->operationInfo.digestLength = 20;
                break;
            case VTEE_ALG_SHA224:
                ret = init_message_digest(&ctx, MBEDTLS_MD_SHA224);
                (*operation)->operationInfo.digestLength = 28;
                break;
            case VTEE_ALG_SHA256:
                ret = init_message_digest(&ctx, MBEDTLS_MD_SHA256);
                (*operation)->operationInfo.digestLength = 32;
                break;
            case VTEE_ALG_SHA384:
                ret = init_message_digest(&ctx, MBEDTLS_MD_SHA384);
                (*operation)->operationInfo.digestLength = 48;
                break;
            case VTEE_ALG_SHA512:
                ret = init_message_digest(&ctx, MBEDTLS_MD_SHA512);
                (*operation)->operationInfo.digestLength = 64;
                break;
            default:
                ret = 1;
                break;
        }
        if(ret == 0) {
            (*operation)->md_context = (mbedtls_md_context_t *) malloc (sizeof(mbedtls_md_context_t));
            memcpy((*operation)->md_context, &ctx, sizeof(mbedtls_md_context_t));
        }
    }
    else if (VTEE_MODE_DERIVE == mode) {
        mbedtls_ecdh_context ecdh;
        mbedtls_ecdh_init(&ecdh);
        mbedtls_ecp_group_load(&ecdh.grp, MBEDTLS_ECP_DP_SECP256R1);

        (*operation)->ecdh_context = (mbedtls_ecdh_context *) malloc (sizeof(mbedtls_ecdh_context));
        memcpy((*operation)->ecdh_context, &ecdh, sizeof(mbedtls_ecdh_context));
    } else {
        return VTEE_ERROR_BAD_PARAMETERS;
    }
    (*operation)->operationInfo.algorithm = algorithm;
    (*operation)->operationInfo.mode = mode;
    return VTEE_SUCCESS;
}

void VTEE_FreeOperation(VTEE_OperationHandle operation)
{
    if (operation == NULL) {
        return;
    }

    int mode = operation->operationInfo.mode;
    int algorithm = operation->operationInfo.algorithm;

    if (VTEE_MODE_DIGEST == mode) {
        mbedtls_md_free(operation->md_context);
    } else if (VTEE_MODE_DERIVE == mode) {
        mbedtls_ecdh_free(operation->ecdh_context);
    }

    if (algorithm == VTEE_ALG_RSASSA_PKCS1_V1_5_SHA256 ||
        algorithm == VTEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA256) {
        mbedtls_rsa_free(operation->rsa_context);
    } else if (algorithm == AES_ECB || algorithm == AES_CBC || algorithm == AES_CTR) {
        mbedtls_aes_free(operation->aes_ctx);
    } else if (algorithm == AES_CCM) {
        mbedtls_ccm_free(operation->ccm_context);
    } else if (algorithm == AES_GCM) {
        mbedtls_gcm_free(operation->gcm_context);
    } else if (algorithm == VTEE_ALG_ECDSA_P256) {
        mbedtls_ecdsa_free(operation->ecdsa_context);
    }
    if (operation->aes_iv)
        free(operation->aes_iv);
}

VTEE_Result VTEE_SetOperationKey(VTEE_OperationHandle operation, VTEE_ObjectHandle key)
{
    int algorithm = operation->operationInfo.algorithm;

    if (operation->operationInfo.mode == VTEE_MODE_ENCRYPT)
    {
        if (algorithm == AES_ECB || algorithm == AES_CBC || algorithm == AES_CTR) {
             mbedtls_aes_setkey_enc(operation->aes_ctx, key->buffer, key->buffer_len);
        } else if (algorithm == AES_CCM) {
            mbedtls_ccm_setkey(operation->ccm_context, MBEDTLS_CIPHER_ID_AES, key->buffer, key->buffer_len);
        } else if (algorithm == AES_GCM) {
            mbedtls_gcm_setkey(operation->gcm_context, MBEDTLS_CIPHER_ID_AES, key->buffer, key->buffer_len);
        } else if (algorithm == AES_CTR) {
        }  else {
            return VTEE_ERROR_BAD_PARAMETERS;
        }
    }
    else if (operation->operationInfo.mode == VTEE_MODE_DECRYPT)
    {
        if (algorithm == AES_ECB || algorithm == AES_CBC || algorithm == AES_CTR) {
             mbedtls_aes_setkey_dec(operation->aes_ctx, key->buffer, key->buffer_len);
        } else if (algorithm == AES_CCM) {
            mbedtls_ccm_setkey(operation->ccm_context, MBEDTLS_CIPHER_ID_AES, key->buffer, key->buffer_len);
        } else if (algorithm == AES_GCM) {
            mbedtls_gcm_setkey(operation->gcm_context, MBEDTLS_CIPHER_ID_AES, key->buffer, key->buffer_len);
        } else {
            return VTEE_ERROR_BAD_PARAMETERS;
        }
    }
    else
    {
        return VTEE_ERROR_BAD_PARAMETERS;
    }
    return VTEE_SUCCESS;
}
