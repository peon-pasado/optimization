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
 *  $Id: heuristics.c,v 1.13 2015/01/12 06:50:30 tanaka Rel $
 *  $Revision: 1.13 $
 *  $Date: 2015/01/12 06:50:30 $
 *  $Author: tanaka $
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "define.h"
#include "sips_common.h"
#include "bmemory.h"
#include "func.h"
#include "heuristics.h"
#include "job_sort.h"
#include "memory.h"
#include "print.h"
#include "objective.h"
#include "sol.h"

static void _insert_jobs_optimally(sips *, _solution_t *, _solution_t *,
				   _solution_t *);
static int _count_bit(int);

/*
 * spt(prob, sol)
 *   returns an SPT sequence.
 *      sol: solution
 *
 */
void spt(sips *prob, _solution_t *sol)
{
  int i;

  for(i = 0; i < prob->n; i++) {
    sol->job[i] = &(prob->job[i]);
  }
  sol->n = prob->n;
  sort_jobs(prob->n, sol->job, 1);

  solution_set_c(prob, sol);
}

/*
 * edd(prob, sol)
 *   returns an EDD sequence.
 *      sol: solution
 *
 */
void edd(sips *prob, _solution_t *sol)
{
  int i;

  for(i = 0; i < prob->n; i++) {
    sol->job[i] = &(prob->job[i]);
  }
  sol->n = prob->n;
  sort_jobs(prob->n, sol->job, 0);

  solution_set_c(prob, sol);
}

/*
 * partialdp(prob, u, sol)
 *   constructs a feasible solution from a solution of a relaxation.
 *   A solution is constructed so as not to break the precedence
 *   relations in the partial solution.
 *   If the number of jobs to be inserted is small, the rest of the jobs
 *   are inserted optimally, while greedily if it is large.
 *        u: Lagrangian multipliers
 *      sol: solution
 *
 */
void partialdp(sips *prob, _real *u, _solution_t *sol)
{
  int i;
  int *e;
  _solution_t *psol, *isol;

  e = (int *) xcalloc(prob->n, sizeof(int));
  psol = create_solution(prob);
  isol = create_solution(prob);

  for(i = 0; i < sol->n; i++) {
    e[sol->job[i]->no]++;
  }
  if(prob->graph->hdir == SIPS_FORWARD) { /* forward */
    for(i = 0; i < sol->n; i++) {
      if(e[sol->job[i]->no] >= 1) {
	psol->job[psol->n++] = sol->job[i];
	e[sol->job[i]->no] = -1;
      }
    }
  } else { /* backward */
    for(i = sol->n - 1; i >= 0; i--) {
      if(e[sol->job[i]->no] >= 1) {
	psol->job[psol->n++] = sol->job[i];
	e[sol->job[i]->no] = -1;
      }
    }
    reverse_solution(psol);
  }

  for(i = 0; i < prob->n; i++) {
    if(e[i] != -1) {
      isol->job[isol->n++] = prob->sjob[i];
    }
  }
  xfree(e);

  if(isol->n > prob->param->dpsize) {
#ifdef DONT_USE_GREEDY
    sol->f = LARGE_COST;
    sol->n = 0;
#else
    copy_solution(prob, sol, psol);
    insert_ordered_jobs_greedily(prob, sol, isol->n, isol->job);
#endif
  } else if(isol->n > 0) {
    _insert_jobs_optimally(prob, sol, psol, isol);
  } else {
    copy_solution(prob, sol, psol);
    solution_set_c(prob, sol);
  }

  free_solution(isol);
  free_solution(psol);

  prob->graph->hdir = 1 - prob->graph->hdir; /* reverse the direction */

  return;
}

/*
 * _insert_jobs_optimally(sips, sol, psol, isol)
 *   inserts the jobs in isol optimally with the processing order of psol
 *   kept unchanged.  This algorithm was originally proposed in
 *   
 *     T. Ibaraki and Y. Nakamura:
 *     A Dynamic Programming Method for Single Machine Scheduling,
 *     European Journal of Operational Research,
 *     Vol. 76, pp. 72-82 (1994).
 *
 *       sol: solution
 *      psol: partial processing order
 *      isol: jobs to be inserted
 *
 *
 */
void _insert_jobs_optimally(sips *prob, _solution_t *sol, _solution_t *psol,
			    _solution_t *isol)
{
  int t;
  int i, j, k, l, m;
  int s;
  int nsize;
  unsigned char flag;
  int *nn;
  _benv_t *benv;
  _point_t ***dp, **cdp, *fp;

  nsize = 1 << isol->n;

  nn = (int *) xmalloc(nsize*sizeof(int));

  m = 0;
  for(i = 1; i < nsize; i++) {
    nn[i] = _count_bit(i);

    j = i ^ (i - 1);
    k = 1 << (isol->n - 1);
    for(l = 0; (k & j) == 0; l++, k >>= 1);
    j = isol->n - l - 1;
    /* m: Gray code */
    m ^= k;
  }

  benv = create_benv(sizeof(_point_t));

  dp = (_point_t ***) xmalloc((psol->n + 1)*sizeof(_point_t **));
  dp[0] = (_point_t **) xmalloc((psol->n + 1)*nsize*sizeof(_point_t *));
  for(i = 1; i <= psol->n; i++) {
    dp[i] = dp[i - 1] + nsize;
  }

  for(i = 0; i <= psol->n; i++) {
    cdp = dp[i];
    if(i > 0) {
      cdp[0] = duplicate_function_with_shift(benv, dp[i - 1][0], prob->T,
					     psol->job[i - 1]->p);
      add_functions(benv, &(cdp[0]), psol->job[i - 1]->func);
      convert_function(benv, cdp[0]);
      set_function_state(cdp[0], nsize);
    } else {
      cdp[0] = zero_function(benv, prob->T);
    }
#ifdef DEBUG
    printf("A --- %d\n", i);
    print_function(cdp[0], stdout);
#endif

    for(j = 1; j <= isol->n; j++) {
      for(k = 1; k < nsize; k++) {
	if(j == nn[k]) {
	  if(i > 0) {
	    cdp[k] = duplicate_function_with_shift(benv, dp[i - 1][k], prob->T,
						   psol->job[i - 1]->p);
	    add_functions(benv, &(cdp[k]), psol->job[i - 1]->func);
	    convert_function(benv, cdp[k]);
	  } else {
	    cdp[k] = null_function(benv);
	  }

	  set_function_state(cdp[k], nsize);

	  for(l = 0, m = 1; l < isol->n; l++, m <<= 1) {
	    if(k & m) {
	      fp = duplicate_function_with_shift(benv, cdp[(k ^ m)], prob->T,
						 isol->job[l]->p);
	      add_functions(benv, &fp, isol->job[l]->func);
	      convert_function(benv, fp);
	      set_function_state(fp, (k ^ m));
#ifdef DEBUG
	      printf("B --- %d\n", i);
	      print_function(fp, stdout);
#endif
	      minimum_of_functions(benv, &(cdp[k]), fp);
	      free_function(benv, fp);
#ifdef DEBUG
	      printf("MIN\n");
	      print_function(cdp[k], stdout);
#endif
	    }
	  }
	}    
      }
    }
  }

  t = prob->T;
  s = 0;
  j = psol->n;
  k = nsize - 1;
  sol->n = prob->n;
  sol->f = dp[j][k]->prev->v;
  flag = SIPS_TRUE;
  for(i = prob->n - 1; i >= 0; i--) {
    t = minimize_function(dp[j][k], t, &s);
    if(t < 0) {
      flag = SIPS_FALSE;
      sol->f = LARGE_COST;
      break;
    }
    if(s == nsize) {
      sol->job[i] = psol->job[j - 1];
      j--;
    } else {
      for(m = (s ^ k)>>1, l = 0; m; m >>= 1, l++);
      sol->job[i] = isol->job[l];
      k = s;
    }
    sol->c[i] = t;
    t -= sol->job[i]->p;
  }

  xfree(dp[0]);
  xfree(dp);
  xfree(nn);
  free_benv(benv);

  if(is_true(flag)) {
    objective(sol);
  }
}

/*
 * insert_ordered_jobs_greedily(sips, sol, n, ijob)
 *   inserts the jobs in ijob one by one with the processing order
 *   of sol kept unchanged.  The insertion is performed according to
 *   the order in ijob.
 *       sol: partial solution
 *        in: number of jobs in ijob
 *      ijob: jobs to be inserted
 *
 */
void insert_ordered_jobs_greedily(sips *prob, _solution_t *sol, int in,
				  _job_t **ijob)
{
  int i, j, k, n;
  int argmin;
  cost_t min_cost, f;
  _benv_t *benv;
  _point_t **func, **rfunc, *fp;

  if(in == 0) {
    solution_set_c(prob, sol);
    return;
  }

  n = sol->n + in;
  benv = create_benv(sizeof(_point_t));
  func = (_point_t **) xcalloc(2*(n + 1), sizeof(_point_t *));
  rfunc = func + n + 1;

  func[0] = rfunc[n] = zero_function(benv, prob->T);

  for(i = 1; i <= sol->n; i++) {
    func[i] = duplicate_function_with_shift(benv, func[i - 1], prob->T,
					    sol->job[i - 1]->p);
    add_functions(benv, &(func[i]), sol->job[i - 1]->func);
    convert_function(benv, func[i]);

    rfunc[n - i] = duplicate_function(benv, rfunc[n - i + 1]);
    add_functions(benv, &(rfunc[n - i]), sol->job[sol->n - i]->func);
    rshift_function(benv, &(rfunc[n - i]), sol->job[sol->n - i]->p);
    rconvert_function(benv, &(rfunc[n - i]));
  }

  for(i = 0;; i++) {
    min_cost = LARGE_COST;
    argmin = 0;
    for(j = 0; j <= sol->n; j++) {
      fp = duplicate_function_with_shift(benv, func[j], prob->T, ijob[i]->p);
      add_functions(benv, &fp, ijob[i]->func);
      convert_function(benv, fp);
      minimize_functions_when_added(fp, rfunc[n - sol->n + j], &f);
      if(f < min_cost) {
	min_cost = f;
	argmin = j;
      }
      free_function(benv, fp);
    }

    for(j = sol->n; j > argmin; j--) {
      sol->job[j] = sol->job[j - 1];
    }
    sol->job[argmin] = ijob[i];
    sol->n++;

    if(i == in - 1) {
      break;
    }

    for(j = argmin; j < sol->n; j++) {
      free_function(benv, func[j + 1]);
      func[j + 1] = duplicate_function_with_shift(benv, func[j], prob->T,
						  sol->job[j]->p);
      add_functions(benv, &(func[j + 1]), sol->job[j]->func);
      convert_function(benv, func[j + 1]);
    }

    for(j = argmin; j >= 0; j--) {
      k = n - sol->n + j;
      free_function(benv, rfunc[k]);
      rfunc[k] = duplicate_function(benv, rfunc[k + 1]);
      add_functions(benv, &(rfunc[k]), sol->job[j]->func);
      rshift_function(benv, &(rfunc[k]), sol->job[j]->p);
      rconvert_function(benv, &(rfunc[k]));
    }
  }

  for(i = 0; i < 2*(n + 1); i++) {
    free_function(benv, func[i]);
  }

  xfree(func);
  free_benv(benv);

  insert_idle_time(prob, sol);
#if 0
  if(!obj_equal(sol->f, min_cost)) {
    fprintf(stderr, "insert_ordered_jobs_greedily(): ");
    print_cost(stderr, sol->f + prob->off);
    fprintf(stderr, "!=");
    print_cost(stderr, min_cost + prob->off);
    fprintf(stderr, "\n");
    print_sol(prob, sol, stderr);
    exit(1);
  }
#endif /* 0 */
  return;
}

/*
 * _count_bit(k)
 *   counts the number of ones in a given binary number.
 *        k: binary number
 *
 */
int _count_bit(int k)
{
  int i;

  for(i = 0; k; i++, k &= k - 1);

  return(i);
}
