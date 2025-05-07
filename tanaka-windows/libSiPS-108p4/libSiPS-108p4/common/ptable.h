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
 *  $Id: ptable.h,v 1.10 2013/05/28 13:27:26 tanaka Rel $
 *  $Revision: 1.10 $
 *  $Date: 2013/05/28 13:27:26 $
 *  $Author: tanaka $
 *
 */
#ifndef PTABLE_H
#define PTABLE_H
#include "define.h"
#include "sips_common.h"

#define prec_set_adj_tij(t, i, j)					\
  *(ptable->adj + ((t)*ptable->adj_per_t				\
		   + ((i)*ptable->n + (j))/my_char_bit))		\
  |= 1<<(((i)*ptable->n + j)%my_char_bit)
#define prec_get_adj_tij0(t, i, j)					\
  (*(ptable->adj + ((t)*ptable->adj_per_t				\
		    + ((i)*ptable->n + (j))/my_char_bit))		\
   & 1<<(((i)*ptable->n + (j))%my_char_bit))
#define prec_get_adj_tij1(t, i, j)					\
  ((i) < ptable->n							\
   && (*(ptable->adj + ((t)*ptable->adj_per_t				\
			+ ((i)*ptable->n + (j))/my_char_bit))		\
       & 1<<(((i)*ptable->n + (j))%my_char_bit)))
#define prec_get_adj_tij2(t, i, j)					\
  ((j) < ptable->n							\
   && (*(ptable->adj + ((t)*ptable->adj_per_t				\
			+ ((i)*ptable->n + (j))/my_char_bit))		\
       & 1<<(((i)*ptable->n + (j))%my_char_bit)))
#define prec_get_adj_tij(t, i, j)					\
  ((i) < ptable->n && (j) < ptable->n					\
   && (*(ptable->adj + ((t)*ptable->adj_per_t				\
			+ ((i)*ptable->n + (j))/my_char_bit))		\
       & 1<<(((i)*ptable->n + (j))%my_char_bit)))
#define prec_set_dom_ti(t, i)						\
  *(ptable->dom + ((t)*ptable->dom_per_t + (i)/my_char_bit))		\
  |= 1<<((i)%my_char_bit)
#define prec_rev_dom_ti(t, i)						\
  *(ptable->dom + ((t)*ptable->dom_per_t + (i)/my_char_bit))		\
  ^= 1<<((i)%my_char_bit)
#define prec_get_dom_ti(t, i)						\
  (*(ptable->dom + ((t)*ptable->dom_per_t + (i)/my_char_bit))		\
   & 1<<((i)%my_char_bit))
#ifndef SIPSI
#define prec_set_sd_t(t)						\
  *(ptable->sd + (t)/my_char_bit) |= 1<<((t)%my_char_bit)
#define prec_get_sd_t(t)						\
  (*(ptable->sd + (t)/my_char_bit) & 1<<((t)%my_char_bit))
#endif /* !SIPSI */

#define _set_inc(x, y) {				\
    inc[(x)][(y)] = 1;					\
    inc[(y)][(x)] = -1;					\
    ptable->pre_psum[(y)] += prob->sjob[(x)]->p;	\
    ptable->suc_psum[(x)] += prob->sjob[(y)]->p;	\
  }  

int ptable_initialize(sips *);
int ptable_initialize_adj(sips *, _ptable_t *);
_ptable_t *duplicate_ptable(sips *, _ptable_t *);
void ptable_free_adj(_ptable_t *);
void ptable_free(_ptable_t *);
_real prec_get_memory_in_MB(sips *);

#endif /* PTABLE_H */
