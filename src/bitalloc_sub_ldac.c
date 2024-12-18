/*******************************************************************************
 *
 * Copyright (C) 2003 - 2021 Sony Corporation
 *
 ******************************************************************************/

#include "ldac.h"

#ifndef _DECODE_ONLY
/***************************************************************************************************
    Calculate Bits for Band Info
***************************************************************************************************/
static int encode_band_info_ldac(
#ifdef __GNUC__
__attribute__((unused)) AB *p_ab)
#else
AB *p_ab)
#endif /* __GNUC__ */
{
    int nbits;

    nbits = LDAC_NBANDBITS + LDAC_FLAGBITS;

    return nbits;
}

/***************************************************************************************************
    Calculate Bits for Gradient Data
***************************************************************************************************/
static int encode_gradient_ldac(
AB *p_ab)
{
    int nbits;

    if (p_ab->grad_mode == LDAC_MODE_0) {
        nbits = LDAC_GRADMODEBITS + LDAC_GRADQU0BITS*2 + LDAC_GRADOSBITS*2 + LDAC_NADJQUBITS;
    }
    else {
        nbits = LDAC_GRADMODEBITS + LDAC_GRADQU1BITS + LDAC_GRADOSBITS + LDAC_NADJQUBITS;
    }

    return nbits;
}

/***************************************************************************************************
    Subfunction: Get Index of Minimum Value
***************************************************************************************************/
__inline static int get_minimum_id_ldac(
int *p_nbits,
int n)
{
    int i;
    int id, nbits;

    id = 0;
    nbits = p_nbits[0];

    for (i = 1; i < n; i++) {
        if (nbits > p_nbits[i]) {
            id = i;
            nbits = p_nbits[i];
        }
    }

    return id;
}

typedef struct {
    int bitlen;
    int offset;
    int weight;
} SFCINF;

/***************************************************************************************************
    Subfunction: Calculate Bits for Scale Factor Data - Mode 0
***************************************************************************************************/
static const unsigned char sa_bitlen_maxdif_0_ldac[LDAC_NIDSF] = {
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
};

static int encode_scale_factor_0_ldac(
AC *p_ac,
SFCINF *p_sfcinf)
{
    HCENC *p_hcsf;
    int iqu, iwt;
    int nqus = p_ac->p_ab->nqus;
    int nbits = LDAC_MAXBITNUM;
    int bitlen, vmin, vmax, val0, val1;
    int *p_idsf = p_ac->a_idsf;
    int *p_idsf_dif = p_ac->a_tmp;
    const unsigned char *p_tbl;

    for (iwt = 0; iwt < LDAC_NSFCWTBL; iwt++) {
        p_tbl = gaa_sfcwgt_ldac[iwt];
        vmin = vmax = val0 = p_idsf[0] + p_tbl[0];
        for (iqu = 1; iqu < nqus; iqu++) {
            val1 = p_idsf[iqu] + p_tbl[iqu];
            if (vmin > val1) {
                vmin = val1;
            }
            if (vmax < val1) {
                vmax = val1;
            }
            p_idsf_dif[iqu] = val1 - val0;
            val0 = val1;
        }

        val1 = bitlen = sa_bitlen_maxdif_0_ldac[(vmax-vmin)>>1];
        p_hcsf = ga_hcenc_sf0_ldac + (bitlen-LDAC_MINSFCBLEN_0);
        for (iqu = 1; iqu < nqus; iqu++) {
            val0 = p_idsf_dif[iqu] & p_hcsf->mask;
            val1 += hc_len_ldac(p_hcsf->p_tbl+val0);
        }

        if (nbits > val1) {
            p_sfcinf->bitlen = bitlen;
            p_sfcinf->offset = vmin;
            p_sfcinf->weight = iwt;
            nbits = val1;
        }
    }
    nbits += LDAC_SFCBLENBITS + LDAC_IDSFBITS + LDAC_SFCWTBLBITS;

    return nbits;
}

/***************************************************************************************************
    Subfunction: Calculate Bits for Scale Factor Data - Mode 1
***************************************************************************************************/
static const unsigned char sa_bitlen_maxdif_1_ldac[LDAC_NIDSF] = {
    2, 2, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
};

static int encode_scale_factor_1_ldac(
AC *p_ac,
SFCINF *p_sfcinf)
{
    int iqu, iwt;
    int nqus = p_ac->p_ab->nqus;
    int nbits = LDAC_MAXBITNUM;
    int bitlen, vmin, vmax, val;
    int *p_idsf = p_ac->a_idsf;
    const unsigned char *p_tbl;

    for (iwt = 0; iwt < LDAC_NSFCWTBL; iwt++) {
        p_tbl = gaa_sfcwgt_ldac[iwt];
        vmin = vmax = p_idsf[0] + p_tbl[0];
        for (iqu = 1; iqu < nqus; iqu++) {
            val = p_idsf[iqu] + p_tbl[iqu];
            if (vmin > val) {
                vmin = val;
            }
            if (vmax < val) {
                vmax = val;
            }
        }

        bitlen = sa_bitlen_maxdif_1_ldac[(vmax-vmin)>>1];
        if (bitlen > 4) {
            val = LDAC_SFCBLENBITS;
        }
        else {
            val = LDAC_SFCBLENBITS + LDAC_IDSFBITS + LDAC_SFCWTBLBITS;
        }
        val += bitlen * nqus;

        if (nbits > val) {
            p_sfcinf->bitlen = bitlen;
            p_sfcinf->offset = vmin;
            p_sfcinf->weight = iwt;
            nbits = val;
        }
    }

    return nbits;
}

/***************************************************************************************************
    Subfunction: Calculate Bits for Scale Factor Data - Mode 2
***************************************************************************************************/
static const unsigned char sa_bitlen_absmax_2_ldac[LDAC_NIDSF>>1] = {
    2, 3, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
};

static int encode_scale_factor_2_ldac(
AC *p_ac,
SFCINF *p_sfcinf)
{
    HCENC *p_hcsf;
    int iqu;
    int nqus = p_ac->p_ab->nqus;
    int nbits, bitlen, vmax, val;
    int *p_idsf_dif = p_ac->a_tmp;

    p_idsf_dif[0] = p_ac->a_idsf[0] - p_ac->p_ab->ap_ac[0]->a_idsf[0];
    vmax = abs(p_idsf_dif[0]);
    for (iqu = 1; iqu < nqus; iqu++) {
        p_idsf_dif[iqu] = p_ac->a_idsf[iqu] - p_ac->p_ab->ap_ac[0]->a_idsf[iqu];
        val = abs(p_idsf_dif[iqu]);
        if (vmax < val) {
            vmax = val;
        }
    }

    nbits = LDAC_SFCBLENBITS;
    bitlen = sa_bitlen_absmax_2_ldac[vmax>>1];
    p_hcsf = ga_hcenc_sf1_ldac + (bitlen-LDAC_MINSFCBLEN_2);
    for (iqu = 0; iqu < nqus; iqu++) {
        val = p_idsf_dif[iqu] & p_hcsf->mask;
        nbits += hc_len_ldac(p_hcsf->p_tbl+val);
    }

    p_sfcinf->bitlen = bitlen;
    p_sfcinf->offset = 0;
    p_sfcinf->weight = 0;

    return nbits;
}

/***************************************************************************************************
    Calculate Bits for Scale Factor Data
***************************************************************************************************/
static int encode_scale_factor_ldac(
AC *p_ac)
{
    SFCINF a_sfcinf[LDAC_NSFCMODE];
    SFCINF *p_sfcinf;
    int nbits, sfc_mode;
    int a_nbits[LDAC_NSFCMODE];

    if (p_ac->ich == 0) {
        a_nbits[LDAC_MODE_0] = encode_scale_factor_0_ldac(p_ac, a_sfcinf+LDAC_MODE_0);
        a_nbits[LDAC_MODE_1] = encode_scale_factor_1_ldac(p_ac, a_sfcinf+LDAC_MODE_1);
    }
    else {
        a_nbits[LDAC_MODE_0] = encode_scale_factor_0_ldac(p_ac, a_sfcinf+LDAC_MODE_0);
        a_nbits[LDAC_MODE_1] = encode_scale_factor_2_ldac(p_ac, a_sfcinf+LDAC_MODE_1);
    }

    p_ac->sfc_mode = sfc_mode = get_minimum_id_ldac(a_nbits, LDAC_MODE_1+1);
    p_sfcinf = a_sfcinf + sfc_mode;
    p_ac->sfc_bitlen = p_sfcinf->bitlen;
    p_ac->sfc_offset = p_sfcinf->offset;
    p_ac->sfc_weight = p_sfcinf->weight;
    nbits = a_nbits[sfc_mode] + LDAC_SFCMODEBITS;

    return nbits;
}

/***************************************************************************************************
    Calculate Bits for Side Information (Band Info, Gradient Data & Scale Factor Data)
***************************************************************************************************/
DECLFUNC int encode_side_info_ldac(
AB *p_ab)
{
    AC *p_ac;
    int ich;
    int nchs = p_ab->blk_nchs;
    int nbits, nbits_band, nbits_grad, nbits_scfc = 0;

    p_ab->nbits_band = nbits_band = encode_band_info_ldac(p_ab);
    p_ab->nbits_grad = nbits_grad = encode_gradient_ldac(p_ab);
    for (ich = 0; ich < nchs; ich++) {
        p_ac = p_ab->ap_ac[ich];
        nbits_scfc += encode_scale_factor_ldac(p_ac);
        calc_add_word_length_ldac(p_ac);
    }
    p_ab->nbits_scfc = nbits_scfc;

    nbits = nbits_band + nbits_grad + nbits_scfc;

    return nbits;
}
#endif /* _DECODE_ONLY */

/***************************************************************************************************
    Calculate Additional Word Length Data
***************************************************************************************************/
DECLFUNC void calc_add_word_length_ldac(
AC *p_ac)
{
    int iqu;
    int nqus = p_ac->p_ab->nqus;
    int dif;
    int *p_idsf = p_ac->a_idsf;
    int *p_addwl = p_ac->a_addwl;

    clear_data_ldac(p_addwl, LDAC_MAXNQUS*sizeof(int));

    if (p_ac->p_ab->grad_mode != LDAC_MODE_0) {
        for (iqu = 1; iqu < nqus; iqu++) {
            dif = p_idsf[iqu] - p_idsf[iqu-1];

            if (dif > 0) {
                if (dif > 5) {
                    p_addwl[iqu] += 5;
                }
                else if (dif > 4) {
                    p_addwl[iqu] += 4;
                }
                else if (dif > 3) {
                    p_addwl[iqu] += 3;
                }
                else if (dif > 2) {
                    p_addwl[iqu] += 2;
                }
                else if (dif > 1) {
                    p_addwl[iqu] += 1;
                }
            }
            else {
                if (dif < -5) {
                    p_addwl[iqu-1] += 5;
                }
                else if (dif < -4) {
                    p_addwl[iqu-1] += 4;
                }
                else if (dif < -3) {
                    p_addwl[iqu-1] += 3;
                }
                else if (dif < -2) {
                    p_addwl[iqu-1] += 2;
                }
                else if (dif < -1) {
                    p_addwl[iqu-1] += 1;
                }
            }
        }
    }

    return;
}

#ifndef _ENCODE_ONLY
/***************************************************************************************************
    Reconstruct Gradient Curve Data
***************************************************************************************************/
DECLFUNC void reconst_gradient_ldac(
AB *p_ab,
int lqu,
int hqu)
{
    int iqu;
    int tmp;
    int grad_qu_l = p_ab->grad_qu_l;
    int grad_qu_h = p_ab->grad_qu_h;
    int grad_os_l = p_ab->grad_os_l;
    int grad_os_h = p_ab->grad_os_h;
    int *p_grad = p_ab->a_grad;
    const unsigned char *p_t;

    tmp = grad_qu_h - grad_qu_l;

    for (iqu = lqu; iqu < grad_qu_h; iqu++) {
        p_grad[iqu] = -grad_os_l;
    }
    for (iqu = grad_qu_h; iqu < hqu; iqu++) {
        p_grad[iqu] = -grad_os_h;
    }

    if (tmp > 0) {
        p_t = gaa_resamp_grad_ldac[tmp-1];

        tmp = grad_os_h - grad_os_l;
        if (tmp > 0) {
            tmp = tmp-1;
            for (iqu = grad_qu_l; iqu < grad_qu_h; iqu++) {
                p_grad[iqu] -= ((*p_t++ * tmp) >> 8) + 1;
            }
        }
        else if (tmp < 0) {
            tmp = -tmp-1;
            for (iqu = grad_qu_l; iqu < grad_qu_h; iqu++) {
                p_grad[iqu] += ((*p_t++ * tmp) >> 8) + 1;
            }
        }
    }

    return;
}

/***************************************************************************************************
    Reconstruct Word Length Data
***************************************************************************************************/
DECLFUNC void reconst_word_length_ldac(
AC *p_ac)
{
    int iqu;
    int nqus = p_ac->p_ab->nqus;
    int nadjqus = p_ac->p_ab->nadjqus;
    int grad_mode = p_ac->p_ab->grad_mode;
    int *p_grad = p_ac->p_ab->a_grad;
    int *p_idsf = p_ac->a_idsf;
    int *p_addwl = p_ac->a_addwl;
    int *p_idwl1 = p_ac->a_idwl1;
    int *p_idwl2 = p_ac->a_idwl2;

    if (grad_mode == LDAC_MODE_0) {
        for (iqu = 0; iqu < nqus; iqu++) {
            p_idwl1[iqu] = p_idsf[iqu] + p_grad[iqu];
        }
    }
    else if (grad_mode == LDAC_MODE_1) {
        for (iqu = 0; iqu < nqus; iqu++) {
            p_idwl1[iqu] = p_idsf[iqu] + p_grad[iqu] + p_addwl[iqu];
            if (p_idwl1[iqu] > 0) {
                p_idwl1[iqu] = (p_idwl1[iqu]) >> 1;
            }
        }
    }
    else if (grad_mode == LDAC_MODE_2) {
        for (iqu = 0; iqu < nqus; iqu++) {
            p_idwl1[iqu] = p_idsf[iqu] + p_grad[iqu] + p_addwl[iqu];
            if (p_idwl1[iqu] > 0) {
                p_idwl1[iqu] = (p_idwl1[iqu]*3) >> 3;
            }
        }
    }
    else if (grad_mode == LDAC_MODE_3) {
        for (iqu = 0; iqu < nqus; iqu++) {
            p_idwl1[iqu] = p_idsf[iqu] + p_grad[iqu] + p_addwl[iqu];
            if (p_idwl1[iqu] > 0) {
                p_idwl1[iqu] = (p_idwl1[iqu]) >> 2;
            }
        }
    }

    for (iqu = 0; iqu < nqus; iqu++) {
        if (p_idwl1[iqu] < LDAC_MINIDWL1) {
            p_idwl1[iqu] = LDAC_MINIDWL1;
        }
    }

    for (iqu = 0; iqu < nadjqus; iqu++) {
        p_idwl1[iqu]++;
    }

    for (iqu = 0; iqu < nqus; iqu++) {
        p_idwl2[iqu] = 0;
        if (p_idwl1[iqu] > LDAC_MAXIDWL1) {
            p_idwl2[iqu] = p_idwl1[iqu] - LDAC_MAXIDWL1;
            if (p_idwl2[iqu] > LDAC_MAXIDWL2) {
                p_idwl2[iqu] = LDAC_MAXIDWL2;
            }
            p_idwl1[iqu] = LDAC_MAXIDWL1;
        }
    }

    return;
}
#endif /* _ENCODE_ONLY */

