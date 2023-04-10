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
SCALAR *p_y,
SCALAR *p_x,
int nlnn)
{
    int i, j;
    int loop1, loop2;
    int coef, index0, index1, offset;
    int nsmpl = npow2_ldac(nlnn);
    const int *p_p;
    const SCALAR *p_w, *p_c, *p_s;
    SCALAR a_work[LDAC_MAXLSU];
    SCALAR *p_work = a_work;
    SCALAR a, b, c, d;
    SCALAR cc, cs;

    i = nlnn - LDAC_1FSLNN;
    p_w = gaa_bwin_ldac[i];
    p_c = gaa_wcos_ldac[i];
    p_s = gaa_wsin_ldac[i];
    p_p = gaa_rev_perm_ldac[i];

    /* Butterfly 1st Stage & Reorder */
    coef = 0;
    loop1 = nsmpl / 4;
    index0 = 0;
    offset = 4;

    cc = p_c[coef];
    cs = p_s[coef++]; /* for Butterfly 2nd Stage, Update coef Index */

    for (j = 0; j < loop1; j++)
    {
        a = p_y[p_p[index0  ]];
        b = p_y[p_p[index0+1]];
        c = a * cc + b * cs;
        d = a * cs - b * cc;
        a = p_y[p_p[index0+2]];
        b = p_y[p_p[index0+3]];

        p_work[index0+0] = a + c;
        p_work[index0+1] = b + d;
        p_work[index0+2] = a - c;
        p_work[index0+3] = b - d;

        index0 += offset;
    }

    /* for (i = 1; i < nlnn-1; i++) */
    i = 1;
    do {
        loop2 = 1 << i;
        loop1 = nsmpl >> (i+2);
        index0 = 0;
        index1 = 1 << (i+1);
        offset = 1 << (i+2);

        /* for (k = 0; k < loop2; k++) */
        do {
            cc = p_c[coef];
            cs = p_s[coef++];

            /* for (j = 0; j < loop1; j++) */
            j = loop1;
            do {

                a = p_work[index1];
                b = p_work[index1+1];
                c = a * cc + b * cs;
                d = a * cs - b * cc;
                a = p_work[index0];
                b = p_work[index0+1];

                p_work[index0] = a + c;
                p_work[index0+1] = b + d;
                p_work[index1] = a - c;
                p_work[index1+1] = b - d;

                index0 += offset;
                index1 += offset;
            }
            while (--j > 0);

            index0 += 2-nsmpl;
            index1 += 2-nsmpl;
        }
        while (--loop2 > 0);
    }
    while (++i < nlnn-1);

    /* Butterfly Last Stage */
    for (i = 0; i < nsmpl>>1; i++) {
        index0 = i << 1;
        cc = p_c[coef];
        cs = p_s[coef++];

        a = p_work[index0] * cc + p_work[index0+1] * cs;
        b = p_work[index0] * cs - p_work[index0+1] * cc;
        p_y[index0] = a;
        p_y[nsmpl-index0-1] = b;
    }

    /* Windowing */
    for (i = 0; i < nsmpl>>1; i++) {
        p_x[i] = p_y[nsmpl/2+i] * p_w[i] - p_x[nsmpl+i] * p_w[nsmpl-1-i];
        p_x[nsmpl/2+i] = -p_y[nsmpl-1-i] * p_w[nsmpl/2+i] - p_x[nsmpl+nsmpl/2+i] * p_w[nsmpl/2-1-i];

        p_x[nsmpl+i] = p_y[nsmpl/2-1-i];
        p_x[nsmpl+nsmpl/2+i] = p_y[i];
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

