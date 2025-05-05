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
 *  $Id: heuristics.c,v 1.10 2013/05/28 13:26:46 tanaka Rel $
 *  $Revision: 1.10 $
 *  $Date: 2013/05/28 13:26:46 $
 *  $Author: tanaka $
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "define.h"
#include "sips_common.h"
#include "heuristics.h"
#include "job_sort.h"
#include "memory.h"
#include "objective.h"
#include "sol.h"

#define ELIMINATED (prob->n)

struct _partdp_t {
  cost_t f;
  unsigned short j;
  struct _partdp_t *pr;
};

typedef struct _partdp_t _partdp_t;

static void _insert_jobs_optimally(sips *, _solution_t *, _solution_t *,
				   _solution_t *);
static void insert_jobs_greedily(sips *, _solution_t *, int, _job_t **);
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
 * partialdp(prob, sol)
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

  for(i = prob->n - 1; i >= 0; i--) {
    if(e[i] != -1) {
      isol->job[isol->n++] = prob->sjob[i];
    }
  }
  xfree(e);

  if(isol->n > prob->param->dpsize) {
    copy_solution(prob, sol, psol);
#if 1
    insert_jobs_greedily(prob, sol, isol->n, isol->job);
#else
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
  int i, j, k, l, m, n;
  int nsize, cpsum;
  cost_t f;
  int *nn, *ppsum, *ipsum;
  _partdp_t **dp, *cdp;

  nsize = 1 << isol->n;

  dp = (_partdp_t **) xmalloc((psol->n + 1)*sizeof(_partdp_t *));
  dp[0] = (_partdp_t *) xmalloc((psol->n + 1)*nsize*sizeof(_partdp_t));
  for(i = 1; i <= psol->n; i++) {
    dp[i] = dp[i - 1] + nsize;
  }

  nn = (int *) xmalloc(nsize*sizeof(int));
  ppsum = (int *) xmalloc((psol->n + 1)*sizeof(int));
  ipsum = (int *) xmalloc(nsize*sizeof(int));

  m = 0;
  cpsum = 0;
  ipsum[0] = 0;
  for(i = 1; i < nsize; i++) {
    nn[i] = _count_bit(i);

    j = i ^ (i - 1);
    k = 1 << (isol->n - 1);
    for(l = 0; (k & j) == 0; l++, k >>= 1);
    j = isol->n - l - 1;
    /* m: Gray code */
    m ^= k;

    if(m & k) {
      cpsum += isol->job[j]->p;
    } else {
      cpsum -= isol->job[j]->p;
    }

    ipsum[m] = cpsum;
  }

  ppsum[0] = 0;
  for(i = 0; i < psol->n; i++) {
    ppsum[i + 1] = ppsum[i] + psol->job[i]->p;
  }

  for(i = 0; i <= psol->n; i++) {
    cdp = dp[i];
    if(i > 0) {
      cdp->j = psol->job[i - 1]->no;
      cdp->f = dp[i - 1][0].f + psol->job[i - 1]->f[ppsum[i]];
      cdp->pr = &(dp[i - 1][0]);
    } else {
      cdp->j = ELIMINATED;
      cdp->f = ZERO_COST;
      cdp->pr = NULL;
    }

    for(j = 1; j <= isol->n; j++) {
      for(k = 1; k < nsize; k++) {
	cdp = dp[i] + k;
	if(j == nn[k]) {
	  if(i > 0) {
	    cdp->j = psol->job[i - 1]->no;
	    cdp->f = dp[i - 1][k].f + psol->job[i - 1]->f[ppsum[i] + ipsum[k]];
	    cdp->pr = &(dp[i - 1][k]);
	  } else {
	    cdp->j = ELIMINATED;
	    cdp->f = LARGE_COST;
	    cdp->pr = NULL;
	  } 

	  for(l = 0, m = 1; l < isol->n; l++, m <<= 1) {
	    if(k & m) {
	      n = k ^ m;
	      f = dp[i][n].f + isol->job[l]->f[ppsum[i] + ipsum[k]];

	      if(obj_lesser(f, cdp->f)) {
		cdp->j = isol->job[l]->no;
		cdp->f = f;
		cdp->pr = &(dp[i][n]);
	      }
	    }
	  }
	}    
      }
    }
  }

  sol->n = 0;
  cdp = &(dp[psol->n][nsize - 1]);
  for(i = 0; i < prob->n; i++) {
    sol->job[prob->n - i - 1] = prob->sjob[cdp->j];
    cdp = cdp->pr;
  }
  sol->n = prob->n;

  xfree(ipsum);
  xfree(ppsum);
  xfree(nn);
  xfree(dp[0]);
  xfree(dp);

  solution_set_c(prob, sol);
}

/*
 * insert_jobs_greedily(sips, sol, n, ijob)
 *   inserts the jobs in ijob one by one with the processing order
 *   of sol kept unchanged.
 *       sol: partial solution
 *        in: number of jobs in ijob
 *      ijob: jobs to be inserted
 *
 */
void insert_jobs_greedily(sips *prob, _solution_t *sol, int in, _job_t **ijob)
{
  int i, j, k;
  int argmin, argmini;
  int c;
  cost_t prevf, f, minf;

  solution_set_c(prob, sol);
  if(in == 0) {
    return;
  }

  while(in > 0) {
    argmini = 0;
    argmin = 0;
    minf = LARGE_COST;
    for(i = 0; i < in; i++) {
      c = ijob[i]->p;
      f = ijob[i]->f[c];
      for(j = 0; j < sol->n; j++) {
	      c += sol->job[j]->p;
	      f += sol->job[j]->f[c];
      }
      if(obj_lesser(f, minf)) {
	      argmini = i;
	      argmin = 0;
	      minf = f;
      }

      prevf = ZERO_COST;

      for(j = 1; j <= sol->n; j++) {
	      c = sol->c[j - 1];
	      prevf += sol->job[j - 1]->f[c];

	      c += ijob[i]->p;
	      f = prevf + ijob[i]->f[c];

	      for(k = j; k < sol->n; k++) {
	        c += sol->job[k]->p;
	        f += sol->job[k]->f[c];
	      }

	      if(obj_lesser(f, minf)) {
	        argmini = i;
	        argmin = j;
	        minf = f;
	      }
      }
    }

    c = ijob[argmini]->p;
    for(j = sol->n; j > argmin; j--) {
      sol->c[j] = sol->c[j - 1] + c;
      sol->job[j] = sol->job[j - 1];
    }

    if(argmin == 0) {
      sol->c[argmin] = c;
    } else {
      sol->c[argmin] = sol->c[argmin - 1] + c;
    }
    sol->job[argmin] = ijob[argmini];

    sol->n++;
    sol->f = minf;

    in--;
    for(j = argmini; j < in; j++) {
      ijob[j] = ijob[j + 1];
    }
  }  

  return;
}

/*
 * insert_ordered_jobs_greedily(sips, sol, n, ijob)
 *   inserts the jobs in ijob one by one with the processing order
 *   of sol kept unchanged.  The insertion is performed according to
 *   the order in ijob.
 *       sol: partial solution
 *        in: number of jobs in ijob
 *      ijob: ordered list of jobs to be inserted
 *
 */
void insert_ordered_jobs_greedily(sips *prob, _solution_t *sol, int in,
				  _job_t **ijob)
{
  int i, j, k;
  int argmin;
  int c;
  cost_t prevf, f, minf;

  solution_set_c(prob, sol);
  if(in == 0) {
    return;
  }

  for(i = 0; i < in; i++) {
    argmin = 0;
    c = ijob[i]->p;
    minf = ijob[i]->f[c];
    for(k = 0; k < sol->n; k++) {
      c += sol->job[k]->p;
      minf += sol->job[k]->f[c];
    }
    prevf = ZERO_COST;

    for(j = 1; j <= sol->n; j++) {
      c = sol->c[j - 1];
      prevf += sol->job[j - 1]->f[c];

      c += ijob[i]->p;
      f = prevf + ijob[i]->f[c];

      for(k = j; k < sol->n; k++) {
	c += sol->job[k]->p;
	f += sol->job[k]->f[c];
      }

      if(obj_lesser(f, minf)) {
	argmin = j;
	minf = f;
      }
    }

    c = ijob[i]->p;
    for(j = sol->n; j > argmin; j--) {
      sol->c[j] = sol->c[j - 1] + c;
      sol->job[j] = sol->job[j - 1];
    }

    if(argmin == 0) {
      sol->c[argmin] = ijob[i]->p;
    } else {
      sol->c[argmin] = sol->c[argmin - 1] + ijob[i]->p;
    }
    sol->job[argmin] = ijob[i];

    sol->f = minf;
    sol->n++;
  }  

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
