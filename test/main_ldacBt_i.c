/*******************************************************************************
 *
 * Copyright (C) 2013 - 2021 Sony Corporation
 *
 ******************************************************************************/

/* Simulation program for LDAC encoding. */

/* Define macro setting */
#define _MAIN_

/* Include */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include    "ldacBT.h"
/* Select endian */
#define __LITTLE_ENDIAN__
#define UPDATE_ENDIAN
#ifdef UPDATE_ENDIAN
#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#undef __LITTLE_ENDIAN__
#define __BIG_ENDIAN__
#endif /* __BYTE_ORDER */
#endif /* UPDATE_ENDIAN */

/* const value */
#define MAXTHREAD       5
#define NFILES          2
#define BYTESIZE        8    /* Number of Bits in a Byte */
#ifndef UNSET
#define UNSET          -1
#endif

#define USE_LDACBT_SPEC_INFO
#ifdef USE_LDACBT_SPEC_INFO
/* The structure for none-a2dp-codec specification element for LDAC.
 * Using as LDAC stream's file header in this simulation program.
 */
struct _ldacbt_spec_info_
{
    unsigned char vendor_id0;   /* octet 0 */
    unsigned char vendor_id1;   /* octet 1 */
    unsigned char vendor_id2;   /* octet 2 */
    unsigned char vendor_id3;   /* octet 3 */
    unsigned char codec_id0;    /* octet 4 */
    unsigned char codec_id1;    /* octet 5 */
    /* ldac param */
#ifdef __LITTLE_ENDIAN__
    unsigned fs_info:6;         /* octet 6 */
    unsigned rfa0:2;            /* octet 6 */

    unsigned cm:3;              /* octet 7 */
    unsigned rfa1:5;            /* octet 7 */
#else
    unsigned rfa0:2;            /* octet 6 */
    unsigned fs_info:6;         /* octet 6 */

    unsigned rfa1:5;            /* octet 7 */
    unsigned cm:3;              /* octet 7 */
#endif
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
;
typedef struct _ldacbt_spec_info_ LDACBT_SPEC_INFO;
#define LDACBT_SPEC_INFO_SIZE 8
#endif /* USE_LDACBT_SPEC_INFO */
/* The structure for LDAC encode setting. */
typedef struct _st_ldacenc_param{
    int ith;
    int cm;
    int eqmid; /* Encode quality mode index */
    char * pPathFileIn;
    char * pPathFileOut;
} LDAC_ENCODE_PARAM;
/* The structure to list the LDAC_ENCODE_PARAM. */
#define N_PARAM_GROWTH 10
typedef struct _st_param_list{
    int nParam;
    int nParamMax;
    void **params;
} PARAM_LIST;

/* Function prototypes. */
#ifdef USE_LDACBT_SPEC_INFO
static int    update_ldac_spec_info(LDACBT_SPEC_INFO*, int, int);
#endif /* USE_LDACBT_SPEC_INFO */
static FILE  *open_wavefile_read(char*, LDACBT_SMPL_FMT_T*, int*, int*, int*);
static size_t fread_pcm(void*, int, int, FILE*);
#ifdef USE_LDACBT_SPEC_INFO
static FILE  *fopen_ldac_write(LDACBT_SPEC_INFO*, char*, int, int);
#else /* USE_LDACBT_SPEC_INFO */
static FILE  *fopen_ldac_write(char*);
#endif /* USE_LDACBT_SPEC_INFO */
static const char *get_error_code_string(int);
static const char *get_smplfmt_string(LDACBT_SMPL_FMT_T);
static void   show_copyright(void);
static void   finish(int);
static char  *delpath(char*);

#include <dlfcn.h>
#include <errno.h>
/* The LDAC encoder shared library, and the functions to use */
#ifdef USE_LDACBT_ENCDEC_LIB
static const char *LDACBT_ENCODER_LIB_NAME = "libldacBT.so";
#else
static const char *LDACBT_ENCODER_LIB_NAME = "libldacBT_enc.so";
#endif
static void *ldacbt_encoder_lib_handle=NULL;

static const char *LDACBT_API_GET_HANDLE="ldacBT_get_handle";
typedef HANDLE_LDAC_BT (*tLDACBT_GET_HANDLE)(void);
static tLDACBT_GET_HANDLE ldacBT_api_get_handle;

static const char *LDACBT_API_FREE_HANDLE="ldacBT_free_handle";
typedef void (*tLDACBT_FREE_HANDLE)(HANDLE_LDAC_BT hLdacBt);
static tLDACBT_FREE_HANDLE ldacBT_api_free_handle;

static const char *LDACBT_API_CLOSE_HANDLE="ldacBT_close_handle";
typedef void (*tLDACBT_CLOSE_HANDLE)(HANDLE_LDAC_BT hLdacBt);
static tLDACBT_CLOSE_HANDLE ldacBT_api_close_handle;

static const char *LDACBT_API_GET_VER="ldacBT_get_version";
typedef int (*tLDACBT_GET_VER)(void);
static tLDACBT_GET_VER ldacBT_api_get_version;

static const char *LDACBT_API_GET_SF="ldacBT_get_sampling_freq";
typedef int (*tLDACBT_GET_SF)(HANDLE_LDAC_BT hLdacBt);
static tLDACBT_GET_SF ldacBT_api_get_sampling_freq;

static const char *LDACBT_API_GET_BITRATE="ldacBT_get_bitrate";
typedef int (*tLDACBT_GET_BITRATE)(HANDLE_LDAC_BT hLdacBt);
static tLDACBT_GET_BITRATE ldacBT_api_get_bitrate;

static const char *LDACBT_API_INIT_HDL_ENC="ldacBT_init_handle_encode";
typedef int (*tLDACBT_INIT_HDL_ENC)(HANDLE_LDAC_BT hLdacBt, int mtu, int eqmid, int cm,
                                               LDACBT_SMPL_FMT_T fmt, int sf);
static tLDACBT_INIT_HDL_ENC ldacBT_api_init_handle_encode;

static const char *LDACBT_API_SET_EQMID="ldacBT_set_eqmid";
typedef int (*tLDACBT_SET_EQMID)(HANDLE_LDAC_BT hLdacBt, int eqmid);
static tLDACBT_SET_EQMID ldacBT_api_set_eqmid;

static const char *LDACBT_API_GET_EQMID="ldacBT_get_eqmid";
typedef int (*tLDACBT_GET_EQMID)(HANDLE_LDAC_BT hLdacBt);
static tLDACBT_GET_EQMID ldacBT_api_get_eqmid;

static const char *LDACBT_API_ALTER_EQMID_PRIORITY="ldacBT_alter_eqmid_priority";
typedef int (*tLDACBT_ALTER_EQMID_PRIORITY)(HANDLE_LDAC_BT hLdacBt, int priority);
static tLDACBT_ALTER_EQMID_PRIORITY ldacBT_api_alter_eqmid_priority;

static const char *LDACBT_API_ENCODE="ldacBT_encode";
typedef int (*tLDACBT_ENCODE)(HANDLE_LDAC_BT hLdacBt, void *p_pcm, int *pcm_used,
                              unsigned char *p_stream, int *stream_sz, int *frame_num );
static tLDACBT_ENCODE ldacBT_api_encode;

static const char *LDACBT_API_GET_ERR="ldacBT_get_error_code";
typedef int (*tLDACBT_GET_ERR)(HANDLE_LDAC_BT hLdacBt);
static tLDACBT_GET_ERR ldacBT_api_get_error_code;


static void *load_func(const char* func_name)
{
    void *func_ptr = dlsym(ldacbt_encoder_lib_handle, func_name);
    if(func_ptr == NULL){
        fprintf(stderr, "[ERR] cannot find function '%s' in the library.\n", func_name);
        return NULL;
    }
    return func_ptr;
}
static int open_lib_and_load_func(void)
{
    // open library
    ldacbt_encoder_lib_handle = dlopen(LDACBT_ENCODER_LIB_NAME, RTLD_NOW);
    if(ldacbt_encoder_lib_handle == NULL){
        char buffer_str[1024];
        strerror_r(errno, buffer_str, sizeof(buffer_str));
        fprintf(stderr, "[ERR] cannot open library '%s': errno = %d (%s)\n",
                LDACBT_ENCODER_LIB_NAME, errno, buffer_str);
        return -1;
    }
    // Load all functions
    if((ldacBT_api_get_handle = load_func(LDACBT_API_GET_HANDLE)) == NULL )
      return -1;

    if((ldacBT_api_free_handle = load_func(LDACBT_API_FREE_HANDLE)) == NULL)
      return -1;

    if((ldacBT_api_close_handle = load_func(LDACBT_API_CLOSE_HANDLE)) == NULL)
      return -1;

    if((ldacBT_api_get_version = load_func(LDACBT_API_GET_VER)) == NULL)
      return -1;

    if((ldacBT_api_get_sampling_freq = load_func(LDACBT_API_GET_SF)) == NULL)
      return -1;

    if((ldacBT_api_get_bitrate = load_func(LDACBT_API_GET_BITRATE)) == NULL)
      return -1;

    if((ldacBT_api_init_handle_encode = load_func(LDACBT_API_INIT_HDL_ENC)) == NULL)
      return -1;

    if((ldacBT_api_set_eqmid = load_func(LDACBT_API_SET_EQMID)) == NULL)
      return -1;

    if((ldacBT_api_get_eqmid = load_func(LDACBT_API_GET_EQMID)) == NULL)
      return -1;

    if((ldacBT_api_alter_eqmid_priority = load_func(LDACBT_API_ALTER_EQMID_PRIORITY)) == NULL)
      return -1;

    if((ldacBT_api_encode = load_func(LDACBT_API_ENCODE)) == NULL)
      return -1;

    if((ldacBT_api_get_error_code = load_func(LDACBT_API_GET_ERR)) == NULL)
      return -1;

    return 0;
}

/* Usage */
static void usage(char *p_prog)
{
    static char    *a_usage[] = {
        "[EncodeQualityModeID] [ChannelMode] [InputFile] [OutputFile]",
        "",
        "EncodeQualityModeID : Encode Quality Mode Index. (HQ/SQ/MQ)",
        "                    : -HQ : HQ mode (default)",
        "                    : -SQ : SQ mode",
        "                    : -MQ : MQ mode",
        "ChannelMode : Channel Mode. (stereo/dual/mono)",
        "            : -stereo : LDACBT_CHANNEL_MODE_STEREO (default for 2ch)",
        "            : -dual   : LDACBT_CHANNEL_MODE_DUAL_CHANNEL",
        "            : -mono   : LDACBT_CHANNEL_MODE_MONO (default for 1ch)",
        "[InputFile *]   : PCM Audio Data File",
        "[OutputFile *]  : LDAC Coded Bit Stream File",
        "",
        "",
        "* : thread/process number.",
        (char *)NULL
    };
    int    i;

    /* Show Usage */
    fprintf(stderr, "\n\n");
    fprintf(stderr, "Usage: %s", p_prog);
    for (i = 0; a_usage[i]; i++) {
        fprintf(stderr, "\t%s\n", a_usage[i]);
    }

    /* Show Version */
    {
        int ver, v0, v1, v2;
        ver = ldacBT_api_get_version();
        v0 = (ver >> 16) & 0xFF;
        v1 = (ver >>  8) & 0xFF;
        v2 = (ver & 0xFF);
        fprintf(stderr, "\n\t*** built w/ LDAC Library, version %d.%02d.%02d ***\n", v0, v1, v2 );
        fprintf(stderr, "\n");
    }

    /* Show Copyright */
    show_copyright();

    exit(EXIT_FAILURE);
}

/* List */
static int growth_param_list_if_need(PARAM_LIST *pList)
{
    if( pList->nParam >= pList->nParamMax ){
        int k;
        LDAC_ENCODE_PARAM **pNew;
        pList->nParamMax += N_PARAM_GROWTH;
        pNew = (LDAC_ENCODE_PARAM**)calloc( pList->nParamMax, sizeof(LDAC_ENCODE_PARAM *));
        if( pNew == NULL ){
            fprintf( stderr, "[ERR] calloc() for parameter list (%d).\n", pList->nParamMax );
            return -1;
        }
        for( k = 0; k < pList->nParamMax; k++ ){
            if( k < pList->nParam ){
                pNew[k] = ((LDAC_ENCODE_PARAM**)pList->params)[k];
            }else{
                pNew[k] = (LDAC_ENCODE_PARAM*) calloc ( 1, sizeof(LDAC_ENCODE_PARAM) );
            }
        }
        if( pList->params ){
            free( pList->params);
        }
        pList->params = (void**)pNew;
    }
    return 0;
}

/* Get Option */
static int get_option(int argc, char **argv, PARAM_LIST *pList)
{
    int   i, j;
    char *p_ptr;
    int   eqmid, cm, flgReset;
    char *ap_file[NFILES];

    /* Specify I/O File & Option List */
    for (i = 1, flgReset = 1; i < argc; i++) {
        if(flgReset){
            flgReset = 0;
            j = 0;
            eqmid = LDACBT_EQMID_HQ;
            cm = UNSET;
            ap_file[0] = NULL;
            ap_file[1] = NULL;
        }
        p_ptr = argv[i];
        if (*p_ptr == '-') {
            p_ptr++;
            if(0){;}
            else if (!strcmp(p_ptr, "HQ")) { eqmid = LDACBT_EQMID_HQ;}
            else if (!strcmp(p_ptr, "SQ")) { eqmid = LDACBT_EQMID_SQ;}
            else if (!strcmp(p_ptr, "MQ")) { eqmid = LDACBT_EQMID_MQ;}
            else if (!strcmp(p_ptr, "stereo")) { cm = LDACBT_CHANNEL_MODE_STEREO;}
            else if (!strcmp(p_ptr, "dual")) { cm = LDACBT_CHANNEL_MODE_DUAL_CHANNEL;}
            else if (!strcmp(p_ptr, "mono")) { cm = LDACBT_CHANNEL_MODE_MONO;}

            else {
                fprintf(stderr, "-%s: Unknown Option\n", p_ptr);
                return -1;
            }
        }
        else {
            if (j < NFILES) {
                ap_file[j++] = p_ptr;
            }
            if( j == NFILES) {
                if( growth_param_list_if_need(pList) < 0){return -1;}


                ((LDAC_ENCODE_PARAM **)pList->params)[pList->nParam]->ith = pList->nParam;
                ((LDAC_ENCODE_PARAM **)pList->params)[pList->nParam]->cm = cm;
                ((LDAC_ENCODE_PARAM **)pList->params)[pList->nParam]->eqmid = eqmid;
                ((LDAC_ENCODE_PARAM **)pList->params)[pList->nParam]->pPathFileIn  = ap_file[0];
                ((LDAC_ENCODE_PARAM **)pList->params)[pList->nParam]->pPathFileOut = ap_file[1];
                ++pList->nParam;
                /* set flag to reset params */
                flgReset = 1;
            }
        }
    }
    return 0;
}

#ifdef __GNUC__
void
#else
unsigned int __stdcall
#endif
ldac_encode_proc(void *data);

/******************************************************************************
    main
******************************************************************************/
int main( int argc, char **argv )
{
    PARAM_LIST paramList;
    int version, ith, i;

    paramList.nParam = 0;
    paramList.nParamMax = 0;
    paramList.params = NULL;

    if( open_lib_and_load_func() != 0 ){
        return -1;
    }

    version = ldacBT_api_get_version();
    fprintf(stderr, "\n=== %s : LDAC Encoder Simulation Program (version %d.%02d.%02d) ===\n",
        delpath(argv[0]), (version>>16)&0xFF, (version>>8)&0xFF, version&0xFF );

    if (get_option(argc, argv, &paramList)) { usage(delpath(argv[0])); }

    if ( paramList.nParam <= 0 ) { usage(delpath(argv[0])); }

    /* Print arguments */
    for( i = 0; i < paramList.nParam; i++ ){
        int eqmid, cm;
        eqmid = ((LDAC_ENCODE_PARAM **)paramList.params)[i]->eqmid;
        cm = ((LDAC_ENCODE_PARAM **)paramList.params)[i]->cm;
        fprintf( stderr, "[% 2d] %s %s %s %s\n", ((LDAC_ENCODE_PARAM **)paramList.params)[i]->ith,
                 (cm==LDACBT_CHANNEL_MODE_MONO?"MONO":
                  (cm==LDACBT_CHANNEL_MODE_DUAL_CHANNEL?"DUAL CHANNEL":
                   (cm==LDACBT_CHANNEL_MODE_STEREO?"STEREO":"default"))),
                 (eqmid==LDACBT_EQMID_HQ?"HQ":
                  (eqmid==LDACBT_EQMID_SQ?"SQ":
                   (eqmid==LDACBT_EQMID_MQ?"MQ":"default"))),
                 ((LDAC_ENCODE_PARAM **)paramList.params)[i]->pPathFileIn,
                 ((LDAC_ENCODE_PARAM **)paramList.params)[i]->pPathFileOut);
    }

    ith = 0;
    do{
        ldac_encode_proc( (void *)((LDAC_ENCODE_PARAM **)paramList.params)[ith++] );
    } while( ith < paramList.nParam );

    if( paramList.params != NULL ){
        int k;
        for( k = 0; k < paramList.nParamMax; k++ ){
            if( ((LDAC_ENCODE_PARAM**)paramList.params)[k] != NULL ){
                free(((LDAC_ENCODE_PARAM**)paramList.params)[k]);
            }
        }
        free( paramList.params );
    }
    return 0;
}

/* LDAC encoding process */
#ifdef __GNUC__
void
#else
unsigned int __stdcall
#endif
ldac_encode_proc(void *data)
{
    char *in_file, *out_file;
    FILE *fpi, *fpo;
    HANDLE_LDAC_BT hData;
    int result, isf, nRead, mtu, cm, channel, wlength, sf;
    int eqmid, frame_sample;
    unsigned char a_pcm_in[LDAC_PRCNCH * LDACBT_MAX_LSU * sizeof(int)];
    unsigned char a_stream[LDACBT_MAX_NBYTES];
    LDAC_ENCODE_PARAM * pEncParam;
#ifdef USE_LDACBT_SPEC_INFO
    LDACBT_SPEC_INFO spec_info;
#endif /* USE_LDACBT_SPEC_INFO */
    LDACBT_SMPL_FMT_T fmt;

    pEncParam = (LDAC_ENCODE_PARAM *)data;
    mtu = 679;
    fpi = fpo = NULL;
    hData = NULL;

    /* Get Option */
    in_file  = pEncParam->pPathFileIn;
    out_file = pEncParam->pPathFileOut;
    eqmid = pEncParam->eqmid;
    cm = pEncParam->cm;

    /* Set Encode Quality Mode Index if not initialized */
    if ( eqmid == UNSET ) {
        eqmid = LDACBT_EQMID_HQ;
    }
    /* Set Encode frame_sample/channel */
    frame_sample = LDACBT_ENC_LSU;

    /* Open PCM Audio Data File */
    fpi = open_wavefile_read(in_file, &fmt, &channel, &wlength, &sf);
    if (fpi == (FILE *)NULL) {
        perror(in_file);
        goto ldacBT_encode_proc_end;
    }

    /* Set default channel_mode if not initialized. */
    if( cm == UNSET ){
        if( channel == 1 ){ cm = LDACBT_CHANNEL_MODE_MONO; }
        else if ( channel == 2 ){ cm = LDACBT_CHANNEL_MODE_STEREO; }
        else{
            goto CHANNEL_MODE_CHECK_ERR;
        }
    }else{
        /* check & channel mode */
        switch ( cm ){
          case LDACBT_CHANNEL_MODE_MONO:
            if( channel != 1 ){
                goto CHANNEL_MODE_CHECK_ERR;
            }
            break;
          case LDACBT_CHANNEL_MODE_STEREO:
          case LDACBT_CHANNEL_MODE_DUAL_CHANNEL:
            if( channel != 2 ){ goto CHANNEL_MODE_CHECK_ERR;}
            break;
          default:
            break;
        }
    }
    if( 0 ){
CHANNEL_MODE_CHECK_ERR:
        fprintf(stderr, "Invalid channel_mode %d. Input=%d chan. \n", cm, channel);
        goto ldacBT_encode_proc_end;
    }

    /* Get LDAC Handler */
    if ((hData = ldacBT_api_get_handle()) == (HANDLE_LDAC_BT)NULL)
    {
        fprintf(stderr, "Error: Can not Get LDAC Handle!\n");
        goto ldacBT_encode_proc_end;
    }

    /* Initialize for Encoding */
    result = ldacBT_api_init_handle_encode(hData, mtu, eqmid, cm, fmt, sf);
    if (result) {
        fprintf(stderr, "[ERR] Initializing LDAC Handle for analysis! Error code %s\n",
                get_error_code_string(ldacBT_api_get_error_code(hData)));
        goto ldacBT_encode_proc_end;
    }

    /* Open Coded Bit Stream File */
#ifdef USE_LDACBT_SPEC_INFO
    fpo = fopen_ldac_write( &spec_info, out_file, sf, cm);
#else /* USE_LDACBT_SPEC_INFO */
    fpo = fopen_ldac_write( out_file);
#endif /* USE_LDACBT_SPEC_INFO */
    if (fpo == (FILE *)NULL) {
        fprintf(stderr, "Error: Can not Open LDAC stream file to write. %s!\n", out_file);
        goto ldacBT_encode_proc_end;
    }

    /* Show Encoding Parameter */
    fprintf(stderr, "<%d>[IN]\n", pEncParam->ith);
    fprintf(stderr, "<%d> PCM Audio Data File    : %s\n", pEncParam->ith, delpath(in_file));
    fprintf(stderr, "<%d>  - Format              : %s\n", pEncParam->ith, get_smplfmt_string(fmt));
    fprintf(stderr, "<%d>  - Sampling Frequency  : %d Hz\n", pEncParam->ith, sf);
    fprintf(stderr, "<%d>  - Num of Channel      : %d ch\n", pEncParam->ith, channel);
    fprintf(stderr, "<%d>  - Bit Rate            : %d kbps (%d bytes/frame)\n", pEncParam->ith,
            (int)((BYTESIZE*wlength*sf*channel)/1000.0), wlength*frame_sample*channel);
    fprintf(stderr, "<%d>[OUT]\n", pEncParam->ith );
    fprintf(stderr, "<%d> LDACBT Bit Stream File : %s\n", pEncParam->ith, delpath(out_file));
    fprintf(stderr, "<%d>  - EQMID               : %d (%d kbps)\n", pEncParam->ith, eqmid,
            ldacBT_api_get_bitrate(hData));
    fprintf(stderr, "<%d>  - Sampling Frequency  : %d Hz\n", pEncParam->ith,
            ldacBT_api_get_sampling_freq(hData));
    fprintf(stderr, "<%d>  - Channel Mode        : %d (%s)\n", pEncParam->ith, cm,
            cm==LDACBT_CHANNEL_MODE_MONO?"MONO":
            cm==LDACBT_CHANNEL_MODE_DUAL_CHANNEL?"DUAL CHANNEL":
            cm==LDACBT_CHANNEL_MODE_STEREO?"STEREO":"unknown");
#ifdef USE_LDACBT_SPEC_INFO
    fprintf(stderr, "<%d>[Codec Spec Element Info]\n", pEncParam->ith );
    fprintf(stderr, "<%d>  - VENDOR ID           : 0x%02x%02x%02x%02x\n", pEncParam->ith,
           spec_info.vendor_id0, spec_info.vendor_id1, spec_info.vendor_id2, spec_info.vendor_id3);
    fprintf(stderr, "<%d>  - CODEC ID            : 0x%02x%02x\n", pEncParam->ith,
            spec_info.codec_id0, spec_info.codec_id1);
    fprintf(stderr, "<%d>  - Sampling Freq       : 0x%02x\n", pEncParam->ith, spec_info.fs_info);
    fprintf(stderr, "<%d>  - Channel Mode        : 0x%02x\n", pEncParam->ith, spec_info.cm);
    fprintf(stderr, "<%d>  - Info Size           : %d\n", pEncParam->ith, LDACBT_SPEC_INFO_SIZE);
#endif /* USE_LDACBT_SPEC_INFO */
    /* Encoding process */
    isf = 0;
    while (1) {
        int pcm_encoded_byte, frame_length_wrote, frame_num;

        /* Read PCM Data */
        nRead = fread_pcm( a_pcm_in, wlength, frame_sample*channel, fpi );
        if( nRead <= 0 ){ break; }
        else if( nRead < frame_sample*channel ){
            memset( (unsigned char*)a_pcm_in + nRead*wlength, 0,
                    (frame_sample*channel - nRead)*wlength );
        }

        /* Encode Frame */
        result = ldacBT_api_encode( hData, a_pcm_in, &pcm_encoded_byte, a_stream,
                                &frame_length_wrote, &frame_num );
        if (result) {
            int error_code;
            error_code = ldacBT_api_get_error_code( hData );
            fprintf(stderr, "[ERR] LDAC encode. Error code %s\n",
                    get_error_code_string(error_code));
            break;
        }

        if ( frame_length_wrote > 0) {
            /* Write Frame */
            fwrite( a_stream, sizeof(unsigned char), frame_length_wrote, fpo);
            isf+= frame_num;
        }
    }

    /* Flush Encoding */
    {
        int i, pcm_encoded_byte, frame_length_wrote, frame_num;
        i = 2;
        do{
            /* Encode Frame */
            result = ldacBT_api_encode( hData, NULL, &pcm_encoded_byte, a_stream,
                                    &frame_length_wrote, &frame_num );

            if (result) {
                int error_code;
                error_code = ldacBT_api_get_error_code( hData );
                fprintf(stderr, "[ERR] LDAC flush encode! Error code %s\n",
                        get_error_code_string(error_code));
            }

            if ( frame_length_wrote > 0) {
                /* Write Frame */
                fwrite( a_stream, sizeof(unsigned char), frame_length_wrote, fpo);
                isf+= frame_num;
            }
        }while(--i>0);
    }

ldacBT_encode_proc_end:
    /* Close Files */
    if( fpi != NULL ){ fclose(fpi);}
    if( fpo != NULL ){ fclose(fpo);}

    if( pEncParam != NULL ){
        fprintf(stderr, "End of the process ID %d", pEncParam->ith );
        if( hData != NULL ){
            result = ldacBT_api_get_error_code( hData );
            if( result != 0 ){
                fprintf(stderr, " with error_code : %s", get_error_code_string(result));
            }
        }
        fprintf( stderr, ".\n");
    }

    /* Free handle. */
    ldacBT_api_free_handle(hData);


#ifdef    __GNUC__
    return;
#else
    return result;
#endif
}


/* Show Copyright */
static void    show_copyright(void)
{
    fprintf(stderr, "\tCopyright 2013-2021 Sony Corporation\n\n");
}

/* Sample program end procedure */
static void    finish(int status)
{
    if (status == 0) {        fprintf(stderr, "\n  Congratulations !! : ");    }
    else {                    fprintf(stderr, "\n\7  Finished Abnormally !! : ");    }
    show_copyright();
    exit(status);
}

/* Print out the filename only, deleting the path.
 * Returns a pointer to the start of the filename.
 */
static char    *delpath(char *str)
{
    char    *ptr;
    if ((ptr = strrchr(str, '\\')) == (char *)NULL) {return str;}
    else {        return ptr+1;}
}

#ifdef USE_LDACBT_SPEC_INFO
/* Update A2DP Codec Specific Element Info for LDAC */
static int update_ldac_spec_info( LDACBT_SPEC_INFO *p_spec_info, int sf, int cm )
{
    memset( p_spec_info, 0, LDACBT_SPEC_INFO_SIZE );
    /* Vendor ID */
    p_spec_info->vendor_id0 = LDACBT_VENDOR_ID0;
    p_spec_info->vendor_id1 = LDACBT_VENDOR_ID1;
    p_spec_info->vendor_id2 = LDACBT_VENDOR_ID2;
    p_spec_info->vendor_id3 = LDACBT_VENDOR_ID3;
    /* Vendor Codec ID */
    p_spec_info->codec_id0 = LDACBT_CODEC_ID0;
    p_spec_info->codec_id1 = LDACBT_CODEC_ID1;
    /* sampling frequency */
    switch( sf ){
        case 44100:
            p_spec_info->fs_info = LDACBT_SAMPLING_FREQ_044100;
            break;
        case 48000:
            p_spec_info->fs_info = LDACBT_SAMPLING_FREQ_048000;
            break;
        case 88200:
            p_spec_info->fs_info = LDACBT_SAMPLING_FREQ_088200;
            break;
        case 96000:
            p_spec_info->fs_info = LDACBT_SAMPLING_FREQ_096000;
            break;
        case 176400:
            p_spec_info->fs_info = LDACBT_SAMPLING_FREQ_176400;
            break;
        case 192000:
            p_spec_info->fs_info = LDACBT_SAMPLING_FREQ_192000;
            break;
        default:
            fprintf(stderr, "[ERR] Unsupported sampling frequency %d Hz!\n", sf);
            return -1;
    }
    /* channel_mode */
    switch( cm ){
      case LDACBT_CHANNEL_MODE_MONO:
      case LDACBT_CHANNEL_MODE_DUAL_CHANNEL:
      case LDACBT_CHANNEL_MODE_STEREO:
        p_spec_info->cm = cm;
        break;
      default:
        fprintf(stderr, "[ERR] Unsupported Channel Mode %d!\n", cm );
        return -1;
    }
    return 0;
}
#endif /* USE_LDACBT_SPEC_INFO */



typedef unsigned char  UINT8;
typedef char           SINT8;
typedef unsigned short UINT16;
typedef short          SINT16;
typedef unsigned int   UINT32;
typedef int            SINT32;

/* Defines for RIFF file header. */
/* RIFF chunk decriptor */
#define RIFF_CHUNK_ID          0x46464952
#define RIFF_CHUNK_FORMAT_WAVE 0x45564157
/* Sub-chunck */
#define RIFF_SUB_CHUNK_FMT_    0x20746d66
#define RIFF_SUB_CHUNK_FACT    0x74636166
#define RIFF_SUB_CHUNK_DATA    0x61746164

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM        0x1
#endif
#ifndef WAVE_FORMAT_EXTENSIBLE
#define WAVE_FORMAT_EXTENSIBLE 0xfffe
#endif
#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT 0x3
#endif

/* GUID */
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
    UINT32 Data1;    /*  4 */
    UINT16 Data2;    /*  6 */
    UINT16 Data3;    /*  8 */
    UINT8  Data4[8]; /* 16 */
} GUID;
#endif    /* GUID_DEFINED */

/* WAVE header part.
 * size of "fmt" chunk without "chunk type" and "chunk size" fields
 * (may be sizeof(_pcm_header))
 */
#define SIZEOF_PCM_HEADER    16

/* general waveform format structure */
typedef struct {
    UINT16    wFormatTag;      /*  2 : 1(WAVE_FORMAT_PCM) or 3(WAVE_FORMAT_IEEE_FLOAT) */
    UINT16    nChannels;       /*  4 : 1 = Mono, 2 = Stereo */
    UINT32    nSamplesPerSec;  /*  8 : Sampling Freq */
    UINT32    nAvgBytesPerSec; /* 12 : Data per sec */
    UINT16    nBlockAlign;     /* 14 : block alignment */
    UINT16    wBitsPerSample;  /* 16 : bits per sample, 8, 12, 16 */
} _pcm_header;

/* WAVE EXTENSIBLE header part */
#define CBSIZE_PCMEXT_HEADER    22    /* be set to "cbSize" field */
#define SIZEOF_PCMEXT_HEADER    40    /* SIZEOF_PCM_HEADER + cbSize +  CBSIZE_PCMEXT_HEADER */

typedef struct {
    UINT16    wFormatTag;      /*  2 : Compression Format ID: WAVE_FORMAT_EXTENSIBLE */
    UINT16    nChannels;       /*  4 : Number of Channels */
    UINT32    nSamplesPerSec;  /*  8 : Sampling Frequency [Hz] */
    UINT32    nAvgBytesPerSec; /* 12 : Average Number of Bytes per Sec */
    UINT16    nBlockAlign;     /* 14 : Number of Block Alignment Bytes */
    UINT16    wBitsPerSample;  /* 16 : Quantization Bit Number for Audio Sample */
    UINT16    cbSize;          /* 18 : Size of the Extended Header that follows */
    union {
        UINT16    wValidBitsPerSample; /* 2 : Bits of Precision for Each Audio Sample */
        UINT16    wSamplesPerBlock;    /* 2 : The Number of Samples in a Compressed Block */
        UINT16    wReserved;           /* 2 : Reserved */
    } Samples;
    UINT32    dwChannelMask;   /*  6 : The Mapping of Channels to spatial location */
    GUID      SubFormat;       /* 22 : GUID Size = 16 (4 + 2 + 2 + 8) */
} _pcmext_header;

/* fmt chunk definition(without 'chunk type' and 'chunk size' fields) */
typedef union {
    _pcm_header pcm_header;
    _pcmext_header pcmext_header;
} _fmt;

/* get 32-bit data from the FILE stream */
static UINT32 fgetLong(FILE *fp)
{
    UINT32 ret;
    ret  = (fgetc(fp) & 0xff) <<  0;
    ret |= (fgetc(fp) & 0xff) <<  8;
    ret |= (fgetc(fp) & 0xff) << 16;
    ret |= (fgetc(fp) & 0xff) << 24;
    return ret;
}

/* get 16-bit data from the FILE stream */
static UINT16 fgetShort(FILE *fp)
{
    UINT16 ret;
    ret  =  fgetc(fp) & 0xff;
    ret |= (fgetc(fp) & 0xff) <<  8;
    return ret;
}

#ifndef KSDATAFORMAT_SUBTYPE_PCM
static const GUID KSDATAFORMAT_SUBTYPE_PCM
= {0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
#endif
#ifndef KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
= {0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
#endif

/* The function "guidcmp()" compares two GUIDs and returns 1 if they are identical, otherwise
 * returns 0.
 */
static int guidcmp(const GUID *s1, const GUID *s2)
{
    return (
        (s1->Data1 == s2->Data1) &&
        (s1->Data2 == s2->Data2) &&
        (s1->Data3 == s2->Data3) &&
        !memcmp(s1->Data4, s2->Data4, 8)
        );
}

/* parse wave file */
static int ParseWaveFile(
FILE    *fp,
_fmt    *fmt,
UINT32    *p_fact_chunk_samples,
UINT32    *p_data_chunk_length)
{
    UINT32    sub_chunk_id;
    UINT32    sub_chunk_size;
    UINT32    wave_format;
    SINT32    status = 0;
    _pcm_header     *pcm_header;

    /* Sub-chunk */
    *p_fact_chunk_samples = 0;
    *p_data_chunk_length = 0;

    while (!status) {
        /* Get Sub-chunk ID */
        sub_chunk_id = fgetLong(fp);
        /* Get Sub-chunk Size */
        sub_chunk_size = fgetLong(fp);
        if (feof(fp)) {
            return -1;
        }

        switch (sub_chunk_id) {
        case RIFF_SUB_CHUNK_FMT_:
            wave_format = fgetShort(fp);/* wFormatTag */
            switch (wave_format) {
              case WAVE_FORMAT_PCM:
              case WAVE_FORMAT_IEEE_FLOAT:
              case WAVE_FORMAT_EXTENSIBLE:
                pcm_header = &fmt->pcm_header;
                pcm_header->wFormatTag        = (UINT16)wave_format;
                pcm_header->nChannels         = fgetShort(fp);
                pcm_header->nSamplesPerSec    = fgetLong(fp);
                pcm_header->nAvgBytesPerSec   = fgetLong(fp);
                pcm_header->nBlockAlign       = fgetShort(fp);
                pcm_header->wBitsPerSample    = fgetShort(fp);
                sub_chunk_size -= SIZEOF_PCM_HEADER;
                /* rest is unnecessary for this sample so just skip them */
                break;
            default:    /* unsupported wFormatTag */
                return -1;
            }
            break;

        case RIFF_SUB_CHUNK_FACT:
            *p_fact_chunk_samples = fgetLong(fp);
            sub_chunk_size -= 4;
            break;

        case RIFF_SUB_CHUNK_DATA:
            *p_data_chunk_length = sub_chunk_size;
            status = 1;/* stop searching another chunk(break the loop) */
            sub_chunk_size = 0;
            break;

        default: /* unknown chunk */
            break;
        }

        /* skip remain of chunk */
        fseek(fp, sub_chunk_size, SEEK_CUR);
    }
    return (0);
}


/* Open wave file to read */
static FILE *open_wavefile_read( char *fname, LDACBT_SMPL_FMT_T *fmt, int *ch, int *wl, int *sf)
{
    FILE    *fp;
    _fmt    pcmfmt;
    _pcm_header    *pcm_header;
    UINT32 tmp, data_chunk_length, chunk_id, chunk_size, format;

    memset(&pcmfmt, 0, sizeof(_fmt));
    fp = NULL;
    if ((fp = fopen(fname, "rb")) == (FILE *)NULL) { goto open_error; }

    /* Recognition file format (Check chunk descriptor) */
    /* Get Chunk ID */
    if( (chunk_id = fgetLong(fp)) != RIFF_CHUNK_ID) {
        /* Unsupported File Format */
        goto open_error;
    }
    /* Get Chunk Size (Total - 8 byte) */
    chunk_size = fgetLong(fp);
    /* Get Format (Only Support "WAVE") */
    format = fgetLong(fp);
    if (format != RIFF_CHUNK_FORMAT_WAVE){ goto open_error;    }

    /* Parse WAVE Format */
    if (ParseWaveFile(fp, &pcmfmt, &tmp, &data_chunk_length) < 0) { goto open_error; }

    pcm_header = &pcmfmt.pcm_header;

    /* For WAV PCM */
    switch(pcm_header->wFormatTag){
      case WAVE_FORMAT_PCM:
      case WAVE_FORMAT_EXTENSIBLE:
        *ch = pcm_header->nChannels;
        *sf = pcm_header->nSamplesPerSec;
        switch( pcm_header->wBitsPerSample ){
          case 16:
            *fmt = LDACBT_SMPL_FMT_S16;
            *wl = 2;
            break;
          case 24:
            *fmt = LDACBT_SMPL_FMT_S24;
            *wl = 3;
            break;
          case 32:
            *fmt = LDACBT_SMPL_FMT_S32;
            *wl = 4;
            break;
          default:
            fprintf(stderr, "[ERR] unsupported wlength");
            goto open_error;
        }
        break;
      case WAVE_FORMAT_IEEE_FLOAT:
        *ch = pcm_header->nChannels;
        *sf = pcm_header->nSamplesPerSec;
        *fmt = LDACBT_SMPL_FMT_F32;
        *wl = 4;
        break;
      default:
        fprintf(stderr, "[ERR] unsupported wave format");
        goto open_error;
    }

    return fp;

open_error:
    if( fp ){ fclose(fp); }
    return (FILE*)NULL;
}

/* Read PCM data from file */
static size_t fread_pcm(void *ptr, int size, int nmemb, FILE *strem)
{
    size_t  ritem;
#ifndef __LITTLE_ENDIAN__
    size_t  nitem;
    UINT32  n;
#endif    /* __LITTLE_ENDIAN__ */

    if ((ritem = fread(ptr, size, nmemb, strem)) == 0) {
        return ritem;
    }
#ifndef __LITTLE_ENDIAN__
    /* swap byte order */
    nitem = ritem;
    if (size == sizeof(UINT32)) {        /* 4byte data */
        UINT32 *cp = (UINT32 *) ptr;
        while (nitem-- > 0) {
            n = *cp;
            *cp = (((n) >> 24) & 0x000000ff) |
                  (((n) >>  8) & 0x0000ff00) |
                  (((n) <<  8) & 0x00ff0000) |
                  (((n) << 24) & 0xff000000);
            cp++;
        }
    }
    else if (size == sizeof(UINT16)) {    /* 2byte data */
        UINT16 *cp = (UINT16 *) ptr;
        while (nitem-- > 0) {
            n = *cp;
            *cp = (((n) >> 8) & 0x00ff) | (((n) << 8) & 0xff00);
            cp++;
        }
    }
    else if (size == 3) {            /* 3byte data */
        UINT8 *cp = (UINT8 *) ptr;
        UINT8 tmp;
        while (nitem-- > 0) {
            tmp = *cp;
            *cp = *(cp+2);
            *(cp+2) = tmp;
            cp += 3;
        }
    }
#endif    /* __LITTLE_ENDIAN__ */
    return ritem;
}

/* Open file for LDAC Coded data */
#ifdef USE_LDACBT_SPEC_INFO
static FILE *fopen_ldac_write( LDACBT_SPEC_INFO *p_spec_info, char *fname, int sf, int cm )
{
    FILE *fp;

    /* create codec specific infomation */
    if( update_ldac_spec_info( p_spec_info, sf, cm ) ){
        return NULL;
    }

    /* Open Coded Bit Stream File */
    if ((fp = fopen(fname, "wb+")) == (FILE *)NULL) {
        perror(fname);
        return NULL;
    }
    /* write Codec Specific Info as LDAC file header */
    fwrite( p_spec_info, sizeof(unsigned char), LDACBT_SPEC_INFO_SIZE, fp);
    return fp;
}
#else /* USE_LDACBT_SPEC_INFO */
static FILE *fopen_ldac_write( char *fname )
{
    FILE *fp;

    /* Open Coded Bit Stream File */
    if ((fp = fopen(fname, "wb+")) == (FILE *)NULL) {
        perror(fname);
        return NULL;
    }
    return fp;
}
#endif /* USE_LDACBT_SPEC_INFO */


/* Convert LDACBT sample format to string */
static const char * get_smplfmt_string(LDACBT_SMPL_FMT_T fmt)
{
    switch(fmt){
      case LDACBT_SMPL_FMT_S16: return "S16LE";
      case LDACBT_SMPL_FMT_S24: return "S24LE";
      case LDACBT_SMPL_FMT_S32: return "S32LE";
      case LDACBT_SMPL_FMT_F32: return "F32";
      default: return "unknown";
    }
}

/* Convert LDAC Error Code to string */
#define CASE_RETURN_STR(const) case const: return #const;
static const char * ldac_ErrCode2Str( int ErrCode )
{
    switch(ErrCode){
        CASE_RETURN_STR(LDACBT_ERR_NONE);
        CASE_RETURN_STR(LDACBT_ERR_NON_FATAL);
        CASE_RETURN_STR(LDACBT_ERR_BIT_ALLOCATION);
        CASE_RETURN_STR(LDACBT_ERR_NOT_IMPLEMENTED);
        CASE_RETURN_STR(LDACBT_ERR_NON_FATAL_ENCODE);
        CASE_RETURN_STR(LDACBT_ERR_FATAL);
        CASE_RETURN_STR(LDACBT_ERR_SYNTAX_BAND);
        CASE_RETURN_STR(LDACBT_ERR_SYNTAX_GRAD_A);
        CASE_RETURN_STR(LDACBT_ERR_SYNTAX_GRAD_B);
        CASE_RETURN_STR(LDACBT_ERR_SYNTAX_GRAD_C);
        CASE_RETURN_STR(LDACBT_ERR_SYNTAX_GRAD_D);
        CASE_RETURN_STR(LDACBT_ERR_SYNTAX_GRAD_E);
        CASE_RETURN_STR(LDACBT_ERR_SYNTAX_IDSF);
        CASE_RETURN_STR(LDACBT_ERR_SYNTAX_SPEC);
        CASE_RETURN_STR(LDACBT_ERR_BIT_PACKING);
        CASE_RETURN_STR(LDACBT_ERR_ALLOC_MEMORY);
        CASE_RETURN_STR(LDACBT_ERR_FATAL_HANDLE);
        CASE_RETURN_STR(LDACBT_ERR_ILL_SYNCWORD);
        CASE_RETURN_STR(LDACBT_ERR_ILL_SMPL_FORMAT);
        CASE_RETURN_STR(LDACBT_ERR_ILL_PARAM);
        CASE_RETURN_STR(LDACBT_ERR_ASSERT_SAMPLING_FREQ);
        CASE_RETURN_STR(LDACBT_ERR_ASSERT_SUP_SAMPLING_FREQ);
        CASE_RETURN_STR(LDACBT_ERR_CHECK_SAMPLING_FREQ);
        CASE_RETURN_STR(LDACBT_ERR_ASSERT_CHANNEL_CONFIG);
        CASE_RETURN_STR(LDACBT_ERR_CHECK_CHANNEL_CONFIG);
        CASE_RETURN_STR(LDACBT_ERR_ASSERT_FRAME_LENGTH);
        CASE_RETURN_STR(LDACBT_ERR_ASSERT_SUP_FRAME_LENGTH);
        CASE_RETURN_STR(LDACBT_ERR_ASSERT_FRAME_STATUS);
        CASE_RETURN_STR(LDACBT_ERR_ASSERT_NSHIFT);
        CASE_RETURN_STR(LDACBT_ERR_ASSERT_CHANNEL_MODE);
        CASE_RETURN_STR(LDACBT_ERR_ENC_INIT_ALLOC);
        CASE_RETURN_STR(LDACBT_ERR_ENC_ILL_GRADMODE);
        CASE_RETURN_STR(LDACBT_ERR_ENC_ILL_GRADPAR_A);
        CASE_RETURN_STR(LDACBT_ERR_ENC_ILL_GRADPAR_B);
        CASE_RETURN_STR(LDACBT_ERR_ENC_ILL_GRADPAR_C);
        CASE_RETURN_STR(LDACBT_ERR_ENC_ILL_GRADPAR_D);
        CASE_RETURN_STR(LDACBT_ERR_ENC_ILL_NBANDS);
        CASE_RETURN_STR(LDACBT_ERR_PACK_BLOCK_FAILED);
        CASE_RETURN_STR(LDACBT_ERR_DEC_INIT_ALLOC);
        CASE_RETURN_STR(LDACBT_ERR_INPUT_BUFFER_SIZE);
        CASE_RETURN_STR(LDACBT_ERR_UNPACK_BLOCK_FAILED);
        CASE_RETURN_STR(LDACBT_ERR_UNPACK_BLOCK_ALIGN);
        CASE_RETURN_STR(LDACBT_ERR_UNPACK_FRAME_ALIGN);
        CASE_RETURN_STR(LDACBT_ERR_FRAME_LENGTH_OVER);
        CASE_RETURN_STR(LDACBT_ERR_FRAME_ALIGN_OVER);
        CASE_RETURN_STR(LDACBT_ERR_ALTER_EQMID_LIMITED);
        CASE_RETURN_STR(LDACBT_ERR_ILL_EQMID);
        CASE_RETURN_STR(LDACBT_ERR_ILL_SAMPLING_FREQ);
        CASE_RETURN_STR(LDACBT_ERR_ILL_NUM_CHANNEL);
        CASE_RETURN_STR(LDACBT_ERR_ILL_MTU_SIZE);
        CASE_RETURN_STR(LDACBT_ERR_HANDLE_NOT_INIT);
      default:
        return "unknown-error-code";
    }
}
static char a_ErrorCodeStr[128];
static const char * get_error_code_string( int error_code )
{
    int errApi, errHdl, errBlk;

    errApi = LDACBT_API_ERR( error_code );
    errHdl = LDACBT_HANDLE_ERR( error_code );
    errBlk = LDACBT_BLOCK_ERR( error_code );

    a_ErrorCodeStr[0] = '\0';
    strcat( a_ErrorCodeStr, "API:" );
    strcat( a_ErrorCodeStr, ldac_ErrCode2Str( errApi ) );
    strcat( a_ErrorCodeStr, " Handle:" );
    strcat( a_ErrorCodeStr, ldac_ErrCode2Str( errHdl ) );
    strcat( a_ErrorCodeStr, " Block:" );
    strcat( a_ErrorCodeStr, ldac_ErrCode2Str( errBlk ) );
    return a_ErrorCodeStr;
}

