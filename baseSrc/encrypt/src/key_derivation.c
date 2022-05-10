#include "key_derivation.h"
#include "cryptographic_generic.h"
#include "asymmetric.h"
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

static size_t olen;

void print_mpi(mbedtls_mpi *mpi, char* name)
{
    int length = mbedtls_mpi_size(mpi);
    unsigned char *buffer = (unsigned char *) malloc(length);

    uint32_t ret = mbedtls_mpi_write_binary(mpi, buffer, length);
    if( ret != 0 )
    {
        mbedtls_printf( " mbedtls_mpi_write_binary ret %d\n", ret );
    }
print_hex(name, buffer, length);
    free(buffer);
}

void VTEE_DeriveKey(VTEE_OperationHandle operation, VTEE_Attribute_t* params, uint32_t paramCount, VTEE_ObjectHandle derivedKey)
{
    uint32_t ret = 0;
    uint8_t ecdh_buf[1024];

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_ctr_drbg_init( &ctr_drbg );
    mbedtls_entropy_init( &entropy );
    ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) pers, strlen(pers));

    mbedtls_ecp_copy( &operation->ecdh_context->Qp, &operation->ecdh_context->Q );

    for (int i = 0; i < paramCount; i++)
    {
        ref_t ref;
        memcpy(&ref, &params[i].content, sizeof(ref_t));
        if (params[i].attributeID == 1)
        {
            ret = mbedtls_mpi_read_binary( &operation->ecdh_context->Qp.X, ref.buffer, ref.length);
            if (ret != 0)
            {
                mbedtls_printf( " failed\n  ! mbedtls_mpi_read_binary returned %d\n", ret );
            }
        }
        else
        {
            ret = mbedtls_mpi_read_binary( &operation->ecdh_context->Qp.Y, ref.buffer, ref.length);
            if (ret != 0)
            {
                mbedtls_printf( " failed\n  ! mbedtls_mpi_read_binary returned %d\n", ret );
            }
        }
    }

    ret = mbedtls_ecdh_calc_secret( operation->ecdh_context, &olen, ecdh_buf, sizeof( ecdh_buf ), mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0)
    {
        mbedtls_printf( " failed\n  ! mbedtls_ecdh_calc_secret returned %d\n", ret );
    }

    ret = mbedtls_mpi_write_binary( &operation->ecdh_context->z, derivedKey->buffer, derivedKey->buffer_len);
    if (ret != 0)
    {
        mbedtls_printf( " failed\n  ! mbedtls_mpi_write_binary returned %d\n", ret );
    }

    mbedtls_ecdh_free( operation->ecdh_context );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
    return;
}

static const unsigned char secret_key[16] = {
    0xf4, 0x82, 0xc6, 0x70, 0x3c, 0xc7, 0x61, 0x0a,
    0xb9, 0xa0, 0xb8, 0xe9, 0x87, 0xb8, 0xc1, 0x72,
};

void VTEE_GenerateRandom(void* randomBuffer, uint32_t randomBufferLen)
{
    unsigned char ciphertext[128] = { 0 };
    int ret;
    /*
     * Setup random number generator
     * (Note: later this might be done automatically.)
     */
    mbedtls_entropy_context entropy;    /* entropy pool for seeding PRNG */
    mbedtls_ctr_drbg_context drbg;      /* pseudo-random generator */

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&drbg);

    /* Seed the PRNG using the entropy pool, and throw in our secret key as an
     * additional source of randomness. */
    ret = mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &entropy, secret_key, sizeof secret_key);
    if (ret != 0) {
        return;
    }
    mbedtls_ctr_drbg_random(&drbg, (unsigned char *)randomBuffer, randomBufferLen);
    mbedtls_ctr_drbg_free( &drbg );
    mbedtls_entropy_free( &entropy );
}
