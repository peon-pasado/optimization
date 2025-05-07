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
 *  $Id: objective.c,v 1.12 2015/01/12 06:50:25 tanaka Rel $
 *  $Revision: 1.12 $
 *  $Date: 2015/01/12 06:50:25 $
 *  $Author: tanaka $
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "define.h"
#include "sips_common.h"
#include "memory.h"
#include "objective.h"

/*
 * objective_initialize(prob)
 *   initializes the job cost function.
 *   It is called only once after the instance is loaded from a data file.
 *
 */
void objective_initialize(sips *prob)
{
  int t;
  int i;
  int tard;
  cost_t f, fmin;

  for(i = 0; i < prob->n; i++) {
    for(t = 0; t < prob->job[i].p; t++) {
      prob->job[i].f[t] = LARGE_COST;
    }

    if(prob->cfunc != NULL) {
      fmin = LARGE_COST;
      for(t = prob->job[i].p; t <= prob->T; t++) {
	prob->job[i].f[t] = (prob->cfunc)(i, t);
	if(obj_greater(fmin, prob->job[i].f[t])) {
	  fmin = prob->job[i].f[t];
	}
      }
      for(t = prob->job[i].p; t <= prob->T; t++) {
	prob->job[i].f[t] -= fmin;
      }
    } else if(prob->cfuncl != NULL) {
      fmin = LARGE_COST;
      for(t = prob->job[i].p; t <= prob->T; t++) {
	prob->job[i].f[t] = (prob->cfuncl)(prob, i, t);
	if(obj_greater(fmin, prob->job[i].f[t])) {
	  fmin = prob->job[i].f[t];
	}
      }
      for(t = prob->job[i].p; t <= prob->T; t++) {
	prob->job[i].f[t] -= fmin;
      }
    } else {
      tard = prob->T - prob->job[i].d;
      fmin = prob->job[i].ew*(cost_t) max(-tard, 0)
	+ prob->job[i].tw*(cost_t) max(tard, 0);
      tard = prob->job[i].p - prob->job[i].d;
      f = prob->job[i].ew*(cost_t) max(-tard, 0)
	+ prob->job[i].tw*(cost_t) max(tard, 0);
      if(obj_greater(fmin, f)) {
	fmin = f;
      }
      if(obj_greater(fmin, ZERO_COST)) {
	fmin = ZERO_COST;
      }

      for(t = prob->job[i].p; t <= prob->T; t++, tard++) {
	prob->job[i].f[t]
	  = prob->job[i].ew*(cost_t) max(-tard, 0)
	  + prob->job[i].tw*(cost_t) max(tard, 0) - fmin;
      }
    }
    prob->off += fmin;
  }

  memset((void *) prob->job[prob->n].f, 0, (prob->T + 1)*sizeof(cost_t));
}

/*
 * objective(sol)
 *   sets and returns the objective function value of sol.
 *
 */
void objective(_solution_t *sol)
{
  int i;

  sol->f = ZERO_COST;
  for(i = 0; i < sol->n; i++) {
    sol->f += sol->job[i]->f[sol->c[i]];
  }
}
