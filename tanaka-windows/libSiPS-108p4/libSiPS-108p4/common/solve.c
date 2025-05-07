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
 *  $Id: solve.c,v 1.21 2015/01/12 06:50:51 tanaka Rel $
 *  $Revision: 1.21 $
 *  $Date: 2015/01/12 06:50:51 $
 *  $Author: tanaka $
 *
 */
#include <stdio.h>

#include "define.h"
#include "sips_common.h"
#include "memory.h"
#include "prob_common.h"
#include "problem.h"
#include "sol.h"
#include "ssdp.h"
#include "timer.h"

/*
 * SiPS_solve(prob)
 *   solve the problem.
 *
 */
int SiPS_solve(sips *prob)
{
  int i;
  int ret;

  if((ret = problem_check_solvability(prob)) != SIPS_OK) {
    return(ret);
  }

  prob->time = 0.0;
  _timer_start(prob);
  if(problem_set_probdata(prob) == SIPS_FAIL) {
    problem_free_objective(prob);
    problem_free_sjob(prob);
    return(SIPS_FAIL);
  }
  free_solution(prob->sol);
  prob->sol = create_solution(prob);
  problem_create_graph(prob);
  ret = ssdp(prob);
  problem_free_graph(prob);
  problem_free_objective(prob);
  problem_free_sjob(prob);
  for(i = 0; i < prob->n; i++) {
    prob->sol->job[i]->sno = i;
  }
  _get_time(prob);

  if(prob->param->verbose >= 2) {
    if(ret != SIPS_SOLVED) {
      fprintf(stdout, "    Best solution: ");
    } else {
      fprintf(stdout, " Optimal solution: ");
    }
    print_current_objective(stdout);
    fprintf(stdout, "\n");
    fprintf(stdout, "       Total time: ");
    _print_time(prob, stdout);
    fflush(stdout);
  } else if(prob->param->verbose >= 1) {
    if(ret != SIPS_SOLVED) {
      fprintf(stdout, "BEST=");
    } else {
      fprintf(stdout, "OPT=");
    }
    print_current_objective(stdout);
    fprintf(stdout, " time=");
    _print_time(prob, stdout);
    fflush(stdout);
  }

  return(ret);
}

/*
 * SiPS_set_initial_secuence(prob, n, job)
 *   specifies the initial job (partial) sequence
 *        n: number of jobs specified
 *      job: job sequence
 *
 */
int SiPS_set_initial_sequence(sips *prob, int n, int *job)
{
  int i;

  null_check(prob);

  if(n <= 0) {
    xfree(prob->iseq);
    prob->iseq = NULL;
    prob->niseq = 0;
    return(SIPS_OK);
  }

  null_check(job);

  prob->iseq = (int *) xrealloc((void *) prob->iseq, n*sizeof(int));
  for(i = 0; i < n; i++) {
    prob->iseq[i] = job[i];
  }
  prob->niseq = n;

  return(SIPS_OK);
}

/*
 * SiPS_get_solution(prob, f, job, c)
 *   retrieves the obtained solution
 *        f: objective function value
 *      job: job sequence
 *        c: completion times
 *
 */
int SiPS_get_solution(sips *prob, cost_t *f, int *job, int *c)
{
  int i;

  null_check(prob);

  null_check(prob->sol);

  if(f != NULL) {
    *f = prob->sol->f + prob->off;
  }

  if(job != NULL) {
    for(i = 0; i < prob->sol->n; i++) {
      job[i] = prob->sol->job[i]->rno;
    }
  }

  if(c != NULL) {
    for(i = 0; i < prob->sol->n; i++) {
      c[i] = prob->sol->c[i] + prob->Toff;
    }
  }

  return(SIPS_OK);
}

/*
 * SiPS_get_cputime(prob, time)
 *   retrieves CPU time
 *     time: CPU time in seconds
 *
 */
int SiPS_get_cputime(sips *prob, double *t)
{
  null_check(prob);

  null_check(prob->sol);

  *t = prob->time;

  return(SIPS_OK);
}
