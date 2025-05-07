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
 *  $Id: lag.h,v 1.10 2013/05/28 13:27:12 tanaka Rel $
 *  $Revision: 1.10 $
 *  $Date: 2013/05/28 13:27:12 $
 *  $Author: tanaka $
 *
 */
#ifndef LAG_H
#define LAG_H
#include "define.h"
#include "sips_common.h"

void lag_free(sips *);

int lag_initialize_LR1_solver(sips *);
void lag_free_LR1_solver(sips *);
int lag_initialize_LR2adj_solver(sips *);
void lag_free_LR2adj_solver(sips *);

_real lag_get_memory_in_MB(sips *);

int lag_solve_LR1(sips *, _real *, _real, _solution_t *, _real *,
		  unsigned int *);
void lag_solve_LR1_backward(sips *, _real *, _real);
int lag_solve_LR2adj(sips *, _real *, _real, _solution_t *, _real *,
		     unsigned int *);
int lag_solve_LR2adj_backward(sips *, _real *, _real);

#endif /* LAG_H */
