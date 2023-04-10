/*******************************************************************************
 *
 * Copyright (C) 2003 - 2021 Sony Corporation
 *
 ******************************************************************************/

#include "ldac.h"

#ifndef _ENCODE_ONLY

/***************************************************************************************************
    Subfunction: Dequantize Spectrum Core
***************************************************************************************************/
__inline static void dequant_spectrum_core_ldac(
AC *p_ac,
int iqu)
{
    int i;
    int isp = ga_isp_ldac[iqu];
    int nsps = ga_nsps_ldac[iqu];
    int shift = LDAC_Q_DEQUANT1 + LDAC_Q_NORM - p_ac->a_idsf[iqu];
    int *p_qspec = p_ac->a_qspec+isp;
    INT32 iqf = ga_iqf_ldac[p_ac->a_idwl1[iqu]];
    INT32 *p_nspec = p_ac->p_acsub->a_spec+isp;

    if (shift > 0){
        for (i = 0; i < nsps; i++) {
            /* Q31 <- Q00 * Q31 */
            p_nspec[i] = mul_rsftrnd_ldac(p_qspec[i], iqf, shift);
        }
    }
    else{
        for (i = 0; i < nsps; i++) {
            /* Q31 <- Q00 * Q31 */
            p_nspec[i] = mul_lsftrnd_ldac(p_qspec[i], iqf, shift);
        }
    }

    return;
}

/***************************************************************************************************
    Dequantize Spectrum
***************************************************************************************************/
DECLFUNC void dequant_spectrum_ldac(
AC *p_ac)
{
    int iqu;
    int nqus = p_ac->p_ab->nqus;

    for (iqu = 0; iqu < nqus; iqu++) {
        dequant_spectrum_core_ldac(p_ac, iqu);
    }

    return;
}

/***************************************************************************************************
    Subfunction: Dequantize Residual Spectrum Core
***************************************************************************************************/
static void dequant_residual_core_ldac(
AC *p_ac,
int iqu)
{
    int i;
    int isp = ga_isp_ldac[iqu];
    int nsps = ga_nsps_ldac[iqu];
    int shift = LDAC_Q_DEQUANT3 + LDAC_Q_NORM - p_ac->a_idsf[iqu];
    int *p_rspec = p_ac->a_rspec+isp;
    INT32 rnspec;
    INT32 iqf = ga_iqf_ldac[p_ac->a_idwl2[iqu]];
    INT32 rsf = ga_rsf_ldac[LDAC_MAXIDWL1];
    INT32 *p_nspec = p_ac->p_acsub->a_spec+isp;

    if (shift > 0){
        for (i = 0; i < nsps; i++) {
            /* Q31 <- Q00 * Q31 */
            rnspec = mul_lsftrnd_ldac(p_rspec[i], iqf, LDAC_Q_DEQUANT2);
            /* Q31 <- Q31 * Q31 */
            p_nspec[i] += mul_rsftrnd_ldac(rnspec, rsf, shift);
        }
    }
    else{
        for (i = 0; i < nsps; i++) {
            /* Q31 <- Q00 * Q31 */
            rnspec = mul_lsftrnd_ldac(p_rspec[i], iqf, LDAC_Q_DEQUANT2);
            /* Q31 <- Q31 * Q31 */
            p_nspec[i] += mul_lsftrnd_ldac(rnspec, rsf, shift);
        }
    }

    return;
}

/***************************************************************************************************
    Dequantize Residual Spectrum
***************************************************************************************************/
DECLFUNC void dequant_residual_ldac(
AC *p_ac)
{
    int iqu;
    int nqus = p_ac->p_ab->nqus;
    int *p_idwl2 = p_ac->a_idwl2;

    for (iqu = 0; iqu < nqus; iqu++) {
        if (p_idwl2[iqu] > 0) {
            dequant_residual_core_ldac(p_ac, iqu);
        }
    }

    return;
}

/***************************************************************************************************
    Clear Spectrum
***************************************************************************************************/
DECLFUNC void clear_spectrum_ldac(
AC *p_ac,
int nsps)
{
    clear_data_ldac(p_ac->p_acsub->a_spec, nsps*sizeof(INT32));

    return;
}
#endif /* _ENCODE_ONLY */

