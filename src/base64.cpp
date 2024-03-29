#include <drift/dslcore.h>
#include <drift/base64.h>

/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
/* vi: set expandtab shiftwidth=4 tabstop=4: */
/**
 * \file modp_b64.c
 * <PRE>
 * MODP_B64 - High performance base64 encoder/decoder
 * http://code.google.com/p/stringencoders/
 *
 * Copyright &copy; 2005, 2006, 2007  Nick Galbreath -- nickg [at] modp [dot] com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *   Neither the name of the modp.com nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This is the standard "new" BSD license:
 * http://www.opensource.org/licenses/bsd-license.php
 * </PRE>
 */

#if defined(BIG_ENDIAN)
#define WORDS_BIGENDIAN 1
#else
#undef WORDS_BIGENDIAN
#endif

#include <drift/base64_data.h>

#define BADCHAR 0x01FFFFFF

/**
 * you can control if we use padding by commenting out this
 * next line.  However, I highly recommend you use padding and not
 * using it should only be for compatability with a 3rd party.
 * Also, 'no padding' is not tested!
 */
#define DOPAD 1

/*
 * if we aren't doing padding
 * set the pad character to NULL
 */
#ifndef DOPAD
#undef CHARPAD
#define CHARPAD '\0'
#endif

int DSL_CC base64_encode(const void * str, size_t len, char * dest) {
    size_t i;
    const uint8* s = (const uint8*) str;
    uint8* p = (uint8*) dest;

    /* unsigned here is important! */
    /* uint8 is fastest on G4, amd */
    /* uint32 is fastest on Intel */
    uint32 t1, t2, t3;

    for (i = 0; i < len - 2; i += 3) {
        t1 = s[i]; t2 = s[i+1]; t3 = s[i+2];
        *p++ = e0[t1];
        *p++ = e1[((t1 & 0x03) << 4) | ((t2 >> 4) & 0x0F)];
        *p++ = e1[((t2 & 0x0F) << 2) | ((t3 >> 6) & 0x03)];
        *p++ = e2[t3];
    }

    switch (len - i) {
    case 0:
        break;
    case 1:
        t1 = s[i];
        *p++ = e0[t1];
        *p++ = e1[(t1 & 0x03) << 4];
        *p++ = CHARPAD;
        *p++ = CHARPAD;
        break;
    default: /* case 2 */
        t1 = s[i]; t2 = s[i+1];
        *p++ = e0[t1];
        *p++ = e1[((t1 & 0x03) << 4) | ((t2 >> 4) & 0x0F)];
        *p++ = e2[(t2 & 0x0F) << 2];
        *p++ = CHARPAD;
    }

    *p = '\0';
    return (int)(p - (uint8*)dest);
}

#ifdef WORDS_BIGENDIAN   /* BIG ENDIAN -- SUN / IBM / MOTOROLA */
int DSL_CC base64_decode(const char* src, size_t len, void * dest) {
    if (len == 0) return 0;

#ifdef DOPAD
    /* if padding is used, then the message must be at least
       4 chars and be a multiple of 4.
       there can be at most 2 pad chars at the end */
    if (len < 4 || (len % 4 != 0)) return -1;
    if (src[len-1] == CHARPAD) {
        len--;
        if (src[len -1] == CHARPAD) {
            len--;
        }
    }
#endif  /* DOPAD */

    int i;
    int leftover = len % 4;
    int chunks = (leftover == 0) ? len / 4 - 1 : len /4;

    uint8* p = (uint8*) dest;
    uint32 x = 0;
    uint32* destInt = (uint32*) p;
    uint32* srcInt = (uint32*) src;
    uint32 y = *srcInt++;
    for (i = 0; i < chunks; ++i) {
        x = d0[y >> 24 & 0xff] | d1[y >> 16 & 0xff] |
            d2[y >> 8 & 0xff] | d3[y & 0xff];

        if (x >= BADCHAR)  return -1;
        *destInt = x << 8;
        p += 3;
        destInt = (uint32*)p;
        y = *srcInt++;
    }

    switch (leftover) {
    case 0:
        x = d0[y >> 24 & 0xff] | d1[y >> 16 & 0xff] |
            d2[y >>  8 & 0xff] | d3[y & 0xff];
        if (x >= BADCHAR)  return -1;
        *p++ = ((uint8*)&x)[1];
        *p++ = ((uint8*)&x)[2];
        *p = ((uint8*)&x)[3];
        return (chunks+1)*3;
#ifndef DOPAD
    case 1:  /* with padding this is an impossible case */
        x = d3[y >> 24];
        *p =  (uint8)x;
        break;
#endif
    case 2:
        x = d3[y >> 24] *64 + d3[(y >> 16) & 0xff];
        *p =  (uint8)(x >> 4);
        break;
    default:  /* case 3 */
        x = (d3[y >> 24] *64 + d3[(y >> 16) & 0xff])*64 +
            d3[(y >> 8) & 0xff];
        *p++ = (uint8) (x >> 10);
        *p = (uint8) (x >> 2);
        break;
    }

    if (x >= BADCHAR) return -1;
    return 3*chunks + (6*leftover)/8;
}

#else /* LITTLE  ENDIAN -- INTEL AND FRIENDS */
int DSL_CC base64_decode(const char* src, size_t len, void * dest) {
    if (len == 0) return 0;

#ifdef DOPAD
    /*
     * if padding is used, then the message must be at least
     * 4 chars and be a multiple of 4
     */
    if (len < 4 || (len % 4 != 0)) return -1; /* error */
    /* there can be at most 2 pad chars at the end */
    if (src[len-1] == CHARPAD) {
        len--;
        if (src[len -1] == CHARPAD) {
            len--;
        }
    }
#endif

    int i;
    int leftover = len % 4;
    int chunks = (leftover == 0) ? len / 4 - 1 : len /4;

    uint8* p = (uint8*) dest;
    uint32 x = 0;
    uint32* destInt = (uint32*) p;
    uint32* srcInt = (uint32*) src;
    uint32 y = *srcInt++;
    for (i = 0; i < chunks; ++i) {
        x = d0[y & 0xff] |
            d1[(y >> 8) & 0xff] |
            d2[(y >> 16) & 0xff] |
            d3[(y >> 24) & 0xff];

        if (x >= BADCHAR) return -1;
        *destInt = x ;
        p += 3;
        destInt = (uint32*)p;
        y = *srcInt++;}


    switch (leftover) {
    case 0:
        x = d0[y & 0xff] |
            d1[(y >> 8) & 0xff] |
            d2[(y >> 16) & 0xff] |
            d3[(y >> 24) & 0xff];

        if (x >= BADCHAR) return -1;
        *p++ =  ((uint8*)(&x))[0];
        *p++ =  ((uint8*)(&x))[1];
        *p =    ((uint8*)(&x))[2];
        return (chunks+1)*3;
        break;
#ifndef DOPAD
    case 1:  /* with padding this is an impossible case */
        x = d0[y & 0xff];
        *p = *((uint8*)(&x)); // i.e. first char/byte in int
        break;
#endif
    case 2: // * case 2, 1  output byte */
        x = d0[y & 0xff] | d1[y >> 8 & 0xff];
        *p = *((uint8*)(&x)); // i.e. first char
        break;
    default: /* case 3, 2 output bytes */
        x = d0[y & 0xff] |
            d1[y >> 8 & 0xff ] |
            d2[y >> 16 & 0xff];  /* 0x3c */
        *p++ =  ((uint8*)(&x))[0];
        *p =  ((uint8*)(&x))[1];
        break;
    }

    if (x >= BADCHAR) return -1;

    return 3*chunks + (6*leftover)/8;
}

#endif  /* if bigendian / else / endif */
