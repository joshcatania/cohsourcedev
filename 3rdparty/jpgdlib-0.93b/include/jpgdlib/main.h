//------------------------------------------------------------------------------
// main.h
// Last updated: Nov. 16, 2000
// Copyright (C) 1994-2000 Rich Geldreich
// richgel@voicenet.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//------------------------------------------------------------------------------

#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <assert.h>

#if !defined(JPGD_USE_STDINT_H)
typedef   signed char  schar;       /*  8 bits     */
typedef unsigned char  uchar;       /*  8 bits     */
typedef   signed short int16;       /* 16 bits     */
typedef unsigned short uint16;      /* 16 bits     */
typedef unsigned short ushort;      /* 16 bits     */
typedef unsigned int   uint;        /* 16/32+ bits */
typedef unsigned long  ulong;       /* 32 bits     */
typedef   signed int   int32;       /* 32+ bits    */
#else
#include <stdint.h>
typedef int_fast8_t    schar;       /*  8 bits     */
typedef uint_fast8_t   uchar;       /*  8 bits     */
typedef int_fast16_t   int16;       /* 16 bits     */
typedef uint_fast16_t  uint16;      /* 16 bits     */
typedef uint_fast32_t  ulong;       /* 32 bits     */
typedef int_fast32_t   int32;       /* 32+ bits    */
#endif

#ifndef max
#define max(a,b) (((a)>(b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a)<(b)) ? (a) : (b))
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#endif

