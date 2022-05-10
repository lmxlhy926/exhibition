#include "message_digest.h"
#include "cryptographic_generic.h"

void VTEE_DigestUpdate(VTEE_OperationHandle operation, void* chunk, uint32_t chunkSize)
{
    if (operation->md_context == NULL)
    {
        return;
    }
    mbedtls_md_starts(operation->md_context);
    mbedtls_md_update(operation->md_context, chunk, chunkSize);
}

VTEE_Result VTEE_DigestDoFinal(VTEE_OperationHandle operation, void* chunk, uint32_t chunkLen, void* hash, uint32_t *hashLen)
{
    if (operation->md_context == NULL)
    {
        return VTEE_ERROR_BAD_PARAMETERS;
    }
    mbedtls_md_finish(operation->md_context, hash);
    *hashLen = operation->operationInfo.digestLength;
    return VTEE_SUCCESS;
}
