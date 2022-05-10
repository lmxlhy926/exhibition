#include "encrypt_sign.h"

/** encrypt message.
     *
     *  Returns: 0 when encrypt success
     *  In:   char* folder: file directory
              uint8_t* file_name: file name
              uint32_t file_name_len: file name length
              char* message: message content
              uint8_t* encrypt_buf: encrypted data
              uint32_t* encrypt_len: encrypted data length
     */
int rsa_encrypt(char* folder, uint8_t* file_name, uint32_t file_name_len, char* message, uint8_t* encrypt_buf, uint32_t* encrypt_len) {
    int MAX_ENCRYPT_BLOCK = 117;
    int message_len = strlen(message);
//    if (message_len > 4 * MAX_ENCRYPT_BLOCK)
//    {
//        printf(" failed\n  ! message too long > 234 Bytes.\n");
//        return -1;
//    }
    VTEE_Attribute_t params[3];
    uint32_t paramCount = 3;
    uint32_t tmp_encrypt_len = 256;
    *encrypt_len = 0;
    VTEE_OperationHandle operation;
    int ret = 0;
    ret = VTEE_AllocateOperation(&operation, VTEE_ALG_RSASSA_PKCS1_V1_5_SHA256, VTEE_MODE_ENCRYPT, 0);
    ret = VTEE_prepairKeyPair(operation,(uint8_t*) folder, (uint32_t) strlen(folder), file_name, file_name_len);

//    if (message_len <= MAX_ENCRYPT_BLOCK)
//    {
//        ret = VTEE_AsymmetricEncrypt(operation, params, paramCount, message, strlen(message), encrypt_buf, encrypt_len);
//        print_hex("encrypt_buf", encrypt_buf, *encrypt_len);
//        VTEE_FreeOperation(operation);
//    }
//    else
//    {
//        ret = VTEE_AsymmetricEncrypt(operation, params, paramCount, message, MAX_ENCRYPT_BLOCK, encrypt_buf, encrypt_len);
//        //print_hex("encrypt_buf", encrypt_buf, *encrypt_len);
//        int tmp_encrypt_len = *encrypt_len;
//        ret = VTEE_AsymmetricEncrypt(operation, params, paramCount, message + MAX_ENCRYPT_BLOCK, message_len - MAX_ENCRYPT_BLOCK, encrypt_buf + tmp_encrypt_len, encrypt_len);
//        //print_hex("encrypt_buf", encrypt_buf + tmp_encrypt_len, *encrypt_len);
//        VTEE_FreeOperation(operation);
//        *encrypt_len = tmp_encrypt_len + *encrypt_len;
//    }

    for (int i = 0; i < message_len/MAX_ENCRYPT_BLOCK; i++)
    {
        ret = VTEE_AsymmetricEncrypt(operation, params, paramCount, message + i * MAX_ENCRYPT_BLOCK, MAX_ENCRYPT_BLOCK, encrypt_buf + *encrypt_len, &tmp_encrypt_len);
        //print_hex("encrypt_buf", encrypt_buf, *encrypt_len);
        //print_hex("encrypt_buf", encrypt_buf + tmp_encrypt_len, *encrypt_len);
        *encrypt_len = tmp_encrypt_len + *encrypt_len;
    }
    if (int remainder = message_len%MAX_ENCRYPT_BLOCK)
    {
        ret = VTEE_AsymmetricEncrypt(operation, params, paramCount, message + message_len/MAX_ENCRYPT_BLOCK * MAX_ENCRYPT_BLOCK, remainder, encrypt_buf + *encrypt_len, &tmp_encrypt_len);
        *encrypt_len = tmp_encrypt_len + *encrypt_len;
    }

    VTEE_FreeOperation(operation);
    return ret;
}


/** sign message.
     *
     *  Returns: null
     *  In:   char* folder: file directory
              uint8_t* file_name: file name
              uint32_t file_name_len: file name length
              char* message: message content
              unsigned char* sign_buf: sign data
     */
void sign_secp256K1_recoverable(char* folder, uint8_t* file_name, uint32_t file_name_len, char* message, unsigned char* sign_buf)
{
    VTEE_OperationHandle operation;
    uint32_t message_len = strlen(message);
    unsigned char digest[32];
    uint32_t digest_len = 32;
    //uint32_t file_name_len = strlen(file_name);

    //hash((uint8_t*)message, strlen(message), digest, &digest_len); //hashֵû�з��򣨴�С��ģʽ��
    keccak_hash(message, message_len, digest, digest_len);
    VTEE_Result ret = VTEE_AllocateOperation(&operation, VTEE_ALG_ECDSA_SECP256K1, VTEE_MODE_SIGN, 0);
    ret = VTEE_prepairKeyPair(operation, (uint8_t*) folder, (uint32_t) strlen(folder), file_name, file_name_len);

    rustsecp256k1_v0_4_0_context* ctx = rustsecp256k1_v0_4_0_context_create(SECP256K1_CONTEXT_SIGN);
    rustsecp256k1_v0_4_0_ecdsa_recoverable_signature rsig;

    unsigned char data[2 * MBEDTLS_ECP_MAX_BYTES];
    unsigned char sk32[32];

    size_t grp_len = (operation->ecdsa_context->grp.nbits + 7) / 8;
    mbedtls_mpi_write_binary(&operation->ecdsa_context->d, data, grp_len); //��˽Կת�ɶ����ƣ����ģʽ
    rustsecp256k1_v0_4_0_ecdsa_sign_recoverable(ctx, &rsig, digest, data, NULL, NULL);

    memcpy(sign_buf, &rsig, 65);
    rustsecp256k1_v0_4_0_context_destroy(ctx);
    VTEE_FreeOperation(operation);

}

/** generate and save secp256k1 key.
     *
     *  Returns: 0 when generate and save success
     *  Out:
     *  In:   char* folder: file directory
     *        uint8_t* file_name: file name
              uint32_t file_name_len: file name length
     */
int generate_save_secp256k1_key(char* folder, uint8_t* file_name, uint32_t* file_name_len)
{
    VTEE_OperationHandle operation;
    VTEE_Result ret = VTEE_AllocateOperation(&operation, VTEE_ALG_ECDSA_SECP256K1, VTEE_MODE_SIGN, 0);
    ret = VTEE_GenerateKey(operation);
    ret = VTEE_SaveKeyPair(operation, (uint8_t*) folder, (uint32_t) strlen(folder), file_name, file_name_len);

    VTEE_FreeOperation(operation);
    return ret;
}

/** get secp256k1 pub key from file.
     *
     *  Returns: 0 get pub key success
     *  In:   char* folder: file directory
     *        uint8_t* file_name: file name
              uint32_t file_name_len: file name length
     */
int get_secp256k1_pub_key_from_file(char* folder, uint8_t* file_name, uint32_t file_name_len, unsigned char* pub_key)
{
    VTEE_OperationHandle operation;
    VTEE_Result ret = VTEE_AllocateOperation(&operation, VTEE_ALG_ECDSA_SECP256K1, VTEE_MODE_SIGN, 0);
    ret = VTEE_prepairKeyPair(operation, (uint8_t*) folder, (uint32_t) strlen(folder), file_name, file_name_len);

    pub_key[0] = 0x04;
    memcpy(pub_key+1, operation->ecdsa_context->Q.X.p, 32);
    memcpy(pub_key+33, operation->ecdsa_context->Q.Y.p, 32);
    VTEE_FreeOperation(operation);
    my_reverse_byte(pub_key+1, 32);
    my_reverse_byte(pub_key+33, 32);
    return ret;
}