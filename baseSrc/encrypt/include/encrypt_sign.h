#include <string.h>
#include "cryptographic_generic.h"

#include "asymmetric.h"
#include "keccak256.h"

#define USE_BASIC_CONFIG 1
#include "basic-config.h"
#include "main_impl.h"
#include "mbedtls/bignum.h"
#include "field_impl.h"
#include "ecmult_impl.h"
#include "scalar_impl.h"
#include "scratch_impl.h"
#include "ecmult_gen_impl.h"