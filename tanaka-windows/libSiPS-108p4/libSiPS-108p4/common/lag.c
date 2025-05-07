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
 *  $Id: lag.c,v 1.21 2013/05/28 13:27:11 tanaka Rel $
 *  $Revision: 1.21 $
 *  $Date: 2013/05/28 13:27:11 $
 *  $Author: tanaka $
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "define.h"
#include "ssdp.h"
#include "bmemory.h"
#include "constraint.h"
#include "lag.h"
#include "lag_common.h"
#include "memory.h"
#include "objective.h"
#include "ptable.h"
#include "sol.h"

typedef struct {
  _real v[2];
  char ty[2];
  unsigned short j[2];
} _node1_t;

struct _edge2_t {
  struct _node2_t *n;    /* connected node (tail in FW and head in BW) */
  struct _edge2_t *next; /* next edge */
};

struct _node2_t {
  unsigned short j;      /* job no */
  char ty;               /* type (0: best, 1: second best) */
  _real v[2];             /* objective values */
  struct _node2_t *n[2]; /* prev. nodes realizing the min and second min */
  struct _node2_t *next; /* next node with the same t */
};

typedef struct _edge2_t _edge2_t;
typedef struct _node2_t _node2_t;

#ifndef SIPSI
static void _lag_solve_LR1_forward_with_domain(sips *, _real *);
#endif /* !SIPSI */
static void _lag_solve_LR1_forward(sips *, _real *);
static void _lag_solve_LR2adj_forward(sips *, _real *);
static int _lag_solve_LR2adj_forward_with_elimination(sips *, _real *,
						      _real);
static int _lag_check_time_window(sips *);
static int _lag_get_sol_LR1_forward(sips *, _solution_t *, _real *,
				    unsigned int *);
#if 0
static int _lag_get_sol_LR1_backward(sips *, _solution_t *, _real *,
				     unsigned int *);
#endif /* 0 */

static int _lag_get_sol_LR2adj_forward(sips *, _solution_t *, _real *,
				       unsigned int *);
#if 0
static int _lag_get_sol_LR2adj_backward(sips *, _solution_t *, _real *,
					unsigned int *);
#endif /* 0 */

/*
 * lag_initialize_LR1_solver(prob)
 *   initializes the (LR1) solver.
 *
 */
int lag_initialize_LR1_solver(sips *prob)
{
  int ret;

  ret = SIPS_OK;
  if(prob->graph->node1 == NULL) {
    prob->graph->node1 = (void *) xcalloc(prob->T + 1, sizeof(_node1_t));
    prob->graph->Tmax = prob->T;
    prob->graph->Tmin = 0;
    ret = ptable_initialize(prob);
  }

  return(ret);
}

/*
 * lag_initialize_LR2adj_solver(prob)
 *   initializes the (LR2adj) solver.
 *
 */
int lag_initialize_LR2adj_solver(sips *prob)
{
  int ret;

  if(prob->graph->node2 == NULL) {
    ret = ptable_initialize_adj(prob, prob->graph->ptable);
    if(ret != SIPS_OK) {
      return(ret);
    }
    prob->graph->node2 = (void *) xmalloc((prob->T + 2)*sizeof(_node2_t *));
    prob->graph->bnode = create_benv(sizeof(_node2_t));
  } else {
    return(SIPS_FAIL);
  }

  return(SIPS_OK);
}

/*
 * lag_free(prob)
 *   frees all the allocated memory.
 *
 */
void lag_free(sips *prob)
{
  lag_free_LR1_solver(prob);
  lag_free_LR2adj_solver(prob);
  ptable_free(prob->graph->ptable);
  prob->graph->ptable = NULL;
}

/*
 * lag_free_LR1_solver(prob)
 *   frees the allocated memory for LR1 solver.
 *
 */
void lag_free_LR1_solver(sips *prob)
{
  if(prob->graph->node1 != NULL) {
    xfree(prob->graph->node1);
    prob->graph->node1 = NULL;
  }
}

/*
 * lag_free_LR2adj_solver(prob)
 *   frees the allocated memory for the (LR2adj) solver.
 *
 */
void lag_free_LR2adj_solver(sips *prob)
{
  if(prob->graph->node2 != NULL) {
    free_benv(prob->graph->bnode);
    xfree(prob->graph->node2);
    prob->graph->bnode = NULL;
    prob->graph->node2 = NULL;
  }
}

/*
 * lag_get_memory_in_MB(prob)
 *   returns memory usage of the DP states.
 *
 */
_real lag_get_memory_in_MB(sips *prob)
{
  _real mem;

  mem = 0.0;
  if(prob->graph->node1 != NULL) {
    mem += (_real) (prob->T + 1)*sizeof(_node1_t);
  }
#if 0
  mem += 2*sizeof(_node2_t)*(_real) n_nodes;
#else /* 0 */
  if(prob->graph->node2 != NULL) {
    mem += (_real) (prob->T + 2)*sizeof(_node2_t *);
    mem += DP_BLOCKSIZE*prob->graph->bnode->nb;
  }
#endif /* 0 */
  mem /= (_real) (1<<20);

  return(mem);
}

/*
 * lag_solve_LR1(prob, u, ub, sol, lb, o)
 *   solves the Lagrangian relaxation (LR1) of the original problem (P)
 *   by dynamic programming.
 *       u: multipliers
 *      ub: current upper bound
 *     sol: solution
 *      lb: objective value
 *       o: vector of the numbers of job occurrences
 *
 */
int lag_solve_LR1(sips *prob, _real *u, _real ub, _solution_t *sol,
		  _real *lb, unsigned int *o)
{
  int ret;

#ifdef SIPSI
  if(prob->graph->node1 == NULL) {
    ret = lag_initialize_LR1_solver(prob);
    if(ret != SIPS_OK) {
      return(ret);
    }
  }
  _lag_solve_LR1_forward(prob, u);
#else /* SIPSI */
  if(prob->graph->node1 == NULL) {
    ret = lag_initialize_LR1_solver(prob);
    if(ret != SIPS_OK) {
      return(ret);
    }
    _lag_solve_LR1_forward_with_domain(prob, u);
  } else {
    _lag_solve_LR1_forward(prob, u);
  }
#endif /* SIPSI */
  ret = _lag_get_sol_LR1_forward(prob, sol, lb, o);

  if(*lb > ub) {
    *lb = ub;
  }

  return(ret);
}

/*
 * lag_solve_LR2adj(prob, u, ub, sol, lb, o)
 *   solves the Lagrangian relaxation (LR2adj) by dynamic programming.
 *       u: vector of Lagrangian multipliers
 *      ub: current upper bound
 *     sol: solution
 *      lb: objective value
 *       o: vector of the numbers of job occurrences
 *
 */
int lag_solve_LR2adj(sips *prob, _real *u, _real ub, _solution_t *sol,
		     _real *lb, unsigned int *o)
{
  int ret;

  if(prob->graph->node2 == NULL) {
    ret = lag_initialize_LR2adj_solver(prob);
    if(ret != SIPS_OK) {
      return(ret);
    }
    ret = _lag_solve_LR2adj_forward_with_elimination(prob, u, ub);
    if(ret == SIPS_MEMLIMIT) {
      return(ret);
    }

    lag_free_LR1_solver(prob);
  } else {
    _lag_solve_LR2adj_forward(prob, u);
  }
  ret = _lag_get_sol_LR2adj_forward(prob, sol, lb, o);

  if(*lb > ub) {
    *lb = ub;
  }

  return(ret);
}

#ifndef SIPSI
/*
 * _lag_solve_LR1_forward_with_domain(prob, u)
 *   solves the Lagrangian relaxation (LR1) by forward DP.
 *   It checks and updates the domain of jobs and the super domain.
 *       u: vector of Lagrangian multipliers
 *
 */
void _lag_solve_LR1_forward_with_domain(sips *prob, _real *u)
{
  int t, tt;
  int i;
  int j1, j2;
  char ty, ty1, ty2;
  _real vmin1, vmin2;
  _real f;
  _ptable_t *ptable;
  _node1_t *node1;
  _node1_t *pr;

  ptable = prob->graph->ptable;
  node1 = (_node1_t *) prob->graph->node1;

  t = prob->graph->Tmin;
  node1[t].v[0] = node1[t].v[1] = 0.0;
  node1[t].j[0] = (unsigned short) N_JOBS;
  node1[t].j[1] = (unsigned short) (N_JOBS + 1);
  node1[t].ty[0] = node1[t].ty[1] = 0;

  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
    vmin1 = vmin2 = LARGE_REAL;
    j1 = j2 = N_JOBS + 1;
    ty1 = ty2 = 0;

    for(i = 0; i < prob->n; i++) {
      if(t < prob->sjob[i]->p) {
	continue;
      }

      tt = t - prob->sjob[i]->p;
      pr = node1 + tt;
      if(NOT_JOB(pr->j[0])) {
	prec_set_dom_ti(t, i);
	continue;
      }

      if(i != pr->j[0]) {
	f = pr->v[0];
	ty = 0;
      } else if(NOT_JOB(pr->j[1])) {
	prec_set_dom_ti(t, i);
	continue;
      } else {
	f = pr->v[1];
	ty = 1;
      }
      f += - u[i] + ctod(prob->sjob[i]->f[t]);

      if(f < vmin1) {
	vmin2 = vmin1;
	vmin1 = f;
	j2 = j1;
	j1 = i;
	ty2 = ty1;
	ty1 = ty;
      } else if(f < vmin2) {
	vmin2 = f;
	j2 = i;
	ty2 = ty;
      }
    }

    node1[t].v[0] = vmin1;
    node1[t].v[1] = vmin2;
    node1[t].j[0] = (unsigned short) j1;
    node1[t].j[1] = (unsigned short) j2;
    node1[t].ty[0] = ty1;
    node1[t].ty[1] = ty2;

    if(NOT_JOB(node1[t].j[0])) {
      prec_set_sd_t(t);
    }
  }
}
#endif /* !SIPSI */

/*
 * _lag_solve_LR1_forward(prob, u)
 *   solves the Lagrangian relaxation (LR1) by forward DP.
 *       u: vector of Lagrangian multipliers
 *
 */
void _lag_solve_LR1_forward(sips *prob, _real *u)
{
  int t, tt;
  int i;
  int j1, j2;
  char ty, ty1, ty2;
  _real vmin1, vmin2;
  _real f;
#ifndef SIPSI
  _ptable_t *ptable;
#endif /* !SIPSI */
  _node1_t *node1;
  _node1_t *pr;

#ifndef SIPSI
  ptable = prob->graph->ptable;
#endif /* !SIPSI */
  node1 = (_node1_t *) prob->graph->node1;

  t = prob->graph->Tmin;
  node1[t].v[0] = node1[t].v[1] = 0.0;
  node1[t].j[0] = (unsigned short) N_JOBS;
  node1[t].j[1] = (unsigned short) (N_JOBS + 1);
  node1[t].ty[0] = node1[t].ty[1] = 0;

  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
#ifdef SIPSI
    vmin1 = node1[t - 1].v[0];
    vmin2 = LARGE_REAL;
    j1 = prob->n;
    j2 = N_JOBS + 1;
#else /* SIPSI */
    if(prec_get_sd_t(t)) {
      continue;
    }
    vmin1 = vmin2 = LARGE_REAL;
    j1 = j2 = N_JOBS + 1;
#endif /* SIPSI */
    ty1 = ty2 = 0;

    for(i = 0; i < prob->n; i++) {
#ifdef SIPSI
      if(t < prob->sjob[i]->r + prob->sjob[i]->p) {
	continue;
      }
#else /* SIPSI */
      if(prec_get_dom_ti(t, i)) {
	continue;
      }
#endif /* SIPSI */

      tt = t - prob->sjob[i]->p;
      pr = node1 + tt;
      if(NOT_JOB(pr->j[0])) {
	continue;
      }

      if(i != pr->j[0]) {
	f = pr->v[0];
	ty = 0;
      } else if(NOT_JOB(pr->j[1])) {
	continue;
      } else {
	f = pr->v[1];
	ty = 1;
      }
      f += - u[i] + ctod(prob->sjob[i]->f[t]);

      if(f < vmin1) {
	vmin2 = vmin1;
	vmin1 = f;
	j2 = j1;
	j1 = i;
	ty2 = ty1;
	ty1 = ty;
      } else if(f < vmin2) {
	vmin2 = f;
	j2 = i;
	ty2 = ty;
      }
    }

    node1[t].v[0] = vmin1;
    node1[t].v[1] = vmin2;
    node1[t].j[0] = (unsigned short) j1;
    node1[t].j[1] = (unsigned short) j2;
    node1[t].ty[0] = ty1;
    node1[t].ty[1] = ty2;
  }
}

/*
 * lag_solve_LR1_backward(prob, u, ub)
 *   solves the Lagrangian relaxation (LR1) by backward DP.
 *   State elimination is also performed.
 *       u: vector of Lagrangian multipliers
 *      ub: current upper bound for state elimination
 *
 */
void lag_solve_LR1_backward(sips *prob, _real *u, _real ub)
{
  int t, tt;
  int i;
  int j1, j2;
  char ty, ty1, ty2;
  _real vmin1, vmin2;
  _real f, g;
  _ptable_t *ptable;
  _node1_t *node1;
  _node1_t *pr;

  ptable = prob->graph->ptable;
  node1 = (_node1_t *) prob->graph->node1;
  t = prob->graph->Tmax;
  node1[t].v[0] = node1[t].v[1] = 0.0;
  node1[t].j[0] = (unsigned short) N_JOBS;
  node1[t].j[1] = (unsigned short) (N_JOBS + 1);
  node1[t].ty[0] = node1[t].ty[1] = 0;

  for(t = prob->graph->Tmax - 1; t >= prob->graph->Tmin; t--) {
#ifdef SIPSI
    if(ub - (node1[t + 1].v[0] + node1[t].v[0]) < prob->param->lbeps) {
      prec_set_dom_ti(t + 1, prob->n);
      vmin1 = LARGE_REAL;
      j1 = N_JOBS + 1;
    } else {
      vmin1 = node1[t + 1].v[0];
      j1 = prob->n;
    }
    vmin2 = LARGE_REAL;
    j2 = N_JOBS + 1;
#else /* SIPSI */
    if(prec_get_sd_t(t)) {
      continue;
    }
    if(NOT_JOB(node1[t].j[0])) {
      for(i = 0; i < N_JOBS; i++) {
	if((tt = t + prob->sjob[i]->p) <= prob->graph->Tmax) {
	  prec_set_dom_ti(tt, i);
	}
      }
      prec_set_sd_t(t);
      continue;
    }
    vmin1 = vmin2 = LARGE_REAL;
    j1 = j2 = N_JOBS + 1;
#endif /* SIPSI */
    ty1 = ty2 = 0;

    for(i = 0; i < prob->n; i++) {
      if((tt = t + prob->sjob[i]->p) > prob->graph->Tmax) {
	continue;
      }
#ifndef SIPSI
      if(prec_get_dom_ti(tt, i)) {
	continue;
      }
#endif /* !SIPSI */
      pr = node1 + tt;
      if(NOT_JOB(pr->j[0])) {
	prec_set_dom_ti(tt, i);
	continue;
      }

      if(i != pr->j[0]) {
	f = pr->v[0];
	ty = 0;
      } else if(NOT_JOB(pr->j[1])) {
	prec_set_dom_ti(tt, i);
	continue;
      } else {
	f = pr->v[1];
	ty = 1;
      }
      f += - u[i] + ctod(prob->sjob[i]->f[tt]);

      if(i != node1[t].j[0]) {
	g = node1[t].v[0];
      } else if(NOT_JOB(node1[t].j[1])) {
	prec_set_dom_ti(tt, i);
	continue;
      } else {
	g = node1[t].v[1];
      }

      if(ub - (f + g) < prob->param->lbeps) {
	prec_set_dom_ti(tt, i);
	continue;
      }

      if(f < vmin1) {
	vmin2 = vmin1;
	vmin1 = f;
	j2 = j1;
	j1 = i;
	ty2 = ty1;
	ty1 = ty;
      } else if(f < vmin2) {
	vmin2 = f;
	j2 = i;
	ty2 = ty;
      }
    }

    node1[t].v[0] = vmin1;
    node1[t].v[1] = vmin2;
    node1[t].j[0] = (unsigned short) j1;
    node1[t].j[1] = (unsigned short) j2;
    node1[t].ty[0] = ty1;
    node1[t].ty[1] = ty2;

#ifndef SIPSI
    if(NOT_JOB(node1[t].j[0])) {
      prec_set_sd_t(t);
    }
#endif /* !SIPSI */
  }
}

/*
 * _lag_solve_LR2adj_forward(prob, u)
 *   solves the Lagrangian relaxation (LR2adj) by forward DP.
 *       u: vector of Lagrangian multipliers
 *
 */
void _lag_solve_LR2adj_forward(sips *prob, _real *u)
{
  int t, tt;
  int i, nn;
  char ty, ty1, ty2;
  _real f, g;
  _real v1, v2;
  unsigned int n_nodes;
  _benv_t *bnode;
  _ptable_t *ptable;
  _node2_t **node;
#ifdef SIPSI
  _node2_t *cnode, *cnode2, **pnode;
#else /* SIPSI */
  _node2_t *cnode, **pnode;
#endif /* SIPSI */
  _node2_t *pr1, *pr2, *pr;

  bnode = prob->graph->bnode;
  ptable = prob->graph->ptable;
  node = (_node2_t **) prob->graph->node2;

  free_bmemory(bnode);

  memset((void *) node, 0, (prob->T + 2)*sizeof(_node2_t *));
  node[prob->graph->Tmin] = cnode = (_node2_t *) alloc_memory(bnode);
  n_nodes = 0;

  cnode->j = (unsigned short) N_JOBS;
  cnode->v[0] = 0.0;
  cnode->v[1] = LARGE_REAL;
  cnode->n[0] = cnode;
  cnode->n[1] = NULL;
  cnode->ty = 0;
  cnode->next = NULL;
  n_nodes++;

#ifdef SIPSI
  nn = prob->n + 1;
#else /* SIPSI */
  nn = prob->n;
#endif /* SIPSI */
  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
#ifndef SIPSI
    if(prec_get_sd_t(t)) {
      continue;
    }
#endif /* !SIPSI */
    pnode = node + t;
    for(i = 0; i < nn; i++) {
      if(prec_get_dom_ti(t, i)) {
	continue;
      }
      if((tt = t - prob->sjob[i]->p) < prob->graph->Tmin) {
	prec_set_dom_ti(t, i);
	continue;
      }

      v1 = v2 = LARGE_REAL;
      pr1 = pr2 = NULL;
      ty1 = ty2 = 0;
      g = - u[i] + ctod(prob->sjob[i]->f[t]);
      for(pr = node[tt]; pr != NULL; pr = pr->next) {
	if(prec_get_adj_tij1(t, pr->j, i)) {
	  continue;
	}
	if(IS_REAL_JOB(i) && i == pr->n[0]->j) {
	  if(pr->n[1] == NULL) {
	    prec_set_adj_tij(t, pr->j, i);
	    continue;
	  }
	  ty = 1;
	  f = g + pr->v[1];
	} else {
	  ty = 0;
	  f = g + pr->v[0];
	}

	if(f < v1) {
	  v2 = v1;
	  v1 = f;
	  pr2 = pr1;
	  pr1 = pr;
	  ty2 = ty1;
	  ty1 = ty;
	} else if(f < v2) {
	  v2 = f;
	  pr2 = pr;
	  ty2 = ty;
	}
      }

      if(pr1 == NULL) {
	prec_set_dom_ti(t, i);
	continue;
      }

      *pnode = cnode = (_node2_t *) alloc_memory(bnode);
      cnode->j = (unsigned short) i;
      cnode->v[0] = v1;
      cnode->v[1] = v2;
      cnode->n[0] = pr1;
      cnode->n[1] = pr2;
      cnode->ty = CONV_TYPE(ty1, ty2);
      cnode->next = NULL;
      pnode = &(cnode->next);
      n_nodes++;
    }

#ifndef SIPSI
    if(node[t] == NULL) {
      prec_set_sd_t(t);
    }
#endif /* !SIPSI */
  }

  if(node[prob->graph->Tmax] == NULL) {
    prob->graph->n_nodes = n_nodes;
    return;
  }

#ifdef SIPSI
  for(t = prob->graph->Tmax; node[t]->j >= prob->n && node[t]->next == NULL;
      t -= prob->sjob[node[t]->j]->p);
  for(tt = prob->graph->Tmax; tt > t; tt--) {
    for(cnode = node[tt]; cnode != NULL; cnode = cnode2) {
      cnode2 = cnode->next;
      free_memory(bnode, cnode);
      n_nodes--;
    }    
    node[tt] = NULL;
  }
  prob->graph->Tmax = t;
#endif /* SIPSI */

  v1 = LARGE_REAL;
  pr1 = NULL;
  for(pr = node[prob->graph->Tmax]; pr != NULL; pr = pr->next) {
    if(pr->v[0] < v1) {
      v1 = pr->v[0];
      pr1 = pr;
    }
  }

  node[prob->graph->Tmax + 1] = cnode = (_node2_t *) alloc_memory(bnode);
  cnode->j = (unsigned short) N_JOBS;
  cnode->v[0] = v1;
  cnode->v[1] = LARGE_REAL;
  cnode->n[0] = pr1;
  cnode->n[1] = NULL;
  cnode->ty = 0;
  cnode->next = NULL;
  n_nodes++;

  prob->graph->n_nodes = n_nodes;
}

/*
 * lag_solve_LR2adj_backward(prob, u, ub)
 *   solves the Lagrangian relaxation (LR2adj) by backward DP.
 *   State elimination is also performed.
 *       u: vector of Lagrangian multipliers
 *      ub: current upper bound for state elimination
 *
 */
int lag_solve_LR2adj_backward(sips *prob, _real *u, _real ub)
{
  int t, tt;
  int i;
  char ty, ty1, ty2;
  _real f, g, fr;
  _real v1, v2;
  unsigned int n_nodes;
  _ptable_t *ptable;
  _node2_t **node;
  _node2_t *cnode, *cnode2, **pnode;
  _node2_t *pr1, *pr2, *pr;

  n_nodes = prob->graph->n_nodes;
  ptable = prob->graph->ptable;
  node = (_node2_t **) prob->graph->node2;
  if(node[prob->graph->Tmax + 1] == NULL) {
    return(SIPS_INFEASIBLE);
  }

  pr = node[prob->graph->Tmin];
  node[prob->graph->Tmin] = NULL;
  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
    cnode = node[t];
    node[t] = NULL;
    while(cnode != NULL) {
      tt = t - prob->sjob[cnode->j]->p;
      cnode2 = cnode->next;
      cnode->next = node[tt];
      node[tt] = cnode;
      cnode = cnode2;
    }
  }

  node[prob->graph->Tmax] = node[prob->graph->Tmax + 1];
  node[prob->graph->Tmax]->v[0] = 0.0;
  node[prob->graph->Tmax]->n[0] = node[prob->graph->Tmax];
  node[prob->graph->Tmax + 1] = pr;

  for(t = prob->graph->Tmax - 1; t >= prob->graph->Tmin; t--) {
    pnode = node + t;
    while(*pnode != NULL) {
      cnode = *pnode;
      i = cnode->j;
      tt = t + prob->sjob[i]->p;

      v1 = v2 = LARGE_REAL;
      pr1 = pr2 = NULL;
      ty1 = ty2 = 0;
      g = - u[i] + ctod(prob->sjob[i]->f[tt]);

      for(pr = node[tt]; pr != NULL; pr = pr->next) {
	if(prec_get_adj_tij2(tt + prob->sjob[pr->j]->p, i, pr->j)) {
	  continue;
	} else if(IS_REAL_JOB(i) && i == pr->n[0]->j) {
	  if(pr->n[1] == NULL) {
	    prec_set_adj_tij(tt + prob->sjob[pr->j]->p, i, pr->j);
	    continue;
	  }
	  ty = 1;
	  f = pr->v[1];
	} else {
	  ty = 0;
	  f = pr->v[0];
	}

	if(IS_REAL_JOB(pr->j) && pr->j == cnode->n[0]->j) {
	  fr = cnode->v[1];
	} else {
	  fr = cnode->v[0];
	}

	if(ub - (f + fr) < prob->param->lbeps) {
	  if(pr->j != N_JOBS) {
	    prec_set_adj_tij(tt + prob->sjob[pr->j]->p, i, pr->j);
	  }
	  continue;
	}

	f += g;

	if(f < v1) {
	  v2 = v1;
	  v1 = f;
	  pr2 = pr1;
	  pr1 = pr;
	  ty2 = ty1;
	  ty1 = ty;
	} else if(f < v2) {
	  v2 = f;
	  pr2 = pr;
	  ty2 = ty;
	}
      }

      if(pr1 == NULL) {
	prec_set_dom_ti(tt, i);
	*pnode = cnode->next; 
	n_nodes--;
	continue;
      }

      cnode->v[0] = v1;
      cnode->v[1] = v2;
      cnode->n[0] = pr1;
      cnode->n[1] = pr2;
      cnode->ty = CONV_TYPE(ty1, ty2);

      pnode = &(cnode->next);
    }
  }

#ifdef SIPSI
  if(node[prob->graph->Tmin] != NULL) {
    for(t = prob->graph->Tmin; node[t]->j >= prob->n && node[t]->next == NULL;
	t += prob->sjob[node[t]->j]->p);
    prob->graph->Tmin = t;
  }
#endif /* SIPSI */

  prob->graph->n_nodes = n_nodes;

  if(_lag_check_time_window(prob) == SIPS_INFEASIBLE) {
    return(SIPS_INFEASIBLE);
  }

  return(SIPS_NORMAL);
}

/*
 * _lag_solve_LR2adj_forward_with_elimination(prob, u, ub)
 *   solves the Lagrangian relaxation (LR2adj) by forward DP.
 *   State elimination is also performed by using the DP states for (LR1).
 *       u: vector of Lagrangian multipliers
 *      ub: current upper bound for state elimination
 *
 */
int _lag_solve_LR2adj_forward_with_elimination(sips *prob, _real *u,
					       _real ub)
{
  int t, tt;
  int i, nn;
  char ty, ty1, ty2;
  _real f, g, fr;
  _real v1, v2;
  unsigned int n_nodes;
  _benv_t *bnode;
  _ptable_t *ptable;
  _node1_t *node1;
  _node2_t **node;
#ifdef SIPSI
  _node2_t *cnode, *cnode2, **pnode;
#else /* SIPSI */
  _node2_t *cnode, **pnode;
#endif /* SIPSI */
  _node2_t *pr1, *pr2, *pr;

  bnode = prob->graph->bnode;
  ptable = prob->graph->ptable;
  node1 = (_node1_t *) prob->graph->node1;
  node = (_node2_t **) prob->graph->node2;

  memset((void *) node, 0, (prob->T + 2)*sizeof(_node2_t *));
  node[prob->graph->Tmin] = cnode = (_node2_t *) alloc_memory(bnode);
  n_nodes = 0;

  cnode->j = (unsigned short) N_JOBS;
  cnode->v[0] = 0.0;
  cnode->v[1] = LARGE_REAL;
  cnode->n[0] = cnode;
  cnode->n[1] = NULL;
  cnode->ty = 0;
  cnode->next = NULL;
  n_nodes++;

#ifdef SIPSI
  nn = prob->n + 1;
#else /* SIPSI */
  nn = prob->n;
#endif /* SIPSI */
  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
#ifndef SIPSI
    if(prec_get_sd_t(t)) {
      continue;
    }
#endif /* !SIPSI */
    pnode = node + t;
    for(i = 0; i < nn; i++) {
      if(prec_get_dom_ti(t, i)) {
	continue;
      }
      tt = t - prob->sjob[i]->p;

      v1 = v2 = LARGE_REAL;
      pr1 = pr2 = NULL;
      ty1 = ty2 = 0;
      g = - u[i] + ctod(prob->sjob[i]->f[t]);
      for(pr = node[tt]; pr != NULL; pr = pr->next) {
	if(prec_get_adj_tij1(t, pr->j, i)) {
	  continue;
	}
	if(IS_REAL_JOB(i) && i == pr->n[0]->j) {
	  if(pr->n[1] == NULL) {
	    prec_set_adj_tij(t, pr->j, i);
	    continue;
	  }	
	  ty = 1;
	  f = g + pr->v[1];
	} else {
	  ty = 0;
	  f = g + pr->v[0];
	}

	if(IS_REAL_JOB(i) && i == node1[t].j[0]) {
	  fr = node1[t].v[1];
	} else {
	  fr = node1[t].v[0];
	}

	if(ub - (f + fr) < prob->param->lbeps) {
	  if(IS_SOURCE_OR_SINK(pr->j)) {
	    prec_set_dom_ti(t, i);
	  } else {
	    prec_set_adj_tij(t, pr->j, i);
	  }
	  continue;
	}

	if(f < v1) {
	  v2 = v1;
	  v1 = f;
	  pr2 = pr1;
	  pr1 = pr;
	  ty2 = ty1;
	  ty1 = ty;
	} else if(f < v2) {
	  v2 = f;
	  pr2 = pr;
	  ty2 = ty;
	}
      }

      if(pr1 == NULL) {
	prec_set_dom_ti(t, i);
	continue;
      }

      *pnode = cnode = (_node2_t *) alloc_memory(bnode);
      cnode->j = (unsigned short) i;
      cnode->v[0] = v1;
      cnode->v[1] = v2;
      cnode->n[0] = pr1;
      cnode->n[1] = pr2;
      cnode->ty = CONV_TYPE(ty1, ty2);
      cnode->next = NULL;
      pnode = &(cnode->next);
      n_nodes++;
    }

    if(lag_get_memory_in_MB(prob) + prec_get_memory_in_MB(prob)
       > (_real) prob->param->mem) {
      prob->graph->n_nodes = n_nodes;
      return(SIPS_MEMLIMIT);
    }
#ifndef SIPSI
    if(node[t] == NULL) {
      prec_set_sd_t(t);
    }
#endif /* !SIPSI */
  }

  if(node[prob->graph->Tmax] == NULL) {
    prob->graph->n_nodes = n_nodes;
    return(SIPS_INFEASIBLE);
  }

#ifdef SIPSI
  for(t = prob->graph->Tmax; node[t]->j >= prob->n && node[t]->next == NULL;
      t -= prob->sjob[node[t]->j]->p);
  for(tt = prob->graph->Tmax; tt > t; tt--) {
    for(cnode = node[tt]; cnode != NULL; cnode = cnode2) {
      cnode2 = cnode->next;
      free_memory(bnode, cnode);
      n_nodes--;
    }    
    node[tt] = NULL;
  }
  prob->graph->Tmax = t;
#endif /* SIPSI */

  v1 = LARGE_REAL;
  pr1 = NULL;
  for(pr = node[prob->graph->Tmax]; pr != NULL; pr = pr->next) {
    if(pr->v[0] < v1) {
      v1 = pr->v[0];
      pr1 = pr;
    }
  }

  node[prob->graph->Tmax + 1] = cnode = (_node2_t *) alloc_memory(bnode);
  cnode->j = (unsigned short) N_JOBS;
  cnode->v[0] = v1;
  cnode->v[1] = LARGE_REAL;
  cnode->n[0] = pr1;
  cnode->n[1] = NULL;
  cnode->ty = 0;
  cnode->next = NULL;
  n_nodes++;

  prob->graph->n_nodes = n_nodes;

  return(SIPS_NORMAL);
}

/*
 * _lag_check_time_window(prob)
 *   checks and updates time windows of jobs.
 *
 */
int _lag_check_time_window(sips *prob)
{
  int t, s, e;
  int i;
#ifndef CONST_PROP_SIMPLE_IN_STAGE2
  _window_t *w;
#endif /* !CONST_PROP_SIMPLE_IN_STAGE2 */
  _ptable_t *ptable;

  ptable = prob->graph->ptable;

  for(i = 0; i < prob->n; i++) {
    s = prob->graph->Tmax + 1;;
    for(t = max(ptable->window[i].s, prob->graph->Tmin);
	t <= ptable->window[i].e && t <= prob->graph->Tmax; t++) {
      if(!prec_get_dom_ti(t, i)) {
	s = t;
	break;
      }
    }

    if(s == prob->graph->Tmax + 1) {
      return(SIPS_INFEASIBLE);
    }

    e = s;
    for(t = min(ptable->window[i].e, prob->graph->Tmax); t > s; t--) {
      if(!prec_get_dom_ti(t, i)) {
	e = t;
	break;
      }
    }

    ptable->window[i].s = s;
    ptable->window[i].e = e;
  }

#ifdef CONST_PROP_SIMPLE_IN_STAGE2
  if(constraint_propagation_simple(prob, ptable->window) == SIPS_INFEASIBLE) {
    return(SIPS_INFEASIBLE);
  }
#else /* CONST_PROP_SIMPLE_IN_STAGE2 */
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
#endif /* CONST_PROP_SIMPLE_IN_STAGE2 */

  return(SIPS_NORMAL);
}

/*
 * _lag_get_sol_LR1_forward(prob, sol, lb, o)
 *   returns the current solution of (LR1) from forward DP states.
 *       sol: solution
 *        lb: objective value
 *         o: vector of the numbers of job occurrences
 *
 */
int _lag_get_sol_LR1_forward(sips *prob, _solution_t *sol, _real *lb,
			     unsigned int *o)
{
  int t;
  int i;
  int ty, ret;
  _node1_t *node1;

  node1 = (_node1_t *) prob->graph->node1;
  *lb = node1[prob->graph->Tmax].v[0];
  if(NOT_JOB(node1[prob->graph->Tmax].j[0])) {
    *lb = LARGE_REAL;
    return(SIPS_INFEASIBLE);
  }

  ty = 0;
  ret = SIPS_OPTIMAL;
  if(sol == NULL) {
    if(o == NULL) {
      return(SIPS_NORMAL);
    }

    memset((void *) o, 0, prob->n*sizeof(unsigned int));

    for(t = prob->graph->Tmax; t > prob->graph->Tmin;) {
      i = node1[t].j[ty];
      ty = (int) node1[t].ty[ty];
      t -= prob->sjob[i]->p;
      if(IS_REAL_JOB(i)) {
	o[i]++;
      }
    }

    for(i = 0; i < prob->n; i++) {
      if(o[i] != 1) {
	ret = SIPS_NORMAL;
	break;
      }
    }

    return(ret);
  }

  sol->n = 0;
  sol->f = ZERO_COST;
  if(o != NULL) {
    memset((void *) o, 0, prob->n*sizeof(unsigned int));

    for(t = prob->graph->Tmax; t > prob->graph->Tmin;) {
      i = node1[t].j[ty];
      ty = (int) node1[t].ty[ty];
      if(IS_REAL_JOB(i)) {
	o[i]++;
	sol->f += prob->sjob[i]->f[t];
	sol->job[sol->n] = prob->sjob[i];
	sol->c[sol->n++] = t;
      }
      t -= prob->sjob[i]->p;
    }

    for(i = 0; i < prob->n; i++) {
      if(o[i] != 1) {
	ret = SIPS_NORMAL;
	break;
      }
    }
  } else {
    for(t = prob->graph->Tmax; t > prob->graph->Tmin;) {
      i = node1[t].j[ty];
      ty = (int) node1[t].ty[ty];
      if(IS_REAL_JOB(i)) {
	sol->f += prob->sjob[i]->f[t];
	sol->job[sol->n] = prob->sjob[i];
	sol->c[sol->n++] = t;
      }
      t -= prob->sjob[i]->p;
    }

    ret = SIPS_NORMAL;
  }

  reverse_solution(sol);

  return(ret);
}

#if 0
/*
 * _lag_get_sol_LR1_backward(prob, sol, lb, o)
 *   returns the current solution of (LR1) from backward DP states.
 *       sol: solution
 *        lb: objective value
 *         o: vector of the numbers of job occurrences
 *
 */
int _lag_get_sol_LR1_backward(sips *prob, _solution_t *sol, _real *lb,
			      unsigned int *o)
{
  int t;
  int i;
  int ty, ret;
  _node1_t *node1;

  node1 = (_node1_t *) prob->graph->node1;
  *lb = node1[prob->graph->Tmin].v[0];
  if(NOT_JOB(node1[prob->graph->Tmin].j[0])) {
    *lb = LARGE_REAL;
    return(SIPS_INFEASIBLE);
  }

  ty = 0;
  ret = SIPS_OPTIMAL;
  if(sol == NULL) {
    if(o == NULL) {
      return(SIPS_NORMAL);
    }

    memset((void *) o, 0, prob->n*sizeof(unsigned int));

    for(t = prob->graph->Tmin; t < prob->graph->Tmax;) {
      i = node1[t].j[ty];
      ty = (int) node1[t].ty[ty];
      t += prob->sjob[i]->p;
      if(IS_REAL_JOB(i)) {
	o[i]++;
      }
    }

    for(i = 0; i < prob->n; i++) {
      if(o[i] != 1) {
	ret = SIPS_NORMAL;
	break;
      }
    }

    return(ret);
  }

  sol->n = 0;
  sol->f = ZERO_COST;
  if(o != NULL) {
    memset((void *) o, 0, prob->n*sizeof(unsigned int));

    for(t = prob->graph->Tmin; t < prob->graph->Tmax;) {
      i = node1[t].j[ty];
      ty = (int) node1[t].ty[ty];
      t += prob->sjob[i]->p;
      if(IS_REAL_JOB(i)) {
	o[i]++;
	sol->f += prob->sjob[i]->f[t];
	sol->job[sol->n] = prob->sjob[i];
	sol->c[sol->n++] = t;
      }
    }

    for(i = 0; i < prob->n; i++) {
      if(o[i] != 1) {
	ret = SIPS_NORMAL;
	break;
      }
    }
  } else {
    for(t = prob->graph->Tmin; t < prob->graph->Tmax;) {
      i = node1[t].j[ty];
      ty = (int) node1[t].ty[ty];
      t += prob->sjob[i]->p;
      if(IS_REAL_JOB(i)) {
	sol->f += prob->sjob[i]->f[t];
	sol->job[sol->n] = prob->sjob[i];
	sol->c[sol->n++] = t;
      }
    }

    ret = SIPS_NORMAL;
  }

  return(ret);
}
#endif /* 0 */

/*
 * _lag_get_sol_LR2adj_forward(prob, sol, lb, o)
 *   returns the current solution of (LR2adj) from forward DP states.
 *       sol: solution
 *        lb: objective value
 *         o: vector of the numbers of job occurrences
 *
 */
int _lag_get_sol_LR2adj_forward(sips *prob, _solution_t *sol, _real *lb,
				unsigned int *o)
{
  int t;
  int i;
  int ty, pty;
  int ret;
  _node2_t **node, *cnode;

  node = (_node2_t **) prob->graph->node2;
  cnode = node[prob->graph->Tmax + 1];
  if(cnode == NULL) {
    *lb = LARGE_REAL;
    if(sol != NULL) {
      sol->f = LARGE_COST;
    }
    return(SIPS_INFEASIBLE);
  }
  *lb = cnode->v[0];
  if(*lb > LARGE_REAL2) {
    if(sol != NULL) {
      sol->f = LARGE_COST;
    }
    return(SIPS_INFEASIBLE);
  }
  ty = 0;
  cnode = cnode->n[0];
  t = prob->graph->Tmax;

  ret = SIPS_OPTIMAL;
  if(sol == NULL) {
    if(o == NULL) {
      return(SIPS_NORMAL);
    }

    memset((void *) o, 0, prob->n*sizeof(unsigned int));

    while(IS_JOB(cnode->j)) {
      if(IS_REAL_JOB(cnode->j)) {
	o[cnode->j]++;
      }
      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    for(i = 0; i < prob->n; i++) {
      if(o[i] != 1) {
	ret = SIPS_NORMAL;
	break;
      }
    }

    return(ret);
  }

  sol->n = 0;
  sol->f = ZERO_COST;

  if(o != NULL) {
    memset((void *) o, 0, prob->n*sizeof(unsigned int));

    while(IS_JOB(cnode->j)) {
      if(IS_REAL_JOB(cnode->j)) {
	o[cnode->j]++;
	sol->f += prob->sjob[cnode->j]->f[t];
	sol->job[sol->n] = prob->sjob[cnode->j];
	sol->c[sol->n++] = t;
      }
      t -= prob->sjob[cnode->j]->p;

      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    for(i = 0; i < prob->n; i++) {
      if(o[i] != 1) {
	ret = SIPS_NORMAL;
	break;
      }
    }
  } else {
    while(IS_JOB(cnode->j)) {
      if(IS_REAL_JOB(cnode->j)) {
	sol->f += prob->sjob[cnode->j]->f[t];
	sol->job[sol->n] = prob->sjob[cnode->j];
	sol->c[sol->n++] = t;
      }
      t -= prob->sjob[cnode->j]->p;

      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    ret = SIPS_NORMAL;
  }

  reverse_solution(sol);

  return(ret);
}

#if 0
/*
 * _lag_get_sol_LR2adj_backward(prob, sol, lb, o)
 *   returns the current solution of (LR2adj) from backward DP states.
 *       sol: solution
 *        lb: objective value
 *         o: vector of the numbers of job occurrences
 *
 */
int _lag_get_sol_LR2adj_backward(sips *prob, _solution_t *sol, _real *lb,
				 unsigned int *o)
{
  int t;
  int i
  int ty, pty;
  int ret;
#ifdef SIPSI
  _node2_t **node, *cnode, *cnode2;
#else /* SIPSI */
  _node2_t **node, *cnode;
#endif /* SIPSI */

  node = (_node2_t **) prob->graph->node2;
  *lb = LARGE_REAL;
  if(node[prob->graph->Tmin] == NULL) {
    if(sol != NULL) {
      sol->f = LARGE_COST;
    }
    return(SIPS_INFEASIBLE);
  }

  cnode = NULL;
  for(cnode2 = node[prob->graph->Tmin]; cnode2 != NULL;
      cnode2 = cnode2->next) {
    if(*lb > cnode2->v[0]) {
      *lb = cnode2->v[0];
      cnode = cnode2;
    }
  }

  if(*lb > LARGE_REAL2) {
    if(sol != NULL) {
      sol->f = LARGE_COST;
    }
    return(SIPS_INFEASIBLE);
  }
  ty = 0;
  t = prob->graph->Tmin;

  ret = SIPS_OPTIMAL;
  if(sol == NULL) {
    if(o == NULL) {
      return(SIPS_NORMAL);
    }

    memset((void *) o, 0, prob->n*sizeof(unsigned int));

    while(IS_JOB(cnode->j)) {
      if(IS_REAL_JOB(cnode->j)) {
	o[cnode->j]++;
      }
      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    for(i = 0; i < prob->n; i++) {
      if(o[i] != 1) {
	ret = SIPS_NORMAL;
	break;
      }
    }

    return(ret);
  }

  sol->n = 0;
  sol->f = ZERO_COST;

  if(o != NULL) {
    memset((void *) o, 0, prob->n*sizeof(unsigned int));

    while(IS_JOB(cnode->j)) {
      t += prob->sjob[cnode->j]->p;
      if(IS_REAL_JOB(cnode->j)) {
	o[cnode->j]++;
	sol->f += prob->sjob[cnode->j]->f[t];
	sol->job[sol->n] = prob->sjob[cnode->j];
	sol->c[sol->n++] = t;
      }

      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    for(i = 0; i < prob->n; i++) {
      if(o[i] != 1) {
	ret = SIPS_NORMAL;
	break;
      }
    }
  } else {
    while(IS_JOB(cnode->j)) {
      t += prob->sjob[cnode->j]->p;
      if(IS_REAL_JOB(cnode->j)) {
	sol->f += prob->sjob[cnode->j]->f[t];
	sol->job[sol->n] = prob->sjob[cnode->j];
	sol->c[sol->n++] = t;
      }

      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    ret = SIPS_NORMAL;
  }

  return(ret);
}
#endif /* 0 */
