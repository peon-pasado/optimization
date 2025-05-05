/*
 * Copyright 2006-2013 Shunji Tanaka.  All rights reserved.
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
 *  $Id: define.h,v 1.16 2015/01/12 06:50:35 tanaka Rel $
 *  $Revision: 1.16 $
 *  $Date: 2015/01/12 06:50:35 $
 *  $Author: tanaka $
 *
 */
#ifndef DEFINE_H
#define DEFINE_H
#include "sips_common.h"

#ifndef MAXBUFLEN
#define MAXBUFLEN        (4096)
#endif /* MAXBUFLEN */

#ifndef MAXNAMELEN
#define MAXNAMELEN       (64)
#endif /* MAXNAMELEN */

#ifndef LARGE_INTEGER
#define LARGE_INTEGER    (536870911)
#endif /* LARGE_INTEGER */

#ifndef LARGE_INTEGER2
#define LARGE_INTEGER2   (536870910)
#endif /* LARGE_INTEGER2 */

#ifdef REAL_LONG
#ifndef LARGE_REAL
#define LARGE_REAL     (1.0e+50)
#endif /* LARGE_REAL */

#ifndef LARGE_REAL2
#define LARGE_REAL2    (9.9e+49)
#endif /* LARGE_REAL2 */
#else /* REAL_LONG */
#ifndef LARGE_REAL
#define LARGE_REAL     (1.0e+40)
#endif /* LARGE_REAL */

#ifndef LARGE_REAL2
#define LARGE_REAL2    (9.9e+39)
#endif /* LARGE_REAL2 */
#endif /* REAL_LONG */

#ifndef DP_BLOCKSIZE
#define DP_BLOCKSIZE     (65536U) /* 64k */
#endif /* DP_BLOCKSIZE */

#ifndef MAX_DPSIZE
#ifdef  SIPSI
#define MAX_DPSIZE          (10)
#else /* SIPSI */
#define MAX_DPSIZE          (12)
#endif /* SIPSI */
#endif /* MAX_DPSIZE */

#ifndef my_char_bit
#define my_char_bit      (8U)
#endif /* my_char_bit */

#ifndef max
#define max(a, b) ((a)>(b)?(a):(b))
#endif /* max */

#ifndef min
#define min(a, b) ((a)<(b)?(a):(b))
#endif /* min */

#ifndef iswap
#define iswap(a, b) { int c; c = a; a = b; b = c; }
#endif /* iswap */

#ifndef dswap
#define dswap(a, b) { double c; c = a; a = b; b = c; }
#endif /* dswap */

#ifndef jswap
#define jswap(a, b) { _job_t c; c = a; a = b; b = c; }
#endif /* jswap */

#ifndef jpswap
#define jpswap(a,b) { _job_t *c; c = a; a = b; b = c; }
#endif /* jpswap */

#ifdef SIPSI
#define N_JOBS prob->tn
#else /* SIPSI */
#define N_JOBS prob->n
#endif /* SIPSI */

#ifdef REAL_LONG
#define print_real(fp, format, val) fprintf(fp, "%" #format "Lf", val);
#define print_real2(fp, val) fprintf(fp, "%Lg", val);
#else /* REAL_LONG */
#define print_real(fp, format, val) fprintf(fp, "%" #format "f", val);
#define print_real2(fp, val) fprintf(fp, "%g", val);
#endif /* REAL_LONG */
#ifdef COST_REAL
#define ctod(v) ((_real) v)
#define itoc(v) ((cost_t) (v))
#define ctoi(v) ((int) (v + prob->param->eps))
#define large(v) (v > LARGE_REAL2)
#define obj_lesser(a, b) ((b) - (a) > prob->param->eps)
#define obj_greater(a, b) ((a) - (b) > prob->param->eps)
#define obj_lesser_equal(a, b) ((a) - (b) <= prob->param->eps)
#define obj_greater_equal(a, b) ((b) - (a) <= prob->param->eps)
#define obj_equal(a, b)							\
  ((large(a) && large(b))						\
   || (obj_lesser_equal(a, b) && obj_greater_equal(a, b)))
#ifdef SIPSI
#define fobj_lesser(a, b) ((b) - (a) > prob->param->funceps)
#define fobj_greater(a, b) ((a) - (b) > prob->param->funceps)
#define fobj_lesser_equal(a, b) ((a) - (b) <= prob->param->funceps)
#define fobj_greater_equal(a, b) ((b) - (a) <= prob->param->funceps)
#define fobj_equal(a, b)						\
  ((large(a) && large(b))						\
   || (fobj_lesser_equal(a, b) && fobj_greater_equal(a, b)))
#endif /* SIPSI */
#define LARGE_COST LARGE_REAL
#define ZERO_COST 0.0
#define print_cost(fp, val) print_real(fp, .4, val)
#else /* COST_REAL */
#define ctod(v) ((_real) v)
#define itoc(v) ((cost_t) (v))
#define ctoi(v) ((int) (v))
#ifdef COST_LONGLONG
#define large(v) ((v) > 1152921504606846974LL)
#else /* COST_LONGLONG */
#define large(v) ((v) > LARGE_INTEGER2)
#endif /* COST_LONGLONG */
#define obj_lesser(a, b) (a < b)
#define obj_greater(a, b) (a > b)
#define obj_lesser_equal(a, b) (a <= b)
#define obj_greater_equal(a, b) (a >= b)
#define obj_equal(a, b) (a == b)
#ifdef SIPSI
#define fobj_lesser(a, b) obj_lesser(a, b)
#define fobj_greater(a, b) obj_greater(a, b)
#define fobj_lesser_equal(a, b) obj_lesser_equal(a, b)
#define fobj_greater_equal(a, b) obj_greater_equal(a, b)
#define fobj_equal(a, b) obj_equal(a, b)
#endif /* SIPSI */
#ifdef COST_LONGLONG
#define LARGE_COST (1152921504606846975LL)
#else /* COST_LONGLONG */
#define LARGE_COST LARGE_INTEGER
#endif /* COST_LONGLONG */
#define ZERO_COST 0
#ifdef COST_LONGLONG
#define print_cost(fp, val) fprintf(fp, "%Ld", val)
#else /* COST_LONGLONG */
#define print_cost(fp, val) fprintf(fp, "%d", val)
#endif /* COST_LONGLONG */
#endif /* COST_REAL */

#define is_true(x) (x == SIPS_TRUE)
#define not_true(x) (x != SIPS_TRUE)
#define is_false(x) (x == SIPS_FALSE)
#define not_false(x) (x != SIPS_FALSE)

#define print_current_objective(fp) print_cost(fp, prob->sol->f + prob->off)

#define null_check(x) { if(x == NULL) { return(SIPS_FAIL); } }

#endif /* DEFINE_H */
