#include "asymmetric.h"
#include "cryptographic_generic.h"
#include "HUKFactor.h"
#include <mbedtls/ecdh.h>
#include <mbedtls/ecp.h>

#include <strings.h>

#define ECPARAMS    MBEDTLS_ECP_DP_SECP256R1
#define ECC256K1PARAMS    MBEDTLS_ECP_DP_SECP256K1

#if defined(MBEDTLS_CHECK_PARAMS)
#include "mbedtls/platform_util.h"
void mbedtls_param_failed( const char *failure_condition,
                           const char *file,
                           int line )
{
    mbedtls_printf( "%s:%i: Input param failed - %s\n",
                    file, line, failure_condition );
    mbedtls_exit( MBEDTLS_EXIT_FAILURE );
}
#endif

#define BUFSIZE 1024
static unsigned char buf[BUFSIZE];
static unsigned char tmp[150];

static void hexify(uint8_t *buf, uint32_t buf_len, char *name)
{
    int i = 0;
    memset(name, 0, strlen(name));
    for (i = 0; i < buf_len; i++)
    {
        sprintf(name, "%s%02X", name, buf[i]);
    }
}

void ecp_clear_precomputed(mbedtls_ecp_group *grp)
{
    if (grp->T != NULL) {
        size_t i;
        for (i = 0; i < grp->T_size; i++) {
            mbedtls_ecp_point_free(&grp->T[i]);
        }
        mbedtls_free(grp->T);
    }
    grp->T = NULL;
    grp->T_size = 0;
}

VTEE_Result VTEE_GenerateKey(VTEE_OperationHandle operation)
{
    int ret = 1;
    int exit_code = MBEDTLS_EXIT_FAILURE;

    size_t len = 1024;
    size_t olen;
    unsigned char buf[1024];

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_ctr_drbg_init( &ctr_drbg );
    mbedtls_entropy_init( &entropy );
    ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) pers, strlen(pers));
    if (ret != 0)
    {
        mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
        ret = VTEE_ERROR_BAD_PARAMETERS;
        goto exit;
    }

    if (operation->operationInfo.mode == VTEE_MODE_DERIVE)
    {
        mbedtls_ecdh_make_public(operation->ecdh_context, &olen, buf, sizeof(buf), mbedtls_ctr_drbg_random, &ctr_drbg);
        ecp_clear_precomputed(&operation->ecdh_context->grp);
    }
    else if (operation->operationInfo.mode == VTEE_MODE_ENCRYPT || operation->operationInfo.mode == VTEE_MODE_DECRYPT ||
             operation->operationInfo.mode == VTEE_MODE_SIGN || operation->operationInfo.mode == VTEE_MODE_VERIFY)
    {
        if (operation->operationInfo.algorithm == VTEE_ALG_ECDSA_P256)
        {
            ret = mbedtls_ecdsa_genkey(operation->ecdsa_context, ECPARAMS, mbedtls_ctr_drbg_random, &ctr_drbg);
            ecp_clear_precomputed(&operation->ecdsa_context->grp);
            if (ret != 0)
            {
                mbedtls_printf( "Failed to GenerateKey. Error code %d\n", ret );
                goto exit;
            }
            mbedtls_ecp_point_write_binary(&(operation->ecdsa_context->grp), &(operation->ecdsa_context->Q), MBEDTLS_ECP_PF_UNCOMPRESSED, &len, buf, sizeof(buf));
        }
        else if (operation->operationInfo.algorithm == VTEE_ALG_ECDSA_SECP256K1)
        {
            ret = mbedtls_ecdsa_genkey(operation->ecdsa_context, ECC256K1PARAMS, mbedtls_ctr_drbg_random, &ctr_drbg);
            ecp_clear_precomputed(&operation->ecdsa_context->grp);
            if (ret != 0)
            {
                mbedtls_printf("Failed to GenerateKey. Error code %d\n", ret);
                goto exit;
            }
            mbedtls_ecp_point_write_binary(&(operation->ecdsa_context->grp), &(operation->ecdsa_context->Q), MBEDTLS_ECP_PF_UNCOMPRESSED, &len, buf, sizeof(buf));
        }
        else if (operation->operationInfo.algorithm == VTEE_ALG_RSASSA_PKCS1_V1_5_SHA256 ||
                 operation->operationInfo.algorithm == VTEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA256)
        {
            ret = mbedtls_rsa_gen_key(operation->rsa_context, mbedtls_ctr_drbg_random, &ctr_drbg, KEY_SIZE, EXPONENT);
            if (ret != 0)
            {
                mbedtls_printf( "Failed to GenerateKey. Error code %d\n", ret );
                goto exit;
            }
        }
        else
        {
            ret = VTEE_ERROR_BAD_PARAMETERS;
            goto exit;
        }
    }
    else
    {
        ret = VTEE_ERROR_BAD_PARAMETERS;
        goto exit;
    }
exit:
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    return ret;
}

VTEE_Result VTEE_AsymmetricSignDigest(VTEE_OperationHandle operation, VTEE_Attribute_t* params, uint32_t paramCount, void* digest, uint32_t digestLen, void* signature, uint32_t *signatureLen)
{
    VTEE_Result ret = VTEE_SUCCESS;
    size_t sig_len = 256;
    unsigned char sig[256];
    memset(sig, 0, sig_len);

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_ctr_drbg_init( &ctr_drbg );
    mbedtls_entropy_init( &entropy );

    ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) pers, strlen(pers));
    if (ret != 0)
    {
        mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
        ret = VTEE_ERROR_BAD_PARAMETERS;
        goto exit;
    }

    if (operation->operationInfo.algorithm == VTEE_ALG_ECDSA_P256 || operation->operationInfo.algorithm == VTEE_ALG_ECDSA_SECP256K1)
    {
       ret = mbedtls_ecdsa_write_signature(operation->ecdsa_context, MBEDTLS_MD_SHA256, digest, digestLen, tmp, &sig_len, mbedtls_ctr_drbg_random, &ctr_drbg);
       if (ret != 0)
       {
           mbedtls_printf( "Failed to signature. Error code %d\n", ret );
           ret = VTEE_ERROR_BAD_PARAMETERS;
           goto exit;
       }
       memcpy(sig, tmp, sig_len);
       *signatureLen = sig_len;
    }
    else if (operation->operationInfo.algorithm == VTEE_ALG_RSASSA_PKCS1_V1_5_SHA256)
    {
        ret = mbedtls_rsa_pkcs1_sign(operation->rsa_context, NULL, NULL, MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256, 20, digest, sig);
        if (ret != 0)
        {
            mbedtls_printf( "Failed to signature. Error code %d\n", ret );
            ret = VTEE_ERROR_BAD_PARAMETERS;
            goto exit;
        }
    }
    else if (operation->operationInfo.algorithm == VTEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA256)
    {
        // case MBEDTLS_RSA_PKCS_V15:
            // return mbedtls_rsa_rsassa_pkcs1_v15_sign( ctx, f_rng, p_rng, mode, md_alg, hashlen, hash, sig );
        // case MBEDTLS_RSA_PKCS_V21:
        ret = mbedtls_rsa_rsassa_pss_sign(operation->rsa_context, NULL, NULL, MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256, sig_len, digest, sig);
        {
            mbedtls_printf( "Failed to signature. Error code %d\n", ret );
            ret = VTEE_ERROR_BAD_PARAMETERS;
            goto exit;
        }
    }
    else
    {
        ret = VTEE_ERROR_BAD_PARAMETERS;
        goto exit;
    }
    memcpy(signature, sig, sig_len);
    *signatureLen = sig_len;
exit:
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
    return ret;
}

VTEE_Result VTEE_AsymmetricVerifyDigest(VTEE_OperationHandle operation, VTEE_Attribute_t* params, uint32_t paramCount, void* digest, uint32_t digestLen, void* signature, uint32_t signatureLen)
{
    VTEE_Result ret = VTEE_SUCCESS;
    if (operation->operationInfo.algorithm == VTEE_ALG_ECDSA_P256 || operation->operationInfo.algorithm == VTEE_ALG_ECDSA_SECP256K1)
    {
        ret = mbedtls_ecdsa_read_signature(operation->ecdsa_context, digest, digestLen, signature, signatureLen);
    }
    else if (operation->operationInfo.algorithm == VTEE_ALG_RSASSA_PKCS1_V1_5_SHA256)
    {
        ret = mbedtls_rsa_pkcs1_verify(operation->rsa_context, NULL, NULL, MBEDTLS_RSA_PUBLIC, MBEDTLS_MD_SHA256, digestLen, digest, signature);
    }
    else if (operation->operationInfo.algorithm == VTEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA256)
    {
        ret = mbedtls_rsa_rsassa_pss_verify(operation->rsa_context, NULL, NULL, MBEDTLS_RSA_PUBLIC, MBEDTLS_MD_SHA256, digestLen, digest, signature);
    }
    else
    {
        return VTEE_ERROR_BAD_PARAMETERS;
    }
    return ret;
}

VTEE_Result VTEE_AsymmetricEncrypt(VTEE_OperationHandle operation, VTEE_Attribute_t* params, uint32_t paramCount, void* srcData, uint32_t srcLen, void* destData, uint32_t *destLen)
{
    VTEE_Result ret = VTEE_SUCCESS;

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_ctr_drbg_init( &ctr_drbg );
    mbedtls_entropy_init( &entropy );
    ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) pers, strlen(pers));
    if (ret != 0)
    {
        mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
        ret = VTEE_ERROR_BAD_PARAMETERS;
        goto exit;
    }

    ret = mbedtls_rsa_pkcs1_encrypt(operation->rsa_context, mbedtls_ctr_drbg_random, &ctr_drbg, MBEDTLS_RSA_PUBLIC, srcLen, srcData, destData);
    *destLen = operation->rsa_context->len;
exit:
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
    return ret;
}

VTEE_Result VTEE_AsymmetricDecrypt(VTEE_OperationHandle operation, VTEE_Attribute_t* params, uint32_t paramCount, void* srcData, uint32_t srcLen, void* destData, uint32_t *destLen)
{
    VTEE_Result ret = VTEE_SUCCESS;

    size_t result_len = 1024;
    unsigned char result[1024];
    memset(result, 0, result_len);

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_ctr_drbg_init( &ctr_drbg );
    mbedtls_entropy_init( &entropy );
    ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) pers, strlen(pers));
    if (ret != 0)
    {
        mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
        ret = VTEE_ERROR_BAD_PARAMETERS;
        goto exit;
    }
    ret = mbedtls_rsa_pkcs1_decrypt(operation->rsa_context, mbedtls_ctr_drbg_random, &ctr_drbg, MBEDTLS_RSA_PRIVATE, &result_len, srcData, result, 1024);
    if( ret != 0 )
    {
        goto exit;
    }
    *destLen = result_len;
    memcpy(destData, result, result_len);

exit:
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
    return ret;
}

VTEE_Result VTEE_ExportPubKey(VTEE_OperationHandle operation, VTEE_Attribute_t* params, uint32_t paramCount)
{
    if (operation->operationInfo.mode == VTEE_MODE_DERIVE)
    {
        ref_t ref1;
        ref1.buffer = malloc(32);
        ref1.length = 32;

        mbedtls_mpi_write_binary(&operation->ecdh_context->Q.X, ref1.buffer, ref1.length);
        memcpy(&params[0].content, &ref1, sizeof(ref1));
        params[0].attributeID = 1;

        ref_t ref2;
        ref2.buffer = malloc(32);
        ref2.length = 32;

        mbedtls_mpi_write_binary(&operation->ecdh_context->Q.Y, ref2.buffer, ref2.length);
        memcpy(&params[1].content, &ref2, sizeof(ref2));
        params[1].attributeID = 2;
    }
    else
    {
        if (operation->operationInfo.algorithm == VTEE_ALG_ECDSA_P256 || operation->operationInfo.algorithm == VTEE_ALG_ECDSA_SECP256K1)
        {
            ref_t ref1;
            ref1.buffer = malloc(32);
            ref1.length = 32;

            mbedtls_mpi_write_binary(&operation->ecdsa_context->Q.X, ref1.buffer, ref1.length);
            memcpy(&params[0].content, &ref1, sizeof(ref1));
            params[0].attributeID = 1;

            ref_t ref2;
            ref2.buffer = malloc(32);
            ref2.length = 32;

            mbedtls_mpi_write_binary(&operation->ecdsa_context->Q.Y, ref2.buffer, ref2.length);
            memcpy(&params[1].content, &ref2, sizeof(ref2));
            params[1].attributeID = 2;
        }
        else
        {
            ref_t ref1;
            ref1.buffer = malloc(256);
            ref1.length = 256;

            mbedtls_mpi_write_binary(&operation->rsa_context->N, ref1.buffer, ref1.length);
            memcpy(&params[0].content, &ref1, sizeof(ref1));
            params[0].attributeID = 1;

            ref_t ref2;
            ref2.buffer = malloc(256);
            ref2.length = 256;

            mbedtls_mpi_write_binary(&operation->rsa_context->E, ref2.buffer, ref2.length);
            memcpy(&params[1].content, &ref2, sizeof(ref2));
            params[1].attributeID = 2;
        }
    }
    return VTEE_SUCCESS;
}

VTEE_Result VTEE_ImportKey(VTEE_OperationHandle operation, VTEE_Attribute_t* params, uint32_t paramCount)
{
    if (operation->operationInfo.algorithm == VTEE_ALG_ECDSA_P256 || operation->operationInfo.algorithm == VTEE_ALG_ECDSA_SECP256K1)
    {
        for (int i = 0; i < paramCount; i++)
        {
            ref_t ref;
            memcpy(&ref, &params[i].content, sizeof(ref));

            int ret = mbedtls_mpi_lset( &operation->ecdsa_context->Q.Z, 1 );
            if( ret != 0 )
            {
                mbedtls_printf( "failed to set z, mbedtls_mpi_lset returned %d\n", ret );
            }

            if (params[i].attributeID == 1)
            {
                mbedtls_mpi_read_binary(&operation->ecdsa_context->Q.X, ref.buffer, ref.length);
            }
            else if (params[i].attributeID == 2)
            {
                mbedtls_mpi_read_binary(&operation->ecdsa_context->Q.Y, ref.buffer, ref.length);
            }
            else if (params[i].attributeID == 3)
            {
                mbedtls_mpi_read_binary(&operation->ecdsa_context->d, ref.buffer, ref.length);
            }
        }
    }
    else
    {
        mbedtls_mpi N;
        mbedtls_mpi D;
        mbedtls_mpi E;
        mbedtls_mpi_init(&N);
        mbedtls_mpi_init(&D);
        mbedtls_mpi_init(&E);
        for (int i = 0; i < paramCount; i++)
        {
            ref_t ref;
            memcpy(&ref, &params[i].content, sizeof(ref));
            if (params[i].attributeID == 1)
            {
                mbedtls_mpi_read_binary(&N, ref.buffer, ref.length);
            }
            else if (params[i].attributeID == 2)
            {
                mbedtls_mpi_read_binary(&E, ref.buffer, ref.length);
            }
            else if (params[i].attributeID == 3)
            {
                mbedtls_mpi_read_binary(&D, ref.buffer, ref.length);
            }
        }
        int ret = mbedtls_rsa_import(operation->rsa_context, &N, NULL, NULL, &D, &E);
        if( ret != 0 )
        {
            mbedtls_printf( "failed to import rsa key %d\n", ret );
            return ret;
        }
    }
    return VTEE_SUCCESS;
}

static int encrypt_file_aes_ctr(uint8_t *input, uint32_t len, uint8_t *output)
{
    int ret = 0;
    size_t nc_offset = 0;
    unsigned char stream_block[16];
    uint8_t iv[16];
    uint8_t a[48];
    uint8_t file_key[16];
    int p1 = 0,p2 = 0,p3 = 0;
    mbedtls_aes_context aes;

    memset(iv, 0, 16);
    memset(a, 0, 48);
    memset(file_key, 0, 16);

    HUKFactor0(&a,p1,p2,p3,p2,p1);
    HUKFactor1(&a,p2,p3,p1,p2,p1);

    ret = mbedtls_md5_ret(a, 48, file_key);
    if (ret != 0)
    {
        mbedtls_printf( "failed to sum md5 %d\n", ret );
        return ret;
    }

    mbedtls_aes_init(&aes);
    ret = mbedtls_aes_setkey_enc(&aes, file_key, 128);
    mbedtls_aes_crypt_ctr(&aes, len, &nc_offset, iv, stream_block, input, output);
    mbedtls_aes_free(&aes);
    return ret;
}

VTEE_Result VTEE_SaveKeyPair(VTEE_OperationHandle operation, uint8_t *file_path, uint32_t file_path_len, uint8_t *file_name, uint32_t *file_name_len)
{
    uint32_t key_buffer_len;
    uint8_t *key_buffer;
    char file_name_string[64];
    char file_path_string[128];
    uint8_t md5_buf[16];
    int ret = 0;
    FILE *fp;
    char name[256];
    uint8_t encrypt_data[1024];

    memset(name, 0, 256);
    memset(encrypt_data, 0, 1024);
    memset(file_path_string, 0, 128);

    if (operation == NULL || file_name_len == NULL || *file_name_len < 16 || file_name == NULL || file_path == NULL)
    {
        return VTEE_ERROR_BAD_PARAMETERS;
    }

    memcpy(file_path_string, file_path, file_path_len);

    if (operation->operationInfo.algorithm == VTEE_ALG_ECDSA_P256 || operation->operationInfo.algorithm == VTEE_ALG_ECDSA_SECP256K1)
    {
        key_buffer_len = 32 * 3;
        key_buffer = (uint8_t*) malloc(key_buffer_len);

        mbedtls_mpi_write_binary(&operation->ecdsa_context->Q.X, key_buffer, key_buffer_len / 3);
        mbedtls_mpi_write_binary(&operation->ecdsa_context->Q.Y, key_buffer + (key_buffer_len / 3), key_buffer_len / 3);
        mbedtls_mpi_write_binary(&operation->ecdsa_context->d, key_buffer + (key_buffer_len / 3 * 2), key_buffer_len / 3);
    }
    else
    {
        key_buffer_len = 256 * 3;
        key_buffer = (uint8_t*) malloc(key_buffer_len);
        mbedtls_mpi_write_binary(&operation->rsa_context->N, key_buffer, key_buffer_len / 3);
        mbedtls_mpi_write_binary(&operation->rsa_context->E, key_buffer + (key_buffer_len / 3), key_buffer_len / 3);
        mbedtls_mpi_write_binary(&operation->rsa_context->D, key_buffer + (key_buffer_len / 3 * 2), key_buffer_len / 3);
    }

    ret = mbedtls_md5_ret(key_buffer, key_buffer_len, md5_buf);
    if (ret != 0)
    {
        mbedtls_printf( "failed to sum md5 %d\n", ret );
        goto exit;
    }

    memset(file_name_string, 0, 64);
    hexify(md5_buf, 16, file_name_string);

    sprintf(name, "%s/%s", file_path_string, file_name_string);

    fp= fopen (name, "wb+");
    if (fp == NULL)
    {
        ret = VTEE_ERROR_BAD_PARAMETERS;
        goto exit;
    }
    fclose(fp);

    fp= fopen (name, "wb");
    if (fp == NULL)
    {
        ret = VTEE_ERROR_BAD_PARAMETERS;
        goto exit;
    }
    encrypt_file_aes_ctr(key_buffer, key_buffer_len, encrypt_data);
    fwrite(encrypt_data, key_buffer_len, 1, fp);
    fclose(fp);
    ret = 0;

    *file_name_len = strlen(file_name_string);
    memcpy(file_name, file_name_string, strlen(name));
exit:
    if (key_buffer) free(key_buffer);

    return ret;
}

VTEE_Result VTEE_prepairKeyPair(VTEE_OperationHandle operation,  uint8_t *file_path, uint32_t file_path_len, uint8_t *file_name, uint32_t file_name_len)
{
    if (operation == NULL || file_name == NULL || file_path == NULL)
    {
        return VTEE_ERROR_BAD_PARAMETERS;
    }

    int ret = 0;
    FILE *fp;
    uint8_t *key_buffer;
    uint32_t key_buffer_len;
    char input_name[64];
    char input_path[128];
    char file_full_name[256];
    uint8_t decrypt_data[1024];
    memset(input_name, 0, 64);
    memcpy(input_name, file_name, file_name_len);
    memset(input_path, 0, 128);
    memcpy(input_path, file_path, file_path_len);
    memset(file_full_name, 0, 256);

    if (operation->operationInfo.algorithm == VTEE_ALG_ECDSA_P256 || operation->operationInfo.algorithm == VTEE_ALG_ECDSA_SECP256K1)
    {
        key_buffer_len = 32 * 3;
    }
    else
    {
        key_buffer_len = 256 * 3;
    }

    key_buffer = (uint8_t *) malloc(key_buffer_len);

    sprintf(file_full_name, "%s/%s", input_path, input_name);
    fp = fopen(file_full_name, "rb");
    if (fp == NULL)
    {
        ret = VTEE_ERROR_BAD_PARAMETERS;
        goto exit;
    }
    fread(key_buffer, key_buffer_len, 1, fp);
    fclose(fp);

    encrypt_file_aes_ctr(key_buffer, key_buffer_len, decrypt_data);

    if (operation->operationInfo.algorithm == VTEE_ALG_ECDSA_P256 || operation->operationInfo.algorithm == VTEE_ALG_ECDSA_SECP256K1)
    {
        ret = mbedtls_mpi_lset(&operation->ecdsa_context->Q.Z, 1);
        if (ret != 0)
        {
            ret = VTEE_ERROR_BAD_PARAMETERS;
            goto exit;
        }
        mbedtls_mpi_read_binary(&operation->ecdsa_context->Q.X, decrypt_data, key_buffer_len / 3);
        mbedtls_mpi_read_binary(&operation->ecdsa_context->Q.Y, decrypt_data + (key_buffer_len / 3), key_buffer_len / 3);
        mbedtls_mpi_read_binary(&operation->ecdsa_context->d, decrypt_data + (key_buffer_len / 3 * 2), key_buffer_len / 3);
    }
    else
    {
        mbedtls_mpi N;
        mbedtls_mpi D;
        mbedtls_mpi E;
        mbedtls_mpi P;
        mbedtls_mpi Q;
        mbedtls_mpi_init(&N);
        mbedtls_mpi_init(&D);
        mbedtls_mpi_init(&E);
        mbedtls_mpi_init(&P);
        mbedtls_mpi_init(&Q);
        mbedtls_mpi_read_binary(&N, decrypt_data, key_buffer_len / 3);
        mbedtls_mpi_read_binary(&E, decrypt_data + (key_buffer_len / 3), key_buffer_len / 3);
        mbedtls_mpi_read_binary(&D, decrypt_data + (key_buffer_len / 3 * 2), key_buffer_len / 3);

        int ret = mbedtls_rsa_import(operation->rsa_context, &N, &P, &Q, &D, &E);
        if( ret != 0 )
        {
            mbedtls_printf( "failed to import rsa key %d\n", ret );
            ret = VTEE_ERROR_BAD_PARAMETERS;
            goto exit;
        }
    }
exit:
    if (key_buffer) free(key_buffer);
    return ret;
}
