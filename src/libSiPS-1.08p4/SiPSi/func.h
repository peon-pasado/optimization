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
 *  $Id: func.h,v 1.9 2013/05/28 13:26:53 tanaka Rel $
 *  $Revision: 1.9 $
 *  $Date: 2013/05/28 13:26:53 $
 *  $Author: tanaka $
 *
 */
#ifndef FUNC_H
#define FUNC_H
#include <stdio.h>
#include "define.h"
#include "sips_common.h"

_point_t *duplicate_function(_benv_t *, _point_t *);
void free_function(_benv_t *, _point_t *);
_point_t *zero_function(_benv_t *, int);
_point_t *null_function(_benv_t *);

#ifdef COST_REAL
_point_t *_convert_function(sips *, _benv_t *, _point_t *);
_point_t *_rconvert_function(sips *, _benv_t *, _point_t **);

#define convert_function(x, y) _convert_function(prob, x, y)
#define rconvert_function(x, y) _rconvert_function(prob, x, y)
#else /* COST_REAL */
_point_t *_convert_function(_benv_t *, _point_t *);
_point_t *_rconvert_function(_benv_t *, _point_t **);

#define convert_function(x, y) _convert_function(x, y)
#define rconvert_function(x, y) _rconvert_function(x, y)
#endif /* COST_REAL */

_point_t *shift_function(_benv_t *, _point_t **, int, int);
_point_t *rshift_function(_benv_t *, _point_t **, int);
_point_t *duplicate_function_with_shift(_benv_t *, _point_t *, int, int);
_point_t *duplicate_function_with_tmax(_benv_t *, _point_t *, int);
_point_t *add_functions(_benv_t *, _point_t **, _point_t *);

#ifdef COST_REAL
_point_t *_minimum_of_functions(sips *, _benv_t *, _point_t **, _point_t *);
int _minimize_function(sips *, _point_t *, int, int *);
int _rminimize_function(sips *, _point_t *, int, int *);

#define minimum_of_functions(x, y, z) _minimum_of_functions(prob, x, y, z)
#define minimize_function(x, y, z) _minimize_function(prob, x, y, z)
#define rminimize_function(x, y, z) _rminimize_function(prob, x, y, z)
#else /* COST_REAL */
_point_t *_minimum_of_functions(_benv_t *, _point_t **, _point_t *);
int _minimize_function(_point_t *, int, int *);
int _rminimize_function(_point_t *, int, int *);

#define minimum_of_functions(x, y, z) _minimum_of_functions(x, y, z)
#define minimize_function(x, y, z) _minimize_function(x, y, z)
#define rminimize_function(x, y, z) _rminimize_function(x, y, z)
#endif /* COST_REAL */

int minimize_functions_when_added(_point_t *, _point_t *, cost_t *);
void set_function_state(_point_t *, int);
void print_function(_point_t *, FILE *);

#endif /* FUNC_H */
