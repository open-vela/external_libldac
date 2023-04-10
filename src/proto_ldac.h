/*******************************************************************************
 *
 * Copyright (C) 2003 - 2021 Sony Corporation
 *
 ******************************************************************************/

#ifndef _PROTO_LDAC_H
#define _PROTO_LDAC_H

/***************************************************************************************************
    Function Declarations
***************************************************************************************************/
#ifndef _DECODE_ONLY
/* encode_ldac.c */
DECLFUNC LDAC_RESULT init_encode_ldac(SFINFO *);
DECLFUNC void calc_initial_bits_ldac(SFINFO *);
DECLFUNC void free_encode_ldac(SFINFO *);
DECLFUNC int encode_ldac(SFINFO *, int, int, int, int, int, int, int);
#endif /* _DECODE_ONLY */

#ifndef _ENCODE_ONLY
/* decode_ldac.c */
DECLFUNC LDAC_RESULT init_decode_ldac(SFINFO *);
DECLFUNC void free_decode_ldac(SFINFO *);
DECLFUNC void decode_ldac(SFINFO *);
#endif /* _ENCODE_ONLY */

/* setpcm_ldac.c */
#ifndef _DECODE_ONLY
DECLFUNC void set_input_pcm_ldac(SFINFO *, char *[], LDAC_SMPL_FMT_T, int);
#endif /* _DECODE_ONLY */
#ifndef _ENCODE_ONLY
DECLFUNC void set_output_pcm_ldac(SFINFO *, char *[], LDAC_SMPL_FMT_T, int);
#endif /* _ENCODE_ONLY */

#ifndef _DECODE_ONLY
/* mdct_ldac.c */
DECLFUNC void proc_mdct_ldac(SFINFO *, int);
#endif /* _DECODE_ONLY */

#ifndef _ENCODE_ONLY
/* imdct_ldac.c */
DECLFUNC void proc_imdct_ldac(SFINFO *, int);
#endif /* _ENCODE_ONLY */


#ifndef _DECODE_ONLY
/* sigana_ldac.c */
DECLFUNC int ana_frame_status_ldac(SFINFO *, int);
#endif /* _DECODE_ONLY */

#ifndef _DECODE_ONLY
/* bitalloc_ldac.c */
DECLFUNC int alloc_bits_ldac(AB *);
#endif /* _DECODE_ONLY */

/* bitalloc_sub_ldac.c */
#ifndef _DECODE_ONLY
DECLFUNC int encode_side_info_ldac(AB *);
#endif /* _DECODE_ONLY */
DECLFUNC void calc_add_word_length_ldac(AC *);
#ifndef _ENCODE_ONLY
DECLFUNC void reconst_gradient_ldac(AB *, int, int);
DECLFUNC void reconst_word_length_ldac(AC *);
#endif /* _ENCODE_ONLY */

#ifndef _DECODE_ONLY
/* quant_ldac.c */
DECLFUNC void norm_spectrum_ldac(AC *);
DECLFUNC void quant_spectrum_ldac(AC *);
DECLFUNC void quant_residual_ldac(AC *);
#endif /* _DECODE_ONLY */

#ifndef _ENCODE_ONLY
/* dequant_ldac.c */
DECLFUNC void dequant_spectrum_ldac(AC *);
DECLFUNC void dequant_residual_ldac(AC *);
DECLFUNC void clear_spectrum_ldac(AC *, int);
#endif /* _ENCODE_ONLY */

#ifndef _DECODE_ONLY
/* pack_ldac.c */
DECLFUNC void pack_frame_header_ldac(int, int, int, int, STREAM *);
DECLFUNC int pack_raw_data_frame_ldac(SFINFO *, STREAM *, int *, int *);
DECLFUNC int pack_null_data_frame_ldac(SFINFO *, STREAM *, int *, int *);
#endif /* _DECODE_ONLY */

#ifndef _ENCODE_ONLY
/* unpack_ldac.c */
DECLFUNC int unpack_frame_header_ldac(int *, int *, int *, int *, STREAM *);
DECLFUNC int unpack_raw_data_frame_ldac(SFINFO *, STREAM *, int *, int *);
#endif /* _ENCODE_ONLY */

/* tables_ldac.c */
DECLFUNC int get_block_nchs_ldac(int);
#ifndef _ENCODE_ONLY
#endif /* _ENCODE_ONLY */

/* tables_sigproc_ldac.c */
#ifndef _DECODE_ONLY
DECLFUNC void set_mdct_table_ldac(int);
#endif /* _DECODE_ONLY */
#ifndef _ENCODE_ONLY
DECLFUNC void set_imdct_table_ldac(int);
#endif /* _ENCODE_ONLY */

/* memory_ldac.c */
DECLFUNC size_t align_ldac(size_t);
DECLFUNC void *calloc_ldac(SFINFO *, size_t, size_t);

#endif /* _PROTO_LDAC_H */

