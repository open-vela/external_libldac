/*******************************************************************************
 *
 * Copyright (C) 2003 - 2021 Sony Corporation
 *
 ******************************************************************************/

#include "ldac.h"

#ifndef _ENCODE_ONLY
/***************************************************************************************************
    Read and Unpack from MSB
***************************************************************************************************/
static void read_unpack_ldac(
int nbits,
STREAM *p_block,
int *p_loc,
int *p_idata)
{
    register STREAM *p_bufptr;
    register int bit_idx;
    register unsigned int tmp;

    p_bufptr = p_block + (*p_loc >> LDAC_LOC_SHIFT);
    bit_idx = *p_loc & LDAC_LOC_MASK;
    if (*p_loc < 8*LDAC_MAXNBYTES) {
        tmp = (*p_bufptr << 16) | (*(p_bufptr+1) << 8) | (*(p_bufptr+2));
        tmp = 0xffffff & (tmp << bit_idx);
        *p_idata = tmp >> (24 - nbits);
    }
    else {
        *p_idata = 0;
    }

    *p_loc += nbits;

    return;
}

/***************************************************************************************************
    Read Huffman Code
***************************************************************************************************/
static int read_varlencode_dectbl_ldac(
HCDEC *p_hcspec,
const unsigned char *p_dectbl,
STREAM *p_stream,
int *p_loc)
{
    int index, val;

    read_unpack_ldac(p_hcspec->maxlen, p_stream, p_loc, &index);
    val = p_dectbl[index];
    *p_loc -= p_hcspec->maxlen - hc_len_ldac(p_hcspec->p_tbl+val);

    return val;
}


/***************************************************************************************************
    Unpack Frame Header
***************************************************************************************************/
DECLFUNC int unpack_frame_header_ldac(
int *p_smplrate_id,
int *p_chconfig_id,
int *p_frame_length,
int *p_frame_status,
STREAM *p_stream)
{
    int loc = 0;
    int val;

    read_unpack_ldac(LDAC_SYNCWORDBITS, p_stream, &loc, &val);

    if (val == LDAC_SYNCWORD) {
        read_unpack_ldac(LDAC_SMPLRATEBITS, p_stream, &loc, p_smplrate_id);

        read_unpack_ldac(LDAC_CHCONFIG2BITS, p_stream, &loc, p_chconfig_id);

        read_unpack_ldac(LDAC_FRAMELEN2BITS, p_stream, &loc, p_frame_length);
        (*p_frame_length)++;

        read_unpack_ldac(LDAC_FRAMESTATBITS, p_stream, &loc, p_frame_status);
    }
    else {
        return LDAC_FALSE;
    }

    return LDAC_TRUE;
}

/***************************************************************************************************
    Unpack Frame Alignment
***************************************************************************************************/
static int unpack_frame_alignment_ldac(
STREAM *p_stream,
int *p_loc,
int nbytes_frame)
{
    int i;
    int nbytes_filled, dummy;

    nbytes_filled = nbytes_frame - *p_loc / LDAC_BYTESIZE;
    if (nbytes_filled < 0) {
        return LDAC_FALSE;
    }

    for (i = 0; i < nbytes_filled; i++) {
        read_unpack_ldac(LDAC_BYTESIZE, p_stream, p_loc, &dummy);
        if (dummy != LDAC_FILLCODE) {
            return LDAC_FALSE;
        }
    }

    return LDAC_TRUE;
}

/***************************************************************************************************
    Unpack Byte Alignment
***************************************************************************************************/
#define unpack_block_alignment_ldac(p_stream, p_loc) unpack_byte_alignment_ldac((p_stream), (p_loc))

static int unpack_byte_alignment_ldac(
STREAM *p_stream,
int *p_loc)
{
    int nbits_padding, dummy;

    nbits_padding = ((*p_loc + LDAC_BYTESIZE - 1) / LDAC_BYTESIZE) * LDAC_BYTESIZE - *p_loc;

    if (nbits_padding > 0) {
        read_unpack_ldac(nbits_padding, p_stream, p_loc, &dummy);
        if (dummy) {
            return LDAC_FALSE;
        }
    }

    return LDAC_TRUE;
}

/***************************************************************************************************
    Unpack Band Info
***************************************************************************************************/
static int unpack_band_info_ldac(
AB *p_ab,
STREAM *p_stream,
int *p_loc)
{
    read_unpack_ldac(LDAC_NBANDBITS, p_stream, p_loc, &p_ab->nbands);
    p_ab->nbands += LDAC_BAND_OFFSET;

    if (p_ab->nbands > ga_max_nbands_ldac[*p_ab->p_smplrate_id]) {
        *p_ab->p_error_code = LDAC_ERR_SYNTAX_BAND;
        return LDAC_FALSE;
    }

    p_ab->nqus = ga_nqus_ldac[p_ab->nbands];

    read_unpack_ldac(LDAC_FLAGBITS, p_stream, p_loc, &p_ab->ext_flag);

    if (p_ab->ext_flag) {
        read_unpack_ldac(LDAC_EXTMODEBITS, p_stream, p_loc, &p_ab->ext_mode);

        if (p_ab->ext_mode == LDAC_MODE_3) {
            AC *p_ac;
            int ich, rest_bits, val;
            int nchs = p_ab->blk_nchs;

            for (ich = 0; ich < nchs; ich++) {
                p_ac = p_ab->ap_ac[ich];

                read_unpack_ldac(LDAC_EXTSIZEBITS, p_stream, p_loc, &p_ac->ext_size);

                rest_bits = p_ac->ext_size;
                while (rest_bits > 0) {
                    if (rest_bits > 16) {
                        read_unpack_ldac(16, p_stream, p_loc, &val);
                        rest_bits -= 16;
                    }
                    else {
                        read_unpack_ldac(rest_bits, p_stream, p_loc, &val);
                        rest_bits = 0;
                    }
                }
            }
        }
    }

    return LDAC_TRUE;
}

/***************************************************************************************************
    Unpack Gradient Data
***************************************************************************************************/
static int unpack_gradient_ldac(
AB *p_ab,
STREAM *p_stream,
int *p_loc)
{
    read_unpack_ldac(LDAC_GRADMODEBITS, p_stream, p_loc, &p_ab->grad_mode);

    if (p_ab->grad_mode == LDAC_MODE_0) {
        read_unpack_ldac(LDAC_GRADQU0BITS, p_stream, p_loc, &p_ab->grad_qu_l);

        if (p_ab->grad_qu_l >= LDAC_MAXGRADQU) {
            *p_ab->p_error_code = LDAC_ERR_SYNTAX_GRAD_A;
            return LDAC_FALSE;
        }

        read_unpack_ldac(LDAC_GRADQU0BITS, p_stream, p_loc, &p_ab->grad_qu_h);
        p_ab->grad_qu_h++;

        if (p_ab->grad_qu_h >= LDAC_MAXGRADQU+1) {
            *p_ab->p_error_code = LDAC_ERR_SYNTAX_GRAD_B;
            return LDAC_FALSE;
        }
        if (p_ab->grad_qu_l > p_ab->grad_qu_h) {
            *p_ab->p_error_code = LDAC_ERR_SYNTAX_GRAD_C;
            return LDAC_FALSE;
        }

        read_unpack_ldac(LDAC_GRADOSBITS, p_stream, p_loc, &p_ab->grad_os_l);

        read_unpack_ldac(LDAC_GRADOSBITS, p_stream, p_loc, &p_ab->grad_os_h);
    }
    else {
        read_unpack_ldac(LDAC_GRADQU1BITS, p_stream, p_loc, &p_ab->grad_qu_l);

        if (p_ab->grad_qu_l > LDAC_DEFGRADQUH) {
            *p_ab->p_error_code = LDAC_ERR_SYNTAX_GRAD_D;
            return LDAC_FALSE;
        }

        read_unpack_ldac(LDAC_GRADOSBITS, p_stream, p_loc, &p_ab->grad_os_l);

        p_ab->grad_qu_h = LDAC_DEFGRADQUH;
        p_ab->grad_os_h = LDAC_DEFGRADOSH;
    }

    read_unpack_ldac(LDAC_NADJQUBITS, p_stream, p_loc, &p_ab->nadjqus);

    if (p_ab->nadjqus > p_ab->nqus) {
        *p_ab->p_error_code = LDAC_ERR_SYNTAX_GRAD_E;
        return LDAC_FALSE;
    }

    return LDAC_TRUE;
}

/***************************************************************************************************
    Subfunction: Unpack Scale Factor Data - Mode 0
***************************************************************************************************/
static int unpack_scale_factor_0_ldac(
AC *p_ac,
STREAM *p_stream,
int *p_loc)
{
    HCDEC *p_hcsf;
    int iqu;
    int nqus = p_ac->p_ab->nqus;
    int dif;
    const unsigned char *p_tbl;

    read_unpack_ldac(LDAC_SFCBLENBITS, p_stream, p_loc, &p_ac->sfc_bitlen);
    p_ac->sfc_bitlen += LDAC_MINSFCBLEN_0;

    read_unpack_ldac(LDAC_IDSFBITS, p_stream, p_loc, &p_ac->sfc_offset);

    read_unpack_ldac(LDAC_SFCWTBLBITS, p_stream, p_loc, &p_ac->sfc_weight);

    read_unpack_ldac(p_ac->sfc_bitlen, p_stream, p_loc, p_ac->a_idsf+0);

    p_tbl = gaa_sfcwgt_ldac[p_ac->sfc_weight];
    p_hcsf = ga_hcdec_sf0_ldac + (p_ac->sfc_bitlen-LDAC_MINSFCBLEN_0);
    for (iqu = 1; iqu < nqus; iqu++) {
        dif = read_varlencode_dectbl_ldac(p_hcsf, p_hcsf->p_dec, p_stream, p_loc);
        p_ac->a_idsf[iqu] = (p_ac->a_idsf[iqu-1] + dif) & p_hcsf->mask;
        p_ac->a_idsf[iqu-1] += -p_tbl[iqu-1] + p_ac->sfc_offset;
    }
    p_ac->a_idsf[nqus-1] += -p_tbl[nqus-1] + p_ac->sfc_offset;

    return LDAC_TRUE;
}

/***************************************************************************************************
    Subfunction: Unpack Scale Factor Data - Mode 1
***************************************************************************************************/
static int unpack_scale_factor_1_ldac(
AC *p_ac,
STREAM *p_stream,
int *p_loc)
{
    int iqu;
    int nqus = p_ac->p_ab->nqus;
    const unsigned char *p_tbl;

    read_unpack_ldac(LDAC_SFCBLENBITS, p_stream, p_loc, &p_ac->sfc_bitlen);
    p_ac->sfc_bitlen += LDAC_MINSFCBLEN_1;

    if (p_ac->sfc_bitlen > 4) {
        for (iqu = 0; iqu < nqus; iqu++) {
            read_unpack_ldac(LDAC_IDSFBITS, p_stream, p_loc, p_ac->a_idsf+iqu);
        }
    }
    else {
        read_unpack_ldac(LDAC_IDSFBITS, p_stream, p_loc, &p_ac->sfc_offset);

        read_unpack_ldac(LDAC_SFCWTBLBITS, p_stream, p_loc, &p_ac->sfc_weight);

        p_tbl = gaa_sfcwgt_ldac[p_ac->sfc_weight];
        for (iqu = 0; iqu < nqus; iqu++) {
            read_unpack_ldac(p_ac->sfc_bitlen, p_stream, p_loc, p_ac->a_idsf+iqu);
            p_ac->a_idsf[iqu] += -p_tbl[iqu] + p_ac->sfc_offset;
        }
    }

    return LDAC_TRUE;
}

/***************************************************************************************************
    Subfunction: Unpack Scale Factor Data - Mode 2
***************************************************************************************************/
#define LDAC_SFC_MASK 31

static int unpack_scale_factor_2_ldac(
AC *p_ac,
STREAM *p_stream,
int *p_loc)
{
    HCDEC *p_hcsf;
    int iqu;
    int nqus = p_ac->p_ab->nqus;
    int dif;

    read_unpack_ldac(LDAC_SFCBLENBITS, p_stream, p_loc, &p_ac->sfc_bitlen);
    p_ac->sfc_bitlen += LDAC_MINSFCBLEN_2;

    p_hcsf = ga_hcdec_sf1_ldac + (p_ac->sfc_bitlen-LDAC_MINSFCBLEN_2);
    for (iqu = 0; iqu < nqus; iqu++) {
        dif = read_varlencode_dectbl_ldac(p_hcsf, p_hcsf->p_dec, p_stream, p_loc);
        dif = bs_to_int_ldac(dif, p_ac->sfc_bitlen) & LDAC_SFC_MASK;
        p_ac->a_idsf[iqu] = (p_ac->p_ab->ap_ac[0]->a_idsf[iqu] + dif) & LDAC_SFC_MASK;
    }

    return LDAC_TRUE;
}

/***************************************************************************************************
    Unpack Scale Factor Data
***************************************************************************************************/
static int unpack_scale_factor_ldac(
AC *p_ac,
STREAM *p_stream,
int *p_loc)
{
    int iqu;
    int status = 0;
    int sfc_mode;

    for (iqu = 0; iqu < LDAC_MAXNQUS; iqu++) {
        p_ac->a_idsf[iqu] = 0;
    }

    read_unpack_ldac(LDAC_SFCMODEBITS, p_stream, p_loc, &p_ac->sfc_mode);
    sfc_mode = p_ac->sfc_mode;

    if (p_ac->ich == 0) {
        if (sfc_mode == LDAC_MODE_0) {
            status = unpack_scale_factor_0_ldac(p_ac, p_stream, p_loc);
        }
        else {
            status = unpack_scale_factor_1_ldac(p_ac, p_stream, p_loc);
        }
    }
    else {
        if (sfc_mode == LDAC_MODE_0) {
            status = unpack_scale_factor_0_ldac(p_ac, p_stream, p_loc);
        }
        else {
            status = unpack_scale_factor_2_ldac(p_ac, p_stream, p_loc);
        }
    }

    if (!status) {
        return LDAC_FALSE;
    }

    for (iqu = 0; iqu < LDAC_MAXNQUS; iqu++) {
        if ((p_ac->a_idsf[iqu] < 0) || (LDAC_NIDSF <= p_ac->a_idsf[iqu])) {
            *p_ac->p_ab->p_error_code = LDAC_ERR_SYNTAX_IDSF;
            return LDAC_FALSE;
        }
    }

    return LDAC_TRUE;
}

/***************************************************************************************************
    Unpack Spectrum Data
***************************************************************************************************/
static int unpack_spectrum_ldac(
AC *p_ac,
STREAM *p_stream,
int *p_loc)
{
    int iqu, isp, i;
    int lsp, hsp;
    int nqus = p_ac->p_ab->nqus;
    int nsps, idwl1, wl, val;

    for (isp = 0; isp < LDAC_MAXLSU; isp++) {
        p_ac->a_qspec[isp] = 0;
    }

    for (iqu = 0; iqu < nqus; iqu++) {
        lsp = ga_isp_ldac[iqu];
        hsp = ga_isp_ldac[iqu+1];
        nsps = ga_nsps_ldac[iqu];
        idwl1 = p_ac->a_idwl1[iqu];
        wl = ga_wl_ldac[idwl1];

        if (idwl1 == 1) {
            isp = lsp;

            if (nsps == 2) {
                read_unpack_ldac(LDAC_2DIMSPECBITS, p_stream, p_loc, &val);
                p_ac->a_qspec[isp  ] = gaa_2dimdec_spec_ldac[val][0];
                p_ac->a_qspec[isp+1] = gaa_2dimdec_spec_ldac[val][1];
            }
            else {
                for (i = 0; i < nsps>>2; i++, isp+=4) {
                    read_unpack_ldac(LDAC_4DIMSPECBITS, p_stream, p_loc, &val);

                    if (val >= LDAC_N4DIMSPECDECTBL) {
                        *p_ac->p_ab->p_error_code = LDAC_ERR_SYNTAX_SPEC;
                        return LDAC_FALSE;
                    }

                    p_ac->a_qspec[isp  ] = gaa_4dimdec_spec_ldac[val][0];
                    p_ac->a_qspec[isp+1] = gaa_4dimdec_spec_ldac[val][1];
                    p_ac->a_qspec[isp+2] = gaa_4dimdec_spec_ldac[val][2];
                    p_ac->a_qspec[isp+3] = gaa_4dimdec_spec_ldac[val][3];
                }
            }
        }
        else {
            for (isp = lsp; isp < hsp; isp++) {
                read_unpack_ldac(wl, p_stream, p_loc, &val);
                p_ac->a_qspec[isp] = bs_to_int_ldac(val, wl);
            }
        }
    }

    return LDAC_TRUE;
}

/***************************************************************************************************
    Unpack Residual Data
***************************************************************************************************/
static int unpack_residual_ldac(
AC *p_ac,
STREAM *p_stream,
int *p_loc)
{
    int iqu, isp;
    int lsp, hsp;
    int nqus = p_ac->p_ab->nqus;
    int idwl2, wl, val;

    for (isp = 0; isp < LDAC_MAXLSU; isp++) {
        p_ac->a_rspec[isp] = 0;
    }

    for (iqu = 0; iqu < nqus; iqu++) {
        idwl2 = p_ac->a_idwl2[iqu];

        if (idwl2 > 0) {
            lsp = ga_isp_ldac[iqu];
            hsp = ga_isp_ldac[iqu+1];
            wl = ga_wl_ldac[idwl2];

            for (isp = lsp; isp < hsp; isp++) {
                read_unpack_ldac(wl, p_stream, p_loc, &val);
                p_ac->a_rspec[isp] = bs_to_int_ldac(val, wl);
            }
        }
    }


    return LDAC_TRUE;
}

/***************************************************************************************************
    Unpack Audio Block
***************************************************************************************************/
static int unpack_audio_block_ldac(
AB *p_ab,
STREAM *p_stream,
int *p_loc)
{
    AC *p_ac;
    int ich;
    int nchs = p_ab->blk_nchs;
    int status = LDAC_TRUE;

    if (status) {
        status = unpack_band_info_ldac(p_ab, p_stream, p_loc);
    }

    if (status) {
        status = unpack_gradient_ldac(p_ab, p_stream, p_loc);
    }

    if (status) {
        reconst_gradient_ldac(p_ab, 0, p_ab->nqus);
    }

    for (ich = 0; ich < nchs; ich++) {
        p_ac = p_ab->ap_ac[ich];

        if (status) {
            status = unpack_scale_factor_ldac(p_ac, p_stream, p_loc);
        }

        if (status) {
            calc_add_word_length_ldac(p_ac);
        }

        if (status) {
            reconst_word_length_ldac(p_ac);
        }

        if (status) {
            status = unpack_spectrum_ldac(p_ac, p_stream, p_loc);
        }

        if (status) {
            status = unpack_residual_ldac(p_ac, p_stream, p_loc);
        }

        if (status != LDAC_TRUE) {
            break;
        }
    }

    return status;
}

/***************************************************************************************************
    Unpack Raw Data Frame
***************************************************************************************************/
DECLFUNC int unpack_raw_data_frame_ldac(
SFINFO *p_sfinfo,
STREAM *p_stream,
int *p_loc,
int *p_nbytes_used)
{
    CFG *p_cfg = &p_sfinfo->cfg;
    AB *p_ab = p_sfinfo->p_ab;
    int status = LDAC_FALSE;
    int ibk;
    int nbk = gaa_block_setting_ldac[p_cfg->chconfig_id][1];
    int frame_length = p_cfg->frame_length;

    for (ibk = 0; ibk < nbk; ibk++) {
        status = unpack_audio_block_ldac(p_ab, p_stream, p_loc);
        if (!status) {
            return LDAC_ERR_UNPACK_BLOCK_FAILED;
        }

        if (*p_loc > frame_length*LDAC_BYTESIZE) {
            return LDAC_ERR_FRAME_LENGTH_OVER;
        }

        status = unpack_block_alignment_ldac(p_stream, p_loc);
        if (!status) {
            return LDAC_ERR_UNPACK_BLOCK_ALIGN;
        }

        p_ab++;
    }

    status = unpack_frame_alignment_ldac(p_stream, p_loc, frame_length);
    if (!status) {
        return LDAC_ERR_UNPACK_FRAME_ALIGN;
    }

    if (*p_loc > frame_length*LDAC_BYTESIZE) {
        return LDAC_ERR_FRAME_ALIGN_OVER;
    }

    *p_nbytes_used = *p_loc / LDAC_BYTESIZE;

    return LDAC_ERR_NONE;
}

#endif /* _ENCODE_ONLY */

