/*******************************************************************************
 *
 * Copyright (C) 2003 - 2021 Sony Corporation
 *
 ******************************************************************************/

#include "ldac.h"

/***************************************************************************************************
    Align Memory
***************************************************************************************************/
#define LDAC_ALLOC_LINE 8

DECLFUNC size_t align_ldac(
size_t size)
{
    if (LDAC_ALLOC_LINE != 0) {
        size = (((size-1)/LDAC_ALLOC_LINE)+1) * LDAC_ALLOC_LINE;
    }

    return size;
}

/***************************************************************************************************
    Clear Allocate Memory
***************************************************************************************************/
DECLFUNC void *calloc_ldac(
SFINFO *p_sfinfo,
size_t nmemb,
size_t size)
{
    char *p_tmp;

    if (p_sfinfo->p_mempos != (char *)NULL) {
        p_tmp = p_sfinfo->p_mempos;
        p_sfinfo->p_mempos += nmemb * align_ldac(size);
    }
    else {
        p_tmp = calloc(nmemb, size);
    }

    return (void *)p_tmp;
}

