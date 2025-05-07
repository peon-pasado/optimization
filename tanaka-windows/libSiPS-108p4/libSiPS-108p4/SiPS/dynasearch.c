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
 *  $Id: dynasearch.c,v 1.10 2016/04/01 15:05:10 tanaka Rel $
 *  $Revision: 1.10 $
 *  $Date: 2016/04/01 15:05:10 $
 *  $Author: tanaka $
 *
 */

/*
 * [NOTE]
 *   Dynasearch and enhanced dynasearch were originally proposed in:
 *
 *     R.K. Congram, C.N. Potts and S.L. van de Velde. 2002.
 *     An iterated dynasearch algorithm for the single-machine
 *     total weighted tardiness scheduling problem,
 *     INFORMS Journal on Computing 14, 52/67.
 *
 *     A. Grosso, F. Della Croce and R. Tadei. 2004.
 *     An enhanced dynasearch neighborhood for the single-machine
 *     total weighted tardiness scheduling problem,
 *     Operations Research Letters 32, 68/72.
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "define.h"
#include "sips_common.h"
#include "dynasearch.h"
#include "memory.h"
#include "objective.h"
#include "sol.h"

typedef struct {
  int j;
  cost_t f;
  unsigned char ty;
} _dyna_dp_t;

static int _dynasearch_internal(sips *, _solution_t *, int, _job_t **,
				_dyna_dp_t *);
static int _edynasearch_internal(sips *, _solution_t *, int, _job_t **,
				 _dyna_dp_t *);

/*
 * Dynasearch(prob, sol, type)
 *   returns a solution obtained by the dynasearch/enhanced dynasearch.
 *      sol: solution
 *     type: dynasearch type
 *             0: dynasearch
 *             1: enhanced dynasearch
 *
 */
void Dynasearch(sips *prob, _solution_t *sol, unsigned char type)
{
  int i;
  cost_t f;
  _dyna_dp_t *dp;
  _job_t **job;

  dp = (_dyna_dp_t *) xmalloc((sol->n + 1)*sizeof(_dyna_dp_t));
  job = (_job_t **) xmalloc(sol->n*sizeof(_job_t *));

  f = sol->f;
  i = 0;
  if(type == 0) {
    while(1) {
      i = _dynasearch_internal(prob, sol, i, job, dp);
      if(obj_lesser_equal(f, sol->f)) {
	break;
      }
      f = sol->f;
    }
  } else {
    while(1) {
      i = _edynasearch_internal(prob, sol, i, job, dp);
      if(obj_lesser_equal(f, sol->f)) {
	break;
      }
      f = sol->f;
    }
  }

  xfree(job);
  xfree(dp);
}

/*
 * _dynasearch_internal(prob, sol, lw, job, dp)
 *   returns the best solution in the dynasearch neighborhood
 *   of the current solution.
 *      sol: best solution
 *       lw: first lw jobs are fixed in the current solution
 *      job: work space for temporary solution
 *       dp: work space for dynamic programming states
 *
 */
int _dynasearch_internal(sips *prob, _solution_t *sol, int lw, _job_t **job,
			 _dyna_dp_t *dp)
{
  int i, j, k;
  cost_t vv;
  int c;

  if(lw == 0) {
    dp[0].f = ZERO_COST;
    dp[0].j = -1;
  }

  for(i = lw + 1; i <= sol->n; i++) {
    dp[i].j = i - 1;
    dp[i].f = dp[i - 1].f + sol->job[i - 1]->f[sol->c[i - 1]];

    for(j = 0; j <= i - 2; j++) {
      if(j > 0) {
	c = sol->c[j - 1] + sol->job[i - 1]->p;
      } else {
	c = sol->job[i - 1]->p;;
      }
      vv = dp[j].f + sol->job[i - 1]->f[c];
      for(k = j + 1; k <= i - 2; k++) {
	c += sol->job[k]->p;
	vv += sol->job[k]->f[c];
      }
      c += sol->job[j]->p;
      vv += sol->job[j]->f[c];

      if(obj_lesser(vv, dp[i].f)) {
	dp[i].f = vv;
	dp[i].j = prob->n + j;
      }
    }
  }

  for(i = sol->n, j = sol->n - 1; i > 0;) {
    if(dp[i].j < prob->n) {
      job[j--] = sol->job[dp[i].j];
    } else {
      dp[i].j -= prob->n;
      job[j--] = sol->job[dp[i].j];
      for(k = i - 2; j > dp[i].j; j--, k--) {
	job[j] = sol->job[k];
      }
      job[j--] = sol->job[i - 1];
    }
    i = dp[i].j;
  }

  for(lw = 0; lw < sol->n; lw++) {
    if(job[lw]->no != sol->job[lw]->no) {
      break;
    }
  }

  memcpy((void *) sol->job, (void *) job, sol->n*sizeof(_job_t *));
  solution_set_c(prob, sol);

  return(lw);
}

/*
 * _edynasearch_internal(prob, sol, lw, job, dp)
 *   returns the best solution in the enhanced dynasearch neighborhood
 *   of the current solution.
 *      sol: best solution
 *       lw: first lw jobs are fixed in the current solution
 *      job: work space for temporary solution
 *       dp: work space for dynamic programming states
 *
 */
int _edynasearch_internal(sips *prob, _solution_t *sol, int lw, _job_t **job,
			  _dyna_dp_t *dp)
{
  int i, j, k;
  cost_t vv;
  int c;

  if(lw == 0) {
    dp[0].f = ZERO_COST;
    dp[0].j = -1;
    dp[0].ty = 0;
  }

  for(i = lw + 1; i <= sol->n; i++) {
    dp[i].j = i - 1;
    dp[i].f = dp[i - 1].f + sol->job[i - 1]->f[sol->c[i - 1]];
    dp[i].ty = 0;

    /* PI */
    for(j = 0; j <= i - 2; j++) {
      if(j > 0) {
	c = sol->c[j - 1] + sol->job[i - 1]->p;
      } else {
	c = sol->job[i - 1]->p;
      }
      vv = dp[j].f + sol->job[i - 1]->f[c];
      for(k = j + 1; k <= i - 2; k++) {
	c += sol->job[k]->p;
	vv += sol->job[k]->f[c];
      }
      c += sol->job[j]->p;
      vv += sol->job[j]->f[c];

      if(obj_lesser(vv, dp[i].f)) {
	dp[i].f = vv;
	dp[i].j = j;
	dp[i].ty = 1;
      }
    }

    /* EBSR */
    for(j = 0; j <= i - 3; j++) {
      if(j > 0) {
	c = sol->c[j - 1] + sol->job[i - 1]->p;
      } else {
	c = sol->job[i - 1]->p;
      }
      vv = dp[j].f + sol->job[i - 1]->f[c];
      for(k = j; k <= i - 2; k++) {
	c += sol->job[k]->p;
	vv += sol->job[k]->f[c];
      }

      if(obj_lesser(vv, dp[i].f)) {
	dp[i].f = vv;
	dp[i].j = j;
	dp[i].ty = 2;
      }
    }

    /* EFSR */
    for(j = 0; j <= i - 3; j++) {
      if(j > 0) {
	c = sol->c[j - 1];
      } else {
	c = 0;
      }
      vv = dp[j].f;

      for(k = j + 1; k <= i - 1; k++) {
	c += sol->job[k]->p;
	vv += sol->job[k]->f[c];
      }
      c += sol->job[j]->p;
      vv += sol->job[j]->f[c];

      if(obj_lesser(vv, dp[i].f)) {
	dp[i].f = vv;
	dp[i].j = j;
	dp[i].ty = 3;
      }
    }
  }

  for(i = sol->n, j = sol->n - 1; i > 0;) {
    switch(dp[i].ty) {
    case 0:
      job[j--] = sol->job[dp[i].j];
      break;
    case 1: /* PI */
      job[j--] = sol->job[dp[i].j];
      for(k = i - 2; j > dp[i].j; j--, k--) {
	job[j] = sol->job[k];
      }
      job[j--] = sol->job[i - 1];
      break;
    case 2: /* EBSR */
      for(k = i - 2; j > dp[i].j; j--, k--) {
	job[j] = sol->job[k];
      }
      job[j--] = sol->job[i - 1];
      break;
    case 3: /* EFSR */
      job[j--] = sol->job[dp[i].j];
      for(k = i - 1; j >= dp[i].j; j--, k--) {
	job[j] = sol->job[k];
      }
      break;
    }

    i = dp[i].j;
  }

  for(lw = 0; lw < sol->n; lw++) {
    if(job[lw]->no != sol->job[lw]->no) {
      break;
    }
  }

  memcpy((void *) sol->job, (void *) job, sol->n*sizeof(_job_t *));
  solution_set_c(prob, sol);

  return(lw);
}
