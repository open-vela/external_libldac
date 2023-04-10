/*******************************************************************************
 *
 * Copyright (C) 2003 - 2021 Sony Corporation
 *
 ******************************************************************************/

#include "ldac.h"


/*******************************************************************************
    Subfunction: Check Saturation
*******************************************************************************/
__inline static INT32 check_sature_ldac(
INT64 val)
{

    return (INT32)val;
}

/*******************************************************************************
    Shift and Round
*******************************************************************************/
DECLFUNC INT32 sftrnd_ldac(
INT32 in,
int shift)
{
    INT64 out;

    if (shift > 0) {
        out = ((INT64)in + ((INT64)1 << (shift-1))) >> shift;
    }
    else {
        out = (INT64)in << (-shift);
    }

    return check_sature_ldac(out);
}


/*******************************************************************************
    Get Bit Length of Value
*******************************************************************************/
DECLFUNC int get_bit_length_ldac(
INT32 val)
{
    int len;

    len = 0;
    while (val > 0) {
        val >>= 1;
        len++;
    }

    return len;
}

/*******************************************************************************
    Get Maximum Absolute Value
*******************************************************************************/
DECLFUNC INT32 get_absmax_ldac(
INT32 *p_x,
int num)
{
    int i;
    INT32 abmax, val;

    abmax = abs(p_x[0]);
    for (i = 1; i < num; i++) {
        val = abs(p_x[i]);
        if (abmax < val) {
            abmax = val;
        }
    }

    return abmax;
}

