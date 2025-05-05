/*
 * Copyright 2006-2015 Shunji Tanaka.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  $Id: default.h,v 1.26 2015/01/12 06:50:28 tanaka Rel $
 *  $Revision: 1.26 $
 *  $Date: 2015/01/12 06:50:28 $
 *  $Author: tanaka $
 *
 */
#ifndef DEFAULT_H
#define DEFAULT_H
#include "sips_common.h"

#ifndef DEFAULT_EPS
#define DEFAULT_EPS         (1.0e-5)
#endif /* DEFAULT_EPS */

#ifndef DEFAULT_FUNCEPS
#define DEFAULT_FUNCEPS     (1.0e-8)
#endif /* DEFAULT_FUNCEPS */

#ifndef DEFAULT_VERBOSE
#define DEFAULT_VERBOSE     (0)
#endif /* DEFAULT_VERBOSE */

#ifndef DEFAULT_LB
#define DEFAULT_LB          SIPS_FALSE
#endif /* DEFAULT_LB */

#ifndef DEFAULT_UB
#define DEFAULT_UB          SIPS_FALSE
#endif /* DEFAULT_UB */

#ifndef DEFAULT_WARMSTART
#define DEFAULT_WARMSTART   SIPS_FALSE
#endif /* DEFAULT_WARMSTART */

#ifndef DEFAULT_MAXMEM
#define DEFAULT_MAXMEM      (1536)
#endif /* DEFAULT_MAXMEM */

#ifndef DEFAULT_TLIMIT
#define DEFAULT_TLIMIT      (-1.0)
#endif /* DEFAULT_TLIMIT */

#ifndef DEFAULT_DUMMY
#define DEFAULT_DUMMY       (0)
#endif /* DEFAULT_DUMMY */

#ifndef DEFAULT_DOMLEVEL
#define DEFAULT_DOMLEVEL    (3)
#endif /* DEFAULT_DOMLEVEL */

#ifndef DEFAULT_TIEBREAK
#define DEFAULT_TIEBREAK    SIPS_TIEBREAK_EDD
#endif /* DEFAULT_TIEBREAK */

#ifndef DEFAULT_LS
#define DEFAULT_LS          SIPS_LS_COMBINED_A
#endif /* DEFAULT_LS */

#ifndef DEFAULT_DPSIZE
#define DEFAULT_DPSIZE      (8)
#endif /* DEFAULT_DPSIZE */

#ifndef DEFAULT_SKIP1
#define DEFAULT_SKIP1       SIPS_FALSE
#endif /* DEFAULT_SKIP1 */

#ifndef DEFAULT_U0
#define DEFAULT_U0          (0.0)
#endif /* DEFAULT_U0 */

#ifndef DEFAULT_INITSTEP1
#define DEFAULT_INITSTEP1   (1.0)
#endif /* DEFAULT_INITSTEP1 */

#ifndef DEFAULT_INITTERMITER1
#define DEFAULT_INITTERMITER1 (0.75)
#endif /* DEFAULT_INITTERMITER1 */

#ifndef DEFAULT_MAXSTEP1
#define DEFAULT_MAXSTEP1    (1.0)
#endif /* DEFAULT_MAXSTEP1 */

#ifndef DEFAULT_TERMITER1
#define DEFAULT_TERMITER1   (1.0)
#endif /* DEFAULT_TERMITER1 */

#ifndef DEFAULT_SHRINKITER1
#define DEFAULT_SHRINKITER1 (2)
#endif /* DEFAULT_SHRINKITER1 */

#ifndef DEFAULT_SHRINK1
#define DEFAULT_SHRINK1     (0.95)
#endif /* DEFAULT_SHRINK1 */

#ifndef DEFAULT_EXPAND1
#define DEFAULT_EXPAND1     (1.15)
#endif /* DEFAULT_EXPAND1 */

#ifndef DEFAULT_TERMRATIO1
#define DEFAULT_TERMRATIO1  (0.02)
#endif /* DEFAULT_TERMRATIO1 */

#ifndef DEFAULT_INITSTEP2
#define DEFAULT_INITSTEP2   (1.0)
#endif /* DEFAULT_INITSTEP2 */

#ifndef DEFAULT_INITTERMITER2
#define DEFAULT_INITTERMITER2 (0.5)
#endif /* DEFAULT_INITTERMITER2 */

#ifndef DEFAULT_MAXSTEP2
#define DEFAULT_MAXSTEP2    (1.0)
#endif /* DEFAULT_MAXSTEP2 */

#ifndef DEFAULT_TERMITER2
#define DEFAULT_TERMITER2   (1.0)
#endif /* DEFAULT_TERMITER2 */

#ifndef DEFAULT_SHRINKITER2
#define DEFAULT_SHRINKITER2 (2)
#endif /* DEFAULT_SHRINKITER2 */

#ifndef DEFAULT_SHRINK2
#define DEFAULT_SHRINK2     (0.95)
#endif /* DEFAULT_SHRINK2 */

#ifndef DEFAULT_EXPAND2
#define DEFAULT_EXPAND2     (1.2)
#endif /* DEFAULT_EXPAND2 */

#ifndef DEFAULT_TERMRATIO2
#define DEFAULT_TERMRATIO2  (0.002)
#endif /* DEFAULT_TERMRATIO2 */

#ifndef DEFAULT_UBITERATION
#define DEFAULT_UBITERATION (50) /* 20*/
#endif /* DEFAULT_UBITERATION */

#ifndef DEFAULT_UBUPDATE1
#define DEFAULT_UBUPDATE1   SIPS_FALSE
#endif /* DEFAULT_UBUPDATE1 */

#ifndef DEFAULT_MINUPDATE
#define DEFAULT_MINUPDATE   (10)
#endif /* DEFAULT_MINUPDATE */

#ifndef DEFAULT_SECSW
#define DEFAULT_SECSW       (6)
#endif /* DEFAULT_SECSW */

#ifndef DEFAULT_SECINIRATIO
#define DEFAULT_SECINIRATIO (-1.0)
#endif /* DEFAULT_SECINIRATIO */

#ifndef DEFAULT_SECRATIO
#define DEFAULT_SECRATIO    (0.0)
#endif /* DEFAULT_SECRATIO */

#ifndef DEFAULT_BISECTION
#define DEFAULT_BISECTION   (0)
#endif /* DEFAULT_BISECTION */

#ifndef DEFAULT_MODSWITCH
#define DEFAULT_MODSWITCH   (0)
#endif /* DEFAULT_MODSWITCH */

#ifndef DEFAULT_MOD
#define DEFAULT_MOD         (3)
#endif /* DEFAULT_MOD */

#ifndef DEFAULT_ZMPLIER
#define DEFAULT_ZMPLIER     SIPS_FALSE
#endif /* DEFAULT_ZMPLIER */

#ifndef DEFAULT_UBUPDATE3
#define DEFAULT_UBUPDATE3   SIPS_FALSE
#endif /* DEFAULT_UBUPDATE3 */

#endif /* DEFAULT_H */
