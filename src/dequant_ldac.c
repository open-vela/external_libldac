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
    int *p_qspec = p_ac->a_qspec+isp;
    SCALAR *p_nspec = p_ac->p_acsub->a_spec+isp;
    SCALAR iqf = ga_iqf_ldac[p_ac->a_idwl1[iqu]] * ga_sf_ldac[p_ac->a_idsf[iqu]];

    for (i = 0; i < nsps; i++) {
        p_nspec[i] = p_qspec[i] * iqf;
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
    int *p_rspec = p_ac->a_rspec+isp;
    SCALAR *p_nspec = p_ac->p_acsub->a_spec+isp;
    SCALAR irqsf = ga_iqf_ldac[p_ac->a_idwl2[iqu]] * ga_rsf_ldac[LDAC_MAXIDWL1]
            * ga_sf_ldac[p_ac->a_idsf[iqu]];

    for (i = 0; i < nsps; i++) {
        p_nspec[i] += p_rspec[i] * irqsf;
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
    clear_data_ldac(p_ac->p_acsub->a_spec, nsps*sizeof(SCALAR));

    return;
}
#endif /* _ENCODE_ONLY */

