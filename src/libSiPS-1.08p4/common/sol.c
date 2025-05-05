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
 *  $Id: sol.c,v 1.9 2015/01/12 06:50:49 tanaka Rel $
 *  $Revision: 1.9 $
 *  $Date: 2015/01/12 06:50:49 $
 *  $Author: tanaka $
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "define.h"
#include "sips_common.h"
#ifdef SIPSI
#include "bmemory.h"
#include "func.h"
#endif /* SIPSI */
#include "memory.h"
#include "objective.h"
#include "sol.h"

/*
 * create_solution(prob)
 *   creates and returns a solution.
 *
 */
_solution_t *create_solution(sips *prob)
{
  _solution_t *sol;

  sol = (_solution_t *) xmalloc(sizeof(_solution_t));

  sol->n = 0;
  sol->off = prob->off;
  sol->f = ZERO_COST;
  sol->c = (int *) xcalloc(prob->T/prob->pmin, sizeof(int));
  sol->job = (_job_t **) xcalloc(prob->T/prob->pmin, sizeof(_job_t *));

  return(sol);
}

/*
 * copy_solution(prob, dest, src)
 *   copies the solution.
 *     dest: destination
 *      src: source
 *
 */
void copy_solution(sips *prob, _solution_t *dest, _solution_t *src)
{
  if(src == NULL || dest == NULL) {
    return;
  }

  dest->f = src->f;
  dest->n = src->n;
  memcpy((void *) dest->c, (void *) src->c, src->n*sizeof(int));
  memcpy((void *) dest->job, (void *) src->job, src->n*sizeof(_job_t *));
}

/*
 * reverse_solution(sol)
 *   reverses the order of the solution.
 *      sol: solution
 *
 */
void reverse_solution(_solution_t *sol)
{
  int c;
  int i, j;
  _job_t *job;

  for(i = 0, j = sol->n - 1; i < j; i++, j--) {
    c = sol->c[i];
    sol->c[i] = sol->c[j];
    sol->c[j] = c;

    job = sol->job[i];
    sol->job[i] = sol->job[j];
    sol->job[j] = job;
  }
}

/*
 * free_solution(sol)
 *   frees the solution.
 *      sol: solution
 *
 */
void free_solution(_solution_t *sol)
{
  if(sol != NULL) {
    xfree(sol->c);
    xfree(sol->job);
    xfree(sol);
  }
}

/*
 * solution_set_c(prob, sol)
 *   sets the job completion times and returns the objective function value.
 *      sol: solution
 *
 */
void solution_set_c(sips *prob, _solution_t *sol)
{
  int i;

#ifdef SIPSI
  if(prob->param->ptype & SIPS_PROB_IDLETIME) {
    insert_idle_time(prob, sol);
    return;
  }
#endif /* SIPSI */
  if(sol->n > 0) {
    sol->c[0] = sol->job[0]->p;
    for(i = 1; i < sol->n; i++) {
      sol->c[i] = sol->c[i - 1] + sol->job[i]->p;
    }
    objective(sol);
  }
}

#ifdef SIPSI
/*
 * insert_idle_time(prob, sol)
 *   inserts idle time optimally.
 *       sol: solution
 *
 */
void insert_idle_time(sips *prob, _solution_t *sol)
{
  int i;
  int t;
  unsigned char flag;
  _point_t **fp;
  _benv_t *benv;

  if(sol->n == 0) {
    return;
  }

  benv = create_benv(sizeof(_point_t));

  fp = (_point_t **) xmalloc(sol->n*sizeof(_point_t *));
  fp[0] = duplicate_function(benv, sol->job[0]->func);
  convert_function(benv, fp[0]);
  for(i = 1; i < sol->n; i++) {
    fp[i] = duplicate_function_with_shift(benv, fp[i - 1], prob->T,
					  sol->job[i]->p);
    add_functions(benv, &(fp[i]), sol->job[i]->func);
    convert_function(benv, fp[i]);
  }

  flag = SIPS_TRUE;
  t = prob->T;
  for(i = sol->n - 1; i >= 0; i--) {
    t = minimize_function(fp[i], t, NULL);
    if(t < 0) {
      flag = SIPS_FALSE;
      break;
    }
    sol->c[i] = t;
    t -= sol->job[i]->p;
  }

  xfree(fp);
  free_benv(benv);

  sol->f = LARGE_COST;
  if(is_true(flag)) {
    objective(sol);
  }
}
#endif /* SIPSI */
