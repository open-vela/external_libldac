/*******************************************************************************
 *
 * Copyright (C) 2013 - 2021 Sony Corporation
 *
 ******************************************************************************/


#include "ldac.h"

/* Common Files */
#include "tables_ldac.c"
#ifndef _32BIT_FIXED_POINT
#include "tables_sigproc_ldac.c"
#include "setpcm_ldac.c"
#else /* _32BIT_FIXED_POINT */
#include "tables_sigproc_fixp_ldac.c"
#include "setpcm_fixp_ldac.c"
#include "func_fixp_ldac.c"
#endif /* _32BIT_FIXED_POINT */
#include "bitalloc_sub_ldac.c"
#include "memory_ldac.c"
#include "ldaclib_api.c"

#ifndef _DECODE_ONLY
/* Encoder Files */
#ifndef _32BIT_FIXED_POINT
#include "mdct_ldac.c"
#include "sigana_ldac.c"
#include "quant_ldac.c"
#else /* _32BIT_FIXED_POINT */
#include "mdct_fixp_ldac.c"
#include "sigana_fixp_ldac.c"
#include "quant_fixp_ldac.c"
#endif /* _32BIT_FIXED_POINT */
#include "bitalloc_ldac.c"
#include "pack_ldac.c"
#include "encode_ldac.c"
#endif /* _DECODE_ONLY */

#ifndef _ENCODE_ONLY
/* Decoder Files */
#ifndef _32BIT_FIXED_POINT
#include "imdct_ldac.c"
#include "dequant_ldac.c"
#else /* _32BIT_FIXED_POINT */
#include "imdct_fixp_ldac.c"
#include "dequant_fixp_ldac.c"
#endif /* _32BIT_FIXED_POINT */
#include "unpack_ldac.c"
#include "decode_ldac.c"
#endif /* _ENCODE_ONLY */

