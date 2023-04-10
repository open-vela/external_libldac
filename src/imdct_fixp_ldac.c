/*******************************************************************************
 *
 * Copyright (C) 2003 - 2021 Sony Corporation
 *
 ******************************************************************************/

#include "ldac.h"

#ifndef _ENCODE_ONLY
/***************************************************************************************************
    Subfunction: Process IMDCT Core
***************************************************************************************************/
static void proc_imdct_core_ldac(
INT32 *p_y,
INT32 *p_x,
int nlnn)
{
    INT32 i, j, k;
    INT32 loop1, loop2;
    INT32 coef, index0, index1, offset;
    int nsmpl = npow2_ldac(nlnn);
    int shift;
    const int *p_p;
    const INT32 *p_w, *p_c, *p_s;
    INT32 a_work[LDAC_MAXLSU];
    INT32 a_work2[LDAC_MAXLSU];
    INT32 g0, g1, g2, g3;

    i = nlnn - LDAC_1FSLNN;
    p_w = gaa_bwin_ldac[i];
    p_c = gaa_wcos_ldac[i];
    p_s = gaa_wsin_ldac[i];
    p_p = gaa_perm_ldac[i];

    /* Block Floating */
    shift = LDAC_C_BLKFLT - get_bit_length_ldac(get_absmax_ldac(p_y, nsmpl)) - 1;
    if (shift <= 0) {
        shift = 0;
        for (i = 0; i < nsmpl; i++) {
            a_work[p_p[i]] = p_y[i];
        }
    }
    else{
        for (i = 0; i < nsmpl; i++) {
            a_work[p_p[i]] = lsftrnd_ldac(p_y[i], -shift);
        }
    }

    /* Butterfly */
    coef = 0;
    for (i = 0; i < nlnn-1; i++) {
        loop1 = 1 << (nlnn-2-i);
        loop2 = 1 << i;
        index0 = 0;
        index1 = 1 << (i+1);
        offset = 1 << (i+1);

        for (j = 0; j < loop1; j++) {
            for (k = 0; k < loop2; k++) {
                g0 = mul_rsftrnd_ldac(a_work[index1], p_c[coef], LDAC_Q_MDCT_COS+1);
                g1 = mul_rsftrnd_ldac(a_work[index1+1], p_s[coef], LDAC_Q_MDCT_SIN+1);
                g2 = g0 + g1;

                g0 = mul_rsftrnd_ldac(a_work[index1], p_s[coef], LDAC_Q_MDCT_SIN+1);
                g1 = mul_rsftrnd_ldac(a_work[index1+1], p_c[coef], LDAC_Q_MDCT_COS+1);
                g3 = g0 - g1;

                g0 = a_work[index0] >> 1;
                g1 = a_work[index0+1] >> 1;

                a_work[index0] = g0 + g2;
                a_work[index0+1] = g1 + g3;
                a_work[index1] = g0 - g2;
                a_work[index1+1] = g1 - g3;

                index0 += 2;
                index1 += 2;
                coef++;
            }
            index0 += offset;
            index1 += offset;
            coef -= loop2;
        }
        coef += loop2;
    }

    for (i = 0; i < nsmpl>>1; i++) {
        index0 = i << 1;

        g0 = mul_rsftrnd_ldac(a_work[index0], p_c[coef], LDAC_Q_MDCT_COS);
        g1 = mul_rsftrnd_ldac(a_work[index0+1], p_s[coef], LDAC_Q_MDCT_SIN);
        a_work2[index0] = g0 + g1;

        g0 = mul_rsftrnd_ldac(a_work[index0], p_s[coef], LDAC_Q_MDCT_SIN);
        g1 = mul_rsftrnd_ldac(a_work[index0+1], p_c[coef], LDAC_Q_MDCT_COS);
        a_work2[nsmpl-index0-1] = g0 - g1;

        coef++;
    }

    /* Windowing */
    for (i = 0; i < nsmpl>>1; i++) {
        p_x[i] = p_x[nsmpl+i] + mul_rsftrnd_ldac(a_work2[nsmpl/2+i], p_w[i], LDAC_Q_MDCT_WIN+shift-(nlnn-1));
        p_x[nsmpl/2+i] = p_x[3*nsmpl/2+i] + mul_rsftrnd_ldac(-a_work2[nsmpl-1-i], p_w[nsmpl/2+i], LDAC_Q_MDCT_WIN+shift-(nlnn-1));

        p_x[nsmpl+i] = mul_rsftrnd_ldac(-a_work2[nsmpl/2-1-i], p_w[nsmpl-1-i], LDAC_Q_MDCT_WIN+shift-(nlnn-1));
        p_x[3*nsmpl/2+i] = mul_rsftrnd_ldac(-a_work2[i], p_w[nsmpl/2-1-i], LDAC_Q_MDCT_WIN+shift-(nlnn-1));
    }

    return;
}

/***************************************************************************************************
    Process IMDCT
***************************************************************************************************/
DECLFUNC void proc_imdct_ldac(
SFINFO *p_sfinfo,
int nlnn)
{
    AC *p_ac;
    int ich;
    int nchs = p_sfinfo->cfg.ch;

    for (ich = 0; ich < nchs; ich++) {
        p_ac = p_sfinfo->ap_ac[ich];
        proc_imdct_core_ldac(p_ac->p_acsub->a_spec, p_ac->p_acsub->a_time, nlnn);
    }

    return;
}
#endif /* _ENCODE_ONLY */

