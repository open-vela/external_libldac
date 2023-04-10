/*******************************************************************************
 *
 * Copyright (C) 2003 - 2021 Sony Corporation
 *
 ******************************************************************************/

#include "ldac.h"

#ifndef _ENCODE_ONLY
/***************************************************************************************************
    Allocate Memory
***************************************************************************************************/
static LDAC_RESULT alloc_decode_ldac(
SFINFO *p_sfinfo)
{
    LDAC_RESULT result = LDAC_S_OK;
    CFG *p_cfg = &p_sfinfo->cfg;
    int ich;
    int nchs = p_cfg->ch;
    int nbks = gaa_block_setting_ldac[p_cfg->chconfig_id][1];

    /* Allocate AC */
    for (ich = 0; ich < nchs; ich++) {
        p_sfinfo->ap_ac[ich] = (AC *)calloc_ldac(p_sfinfo, 1, sizeof(AC));
        if (p_sfinfo->ap_ac[ich] != (AC *)NULL) {
            p_sfinfo->ap_ac[ich]->p_acsub = (ACSUB *)calloc_ldac(p_sfinfo, 1, sizeof(ACSUB));
            if (p_sfinfo->ap_ac[ich]->p_acsub == (ACSUB *)NULL) {
                result = LDAC_E_FAIL;
                break;
            }
        }
        else {
            result = LDAC_E_FAIL;
            break;
        }
    }

    if (result != LDAC_S_OK) {
        return result;
    }

    /* Allocate AB */
    p_sfinfo->p_ab = (AB *)calloc_ldac(p_sfinfo, nbks, sizeof(AB));
    if (p_sfinfo->p_ab == (AB *)NULL) {
        result = LDAC_E_FAIL;
    }

    return result;
}

/***************************************************************************************************
    Initialize Memory
***************************************************************************************************/
DECLFUNC LDAC_RESULT init_decode_ldac(
SFINFO *p_sfinfo)
{
    LDAC_RESULT result = LDAC_S_OK;
    CFG *p_cfg = &p_sfinfo->cfg;
    AB *p_ab;
    int ibk, ich;
    int blk_type, blk_nchs;
    int ch_offset = 0;
    int chconfig_id = p_cfg->chconfig_id;
    int nbks = gaa_block_setting_ldac[chconfig_id][1];

    if (alloc_decode_ldac(p_sfinfo) == LDAC_E_FAIL) {
        p_sfinfo->error_code = LDAC_ERR_ALLOC_MEMORY;
        return LDAC_E_FAIL;
    }

    p_sfinfo->error_code = LDAC_ERR_NONE;

    /* Set AB Information */
    p_ab = p_sfinfo->p_ab;
    for (ibk = 0; ibk < nbks; ibk++) {
        p_ab->blk_type = blk_type = gaa_block_setting_ldac[chconfig_id][ibk+2];
        p_ab->blk_nchs = blk_nchs = get_block_nchs_ldac(blk_type);
        p_ab->p_smplrate_id = &p_cfg->smplrate_id;
        p_ab->p_error_code = &p_sfinfo->error_code;

        /* Set AC Information */
        for (ich = 0; ich < blk_nchs; ich++) {
            p_ab->ap_ac[ich] = p_sfinfo->ap_ac[ch_offset++];
            p_ab->ap_ac[ich]->p_ab = p_ab;
            p_ab->ap_ac[ich]->ich = ich;
        }

        p_ab++;
    }

    return result;
}

/***************************************************************************************************
    Free Memory
***************************************************************************************************/
DECLFUNC void free_decode_ldac(
SFINFO *p_sfinfo)
{
    int ich;
    int nchs = p_sfinfo->cfg.ch;

    /* Free AB */
    if (p_sfinfo->p_ab != (AB *)NULL) {
        free(p_sfinfo->p_ab);
        p_sfinfo->p_ab = (AB *)NULL;
    }

    /* Free AC */
    for (ich = 0; ich < nchs; ich++) {
        if (p_sfinfo->ap_ac[ich] != (AC *)NULL) {
            if (p_sfinfo->ap_ac[ich]->p_acsub != (ACSUB *)NULL) {
                free(p_sfinfo->ap_ac[ich]->p_acsub);
                p_sfinfo->ap_ac[ich]->p_acsub = (ACSUB *)NULL;
            }
            free(p_sfinfo->ap_ac[ich]);
            p_sfinfo->ap_ac[ich] = (AC *)NULL;
        }
    }

    return;
}

/***************************************************************************************************
    Decode Audio Block
***************************************************************************************************/
static void decode_audio_block_ldac(
AB *p_ab)
{
    AC *p_ac;
    int ich;
    int nchs = p_ab->blk_nchs;

    for (ich = 0; ich < nchs; ich++) {
        p_ac = p_ab->ap_ac[ich];

        clear_spectrum_ldac(p_ac, LDAC_MAXLSU);

        dequant_spectrum_ldac(p_ac);

        dequant_residual_ldac(p_ac);


    }

    return;
}

/***************************************************************************************************
    Decode
***************************************************************************************************/
DECLFUNC void decode_ldac(
SFINFO *p_sfinfo)
{
    AB *p_ab = p_sfinfo->p_ab;
    int ibk;
    int nbks = gaa_block_setting_ldac[p_sfinfo->cfg.chconfig_id][1];

    for (ibk = 0; ibk < nbks; ibk++) {
        decode_audio_block_ldac(p_ab);

        p_ab++;
    }

    return;
}
#endif /* _ENCODE_ONLY */

