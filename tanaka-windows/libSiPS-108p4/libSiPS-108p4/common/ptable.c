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
 *  $Id: ptable.c,v 1.20 2013/05/28 13:27:26 tanaka Rel $
 *  $Revision: 1.20 $
 *  $Date: 2013/05/28 13:27:26 $
 *  $Author: tanaka $
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "define.h"
#include "ssdp.h"
#include "constraint.h"
#include "job_sort.h"
#include "lag.h"
#include "lag_common.h"
#include "lag2.h"
#include "memory.h"
#include "ptable.h"
#include "sol.h"

static int _ptable_initialize_window(sips *, _ptable_t *);
static void _copy_ptable(sips *, _ptable_t *, _ptable_t *);
#ifdef NLOGN_EDGE_FINDING
static void _create_btctable_sub(int *, int, int, int, int);
static int *_create_btctable(int);
#endif /* NLOGN_EDGE_FINDING */

/*
 * ptable_initialize(prob)
 *   initializes the precedence table.
 *   Only the domains of jobs are set in this stage.
 *
 */
int ptable_initialize(sips *prob)
{
  int t;
  int i;
  _real estimated_memory;
  _ptable_t *ptable;

  if(prob == NULL || prob->graph == NULL || prob->graph->ptable != NULL) {
    return(SIPS_FAIL);
  }

  prob->graph->ptable = ptable = (_ptable_t *) xcalloc(sizeof(_ptable_t), 1);

#ifdef SIPSI
  ptable->n = prob->n + 1;
#else /* SIPSI */
  ptable->n = prob->n;
  ptable->sd_size = (size_t) prob->T/my_char_bit + 1;
#endif /* SIPSI */
  ptable->adj_per_t = 0;
  ptable->dom_per_t = (size_t) (ptable->n - 1)/my_char_bit + 1;
  ptable->T = prob->T;

#ifdef SIPSI
  ptable->mem
    = (_real) ((prob->T + 1)*ptable->dom_per_t)/(_real) (1<<20);
#else /* SIPSI */
  ptable->mem
    = (_real) ((prob->T + 1)*ptable->dom_per_t + ptable->sd_size)
    /(_real) (1<<20);
#endif /* SIPSI */

  estimated_memory = lag_get_memory_in_MB(prob) + ptable->mem;

  if(estimated_memory > (_real) prob->param->mem) {
    if(prob->param->verbose >= 2) {
      fprintf(stdout, "Not enough memory for ptable (");
      print_real(stdout, .2, estimated_memory);
      fprintf(stdout, "MB > %dMB\n", prob->param->mem);
    }
    xfree(ptable);

    return(SIPS_MEMLIMIT);
  }

  ptable->adj = NULL;
  ptable->dom = (char *) xcalloc((prob->T + 1)*ptable->dom_per_t, 1);
#ifndef SIPSI
  ptable->sd = (char *) xcalloc(ptable->sd_size, 1);
#endif /* !SIPSI */

#ifdef SIPSI
  for(i = 0; i <= prob->n; i++) {
    for(t = 0; t < prob->sjob[i]->r + prob->sjob[i]->p; t++) {
      prec_set_dom_ti(t, i);
    }
  }

  if(!(prob->param->ptype & SIPS_PROB_IDLETIME)) {
    for(t = 1; t <= prob->T; t++) {
      prec_set_dom_ti(t, prob->n);
    }
  }
#else /* SIPSI */
  for(i = 0; i < prob->n; i++) {
    for(t = 0; t < prob->sjob[i]->p; t++) {
      prec_set_dom_ti(t, i);
    }
  }
#endif /* SIPSI */

  return(SIPS_OK);
}

/*
 * ptable_initialize_adj(prob, ptable)
 *   initializes the table for the dominance of adjacent two jobs.
 *   Ties are broken by the order of the current best solution when
 *   prob->param->tiebreak = SIPS_TIEBREAK_HEURISTIC,
 *   the reverse order of it when
 *   prob->param->tiebreak = SIPS_TIEBREAK_RHEURISTIC.
 *      ptable: precedence table
 *
 */
int ptable_initialize_adj(sips *prob, _ptable_t *ptable)
{
  int t;
#ifdef AGGRESSIVE
  int i, j, k;
  char **inc;
  _window_t *w;
#else /* AGGRESSIVE */
  int i, j;
#endif /* AGGRESSIVE */
  int nn;
  int ret;
  cost_t f1, f2;
  _real estimated_memory;
  _solution_t *tmpsol;

  if(ptable == NULL || ptable->adj != NULL) {
    return(SIPS_FAIL);
  }

  ptable->adj_per_t = (size_t) (ptable->n*ptable->n - 1)/my_char_bit + 1;
  ptable->mem += (_real) ((prob->T + 1)*ptable->adj_per_t)/(_real) (1<<20);

  estimated_memory = lag_get_memory_in_MB(prob) + ptable->mem;
  if(estimated_memory > (_real) prob->param->mem) {
    if(prob->param->verbose >= 2) {
      fprintf(stdout, "Not enough memory for ptable (");
      print_real(stdout, .2, estimated_memory);
      fprintf(stdout, "MB > %dMB\n", prob->param->mem);
    }

    return(SIPS_MEMLIMIT);
  }

  ret = _ptable_initialize_window(prob, prob->graph->ptable);
  if(ret != SIPS_OK) {
    return(ret);
  }

  ptable->adj = (char *) xcalloc((prob->T + 1)*ptable->adj_per_t, 1);

  for(i = 0; i < prob->n; i++) {
#ifdef SIPSI
    for(t = prob->sjob[i]->r + prob->sjob[i]->p; t <= prob->T; t++) {
      prec_set_adj_tij(t, i, i);
    }
#else /* SIPSI */
    for(t = prob->sjob[i]->p; t <= prob->T; t++) {
      prec_set_adj_tij(t, i, i);
    }
#endif /* SIPSI */
  }

  tmpsol = create_solution(prob);

  if(prob->sol != NULL && (prob->param->tiebreak < SIPS_TIEBREAK_EDD)) {
    copy_solution(prob, tmpsol, prob->sol);
    if(prob->param->tiebreak == SIPS_TIEBREAK_RHEURISTIC) {
      reverse_solution(tmpsol);
    }
  } else {
    for(i = 0; i < prob->n; i++) {
      tmpsol->job[i] = prob->sjob[i];
    }
    tmpsol->n = prob->n;

    if(prob->param->tiebreak >= SIPS_TIEBREAK_EDD) {
      sort_jobs(tmpsol->n, tmpsol->job,
		prob->param->tiebreak - SIPS_TIEBREAK_EDD);
    } else {
      sort_jobs(tmpsol->n, tmpsol->job, 0);
    }
  }

  for(i = 0; i < prob->n; i++) {
    prob->sjob[tmpsol->job[i]->no]->tno = i;
  }
#ifdef SIPSI
  for(i = prob->n; i < N_JOBS; i++) {
    prob->sjob[i]->tno = prob->n;
  }
#endif /* SIPSI */

  free_solution(tmpsol);

#ifdef AGGRESSIVE
  inc = ptable->inc;
  if(prob->param->ptype & SIPS_PROB_REGULAR) {
    for(i = 0; i < prob->n;) {
      for(k = i + 1; k < prob->n && prob->sjob[i]->p == prob->sjob[k]->p; k++);
      for(; i < k; i++) {
	for(j = i + 1; j < k; j++) {
	  if(prob->sjob[i]->d <= prob->sjob[j]->d
	     && prob->sjob[i]->tno < prob->sjob[j]->tno
	     && obj_greater_equal(prob->sjob[i]->tw, prob->sjob[j]->tw)) {
#ifdef SIPSI
	    if(prob->sjob[i]->r <= prob->sjob[j]->r) {
#endif /* SIPSI */
#ifdef DEBUG
	      printf("NEW DOMINANCE no%d=>no%d\n",
		     prob->sjob[i]->rno, prob->sjob[j]->rno);
#endif /* DEBUG */
	      _set_inc(i, j);
#ifdef SIPSI
	    }
#endif /* SIPSI */
	  } else if(prob->sjob[i]->d >= prob->sjob[j]->d
		    && prob->sjob[i]->tno > prob->sjob[j]->tno
		    && obj_lesser_equal(prob->sjob[i]->tw,
					prob->sjob[j]->tw)) {
#ifdef SIPSI
	    if(prob->sjob[i]->r >= prob->sjob[j]->r) {
#endif /* SIPSI */
#ifdef DEBUG
	      printf("NEW DOMINANCE no%d=>no%d\n",
		     prob->sjob[j]->rno, prob->sjob[i]->rno);
#endif /* DEBUG */
	      _set_inc(j, i);
#ifdef SIPSI
	    }
#endif /* SIPSI */
	  }
	}
      }
    }
  }

  w = (_window_t *) xmalloc(prob->n*sizeof(_window_t));
  memcpy((void *) w, (void *) ptable->window, prob->n*sizeof(_window_t));
  if(constraint_propagation(prob, w) == SIPS_INFEASIBLE) {
    xfree(w);
    return(SIPS_INFEASIBLE);
  }
  for(i = 0; i < prob->n; i++) {
    for(t = ptable->window[i].s; t < w[i].s; t++) {
      prec_set_dom_ti(t, i);
    }
    for(t = ptable->window[i].e; t > w[i].e; t--) {
      prec_set_dom_ti(t, i);
    }
  }
  memcpy((void *) ptable->window, (void *) w, prob->n*sizeof(_window_t));
  xfree(w);
#endif /* AGGRESSIVE */

#ifdef SIPSI
  if(prob->rmax != 0) {
    for(t = 0; t <= prob->T; t++) {
      for(i = 0; i < prob->n; i++) {
	for(j = i + 1; j <= prob->n; j++) {
	  if(t < prob->sjob[i]->r + prob->sjob[i]->p + prob->sjob[j]->p
	     || t < prob->sjob[j]->r + prob->sjob[i]->p + prob->sjob[j]->p) {
	    continue;
	  }

	  f1 = prob->sjob[i]->f[t] + prob->sjob[j]->f[t - prob->sjob[i]->p];
	  f2 = prob->sjob[i]->f[t - prob->sjob[j]->p] + prob->sjob[j]->f[t];
	  /* f1: j->i, f2: i->j */
	  if(obj_lesser(f1, f2)) {
	    prec_set_adj_tij(t, i, j);
	  } else if(obj_greater(f1, f2)) {
	    prec_set_adj_tij(t, j, i);
	  } else if(prob->sjob[i]->tno < prob->sjob[j]->tno) {
	    prec_set_adj_tij(t, j, i);
	  } else {
	    prec_set_adj_tij(t, i, j);
	  }
	}
      }
    }
  } else {
    nn = prob->n + 1;
#else /* SIPSI */
    nn = prob->n;
#endif /* SIPSI */
    for(t = 0; t <= prob->T; t++) {
      for(i = 0; i < nn - 1; i++) {
	for(j = i + 1; j < nn; j++) {
	  if(t < prob->sjob[i]->p + prob->sjob[j]->p) {
	    prec_set_adj_tij(t, i, j);
	    prec_set_adj_tij(t, j, i);
	    continue;
	  }

	  f1 = prob->sjob[i]->f[t] + prob->sjob[j]->f[t - prob->sjob[i]->p];
	  f2 = prob->sjob[i]->f[t - prob->sjob[j]->p] + prob->sjob[j]->f[t];
	  /* f1: j->i, f2: i->j */
	  if(obj_lesser(f1, f2)) {
	    prec_set_adj_tij(t, i, j);
	  } else if(obj_greater(f1, f2)) {
	    prec_set_adj_tij(t, j, i);
	  } else if(prob->sjob[i]->tno < prob->sjob[j]->tno) {
	    prec_set_adj_tij(t, j, i);
	  } else {
	    prec_set_adj_tij(t, i, j);
	  }
	}
      }
    }
#ifdef SIPSI
  }
#endif /* SIPSI */

  return(SIPS_OK);
}

/*
 * _ptable_initialize_window(prob, ptable)
 *   initializes the precedence table for constraint propagation.
 *      ptable: precedence table
 *
 */
int _ptable_initialize_window(sips *prob, _ptable_t *ptable)
{
  int t;
  int i;
  _real estimated_memory;

  if(ptable == NULL || ptable->inc != NULL) {
    return(SIPS_FAIL);
  }

  estimated_memory = (_real) (prob->n*sizeof(char *)
			      + N_JOBS*sizeof(_window_t)
			      + prob->n*prob->n
#ifdef NLOGN_EDGE_FINDING
			      + prob->n*sizeof(int)
#endif /* NLOGN_EDGE_FINDING */
			      + (2*prob->n + N_JOBS + 2)*sizeof(int))
    /(_real) (1<<20);

  if(estimated_memory + ptable->mem + lag_get_memory_in_MB(prob)
     > (_real) prob->param->mem) {
    if(prob->param->verbose >= 2) {
      fprintf(stdout, "Not enough memory for ptable (");
      print_real(stdout, .2, estimated_memory);
      fprintf(stdout, "MB > %dMB\n", prob->param->mem);
    }

    return(SIPS_MEMLIMIT);
  }

  ptable->inc = (char **) xmalloc(prob->n*sizeof(char *));
  ptable->inc[0] = (char *) xcalloc(prob->n, prob->n);
  for(i = 1; i < prob->n; i++) {
    ptable->inc[i] = ptable->inc[i - 1] + prob->n;
  }

  ptable->pre_psum = (int *) xcalloc(2*prob->n, sizeof(int));
  ptable->suc_psum = ptable->pre_psum + prob->n;
#ifdef NLOGN_EDGE_FINDING
  ptable->btctable = _create_btctable(prob->n);
#endif /* NLOGN_EDGE_FINDING */
  ptable->occ = (unsigned int *) xcalloc(N_JOBS + 2, sizeof(unsigned int));
  ptable->window = (_window_t *) xcalloc(N_JOBS, sizeof(_window_t));
  for(i = 0; i < N_JOBS; i++) {
    ptable->window[i].j = i;
    ptable->window[i].s = prob->graph->Tmin + prob->sjob[i]->p;
    ptable->window[i].e = prob->graph->Tmax;
    if(IS_REAL_JOB(i)) {
      for(t = prob->graph->Tmin + prob->sjob[i]->p;
	  t <= prob->graph->Tmax; t++) {
	if(!prec_get_dom_ti(t, i)) {
	  ptable->window[i].s = t;
	  break;
	}
      }
      for(t = prob->graph->Tmax; t > ptable->window[i].s; t--) {
	if(!prec_get_dom_ti(t, i)) {
	  ptable->window[i].e = t;
	  break;
	}
      }
    }
  }

  ptable->mem += estimated_memory;

  return(SIPS_OK);
}

/*
 * duplicate_ptable(prob, src)
 *   duplicate the precedence table.
 *       src: source
 *
 */
_ptable_t *duplicate_ptable(sips *prob, _ptable_t *src)
{
  int i;
  _ptable_t *dest;

  if(src == NULL) {
    return(NULL);
  }

  dest = (_ptable_t *) xcalloc(sizeof(_ptable_t), 1);

  if(src->adj != NULL) {
    dest->adj = (char *) xmalloc((prob->T + 1)*src->adj_per_t);
  }
  if(src->dom != NULL) {
    dest->dom = (char *) xmalloc((prob->T + 1)*src->dom_per_t);
  }
#ifndef SIPSI
  if(src->sd != NULL) {
    dest->sd = (char *) xmalloc(src->sd_size);
  }
#endif /* !SIPSI */
  if(src->inc != NULL) {
    dest->inc = (char **) xmalloc(prob->n*sizeof(char *));
    dest->inc[0] = (char *) xmalloc(prob->n*prob->n);
    for(i = 1; i < prob->n; i++) {
      dest->inc[i] = dest->inc[i - 1] + prob->n;
    }
  }
  if(src->pre_psum != NULL) {
    dest->pre_psum = (int *) xmalloc(2*prob->n*sizeof(int));
    dest->suc_psum = dest->pre_psum + prob->n;
  }
#ifdef NLOGN_EDGE_FINDING
  if(src->btctable != NULL) {
    dest->btctable = (int *) xmalloc(prob->n*sizeof(int));
  }
#endif /* NLOGN_EDGE_FINDING */
  if(src->occ != NULL) {
    dest->occ = (unsigned int *) xmalloc((N_JOBS + 2)*sizeof(unsigned int));
  }
  if(src->window != NULL) {
    dest->window = (_window_t *) xmalloc(N_JOBS*sizeof(_window_t));
  }

  _copy_ptable(prob, dest, src);

  return(dest);
}

/*
 * _copy_ptable(prob, dest, src)
 *   copies ptable.
 *     dest: destination
 *      src: source
 *
 */
void _copy_ptable(sips *prob, _ptable_t *dest, _ptable_t *src)
{
  if(src == NULL || dest == NULL) {
    return;
  }

  dest->dom_per_t = src->dom_per_t;
  dest->adj_per_t = src->adj_per_t;
#ifndef SIPSI
  dest->sd_size = src->sd_size;
#endif /* !SIPSI */
  dest->n = src->n;
  dest->T = src->T;
  dest->mem = src->mem;

  if(src->adj != NULL) {
    memcpy((void *) dest->adj, (void *) src->adj,
	   (prob->T + 1)*src->adj_per_t);
  }
  if(src->dom != NULL) {
    memcpy((void *) dest->dom, (void *) src->dom,
	   (prob->T + 1)*src->dom_per_t);
  }
#ifndef SIPSI
  if(src->sd != NULL) {
    memcpy((void *) dest->sd, (void *) src->sd, src->sd_size);
  }
#endif /* !SIPSI */
  if(src->inc != NULL) {
    memcpy((void *) dest->inc[0], (void *) src->inc[0], prob->n*prob->n);
  }
  if(src->pre_psum != NULL) {
    memcpy((void *) dest->pre_psum, (void *) src->pre_psum,
	   2*prob->n*sizeof(int));
  }
#ifdef NLOGN_EDGE_FINDING
  if(src->btctable != NULL) {
    memcpy((void *) dest->btctable, (void *) src->btctable,
	   prob->n*sizeof(int));
  }
#endif /* NLOGN_EDGE_FINDING */
  if(src->occ != NULL) {
    memcpy((void *) dest->occ, (void *) src->occ,
	   (N_JOBS + 2)*sizeof(unsigned int));
  }
  if(src->window != NULL) {
    memcpy((void *) dest->window, (void *) src->window,
	   N_JOBS*sizeof(_window_t));
  }
}

/*
 * ptable_free_adj(ptable)
 *   frees the table for the dominance of two adjacent jobs.
 *     ptable: precedence table
 *
 */
void ptable_free_adj(_ptable_t *ptable)
{
  if(ptable != NULL) {
    xfree(ptable->adj);
#ifndef SIPSI
    xfree(ptable->sd);
#endif /* !SIPSI */
    ptable->adj = NULL;
#ifdef SIPSI
    ptable->mem
      -= (_real) ((ptable->T + 1)*ptable->adj_per_t)/(_real) (1<<20);
#else /* SIPSI */
    ptable->sd = NULL;
    ptable->mem
      -= (_real) ((ptable->T + 1)*ptable->adj_per_t + ptable->sd_size)
      /(_real) (1<<20);
#endif /* SIPSI */
  }
}

/*
 * ptable_free(ptable)
 *   frees the precedence table.
 *     ptable: precedence table
 *
 */
void ptable_free(_ptable_t *ptable)
{
  if(ptable != NULL) {
    xfree(ptable->window);
    xfree(ptable->occ);
#ifdef NLOGN_EDGE_FINDING
    xfree(ptable->btctable);
#endif /* NLOGN_EDGE_FINDING */
    xfree(ptable->pre_psum);
    if(ptable->inc != NULL) {
      xfree(ptable->inc[0]);
    }
    xfree(ptable->inc);
    xfree(ptable->adj);
#ifndef SIPSI
    xfree(ptable->sd);
#endif /* !SIPSI */
    xfree(ptable->dom);
    xfree(ptable);
  }
}

/*
 * prec_get_memory_in_MB(prob)
 *   returns memory usage of the dominance table.
 *
 */
_real prec_get_memory_in_MB(sips *prob)
{
  return((prob->graph->ptable != NULL)?prob->graph->ptable->mem:0.0);
}

#ifdef NLOGN_EDGE_FINDING
/*
 * _create_btctable(n)
 *   creates a list -> btree conversion table.
 *          n: number of elements
 *
 */
int *_create_btctable(int n)
{
  int i;
  int *table;

  table = (int *) xmalloc(n*sizeof(int));

  for(i = 1; i <= n; i <<= 1);
  i--;

  _create_btctable_sub(table, 0, n, 0, i);

  return(table);
}

/*
 * _create_btctable_sub(table, s, n, c, rest)
 *   subrutine for create_btctable.
 *      table: list -> btree conversion table
 *          s: start of converted list elements
 *          n: number of elements
 *          c: root node in btree
 *       rest: tree size
 *
 */
void _create_btctable_sub(int *table, int s, int n, int c, int rest)
{
  int left, right;

  if(n == 0) {
    return;
  } else if(n == 1) {
    table[c] = s;
    return;
  }

  rest = (rest - 1)/2;
  if(n - 1 - rest > (rest - 1)/2) {
    left = rest;
    right = n - 1 - left;
    _create_btctable_sub(table, s, left, 2*c + 1, rest);
    _create_btctable_sub(table, s + left + 1, right, 2*c + 2, rest);
  } else {
    right = (rest - 1)/2;
    left = n - 1 - right;
    _create_btctable_sub(table, s, left, 2*c + 1, rest);
    _create_btctable_sub(table, s + left + 1, right, 2*c + 2, right);
  }

  table[c] = s + left;
}
#endif /* NLOGN_EDGE_FINDING */
