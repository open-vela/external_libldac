/*******************************************************************************
 *
 * Copyright (C) 2003 - 2021 Sony Corporation
 *
 ******************************************************************************/

#include "ldac.h"

#ifndef _DECODE_ONLY
/***************************************************************************************************
    Subfunction: Convert from 16bit Signed Integer PCM
***************************************************************************************************/
__inline static void byte_data_to_scalar_s16_ldac(
char *p_in,
SCALAR *p_out,
int nsmpl)
{
    int i;
    short *p_s;

    p_s = (short *)p_in;
    for (i = 0; i < nsmpl; i++) {
        *p_out++ = (SCALAR)*p_s++;
    }

    return;
}

/***************************************************************************************************
    Subfunction: Convert from 24bit Signed Integer PCM
***************************************************************************************************/
__inline static void byte_data_to_scalar_s24_ldac(
char *p_in,
SCALAR *p_out,
int nsmpl)
{
    int i, val;
    char *p_c;
    SCALAR scale = _scalar(1.0) / _scalar(65536.0);

    p_c = (char *)p_in;
    for (i = 0; i < nsmpl; i++) {
#ifdef LDAC_HOST_ENDIAN_LITTLE
        val  = 0x000000ff & (*p_c++);
        val |= 0x0000ff00 & (*p_c++ << 8);
        val |= 0xffff0000 & (*p_c++ << 16);
#else /* LDAC_HOST_ENDIAN_LITTLE */
        val  = 0xffff0000 & (*p_c++ << 16);
        val |= 0x0000ff00 & (*p_c++ << 8);
        val |= 0x000000ff & (*p_c++);
#endif /* LDAC_HOST_ENDIAN_LITTLE */
        *p_out++ = scale * (SCALAR)(val << 8); /* Sign Extension */
    }

    return;
}

/***************************************************************************************************
    Subfunction: Convert from 32bit Signed Integer PCM
***************************************************************************************************/
__inline static void byte_data_to_scalar_s32_ldac(
char *p_in,
SCALAR *p_out,
int nsmpl)
{
    int i;
    int *p_l;
    SCALAR scale = _scalar(1.0) / _scalar(65536.0);

    p_l = (int *)p_in;
    for (i = 0; i < nsmpl; i++) {
        *p_out++ = scale * (SCALAR)*p_l++;
    }

    return;
}

/***************************************************************************************************
    Subfunction: Convert from 32bit Float PCM
***************************************************************************************************/
__inline static void byte_data_to_scalar_f32_ldac(
char *p_in,
SCALAR *p_out,
int nsmpl)
{
    int i;
    float *p_f;
    SCALAR scale = _scalar(32768.0);

    p_f = (float *)p_in;
    for (i = 0; i < nsmpl; i++) {
        *p_out++ = scale * (SCALAR)*p_f++;
    }

    return;
}

/***************************************************************************************************
    Set Input PCM
***************************************************************************************************/
DECLFUNC void set_input_pcm_ldac(
SFINFO *p_sfinfo,
char *pp_pcm[],
LDAC_SMPL_FMT_T format,
int nlnn)
{
    int ich, isp;
    int nchs = p_sfinfo->cfg.ch;
    int nsmpl = npow2_ldac(nlnn);
    SCALAR *p_time;

    if (format == LDAC_SMPL_FMT_S16) {
        for (ich = 0; ich < nchs; ich++) {
            p_time = p_sfinfo->ap_ac[ich]->p_acsub->a_time;
            for (isp = 0; isp < nsmpl; isp++) {
                p_time[isp] = p_time[nsmpl+isp];
            }
            byte_data_to_scalar_s16_ldac(pp_pcm[ich], p_time+nsmpl, nsmpl);
        }
    }
    else if (format == LDAC_SMPL_FMT_S24) {
        for (ich = 0; ich < nchs; ich++) {
            p_time = p_sfinfo->ap_ac[ich]->p_acsub->a_time;
            for (isp = 0; isp < nsmpl; isp++) {
                p_time[isp] = p_time[nsmpl+isp];
            }
            byte_data_to_scalar_s24_ldac(pp_pcm[ich], p_time+nsmpl, nsmpl);
        }
    }
    else if (format == LDAC_SMPL_FMT_S32) {
        for (ich = 0; ich < nchs; ich++) {
            p_time = p_sfinfo->ap_ac[ich]->p_acsub->a_time;
            for (isp = 0; isp < nsmpl; isp++) {
                p_time[isp] = p_time[nsmpl+isp];
            }
            byte_data_to_scalar_s32_ldac(pp_pcm[ich], p_time+nsmpl, nsmpl);
        }
    }
    else if (format == LDAC_SMPL_FMT_F32) {
        for (ich = 0; ich < nchs; ich++) {
            p_time = p_sfinfo->ap_ac[ich]->p_acsub->a_time;
            for (isp = 0; isp < nsmpl; isp++) {
                p_time[isp] = p_time[nsmpl+isp];
            }
            byte_data_to_scalar_f32_ldac(pp_pcm[ich], p_time+nsmpl, nsmpl);
        }
    }

    return;
}
#endif /* _DECODE_ONLY */

#ifndef _ENCODE_ONLY
/***************************************************************************************************
    Subfunction: Convert to 16bit Signed Integer PCM
***************************************************************************************************/
__inline static void scalar_to_byte_data_s16_ldac(
SCALAR *p_in,
char *p_out,
int nsmpl)
{
    int i;
    int val;
    short *p_s;

    p_s = (short *)p_out;
    for (i = 0; i < nsmpl; i++) {
        val = (int)floor(*p_in++ + _scalar(0.5));
        if (val > (short)0x7fff) {
            val = (short)0x7fff;
        }
        else if (val < (short)0x8000) {
            val = (short)0x8000;
        }
        *p_s++ = (short)val;
    }

    return;
}

/***************************************************************************************************
    Subfunction: Convert to 24bit Signed Integer PCM
***************************************************************************************************/
__inline static void scalar_to_byte_data_s24_ldac(
SCALAR *p_in,
char *p_out,
int nsmpl)
{
    int i;
    int val;
    char *p_c;

    p_c = (char *)p_out;
    for (i = 0; i < nsmpl; i++) {
        val = (int)floor(_scalar(256.0) * *p_in++ + _scalar(0.5));
        if (val > (int)0x007fffff) {
            val = (int)0x007fffff;
        }
        else if (val < (int)0xff800000) {
            val = (int)0xff800000;
        }
#ifdef LDAC_HOST_ENDIAN_LITTLE
        *p_c++ = (char)(val);
        *p_c++ = (char)(val >> 8);
        *p_c++ = (char)(val >> 16);
#else /* LDAC_HOST_ENDIAN_LITTLE */
        *p_c++ = (char)(val >> 16);
        *p_c++ = (char)(val >> 8);
        *p_c++ = (char)(val);
#endif /* LDAC_HOST_ENDIAN_LITTLE */
    }

    return;
}

/***************************************************************************************************
    Subfunction: Convert to 32bit Signed Integer PCM
***************************************************************************************************/
__inline static void scalar_to_byte_data_s32_ldac(
SCALAR *p_in,
char *p_out,
int nsmpl)
{
    int i;
    INT64 val2;
    int *p_l;

    p_l = (int *)p_out;
    for (i = 0; i < nsmpl; i++) {
        val2 = (INT64)floor(_scalar(65536.0) * *p_in++ + _scalar(0.5));
        if (val2 > (int)0x7fffffff) {
            val2 = (int)0x7fffffff;
        }
        else if (val2 < (int)0x80000000) {
            val2 = (int)0x80000000;
        }
        *p_l++ = (int)val2;
    }

    return;
}

/***************************************************************************************************
    Subfunction: Convert to 32bit Float PCM
***************************************************************************************************/
__inline static void scalar_to_byte_data_f32_ldac(
SCALAR *p_in,
char *p_out,
int nsmpl)
{
    int i;
    float *p_f, val2;
    SCALAR scale;

    p_f = (float *)p_out;
    scale = _scalar(1.0) / _scalar(32768.0);
    for (i = 0; i < nsmpl; i++) {
        val2 = (float)(scale * *p_in++);
        if (val2 > 1.0f ) {
            val2 = 1.0f;
        }
        else if (val2 < -1.0f ) {
            val2 = -1.0f;
        }
        *p_f++ = val2;
    }
    return;
}

/***************************************************************************************************
    Set Output PCM
***************************************************************************************************/
DECLFUNC void set_output_pcm_ldac(
SFINFO *p_sfinfo,
char *pp_pcm[],
LDAC_SMPL_FMT_T format,
int nlnn)
{
    int ich;
    int nchs = p_sfinfo->cfg.ch;
    int nsmpl = npow2_ldac(nlnn);
    SCALAR *p_time;

    if (format == LDAC_SMPL_FMT_S16) {
        for (ich = 0; ich < nchs; ich++) {
            p_time = p_sfinfo->ap_ac[ich]->p_acsub->a_time;
            scalar_to_byte_data_s16_ldac(p_time, pp_pcm[ich], nsmpl);
        }
    }
    else if (format == LDAC_SMPL_FMT_S24) {
        for (ich = 0; ich < nchs; ich++) {
            p_time = p_sfinfo->ap_ac[ich]->p_acsub->a_time;
            scalar_to_byte_data_s24_ldac(p_time, pp_pcm[ich], nsmpl);
        }
    }
    else if (format == LDAC_SMPL_FMT_S32) {
        for (ich = 0; ich < nchs; ich++) {
            p_time = p_sfinfo->ap_ac[ich]->p_acsub->a_time;
            scalar_to_byte_data_s32_ldac(p_time, pp_pcm[ich], nsmpl);
        }
    }
    else if (format == LDAC_SMPL_FMT_F32) {
        for (ich = 0; ich < nchs; ich++) {
            p_time = p_sfinfo->ap_ac[ich]->p_acsub->a_time;
            scalar_to_byte_data_f32_ldac(p_time, pp_pcm[ich], nsmpl);
        }
    }

    return;
}
#endif /* _ENCODE_ONLY */

