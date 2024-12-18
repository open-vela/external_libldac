/*******************************************************************************
 *
 * Copyright (C) 2003 - 2021 Sony Corporation
 *
 ******************************************************************************/

#ifndef _STRUCT_H
#define _STRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/***************************************************************************************************
    Macro Definition
***************************************************************************************************/

#define DECLFUNC static

#ifndef PI
#ifdef M_PI
#define PI M_PI
#else /* M_PI */
#define PI (double)(3.14159265358979323846)
#endif /* M_PI */
#endif /* PI */

/***************************************************************************************************
    Type Definition
***************************************************************************************************/
typedef unsigned char STREAM;

typedef short          INT16;
typedef int            INT32;
typedef unsigned int  UINT32;
typedef long long      INT64;

typedef float         SCALAR;
#define _scalar(x) x##f
typedef union {
    float f;
    int i;
} IEEE754_FI;

/***************************************************************************************************
    Macro Functions
***************************************************************************************************/
/* Buffer Operations */
#define clear_data_ldac(p, n)      memset((p), 0, (n))
#define clear_seq_s_ldac(p, n)     memset((char *)(p), 0, (n)*sizeof(short))
#define clear_seq_l_ldac(p, n)     memset((char *)(p), 0, (n)*sizeof(int))
#define clear_seq_f_ldac(p, n)     memset((char *)(p), 0, (n)*sizeof(SCALAR))

#if defined(_MSC_VER) && _MSC_VER >=1400
/* Secured CRT Functions */
#define copy_data_ldac(p1, p2, n)  memcpy_s((p2), (n), (p1), (n))
#define copy_seq_s_ldac(p1, p2, n) memcpy_s((p2), (n)*sizeof(short), (p1), (n)*sizeof(short))
#define copy_seq_l_ldac(p1, p2, n) memcpy_s((p2), (n)*sizeof(int), (p1), (n)*sizeof(int))
#define copy_seq_f_ldac(p1, p2, n) memcpy_s((p2), (n)*sizeof(SCALAR), (p1), (n)*sizeof(SCALAR))
#define move_seq_f_ldac(p1, p2, n) memmove_s((p2), (n)*sizeof(SCALAR), (p1), (n)*sizeof(SCALAR))
#else
#define copy_data_ldac(p1, p2, n)  memcpy((p2), (p1), (n))
#define copy_seq_s_ldac(p1, p2, n) memcpy((p2), (p1), (n)*sizeof(short))
#define copy_seq_l_ldac(p1, p2, n) memcpy((p2), (p1), (n)*sizeof(int))
#define copy_seq_f_ldac(p1, p2, n) memcpy((p2), (p1), (n)*sizeof(SCALAR))
#define move_seq_f_ldac(p1, p2, n) memmove((p2), (p1), (n)*sizeof(SCALAR))
#endif

#endif /* _STRUCT_H */

