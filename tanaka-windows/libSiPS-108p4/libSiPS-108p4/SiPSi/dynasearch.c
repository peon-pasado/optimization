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
 *  $Id: dynasearch.c,v 1.16 2016/04/01 15:05:14 tanaka Rel $
 *  $Revision: 1.16 $
 *  $Date: 2016/04/01 15:05:14 $
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
 *   Their extension to those problems with idle time was originally
 *   proposed in:
 *
 *     F. Sourd. 2006.
 *     Dynasearch for the Earliness-Tardiness Scheduling Problem
 *     with Release Dates and Setup Constraints,
 *     Operations Research Letters 34, 591/598.
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "define.h"
#include "sips_common.h"
#include "bmemory.h"
#include "dynasearch.h"
#include "func.h"
#include "memory.h"
#include "objective.h"
#include "print.h"
#include "sol.h"

static int _dynasearch_internal(sips *, _solution_t *, int, _benv_t *,
				_job_t **, _point_t **);
static int _edynasearch_internal(sips *, _solution_t *, int, _benv_t *,
				 _job_t **, _point_t **);

/*
 * Dynasearch(prob, sol, type)
 *   returns a solution obtained by the (enhanced) dynasearch.
 *      sol: solution
 *     type: type of dynasearch
 *             0: original
 *             1: enhanced
 *
 */
void Dynasearch(sips *prob, _solution_t *sol, unsigned char type)
{
  int i;
  cost_t f;
  _benv_t *benv;
  _point_t **func;
  _job_t **job;

  benv = create_benv(sizeof(_point_t));
  job = (_job_t **) xmalloc(prob->n*sizeof(_job_t *));
  func = (_point_t **) xcalloc(sol->n + 1, sizeof(_point_t *));

  f = sol->f;
  i = 0;
  while(1) {
    if(type == 1) {
      i = _edynasearch_internal(prob, sol, i, benv, job, func);
    } else {
      i = _dynasearch_internal(prob, sol, i, benv, job, func);
    }
    if(obj_lesser_equal(f, sol->f)) {
      break;
    }
    f = sol->f;
  }

  xfree(func);
  xfree(job);
  free_benv(benv);

  insert_idle_time(prob, sol);
}

/*
 * _dynasearch_internal(prob, sol, lw, benv, job, func)
 *   returns the best solution in the dynasearch neighborhood
 *   of the current solution.
 *      sol: best solution
 *       lw: first lw jobs are fixed in the current solution
 *     benv: block memory environment
 *      job: work space for temporary solution
 *     func: work space for dynamic programming states
 *
 */
int _dynasearch_internal(sips *prob, _solution_t *sol, int lw, _benv_t *benv,
			 _job_t **job, _point_t **func)
{
  int i, j, k;
  int t;
  int s;
  _point_t *fp;

  if(lw == 0) {
    free_function(benv, func[0]);
    func[0] = zero_function(benv, prob->T);
  }

  for(i = lw + 1; i <= sol->n; i++) {
    free_function(benv, func[i]);
    func[i] = duplicate_function_with_shift(benv, func[i - 1], prob->T,
					    sol->job[i - 1]->p);
    add_functions(benv, &(func[i]), sol->job[i - 1]->func);
    convert_function(benv, func[i]);
    set_function_state(func[i], i - 1);

    for(j = 0; j <= i - 2; j++) {
      fp = duplicate_function_with_shift(benv, func[j], prob->T,
					 sol->job[i - 1]->p);
      add_functions(benv, &fp, sol->job[i - 1]->func);
      convert_function(benv, fp);
      for(k = j + 1; k <= i - 2; k++) {
	shift_function(benv, &fp, prob->T, sol->job[k]->p);
	add_functions(benv, &fp, sol->job[k]->func);
	convert_function(benv, fp);
      }

      shift_function(benv, &fp, prob->T, sol->job[j]->p);
      add_functions(benv, &fp, sol->job[j]->func);
      convert_function(benv, fp);
      set_function_state(fp, j);
      minimum_of_functions(benv, &(func[i]), fp);
      free_function(benv, fp);
    }
  }

  if(func[sol->n] == NULL) {
    sol->f = LARGE_COST;
    return(sol->n);
  }

  if(sol->f == func[sol->n]->prev->v) {
    return(sol->n);
  }

  t = prob->T;
  s = 0;
  for(i = sol->n; i >= 1; i--) {
    t = minimize_function(func[i], t, &s);
    if(t < 0) {
      sol->f = LARGE_COST;
      return(sol->n);
    }

    if(s == i - 1 || i == 1) {
      job[i - 1] = sol->job[i - 1];
      t -= sol->job[i - 1]->p;
    } else {
      job[i - 1] = sol->job[s];
      fp = duplicate_function_with_tmax(benv, sol->job[s]->func, t);
      rshift_function(benv, &fp, sol->job[s]->p);
      rconvert_function(benv, &fp);

      for(j = i - 2; j > s; j--) {
	job[j] = sol->job[j];
	add_functions(benv, &fp, sol->job[j]->func);
	rshift_function(benv, &fp, sol->job[j]->p);
	rconvert_function(benv, &fp);
      }
      job[s] = sol->job[i - 1];
      add_functions(benv, &fp, sol->job[i - 1]->func);
      rshift_function(benv, &fp, sol->job[i - 1]->p);
      rconvert_function(benv, &fp);
      t = minimize_functions_when_added(func[s], fp, NULL);
      free_function(benv, fp);
      i = s + 1;
    }
  }

  for(lw = 0; lw < sol->n && sol->job[lw]->no == job[lw]->no; lw++);

  sol->f = func[sol->n]->prev->v;
  memcpy((void *) sol->job, (void *) job, sol->n*sizeof(_job_t *));

  return(lw);
}

/*
 * _edynasearch_internal(prob, sol, lw, benv, job, func)
 *   returns the best solution in the enhanced dynasearch neighborhood
 *   of the current solution.
 *      sol: best solution
 *       lw: first lw jobs are fixed in the current solution
 *     benv: block memory environment
 *      job: work space for temporary solution
 *     func: work space for dynamic programming states
 *
 */
int _edynasearch_internal(sips *prob, _solution_t *sol, int lw, _benv_t *benv,
			  _job_t **job, _point_t **func)
{
  int i, j, k;
  int t;
  int s;
  _point_t *fp, *fp2;

  if(lw == 0) {
    free_function(benv, func[0]);
    func[0] = zero_function(benv, prob->T);
  }

  for(i = lw + 1; i <= sol->n; i++) {
    free_function(benv, func[i]);
    func[i] = duplicate_function_with_shift(benv, func[i - 1], prob->T,
					    sol->job[i - 1]->p);
    add_functions(benv, &(func[i]), sol->job[i - 1]->func);
    convert_function(benv, func[i]);
    set_function_state(func[i], i - 1);

    for(j = 0; j <= i - 2; j++) {
      fp = duplicate_function_with_shift(benv, func[j], prob->T,
					 sol->job[i - 1]->p);
      add_functions(benv, &fp, sol->job[i - 1]->func);
      convert_function(benv, fp);

      if(j <= i - 3) {
	/* EBSR */
	fp2 = duplicate_function(benv, fp);
	for(k = j; k <= i - 2; k++) {
	  shift_function(benv, &fp2, prob->T, sol->job[k]->p);
	  add_functions(benv, &fp2, sol->job[k]->func);
	  convert_function(benv, fp2);
	}

	set_function_state(fp2, j + prob->n);
	minimum_of_functions(benv, &(func[i]), fp2);
	free_function(benv, fp2);
      }

      /* PI */
      for(k = j + 1; k <= i - 2; k++) {
	shift_function(benv, &fp, prob->T, sol->job[k]->p);
	add_functions(benv, &fp, sol->job[k]->func);
	convert_function(benv, fp);
      }

      shift_function(benv, &fp, prob->T, sol->job[j]->p);
      add_functions(benv, &fp, sol->job[j]->func);
      convert_function(benv, fp);
      set_function_state(fp, j);
      minimum_of_functions(benv, &(func[i]), fp);
      free_function(benv, fp);
    }

    for(j = 0; j <= i - 3; j++) {
      /* EFSR */
      fp = duplicate_function_with_shift(benv, func[j], prob->T,
					 sol->job[j + 1]->p);
      add_functions(benv, &fp, sol->job[j + 1]->func);
      convert_function(benv, fp);
      for(k = j + 2; k <= i - 1; k++) {
	shift_function(benv, &fp, prob->T, sol->job[k]->p);
	add_functions(benv, &fp, sol->job[k]->func);
	convert_function(benv, fp);
      }

      shift_function(benv, &fp, prob->T, sol->job[j]->p);
      add_functions(benv, &fp, sol->job[j]->func);
      convert_function(benv, fp);
      set_function_state(fp, j + 2*prob->n);
      minimum_of_functions(benv, &(func[i]), fp);
      free_function(benv, fp);
    }
  }

  if(sol->f == func[sol->n]->prev->v) {
    return(sol->n);
  }

  t = prob->T;
  s = 0;
  for(i = sol->n; i >= 1; i--) {
    t = minimize_function(func[i], t, &s);
    if(t < 0) {
      sol->f = LARGE_COST;
      return(sol->n);
    }

    if(s == i - 1 || i == 1) {
      job[i - 1] = sol->job[i - 1];
      t -= sol->job[i - 1]->p;
    } else {
      if(s >= 2*prob->n) { /* EFSR */
	s -= 2*prob->n;
	job[i - 1] = sol->job[s];
	fp = duplicate_function_with_tmax(benv, sol->job[s]->func, t);
	rshift_function(benv, &fp, sol->job[s]->p);
	rconvert_function(benv, &fp);

	for(j = i - 2; j >= s; j--) {
	  job[j] = sol->job[j + 1];
	  add_functions(benv, &fp, sol->job[j + 1]->func);
	  rshift_function(benv, &fp, sol->job[j + 1]->p);
	  rconvert_function(benv, &fp);
	}
      } else if(s >= prob->n) { /* EBSR */
	s -= prob->n;
	job[i - 1] = sol->job[i - 2];
	fp = duplicate_function_with_tmax(benv, sol->job[i - 2]->func, t);
	rshift_function(benv, &fp, sol->job[i - 2]->p);
	rconvert_function(benv, &fp);

	for(j = i - 2; j > s; j--) {
	  job[j] = sol->job[j - 1];
	  add_functions(benv, &fp, sol->job[j - 1]->func);
	  rshift_function(benv, &fp, sol->job[j - 1]->p);
	  rconvert_function(benv, &fp);
	}
	job[s] = sol->job[i - 1];
	add_functions(benv, &fp, sol->job[i - 1]->func);
	rshift_function(benv, &fp, sol->job[i - 1]->p);
	rconvert_function(benv, &fp);
      } else { /* PI */
	job[i - 1] = sol->job[s];
	fp = duplicate_function_with_tmax(benv, sol->job[s]->func, t);
	rshift_function(benv, &fp, sol->job[s]->p);
	rconvert_function(benv, &fp);

	for(j = i - 2; j > s; j--) {
	  job[j] = sol->job[j];
	  add_functions(benv, &fp, sol->job[j]->func);
	  rshift_function(benv, &fp, sol->job[j]->p);
	  rconvert_function(benv, &fp);
	}
	job[s] = sol->job[i - 1];
	add_functions(benv, &fp, sol->job[i - 1]->func);
	rshift_function(benv, &fp, sol->job[i - 1]->p);
	rconvert_function(benv, &fp);
      }

      t = minimize_functions_when_added(func[s], fp, NULL);
      free_function(benv, fp);
      i = s + 1;
    }
  }

  for(lw = 0; lw < sol->n && sol->job[lw]->no == job[lw]->no; lw++);

  sol->f = func[sol->n]->prev->v;
  memcpy((void *) sol->job, (void *) job, sol->n*sizeof(_job_t *));

  return(lw);
}
