/*******************************************************************************
 *
 * Copyright (C) 2013 - 2021 Sony Corporation
 *
 ******************************************************************************/

/* Simulation program for LDAC decoding */

/* Define macro setting */
#define _MAIN_
#undef STOP_WHEN_ERROR        /* Stop decoding when error occured */

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
#endif /* #if __BYTE_ORDER == __BIG_ENDIAN */
#endif /* UPDATE_ENDIAN */

/* const value */
#define MAXTHREAD   5
#define NFILES      2
#define BYTESIZE    8    /* Number of Bits in a Byte */
#define WORD_MIDDLE 3    /* 24bits/sample 2's Complement Integer Data */

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
};

typedef struct _ldacbt_spec_info_ LDACBT_SPEC_INFO;
#define LDACBT_SPEC_INFO_SIZE 8
#endif /* USE_LDACBT_SPEC_INFO */

/* The structure for LDAC decode setting. */
typedef struct _st_ldacdec_param{
    int ith;
    int wext;
    LDACBT_SMPL_FMT_T fmt;
    int wlen;
    int nodelay;
    char * pPathFileIn;
    char * pPathFileOut;
} LDAC_DECODE_PARAM;

/* The structure to list the LDAC_DECODE_PARAM. */
#define N_PARAM_GROWTH 10
typedef struct _st_param_list{
    int nParam;
    int nParamMax;
    void **params;
} PARAM_LIST;

/* Function prototypes. */
#ifdef USE_LDACBT_SPEC_INFO
static FILE *fopen_ldac_read(LDACBT_SPEC_INFO*, char*, int*, int*, int*);
#else /* USE_LDACBT_SPEC_INFO */
static FILE *fopen_ldac_read(char*);
static int get_ldac_spec_info(unsigned char*, int*, int*, int*);
#endif /* USE_LDACBT_SPEC_INFO */
static FILE *open_wavefile_write(char*, LDACBT_SMPL_FMT_T, int, int, int);
static int fclose_wave_write(FILE*);
static const char * get_error_code_string(int);
static const char * get_smplfmt_string(LDACBT_SMPL_FMT_T fmt);
static void usage(char*);
static void show_copyright(void);
static void finish(int);
static char *delpath(char*);

#include <dlfcn.h>
#include <errno.h>
/* The LDAC decoder shared library, and the functions to use */
#ifdef USE_LDACBT_ENCDEC_LIB
static const char *LDACBT_DECODER_LIB_NAME = "libldacBT.so";
#else
static const char *LDACBT_DECODER_LIB_NAME = "libldacBT_dec.so";
#endif
static void *ldacbt_decoder_lib_handle=NULL;

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

static const char *LDACBT_API_INIT_HDL_DEC="ldacBT_init_handle_decode";
typedef int (*tLDACBT_INIT_HDL_DEC)(HANDLE_LDAC_BT hLdacBt, int cm, int sf,
                                    int var0, int var1, int var2 );
static tLDACBT_INIT_HDL_DEC ldacBT_api_init_handle_decode;

static const char *LDACBT_API_DECODE="ldacBT_decode";
typedef int (*tLDACBT_DECODE)(HANDLE_LDAC_BT hLdacBt, unsigned char *p_bs, unsigned char *p_pcm,
                              LDACBT_SMPL_FMT_T fmt, int bs_bytes,
                              int *used_bytes, int *wrote_bytes );
static tLDACBT_DECODE ldacBT_api_decode;

static const char *LDACBT_API_GET_ERR="ldacBT_get_error_code";
typedef int (*tLDACBT_GET_ERR)(HANDLE_LDAC_BT hLdacBt);
static tLDACBT_GET_ERR ldacBT_api_get_error_code;


static void *load_func(const char* func_name)
{
    void *func_ptr = dlsym(ldacbt_decoder_lib_handle, func_name);
    if(func_ptr == NULL){
        fprintf(stderr, "[ERR] cannot find function '%s' in the library.\n", func_name);
        return NULL;
    }
    return func_ptr;
}
static int open_lib_and_load_func(void)
{
    // open library
//    ldacbt_decoder_lib_handle = dlopen(LDACBT_DECODER_LIB_NAME, RTLD_NOW);
    ldacbt_decoder_lib_handle = dlopen(LDACBT_DECODER_LIB_NAME, RTLD_LAZY);
    if(ldacbt_decoder_lib_handle == NULL){
        char buffer_str[1024];
        strerror_r(errno, buffer_str, sizeof(buffer_str));
        fprintf(stderr, "[ERR] cannot open library '%s': errno = %d (%s)\n",
                LDACBT_DECODER_LIB_NAME, errno, buffer_str);
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

    if((ldacBT_api_init_handle_decode = load_func(LDACBT_API_INIT_HDL_DEC)) == NULL)
      return -1;

    if((ldacBT_api_decode = load_func(LDACBT_API_DECODE)) == NULL)
      return -1;

    if((ldacBT_api_get_error_code = load_func(LDACBT_API_GET_ERR)) == NULL)
      return -1;

    return 0;
}

/* Usage */
static void usage(char *p_prog)
{
    static char    *a_usage[] = {
        "[-option 0] [InputFile 0] [OutputFile 0] [-option *] [InputFile *] [OutputFile *]",
        "",
        "[InputFile *]  : LDAC Coded Bit Stream File",
        "[OutputFile *] : PCM Audio Data File",
        "",
        "-wext       : Output with WAVE_FORMAT_EXTENSIBLE tag",
        "-int16      : 16bit Integer PCM Output",
        "-int24      : 24bit Integer PCM Output (default)",
        "-int32      : 32bit Integer PCM Output",
        "-float      : floating point PCM Output",
        "-nodelay    : Delay Compensated",
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
        LDAC_DECODE_PARAM **pNew;
        pList->nParamMax += N_PARAM_GROWTH;
        pNew = (LDAC_DECODE_PARAM**)calloc( pList->nParamMax, sizeof(LDAC_DECODE_PARAM *));
        if( pNew == NULL ){
            fprintf( stderr, "[ERR] calloc() for parameter list (%d).\n", pList->nParamMax );
            return -1;
        }
        for( k = 0; k < pList->nParamMax; k++ ){
            if( k < pList->nParam ){
                pNew[k] = ((LDAC_DECODE_PARAM**)pList->params)[k];
            }else{
                pNew[k] = (LDAC_DECODE_PARAM*) calloc ( 1, sizeof(LDAC_DECODE_PARAM) );
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
    int  i, j;
    int  wext, wlength, nodelay, flgReset;
    LDACBT_SMPL_FMT_T fmt;
    char *p_ptr;
    char *ap_file[NFILES];

    /* Specify I/O File & Option List */
    for (i = 1, flgReset = 1; i < argc; i++) {
        if(flgReset){
            flgReset = 0;
            j = 0;
            wext = 0;
            fmt = LDACBT_SMPL_FMT_S24;
            wlength = 3;
            nodelay = 0;
            ap_file[0] = NULL;
            ap_file[1] = NULL;
        }
        p_ptr = argv[i];
        if (*p_ptr == '-') {
            p_ptr++;
            if(0){;}
            else if (!strcmp(p_ptr, "wext")) {  wext = 1; }
            else if (!strcmp(p_ptr, "int16")) {
                fmt = LDACBT_SMPL_FMT_S16;
                wlength = 2;
            }
            else if (!strcmp(p_ptr, "int24")) {;} /* default */
            else if (!strcmp(p_ptr, "int32")) {
                fmt = LDACBT_SMPL_FMT_S32;
                wlength = 4;
            }
            else if (!strcmp(p_ptr, "float")) {
                fmt = LDACBT_SMPL_FMT_F32;
                wlength = 4;
            }
            else if (!strcmp(p_ptr, "nodelay")) {        nodelay = 1; }
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

                ((LDAC_DECODE_PARAM **)pList->params)[pList->nParam]->ith = pList->nParam;
                ((LDAC_DECODE_PARAM **)pList->params)[pList->nParam]->wext = wext;
                ((LDAC_DECODE_PARAM **)pList->params)[pList->nParam]->fmt = fmt;
                ((LDAC_DECODE_PARAM **)pList->params)[pList->nParam]->wlen = wlength;
                ((LDAC_DECODE_PARAM **)pList->params)[pList->nParam]->nodelay = nodelay;
                ((LDAC_DECODE_PARAM **)pList->params)[pList->nParam]->pPathFileIn  = ap_file[0];
                ((LDAC_DECODE_PARAM **)pList->params)[pList->nParam]->pPathFileOut = ap_file[1];
                ++pList->nParam;
                /* set flag to reset params */
                flgReset = 1;
            }
        }
    }
    return 0;
}

#ifdef    __GNUC__
void
#else
unsigned int __stdcall
#endif
ldac_decode_proc(void *data);

/* main */
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
    fprintf(stderr, "\n=== %s : LDAC Decoder Simulation Program (version %d.%02d.%02d) ===\n",
        delpath(argv[0]), (version>>16)&0xFF, (version>>8)&0xFF, version&0xFF );

    if (get_option(argc, argv, &paramList)) { usage(delpath(argv[0])); }

    if ( paramList.nParam <= 0 ) { usage(delpath(argv[0])); }

    for( i = 0; i < paramList.nParam; i++ ){
        fprintf( stderr, "[% 2d] ext:%d fmt:%s wlen:%d nodelay:%d %s -> %s\n",
                 ((LDAC_DECODE_PARAM **)paramList.params)[i]->ith,
                 ((LDAC_DECODE_PARAM **)paramList.params)[i]->wext,
                 get_smplfmt_string(((LDAC_DECODE_PARAM **)paramList.params)[i]->fmt),
                 ((LDAC_DECODE_PARAM **)paramList.params)[i]->wlen,
                 ((LDAC_DECODE_PARAM **)paramList.params)[i]->nodelay,
                 ((LDAC_DECODE_PARAM **)paramList.params)[i]->pPathFileIn,
                 ((LDAC_DECODE_PARAM **)paramList.params)[i]->pPathFileOut);
    }

    ith = 0;
    do{
        ldac_decode_proc( (void *)((LDAC_DECODE_PARAM **)paramList.params)[ith++] );
    } while( ith < paramList.nParam );

    if( paramList.params != NULL ){
        int k;
        for( k = 0; k < paramList.nParamMax; k++ ){
            if( ((LDAC_DECODE_PARAM**)paramList.params)[k] != NULL ){
                free(((LDAC_DECODE_PARAM**)paramList.params)[k]);
            }
        }
        free( paramList.params );
    }
    return 0;
}

/* LDAC decoding process */
#ifdef    __GNUC__
void
#else
unsigned int __stdcall
#endif
ldac_decode_proc(void *data)
{
    char *in_file, *out_file;
    FILE *fpi, *fpo;
    HANDLE_LDAC_BT hData;
    LDACBT_SMPL_FMT_T fmt;
#ifdef USE_LDACBT_SPEC_INFO
    LDACBT_SPEC_INFO  spec_info;
#else /* USE_LDACBT_SPEC_INFO */
    int flgInit = 0;
#endif /* USE_LDACBT_SPEC_INFO */
    int channel, wlength, sf, cm;
    int result, isf, status, wext, nodelay, sf_out;
    int n_read_bytes, used_bytes, wrote_bytes;
    LDAC_DECODE_PARAM * pDecParam;
    unsigned char    a_pcm_out[LDACBT_MAX_LSU*LDAC_PRCNCH*sizeof(int)];
    unsigned char    a_stream[LDACBT_MAX_NBYTES+2];

    /* Reset */
    status = 0;
    result = 0;
    hData = NULL;
    fpi = fpo = NULL;
#ifdef USE_LDACBT_SPEC_INFO
    memset(&spec_info, 0, sizeof(LDACBT_SPEC_INFO));
#endif /* USE_LDACBT_SPEC_INFO */

    /* Setup options */
    pDecParam = (LDAC_DECODE_PARAM *)data;
    wext = pDecParam->wext;
    fmt =  pDecParam->fmt;
    wlength = pDecParam->wlen;
    nodelay = pDecParam->nodelay;
    in_file = pDecParam->pPathFileIn;
    out_file = pDecParam->pPathFileOut;

    /* Open Coded Bit Stream File */
#ifdef USE_LDACBT_SPEC_INFO
    if ((fpi = fopen_ldac_read( &spec_info, in_file, &sf, &cm, &channel )) == (FILE *)NULL) {
#else /* USE_LDACBT_SPEC_INFO */
    if ((fpi = fopen_ldac_read( in_file )) == (FILE *)NULL) {
#endif /* USE_LDACBT_SPEC_INFO */
        status = -1;
        goto FINISH;
    }

    /* Get LDAC Handler */
    if ((hData = ldacBT_api_get_handle()) == (HANDLE_LDAC_BT)NULL)
    {
        fprintf(stderr, "Error: Can not Get LDAC Handle!\n");
        status = -1;
        goto FINISH;
    }

#ifdef USE_LDACBT_SPEC_INFO

    /* Initialize LDAC Handle as Decoder */
    result = ldacBT_api_init_handle_decode( hData, cm, sf, 0, 0, 0 );
    if (result) {
        fprintf(stderr, "[ERR] Initializing LDAC Handle for synthesis! Error code %s\n",
            get_error_code_string(ldacBT_api_get_error_code(hData)));
        status = -1;
        goto FINISH;
    }

    sf_out = sf;

    /* Open PCM Audio Data File */
    if ((fpo = open_wavefile_write(out_file, fmt, channel, sf_out, wext)) == (FILE *)NULL) {
        perror(out_file);
        status = -1;
        goto FINISH;
    }

    /* Show Decoding Parameter */
    fprintf(stderr, "<%d>[IN]\n", pDecParam->ith);
    fprintf(stderr, "<%d> LDACBT Bit Stream File : %s\n", pDecParam->ith, delpath(in_file));
    fprintf(stderr, "<%d>  - Sampling Frequency  : %d Hz\n", pDecParam->ith,
            ldacBT_api_get_sampling_freq(hData));
    fprintf(stderr, "<%d>  - Num of Channel      : %d ch\n", pDecParam->ith, channel);
    fprintf(stderr, "<%d>  - Channel Mode        : %d (%s)\n", pDecParam->ith, cm, cm==LDACBT_CHANNEL_MODE_MONO?"MONO":
            cm==LDACBT_CHANNEL_MODE_DUAL_CHANNEL?"DUAL Channel":
            cm==LDACBT_CHANNEL_MODE_STEREO?"STEREO":"unknown");
    fprintf(stderr, "<%d>[OUT]\n", pDecParam->ith);
    fprintf(stderr, "<%d> PCM Audio Data File    : %s\n", pDecParam->ith, delpath(out_file));
    fprintf(stderr, "<%d>  - Format              : %s\n", pDecParam->ith, get_smplfmt_string(fmt));
    fprintf(stderr, "<%d>  - Bit Depth           : %d bit\n", pDecParam->ith, BYTESIZE*wlength);
    fprintf(stderr, "<%d>  - Sampling Frequency  : %d Hz\n", pDecParam->ith, sf_out);
    fprintf(stderr, "<%d>  - Num of Channel      : %d ch\n", pDecParam->ith, channel);
#endif /* USE_LDACBT_SPEC_INFO */

    /* Decoding */
    isf = 0;
    while (1) {
        /* For non-transport format (Read Maximum bytes at once ) */
        n_read_bytes = fread(a_stream, sizeof(unsigned char), LDACBT_MAX_NBYTES, fpi);
        if (n_read_bytes <= 0) {
            break;
        }

#ifndef USE_LDACBT_SPEC_INFO
        if (flgInit == 0) {
            if (get_ldac_spec_info( a_stream, &sf, &cm, &channel ) < 0) {
                status = -1;
                goto FINISH;
            }

            /* Initialize LDAC Handle as Decoder */
            result = ldacBT_api_init_handle_decode( hData, cm, sf, 0, 0, 0 );
            if (result) {
                fprintf(stderr, "[ERR] Initializing LDAC Handle for synthesis! Error code %s\n",
                    get_error_code_string(ldacBT_api_get_error_code(hData)));
                status = -1;
                goto FINISH;
            }

            sf_out = sf;

            /* Open PCM Audio Data File */
            if ((fpo = open_wavefile_write(out_file, fmt, channel, sf_out, wext)) == (FILE *)NULL) {
                perror(out_file);
                status = -1;
                goto FINISH;
            }

            /* Show Decoding Parameter */
            fprintf(stderr, "<%d>[IN]\n", pDecParam->ith);
            fprintf(stderr, "<%d> LDACBT Bit Stream File : %s\n", pDecParam->ith, delpath(in_file));
            fprintf(stderr, "<%d>  - Sampling Frequency  : %d Hz\n", pDecParam->ith,
                    ldacBT_api_get_sampling_freq(hData));
            fprintf(stderr, "<%d>  - Num of Channel      : %d ch\n", pDecParam->ith, channel);
            fprintf(stderr, "<%d>  - Channel Mode        : %d (%s)\n", pDecParam->ith, cm, cm==LDACBT_CHANNEL_MODE_MONO?"MONO":
                    cm==LDACBT_CHANNEL_MODE_DUAL_CHANNEL?"DUAL Channel":
                    cm==LDACBT_CHANNEL_MODE_STEREO?"STEREO":"unknown");
            fprintf(stderr, "<%d>[OUT]\n", pDecParam->ith);
            fprintf(stderr, "<%d> PCM Audio Data File    : %s\n", pDecParam->ith, delpath(out_file));
            fprintf(stderr, "<%d>  - Format              : %s\n", pDecParam->ith, get_smplfmt_string(fmt));
            fprintf(stderr, "<%d>  - Bit Depth           : %d bit\n", pDecParam->ith, BYTESIZE*wlength);
            fprintf(stderr, "<%d>  - Sampling Frequency  : %d Hz\n", pDecParam->ith, sf_out);
            fprintf(stderr, "<%d>  - Num of Channel      : %d ch\n", pDecParam->ith, channel);

            flgInit = 1;
        }
#endif /* USE_LDACBT_SPEC_INFO */

        /* Decode Frame */
        result = ldacBT_api_decode( hData, a_stream, a_pcm_out, fmt, n_read_bytes, &used_bytes,
                                &wrote_bytes );

        if (result) {
            int error_code;
            error_code = ldacBT_api_get_error_code( hData );
            fprintf(stderr, "<%d>[ERR] At %d-th su. error_code = %4d, %4d, %4d"
                    "FrameHeader[%02x:%02x:%02x]\n", pDecParam->ith, isf,
                    LDACBT_API_ERR(error_code), LDACBT_HANDLE_ERR(error_code),
                    LDACBT_BLOCK_ERR(error_code), a_stream[0], a_stream[1], a_stream[2]);
#ifdef    STOP_WHEN_ERROR
            goto FINISH;
#endif    /* STOP_WHEN_ERROR */
            if( LDACBT_FATAL(error_code) ){
                /* Recovery Process */
                ldacBT_api_close_handle( hData );
                /* Initialize LDAC Handle as Decoder */
                if( ldacBT_api_init_handle_decode( hData, cm, sf, 0, 0, 0 ) < 0 ){
                    fprintf(stderr, "<%d>Error: Can not Get LDAC Handle!\n", pDecParam->ith);
                    status = -1;
                    goto FINISH;
                }
                /* update used_bytes to skip current ldac frame */
                if(1){
                    unsigned char *pStream, *p1st, *p2nd, *pTail;
                    unsigned char sync2, cci;
                    p1st = p2nd = NULL;
                    pStream = a_stream;
                    pTail = a_stream + n_read_bytes - 2;
                    switch(cm){
                      case LDACBT_CHANNEL_MODE_MONO:
                        cci = LDAC_CCI_MONO;
                        break;
                      case LDACBT_CHANNEL_MODE_DUAL_CHANNEL:
                        cci = LDAC_CCI_DUAL_CHANNEL;
                        break;
                      case LDACBT_CHANNEL_MODE_STEREO:
                      default:
                        cci = LDAC_CCI_STEREO;
                        break;
                    }
                    if(0){;}
                    else if(sf == 1*44100){ sync2 = (0 << 5) | (cci << 3); }
                    else if(sf == 1*48000){ sync2 = (1 << 5) | (cci << 3); }
                    else if(sf == 2*44100){ sync2 = (2 << 5) | (cci << 3); }
                    else if(sf == 2*48000){ sync2 = (3 << 5) | (cci << 3); }
                    else if(sf == 4*44100){ sync2 = (4 << 5) | (cci << 3); }
                    else if(sf == 4*48000){ sync2 = (5 << 5) | (cci << 3); }
                    while( pStream < pTail ){
                        if( pStream[0] == 0xAA ){ /* syncword */
                            if( (pStream[1] & 0xF8) == sync2 ){
                                if( p1st == NULL ){ p1st = pStream; } /* 1st syncword found */
                                else{ /* 2nd syncword found, this frame must decode next time */
                                    p2nd = pStream;
                                    used_bytes = pStream - a_stream;
                                    break;
                                }
                            }
                        }
                        ++pStream;
                    }
                    if( p2nd == NULL ){
                        /* faild to find next frame header */
                        fprintf(stderr, "<%d>Error: Can not get next LDAC frame_header!\n",
                                pDecParam->ith);
                        status = -1;
                        goto FINISH;
                    }
                }

                status = -1;
                memset(a_pcm_out, 0, sizeof(a_pcm_out));
                wrote_bytes = 0;
                if(0){;}
                else if(sf == 1*44100){ wrote_bytes = 128; }
                else if(sf == 1*48000){ wrote_bytes = 128; }
                else if(sf == 2*44100){ wrote_bytes = 256; }
                else if(sf == 2*48000){ wrote_bytes = 256; }
                else if(sf == 4*44100){ wrote_bytes = 512; }
                else if(sf == 4*48000){ wrote_bytes = 512; }
                wrote_bytes *= channel * wlength;
            }
        }

        /* Write PCM Data */
        if (!nodelay) {
            if ( wrote_bytes > 0) {
                fwrite( a_pcm_out, sizeof(unsigned char), wrote_bytes, fpo);
            }
        }
        else {
            if ( isf > 0 && wrote_bytes > 0) {
                fwrite( a_pcm_out, sizeof(unsigned char), wrote_bytes, fpo);
            }
        }

        fseek(fpi, used_bytes-n_read_bytes, SEEK_CUR);

        isf++;
    }

FINISH:

    /* Close Files */
    if (fpi) fclose(fpi);
    if (fpo) fclose_wave_write(fpo);

    if( pDecParam != NULL ){
        fprintf(stderr, "End of the process ID %d", pDecParam->ith );
        if( hData != NULL ){
            result = ldacBT_api_get_error_code( hData );
            if( result != 0 ){
                fprintf(stderr, " with error_code : %d, %d, %d", LDACBT_API_ERR(result),
                        LDACBT_HANDLE_ERR(result), LDACBT_BLOCK_ERR(result) );
            }
        }
        fprintf( stderr, ".\n");
    }

    ldacBT_api_free_handle( hData );

#ifdef __GNUC__
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

/* Program End Procedure */
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

/* Open LDAC bit stream file for read */
#ifdef USE_LDACBT_SPEC_INFO
static FILE *fopen_ldac_read( LDACBT_SPEC_INFO *p_spec_info, char *in_file,
                              int *sf, int *cm, int *channel )
{
    int n;
    FILE *fp;
    /* Open Coded Bit Stream File */
    if ((fp = fopen(in_file, "rb")) == (FILE *)NULL) {
        perror(in_file);
        return NULL;
    }

    /* Read Codec Specific Info at Head of LDAC file */
    n = fread( p_spec_info, sizeof(unsigned char), LDACBT_SPEC_INFO_SIZE, fp);
    if ( n != LDACBT_SPEC_INFO_SIZE ) {
        fprintf(stderr, "Error: Can not Get Codec Specific Info from file!\n");
        fclose(fp);
        return NULL;
    }

    /* check */
    if( ( p_spec_info->vendor_id0 != LDACBT_VENDOR_ID0 ) ||
        ( p_spec_info->vendor_id1 != LDACBT_VENDOR_ID1 ) ||
        ( p_spec_info->vendor_id2 != LDACBT_VENDOR_ID2 ) ||
        ( p_spec_info->vendor_id3 != LDACBT_VENDOR_ID3 ) ||
        ( p_spec_info->codec_id0 != LDACBT_CODEC_ID0 ) ||
        ( p_spec_info->codec_id1 != LDACBT_CODEC_ID1 ) )
    {
        fprintf(stderr, "Error: Not a LDAC BT stream file!\n");
        fclose(fp);
        return NULL;
    }
    /* get sampling frequency */
    if( p_spec_info->fs_info & LDACBT_SAMPLING_FREQ_192000 ){        *sf = 4*48000;}
    else if( p_spec_info->fs_info & LDACBT_SAMPLING_FREQ_176400 ){    *sf = 4*44100;}
    else if( p_spec_info->fs_info & LDACBT_SAMPLING_FREQ_096000 ){    *sf = 2*48000;}
    else if( p_spec_info->fs_info & LDACBT_SAMPLING_FREQ_088200 ){    *sf = 2*44100;}
    else if( p_spec_info->fs_info & LDACBT_SAMPLING_FREQ_048000 ){    *sf = 48000;    }
    else if( p_spec_info->fs_info & LDACBT_SAMPLING_FREQ_044100 ){    *sf = 44100;    }
    else{
        fprintf(stderr, "Error: Unsupported FS INFO 0x%02x!\n", p_spec_info->fs_info);
        fclose(fp);
        return NULL;
    }

    /* get channel mode & channel */
    *cm = p_spec_info->cm;
    if( p_spec_info->cm == LDACBT_CHANNEL_MODE_MONO ){ 
        *channel = 1;
    }
    else if( p_spec_info->cm == LDACBT_CHANNEL_MODE_DUAL_CHANNEL ){ 
        *channel = 2;
    }
    else if( p_spec_info->cm == LDACBT_CHANNEL_MODE_STEREO ){ 
        *channel = 2;
    }
    else{
        fprintf(stderr, "Error: Unsupported Channel Mode 0x%02x!\n", p_spec_info->cm);
        fclose(fp);
        return NULL;
    }
    return fp;
}
#else /* USE_LDACBT_SPEC_INFO */
static FILE *fopen_ldac_read( char *in_file )
{
    FILE *fp;
    /* Open Coded Bit Stream File */
    if ((fp = fopen(in_file, "rb")) == (FILE *)NULL) {
        perror(in_file);
        return NULL;
    }
    return fp;
}

static int get_ldac_spec_info( unsigned char *p_stream, int *sf, int *cm, int *channel )
{
    int sampling_rate_index, channel_config_index;

    if (p_stream[0] == 0xAA) {
        sampling_rate_index = (p_stream[1] >> 5) & 0x7;
        channel_config_index = (p_stream[1] >> 3) & 0x3;

        /* get sampling frequency */
        if( sampling_rate_index == 0 ){         *sf = 44100;}
        else if( sampling_rate_index == 1 ){    *sf = 48000;}
        else if( sampling_rate_index == 2 ){    *sf = 2*44100;}
        else if( sampling_rate_index == 3 ){    *sf = 2*48000;}
        else if( sampling_rate_index == 4 ){    *sf = 4*44100;}
        else if( sampling_rate_index == 5 ){    *sf = 4*48000;}
        else{
            fprintf(stderr, "Error: Unsupported Sampling Frequency Info 0x%02x!\n", sampling_rate_index);
            return -1;
        }

        /* get channel mode & channel */
        if( channel_config_index == 0 ){
            *cm = LDACBT_CHANNEL_MODE_MONO;
            *channel = 1;
        }
        else if( channel_config_index == 1 ){
            *cm = LDACBT_CHANNEL_MODE_DUAL_CHANNEL;
            *channel = 2;
        }
        else if( channel_config_index == 2 ){
            *cm = LDACBT_CHANNEL_MODE_STEREO;
            *channel = 2;
        }
        else{
            fprintf(stderr, "Error: Unsupported Channel Info 0x%02x!\n", channel_config_index);
            return -1;
        }
    }
    else {
        fprintf(stderr, "Error: Not a LDAC BT stream file!\n");
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

/* RIFF file header */
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
 * Size of "fmt" chunk without "chunk type" and "chunk size" fields may be sizeof(_pcm_header).
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
#define    WAVEXT_NCH_1        1
#define    WAVEXT_NCH_2        2
#define    WAVEXT_CHMASK_1     (0x1)  /* 0x1       */
#define    WAVEXT_CHMASK_2     (0x3)  /* 0x1 | 0x2 */

#define CBSIZE_PCMEXT_HEADER    22    /* be set to "cbSize" field */
#define SIZEOF_PCMEXT_HEADER    40    /* SIZEOF_PCM_HEADER + cbSize +  CBSIZE_PCMEXT_HEADER */

typedef struct {
    UINT16    wFormatTag;              /*  2 : Compression Format ID: WAVE_FORMAT_EXTENSIBLE */
    UINT16    nChannels;               /*  4 : Number of Channels */
    UINT32    nSamplesPerSec;          /*  8 : Sampling Frequency [Hz] */
    UINT32    nAvgBytesPerSec;         /* 12 : Average Number of Bytes per Sec. */
    UINT16    nBlockAlign;             /* 14 : Number of Block Alignment Bytes */
    UINT16    wBitsPerSample;          /* 16 : Quantization Bit Number for Audio Sample */
    UINT16    cbSize;                  /* 18 : Size of the Extended Header that follows */
    union {
        UINT16    wValidBitsPerSample; /*  2 : Bits of Precision for Each Audio Sample */
        UINT16    wSamplesPerBlock;    /*  2 : The Number of Samples in a Compressed Block */
        UINT16    wReserved;           /*  2 : Reserved */
    }    Samples;
    UINT32    dwChannelMask;           /*  6 : The Mapping of Channels to spatial location */
    GUID     SubFormat;                /* 22 : GUID Size = 16 (4 + 2 + 2 + 8) */
} _pcmext_header;

/* fmt chunk definition(without 'chunk type' and 'chunk size' fields) */
typedef union {
    _pcm_header pcm_header;
    _pcmext_header pcmext_header;
}       _fmt;

#define MAX_HEADER_SIZE (8+4+8+SIZEOF_PCMEXT_HEADER+8)

static int writeLong(char *p, unsigned int d)
{
    p[0] = (char)(d & 0xff);
    p[1] = (char)((d >>  8) & 0xff);
    p[2] = (char)((d >> 16) & 0xff);
    p[3] = (char)((d >> 24) & 0xff);
    return 4;
}

static int writeShort(char *p, unsigned short d)
{
    p[0] =  d & 0xff;
    p[1] = (d >> 8) & 0xff;
    return 2;
}

#ifndef KSDATAFORMAT_SUBTYPE_PCM
static const GUID KSDATAFORMAT_SUBTYPE_PCM
= {0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
/* 00000001-0000-0010-8000-00aa00389b71 */
#endif
#ifndef KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
static const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
= {0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
/* 00000003-0000-0010-8000-00aa00389b71 */
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

/* the function "guidcpy()" copies GUID s2 to s1 and returns s1 */
static GUID *guidcpy(GUID *s1, const GUID *s2)
{
    s1->Data1 = s2->Data1;
    s1->Data2 = s2->Data2;
    s1->Data3 = s2->Data3;
    memcpy(s1->Data4, s2->Data4, 8);

    return (s1);
}

/* create wave header */
int    CreateWaveHeader(char *tmp_buf, _fmt *fmt)
{
    char    *ptr = tmp_buf;

    /* make main chunk header */
    *ptr++ = 'R'; *ptr++ = 'I'; *ptr++ = 'F'; *ptr++ = 'F';
    ptr += writeLong(ptr, 0);    /*** file size: should be updated lately ***/

    /* make wave chunk */
    *ptr++ = 'W'; *ptr++ = 'A'; *ptr++ = 'V'; *ptr++ = 'E';
    *ptr++ = 'f'; *ptr++ = 'm'; *ptr++ = 't'; *ptr++ = ' ';

    if (fmt->pcm_header.wFormatTag == WAVE_FORMAT_IEEE_FLOAT){
        if( guidcmp(&fmt->pcmext_header.SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
            _pcmext_header *pcmext_header = &fmt->pcmext_header;
            ptr += writeLong (ptr, SIZEOF_PCMEXT_HEADER);    /* may be sizeof(_pcmext_header) */
            ptr += writeShort(ptr, pcmext_header->wFormatTag);
            ptr += writeShort(ptr, pcmext_header->nChannels);
            ptr += writeLong (ptr, pcmext_header->nSamplesPerSec);
            ptr += writeLong (ptr, pcmext_header->nAvgBytesPerSec);
            ptr += writeShort(ptr, pcmext_header->nBlockAlign);
            ptr += writeShort(ptr, pcmext_header->wBitsPerSample);
            ptr += writeShort(ptr, pcmext_header->cbSize);
            ptr += writeShort(ptr, pcmext_header->Samples.wValidBitsPerSample);
            ptr += writeLong (ptr, pcmext_header->dwChannelMask);
            ptr += writeLong (ptr, pcmext_header->SubFormat.Data1);
            ptr += writeShort(ptr, pcmext_header->SubFormat.Data2);
            ptr += writeShort(ptr, pcmext_header->SubFormat.Data3);
            memcpy(ptr, pcmext_header->SubFormat.Data4, 8);        ptr += 8;
        }
        else{
            _pcm_header *pcm_header = &fmt->pcm_header;
            ptr += writeLong (ptr, SIZEOF_PCM_HEADER);    /* may be sizeof(_pcm_header) */
            ptr += writeShort(ptr, pcm_header->wFormatTag);
            ptr += writeShort(ptr, pcm_header->nChannels);
            ptr += writeLong (ptr, pcm_header->nSamplesPerSec);
            ptr += writeLong (ptr, pcm_header->nAvgBytesPerSec);
            ptr += writeShort(ptr, pcm_header->nBlockAlign);
            ptr += writeShort(ptr, pcm_header->wBitsPerSample);
        }
    }
    else if (fmt->pcm_header.wFormatTag == WAVE_FORMAT_EXTENSIBLE
         && guidcmp(&fmt->pcmext_header.SubFormat, &KSDATAFORMAT_SUBTYPE_PCM)) {
        _pcmext_header *pcmext_header = &fmt->pcmext_header;
        ptr += writeLong (ptr, SIZEOF_PCMEXT_HEADER);    /* may be sizeof(_pcmext_header) */
        ptr += writeShort(ptr, pcmext_header->wFormatTag);
        ptr += writeShort(ptr, pcmext_header->nChannels);
        ptr += writeLong (ptr, pcmext_header->nSamplesPerSec);
        ptr += writeLong (ptr, pcmext_header->nAvgBytesPerSec);
        ptr += writeShort(ptr, pcmext_header->nBlockAlign);
        ptr += writeShort(ptr, pcmext_header->wBitsPerSample);
        ptr += writeShort(ptr, pcmext_header->cbSize);
        ptr += writeShort(ptr, pcmext_header->Samples.wValidBitsPerSample);
        ptr += writeLong (ptr, pcmext_header->dwChannelMask);
        ptr += writeLong (ptr, pcmext_header->SubFormat.Data1);
        ptr += writeShort(ptr, pcmext_header->SubFormat.Data2);
        ptr += writeShort(ptr, pcmext_header->SubFormat.Data3);
        memcpy(ptr, pcmext_header->SubFormat.Data4, 8);        ptr += 8;
    }
    else {
        _pcm_header *pcm_header = &fmt->pcm_header;
        ptr += writeLong (ptr, SIZEOF_PCM_HEADER);    /* may be sizeof(_pcm_header) */
        ptr += writeShort(ptr, pcm_header->wFormatTag);
        ptr += writeShort(ptr, pcm_header->nChannels);
        ptr += writeLong (ptr, pcm_header->nSamplesPerSec);
        ptr += writeLong (ptr, pcm_header->nAvgBytesPerSec);
        ptr += writeShort(ptr, pcm_header->nBlockAlign);
        ptr += writeShort(ptr, pcm_header->wBitsPerSample);
    }
    *ptr++ = 'd'; *ptr++ = 'a'; *ptr++ = 't'; *ptr++ = 'a';
    ptr += writeLong(ptr, 0);     /*** data chunk size: should be updated lately ***/

    return ((int)(ptr-tmp_buf));
}

/* Open wave file for write */
static FILE *open_wavefile_write(char *fname, LDACBT_SMPL_FMT_T fmt, int channel, int sf, int wext)
{
    FILE    *fp;
    _fmt    pcmfmt;
    _pcmext_header    *p_header;
    char tmp_buf[MAX_HEADER_SIZE];
    int nbytes, wlength;

    fp = NULL;
    if ((fp = fopen(fname, "wb+")) == (FILE *)NULL) { return fp;}

    switch (fmt){
      case LDACBT_SMPL_FMT_S16:
        wlength = 2;
        break;
      case LDACBT_SMPL_FMT_S24:
        wlength = 3;
        break;
      case LDACBT_SMPL_FMT_S32:
      case LDACBT_SMPL_FMT_F32:
        wlength = 4;
        break;
      default:
        fprintf( stderr, "[ERR] unsupported pcm sample format\n" );
        return NULL;
    }

    p_header = &pcmfmt.pcmext_header;
    p_header->nChannels = channel;
    p_header->nSamplesPerSec = sf;
    p_header->nAvgBytesPerSec = sf * wlength * channel;
    p_header->nBlockAlign = wlength * channel;
    p_header->wBitsPerSample = wlength * 8;
    if (wext) {
        if( fmt == LDACBT_SMPL_FMT_F32 ){
            p_header->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
            guidcpy(&p_header->SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
        }else{
            p_header->wFormatTag = WAVE_FORMAT_EXTENSIBLE;
            guidcpy(&p_header->SubFormat, &KSDATAFORMAT_SUBTYPE_PCM);
        }
        p_header->cbSize = CBSIZE_PCMEXT_HEADER;
        p_header->Samples.wValidBitsPerSample = wlength * 8;
        if(0){;}
        else if (channel == WAVEXT_NCH_1) { p_header->dwChannelMask = WAVEXT_CHMASK_1;}
        else if (channel == WAVEXT_NCH_2) {    p_header->dwChannelMask = WAVEXT_CHMASK_2;}
        else { p_header->dwChannelMask = 0; }
    }
    else {
        if( fmt == LDACBT_SMPL_FMT_F32 ){
            p_header->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
        }else{
            p_header->wFormatTag = WAVE_FORMAT_PCM;
        }
    }

    memset(tmp_buf, 0, MAX_HEADER_SIZE);
    nbytes = CreateWaveHeader(tmp_buf, &pcmfmt);

    if (fwrite(tmp_buf, 1, nbytes, fp) != (size_t)nbytes) {
        fclose(fp);
        return (FILE *)NULL;
    }

    return fp;
}


/* get 32-bit data from the char stream */
static unsigned int getLong(char *ptr)
{
    unsigned int ret;
    ret  =  ptr[0] & 0xff;
    ret |= (ptr[1] & 0xff) <<  8;
    ret |= (ptr[2] & 0xff) << 16;
    ret |= (ptr[3] & 0xff) << 24;
    return ret;
}
/* update wave file */
static int UpdateWaveHeader(char *tmp_buf, SINT32 file_size)
{
    char     *ptr = tmp_buf;
    UINT32    sub_chunk_id;
    UINT32    sub_chunk_size;
    UINT32    chunk_size = file_size;
    int       status = 0;
    /* Check (RIFF) Chunk ID */
    if (getLong(ptr) != RIFF_CHUNK_ID) return -1;
    ptr += 4;
    ptr += writeLong(ptr, file_size - 8);/* update 'RIFF' chunk size */
    chunk_size -= 8;

    /* Check WAVE_FORMAT */
    if (getLong(ptr) != RIFF_CHUNK_FORMAT_WAVE) return -1;
    ptr += 4;
    chunk_size -= 4;

    while (!status) {
        sub_chunk_id = getLong(ptr);    
        ptr += 4;
        sub_chunk_size = getLong(ptr);

        switch (sub_chunk_id) {
        case RIFF_SUB_CHUNK_FMT_:/* 'fmt ' */
            ptr += 4;
            break;
        case RIFF_SUB_CHUNK_FACT:/* 'fact' */
            ptr += 4;
            break;
        case RIFF_SUB_CHUNK_DATA:/* 'data' */
            ptr += writeLong(ptr, chunk_size - 8);/* update 'data' chunk size */
            status = 1;
            break;
        default: /* unknown chunk */
            return -1;
        }

        /* skip remain of chunk */
        ptr += sub_chunk_size;
        chunk_size -= sub_chunk_size + 8;
    }
    
    return (0);
}

/* WAVE file close for write */
/* maximum header size expected */
static int fclose_wave_write(FILE *fp)
{
    SINT32  file_size;
    char    tmp_buf[MAX_HEADER_SIZE];
    int     nbytes;

    if ((file_size = ftell(fp)) > MAX_HEADER_SIZE) {
        if ((nbytes = fseek(fp, 0L, 0)) != 0L ||
            fread(tmp_buf, 1, MAX_HEADER_SIZE, fp) != MAX_HEADER_SIZE) {
            fprintf(stderr, "Cannot read wave header %d\n", nbytes);
        }
        else {
            if (UpdateWaveHeader(tmp_buf, file_size) < 0) {
                fprintf(stderr, "Illegal Wave Header\n");
            }
            else if (fseek(fp, 0L, 0) != 0L ||
                 fwrite(tmp_buf, 1, MAX_HEADER_SIZE, fp) != MAX_HEADER_SIZE) {
                fprintf(stderr, "Cannot update wave header\n");
            }
        }
    }

    return (fclose(fp));
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



