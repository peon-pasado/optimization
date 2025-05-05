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
 *  $Id: lag2.c,v 1.36 2016/04/01 15:05:26 tanaka Rel $
 *  $Revision: 1.36 $
 *  $Date: 2016/04/01 15:05:26 $
 *  $Author: tanaka $
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "define.h"
#include "sips_common.h"
#include "bmemory.h"
#include "constraint.h"
#include "dominance.h"
#include "fixed.h"
#include "lag.h"
#include "lag2.h"
#include "lag_common.h"
#include "memory.h"
#include "objective.h"
#include "print.h"
#include "ptable.h"
#include "sol.h"

typedef struct {
  unsigned short j;
  unsigned int no;
  unsigned int nd;
} _occur_t;

static int _lag2_initialize_node(sips *, _real *);
static int _lag2_initialize_node_forward(sips *, _real *);
static void _lag2_recover_nodes_forward(sips *);
static int _lag2_add_modifiers_LR2m_forward(sips *, _real *, _real, _mod_t *,
					    unsigned char);
static int _lag2_add_modifiers_LR2m_backward(sips *, _real *, _real, _mod_t *,
					     unsigned char);
static int _lag2_solve_LR2m_forward(sips *, _real *, _real, unsigned char);
static int _lag2_solve_LR2m_backward(sips *, _real *, _real, unsigned char);
static int _lag2_get_sol_LR2m(sips *, _real *, _real, _solution_t *, _real *,
			      unsigned int *);
static int _lag2_get_sol_LR2m_forward(sips *, _real *, _solution_t *, _real *,
				      unsigned int *);
static int _lag2_get_sol_LR2m_backward(sips *, _real *, _solution_t *, _real *,
				       unsigned int *);
static int _lag2_check_time_window(sips *);
static void _lag2_move_edges_head(sips *);
static void _lag2_move_edges_tail(sips *);
static void _lag2_free_eliminated_nodes(sips *);
static int _lag2_shrink_horizon_head(sips *, _real *);
static int _lag2_shrink_horizon_tail(sips *, _real *);

static char _check_supernode(sips *, unsigned short *, int);
static char _check_adj_supernode_forward(sips *, _node2m_t *, _node2m_t *,
					 int);
static char _check_adj_supernode_backward(sips *, _node2m_t *, _node2m_t *,
					  int);
static char _check_none(sips *, int, int, _node2m_t *);
static char _check_three_forward(sips *, int, int, _node2m_t *);
static char _check_four_forward(sips *, int, int, _node2m_t *);
static char _check_five_forward(sips *, int, int, _node2m_t *);
static char _check_six_forward(sips *, int, int, _node2m_t *);
static int _compare_occur(const void *, const void *);
static int _compare_occur2(const void *, const void *);

/*
 * lag2_initialize(prob, u, ub, sol, lb, o)
 *   initializes the solver for (LR2m).
 *        u: Lagrangian multipliers
 *       ub: current upper bound
 *      sol: solution of the relaxation
 *       lb: lower bound
 *        o: numbers of job occurrences
 *
 */
int lag2_initialize(sips *prob, _real *u, _real ub, _solution_t *sol,
		    _real *lb, unsigned int *o)
{
  int ret;

  if(prob->graph->bedge == NULL) {
    ret = _lag2_initialize_node(prob, u);
    if(ret != SIPS_MEMLIMIT && ret != SIPS_INFEASIBLE) {
      ret = _lag2_get_sol_LR2m(prob, u, ub, sol, lb, o);
    }
  } else {
    ret = SIPS_NORMAL;
  }

  return(ret);
}

/*
 * _lag2_initialize_node(prob, u)
 *   allocates the memory for DP states.
 *        u: Lagrangian multipliers
 *
 */
int _lag2_initialize_node(sips *prob, _real *u)
{
  int ret;

  if(prob->graph->bedge == NULL) {
    prob->graph->n_nodes = prob->graph->n_edges = 0;
    prob->graph->bnode = create_benv(sizeof(_node2m_t));
    prob->graph->bedge = create_benv(sizeof(_edge2m_t));
    prob->graph->fixed = create_fixed(prob);
    prob->graph->node2 = (void *) xmalloc((prob->T + 2)*sizeof(_node2m_t *));
    ret = _lag2_initialize_node_forward(prob, u);
    ptable_free_adj(prob->graph->ptable);

    if(ret == SIPS_NORMAL) {
      ret = _lag2_check_time_window(prob);
    }
  } else {
    ret = SIPS_NORMAL;
  }

  return(ret);
}

/*
 * _lag2_initialize_node_forward(prob, u)
 *   initializes the forward DP states for (LR2m).
 *        u: Lagrangian multipliers
 *
 */
int _lag2_initialize_node_forward(sips *prob, _real *u) {
  int t, tt;
  int i;
  char ty, ty1, ty2;
  _real f, g;
  _real v1, v2;
  unsigned int n_nodes, n_edges;
  _ptable_t *ptable;
  _benv_t *bnode, *bedge;
  _node2m_t **node, *cnode, *cnode2, *pr, *pr1, *pr2;
  _edge2m_t *e, *e2;
  char (*check_func)(sips *, int, int, _node2m_t *);

  switch(prob->param->dlevel) {
  case 6:
    /* check six succesiive jobs */
    check_func = _check_six_forward;
    break;
  case 5:
    /* check five succesiive jobs */
    check_func = _check_five_forward;
    break;
  case 4:
    /* check four succesiive jobs */
    check_func = _check_four_forward;
    break;
  case 3:
    /* check three succesiive jobs */
    check_func = _check_three_forward;
    break;
  default:
    /* no check */
    check_func = _check_none;
    break;
  }

  n_nodes = 0;
  n_edges = prob->graph->n_edges;
  bnode = prob->graph->bnode;
  bedge = prob->graph->bedge;
  ptable = prob->graph->ptable;
  node = (_node2m_t **) prob->graph->node2;
  prob->graph->direction = SIPS_FORWARD;

  memset((void *) ptable->occ, 0, (N_JOBS + 2)*sizeof(unsigned int));

  memset((void *) node, 0, (prob->T + 2)*sizeof(_node2m_t *));

  node[prob->graph->Tmin] = cnode = (_node2m_t *) alloc_memory(bnode);
  n_nodes++;
  memset((void *) cnode, 0, sizeof(_node2m_t));
  cnode->j[0] = (unsigned short) N_JOBS;
  cnode->j[1] = (unsigned short) (N_JOBS + 1);
  cnode->j[2] = (unsigned short) (N_JOBS + 1);
  cnode->n[0] = cnode;

  for(tt = prob->graph->Tmin; tt < prob->graph->Tmax; tt++) {
    if(prec_get_sd_t(tt)) {
      continue;
    } else if(node[tt] == NULL) {
      prec_set_sd_t(tt);
      continue;
    }

    for(i = 0; i < prob->n; i++) {
      t = tt + prob->sjob[i]->p;
      if(t > prob->graph->Tmax || prec_get_sd_t(t) || prec_get_dom_ti(t, i)) {
	      continue;
      }

      g = - u[i] + ctod(prob->sjob[i]->f[t]);
      v1 = v2 = LARGE_REAL;
      pr1 = pr2 = NULL;
      ty1 = ty2 = 0;
      e = NULL;
      for(cnode2 = node[tt]; cnode2 != NULL; cnode2 = cnode2->next) {
        if(prec_get_adj_tij1(t, cnode2->j[0], i)) {
          continue;
        }

        if(check_func(prob, t, i, cnode2)) {
          continue;
        }

        if(cnode2->n[0]->j[0] == i) {
          if(cnode2->n[1] == NULL) {
            continue;
          }
          f = cnode2->v[1] + g;
          ty = 1;
        } else {
          f = cnode2->v[0] + g;
          ty = 0;
        }

        if(f < v1) {
          v2 = v1;
          v1 = f;
          pr2 = pr1;
          pr1 = cnode2;
          ty2 = ty1;
          ty1 = ty;
        } else if(f < v2) {
          v2 = f;
          pr2 = cnode2;
          ty2 = ty;
        }

        e2 = (_edge2m_t *) alloc_memory(bedge);
        n_edges++;
        e2->n = cnode2;
        e2->next = e;
        e = e2;
      }

      if(e != NULL) {
        cnode = (_node2m_t *) alloc_memory(bnode);
        ptable->occ[i]++;
        n_nodes++;
        cnode->j[0] = (unsigned short) i;
        cnode->j[1] = (unsigned short) (N_JOBS + 1);
        cnode->j[2] = (unsigned short) (N_JOBS + 1);
        cnode->v[0] = v1;
        cnode->v[1] = v2;
        cnode->ty = CONV_TYPE(ty1, ty2);
        cnode->n[0] = pr1;
        cnode->n[1] = pr2;
        cnode->next = node[t];
        node[t] = cnode;
        cnode->e = e;
      } else {
	      prec_set_dom_ti(t, i);
      }
    }

    if(lag_get_memory_in_MB(prob) + lag2_get_real_memory_in_MB(prob) > (_real) prob->param->mem) {
      prob->graph->n_nodes = n_nodes;
      prob->graph->n_edges = n_edges;
      return(SIPS_MEMLIMIT);
    }
  }

  node[prob->graph->Tmax + 1] = cnode = (_node2m_t *) alloc_memory(bnode);
  n_nodes++;
  cnode->j[0] = (unsigned short) N_JOBS;
  cnode->j[1] = (unsigned short) (N_JOBS + 1);
  cnode->j[2] = (unsigned short) (N_JOBS + 1);
  cnode->next = NULL;
  cnode->e = NULL;
  f = LARGE_REAL;
  pr = NULL;
  for(cnode2 = node[prob->graph->Tmax]; cnode2 != NULL; cnode2 = cnode2->next) {
    e = (_edge2m_t *) alloc_memory(bedge);
    n_edges++;
    e->n = cnode2;
    e->next = cnode->e;
    cnode->e = e;
    if(f > cnode2->v[0]) {
      f = cnode2->v[0];
      pr = cnode2;
    }
  }

  cnode->v[0] = f;
  cnode->v[1] = LARGE_REAL;
  cnode->n[0] = pr;
  cnode->n[1] = NULL;
  cnode->ty = 0;

  prob->graph->n_nodes = n_nodes;
  prob->graph->n_edges = n_edges;

  if(cnode->e == NULL) {
    return(SIPS_INFEASIBLE);
  } else if(lag_get_memory_in_MB(prob) + lag2_get_real_memory_in_MB(prob) > (_real) prob->param->mem) {
    return(SIPS_MEMLIMIT);
  }

  for(i = 0; i < prob->n; i++) {
    for(t = ptable->window[i].s; t <= ptable->window[i].e; t++) {
      if(!prec_get_dom_ti(t, i)) {
	      ptable->window[i].s = t;
	      break;
      }
    }

    if(t > ptable->window[i].e) {
      return(SIPS_INFEASIBLE);
    }

    for(t = ptable->window[i].e; t >= ptable->window[i].s; t--) {
      if(!prec_get_dom_ti(t, i)) {
	      ptable->window[i].e = t;
	      break;
      }
    }
  }

  return(_lag2_shrink_horizon_tail(prob, u));
}

/*
 * lag2_free(prob)
 *   frees the allocated memory for the DP solver.
 *
 */
void lag2_free(sips *prob)
{
  if(prob->graph != NULL) {
    free_benv(prob->graph->bedge);
    free_benv(prob->graph->bnode);
    xfree(prob->graph->node2);
    xfree(prob->graph->node1);
    ptable_free(prob->graph->ptable);
    free_fixed(prob->graph->fixed);
    prob->graph->bedge = NULL;
    prob->graph->bnode = NULL;
    prob->graph->node2 = NULL;
    prob->graph->node1 = NULL;
    prob->graph->ptable = NULL;
    prob->graph->fixed = NULL;
    prob->graph->n_nodes = 0;
    prob->graph->n_edges = 0;
  }
}

/*
 * lag2_free_copy(prob)
 *   frees the backup of the DP states.
 *
 */
void lag2_free_copy(sips *prob)
{
  if(prob->graph != NULL && prob->graph->copy->bnode != NULL) {
    free_benv(prob->graph->copy->bedge);
    free_benv(prob->graph->copy->bnode);
    xfree(prob->graph->copy->node2);
    ptable_free(prob->graph->copy->ptable);
    free_fixed(prob->graph->copy->fixed);

    prob->graph->copy->bedge = NULL;
    prob->graph->copy->bnode = NULL;
    prob->graph->copy->node2 = NULL;
    prob->graph->copy->ptable = NULL;
    prob->graph->copy->fixed = NULL;
    prob->graph->copy->n_nodes = 0;
    prob->graph->copy->n_edges = 0;
    prob->graph->copy->Tmin = 0;
    prob->graph->copy->Tmax = 0;
    prob->graph->copy->mem = 0.0;
    prob->graph->copy->rmem = 0.0;
  }
}

/*
 * lag2_push_nodes(sips)
 *   moves the original DP states to the backup area.
 *
 */
int lag2_push_nodes(sips *prob)
{
  free_benv(prob->graph->copy->bnode);
  free_benv(prob->graph->copy->bedge);
  xfree(prob->graph->copy->node2);
  ptable_free(prob->graph->copy->ptable);
  free_fixed(prob->graph->copy->fixed);

  prob->graph->copy->bnode = prob->graph->bnode;
  prob->graph->copy->bedge = prob->graph->bedge;
  prob->graph->copy->node2 = prob->graph->node2;
  prob->graph->copy->ptable = prob->graph->ptable;
  prob->graph->copy->fixed = prob->graph->fixed;

  prob->graph->copy->direction = prob->graph->direction;
  prob->graph->copy->n_nodes = prob->graph->n_nodes;
  prob->graph->copy->n_edges = prob->graph->n_edges;
  prob->graph->copy->Tmin = prob->graph->Tmin;
  prob->graph->copy->Tmax = prob->graph->Tmax;
  prob->graph->copy->mem = lag2_get_memory_in_MB(prob);
  prob->graph->copy->rmem = lag2_get_real_memory_in_MB(prob);

  prob->graph->bnode = NULL;
  prob->graph->bedge = NULL;
  prob->graph->node2 = NULL;
  prob->graph->ptable = NULL;
  prob->graph->fixed = NULL;
  prob->graph->Tmin = 0;
  prob->graph->Tmax = 0;
  prob->graph->n_nodes = 0;
  prob->graph->n_edges = 0;

  return(SIPS_OK);
}

/*
 * lag2_pop_nodes(sips)
 *   recovers the DP states from their backup and deletes the backup.
 *
 */
int lag2_pop_nodes(sips *prob)
{
  free_benv(prob->graph->bnode);
  free_benv(prob->graph->bedge);
  xfree(prob->graph->node2);
  ptable_free(prob->graph->ptable);
  free_fixed(prob->graph->fixed);

  prob->graph->bnode = prob->graph->copy->bnode;
  prob->graph->bedge = prob->graph->copy->bedge;
  prob->graph->node2 = prob->graph->copy->node2;
  prob->graph->ptable = prob->graph->copy->ptable;
  prob->graph->fixed = prob->graph->copy->fixed;

  prob->graph->direction = prob->graph->copy->direction;
  prob->graph->n_nodes = prob->graph->copy->n_nodes;
  prob->graph->n_edges = prob->graph->copy->n_edges;
  prob->graph->Tmin = prob->graph->copy->Tmin;
  prob->graph->Tmax = prob->graph->copy->Tmax;

  prob->graph->copy->bnode = NULL;
  prob->graph->copy->bedge = NULL;
  prob->graph->copy->node2 = NULL;
  prob->graph->copy->ptable = NULL;
  prob->graph->copy->fixed = NULL;
  prob->graph->copy->Tmin = 0;
  prob->graph->copy->Tmax = 0;
  prob->graph->copy->n_nodes = 0;
  prob->graph->copy->n_edges = 0;
  prob->graph->copy->mem = 0.0;
  prob->graph->copy->rmem = 0.0;

  return(SIPS_OK);
}

/*
 * lag2_recover_nodes(sips)
 *   recovers the DP states from their backup.
 *   The backup is preserved.
 *
 */
int lag2_recover_nodes(sips *prob)
{
  if(prob->graph->copy->node2 == NULL) {
    return(SIPS_FAIL);
  }

  if(2.0*prob->graph->copy->rmem > (_real) prob->param->mem) {
    return(SIPS_MEMLIMIT);
  }

  free_benv(prob->graph->bnode);
  free_benv(prob->graph->bedge);
  xfree(prob->graph->node2);
  ptable_free(prob->graph->ptable);
  free_fixed(prob->graph->fixed);

  prob->graph->n_nodes = prob->graph->copy->n_nodes;
  prob->graph->n_edges = prob->graph->copy->n_edges;
  prob->graph->Tmin = prob->graph->copy->Tmin;
  prob->graph->Tmax = prob->graph->copy->Tmax;
  prob->graph->direction = prob->graph->copy->direction;
  prob->graph->ptable = duplicate_ptable(prob, prob->graph->copy->ptable);
  prob->graph->fixed = duplicate_fixed(prob, prob->graph->copy->fixed);

  _lag2_recover_nodes_forward(prob);

  return(SIPS_OK);
}

/*
 * _lag2_recover_nodes_forward(sips)
 *   recovers the forward DP states from their backup.
 *   The backup is preserved.
 *
 */
void _lag2_recover_nodes_forward(sips *prob)
{
  int t, tt, tt2;
  int i;
  _node2m_t **snode, **node, ***ntable;
  _node2m_t *csnode, **pnode, *cnode;
  _edge2m_t *e, **pe;
  _benv_t *bnode, *bedge;

  snode = (_node2m_t **) prob->graph->copy->node2;
  node = (void *) xcalloc(prob->T + 2, sizeof(_node2m_t *));
  bnode = create_benv(sizeof(_node2m_t));
  bedge = create_benv(sizeof(_edge2m_t));

  ntable = (_node2m_t ***) xmalloc((prob->pmax + 1)*sizeof(_node2m_t **));
  ntable[0] = (_node2m_t **) xmalloc((prob->pmax + 1)*(N_JOBS + 1)
				     *sizeof(_node2m_t *));
  for(i = 1; i <= prob->pmax; i++) {
    ntable[i] = ntable[i - 1] + N_JOBS + 1;
  }

  node[prob->graph->Tmin] = cnode = alloc_memory(bnode);
  memset((void *) cnode, 0, sizeof(_node2m_t));
  cnode->j[0] = snode[prob->graph->Tmin]->j[0];
  cnode->j[1] = snode[prob->graph->Tmin]->j[1];
  cnode->j[2] = snode[prob->graph->Tmin]->j[2];
  cnode->n[0] = cnode;
  memset((void *) ntable[prob->graph->Tmin%(prob->pmax + 1)], 0,
	 (N_JOBS + 1)*sizeof(_node2m_t *));
  ntable[prob->graph->Tmin%(prob->pmax + 1)][cnode->j[0]] = cnode;

  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
    pnode = node + t;
    tt = t%(prob->pmax + 1);
    memset((void *) ntable[tt], 0, (N_JOBS + 1)*sizeof(_node2m_t *));

    for(csnode = snode[t]; csnode != NULL; csnode = csnode->next) {
      tt2 = (t - prob->sjob[csnode->j[0]]->p)%(prob->pmax + 1);

      *pnode = cnode = alloc_memory(bnode);
      cnode->j[0] = csnode->j[0];
      cnode->j[1] = csnode->j[1];
      cnode->j[2] = csnode->j[2];
      cnode->ty = csnode->ty;
      cnode->v[0] = csnode->v[0];
      cnode->v[1] = csnode->v[1];

      if(csnode->n[0] != NULL) {
	cnode->n[0] = ntable[tt2][csnode->n[0]->j[0]];
	if(csnode->n[1] != NULL) {
	  cnode->n[1] = ntable[tt2][csnode->n[1]->j[0]];
	} else {
	  cnode->n[1] = NULL;
	}
      } else {
	cnode->n[0] = NULL;
	cnode->n[1] = NULL;
      }

      for(pe = &(cnode->e), e = csnode->e; e != NULL;
	  e = e->next, pe = &((*pe)->next)) {
	*pe = alloc_memory(bedge);
	(*pe)->n = ntable[tt2][e->n->j[0]];
      }
      *pe = NULL;

      ntable[tt][cnode->j[0]] = cnode;
      pnode = &(cnode->next);      
    }
    *pnode = NULL;
  }

  tt2 = prob->graph->Tmax%(prob->pmax + 1);
  csnode = snode[prob->graph->Tmax + 1];

  node[prob->graph->Tmax + 1] = cnode = alloc_memory(bnode);
  cnode->j[0] = csnode->j[0];
  cnode->j[1] = csnode->j[1];
  cnode->j[2] = csnode->j[2];
  cnode->ty = csnode->ty;
  cnode->v[0] = csnode->v[0];
  cnode->v[1] = csnode->v[1];
  cnode->n[0] = ntable[tt2][csnode->n[0]->j[0]];
  cnode->n[1] = NULL;
  cnode->next = NULL;

  for(pe = &(cnode->e), e = csnode->e; e != NULL;
      e = e->next, pe = &((*pe)->next)) {
    *pe = alloc_memory(bedge);
    (*pe)->n = ntable[tt2][e->n->j[0]];
  }
  *pe = NULL;

  prob->graph->node2 = (void **) node;
  prob->graph->bnode = bnode;
  prob->graph->bedge = bedge;

  xfree(ntable[0]);
  xfree(ntable);
}

/*
 * lag2_add_modifiers_LR2m(prob, u, ub, sol, lb, o, mod)
 *   adds modifiers and reconstructs the DP states.
 *   It also solves the relaxation (LR2m).
 *        u: Lagrangian multipliers
 *       ub: upper bound
 *      sol: solution of the relaxation
 *       lb: lower bound
 *        o: numbers of job occurrences
 *      mod: modifiers to be added
 *
 */
int lag2_add_modifiers_LR2m(sips *prob, _real *u, _real ub, _solution_t *sol,
			    _real *lb, unsigned int *o, _mod_t *mod)
{
  int ret;

  if(mod->an == 0) {
    return(SIPS_NORMAL);
  }

  sol->n = 0;
  if(prob->graph->direction == SIPS_FORWARD) {
    ret = _lag2_add_modifiers_LR2m_forward(prob, u, ub, mod, 1);
  } else {
    ret = _lag2_add_modifiers_LR2m_backward(prob, u, ub, mod, 1);
  }

  if(ret == SIPS_INFEASIBLE) {
    *lb = ub;
  } else if(ret != SIPS_MEMLIMIT) {
    ret = _lag2_get_sol_LR2m(prob, u, ub, sol, lb, o);
  }

  return(ret);
}

/*
 * lag2_add_modifiers_LR2m_without_elimination(prob, u, ub, sol, lb, o, mod)
 *   adds modifiers and reconstructs the DP states.
 *   It also solves the relaxation (LR2m) without state elimination.
 *        u: Lagrangian multipliers
 *       ub: upper bound
 *      sol: solution of the relaxation
 *       lb: lower bound
 *        o: numbers of job occurrences
 *      mod: modifiers to be added
 *
 */
int lag2_add_modifiers_LR2m_without_elimination(sips *prob, _real *u, _real ub,
						_solution_t *sol, _real *lb,
						unsigned int *o, _mod_t *mod)
{
  int ret;

  if(mod->an == 0) {
    return(SIPS_NORMAL);
  }

  sol->n = 0;
  if(prob->graph->direction == SIPS_FORWARD) {
    ret = _lag2_add_modifiers_LR2m_forward(prob, u, ub, mod, 0);
  } else {
    ret = _lag2_add_modifiers_LR2m_backward(prob, u, ub, mod, 0);
  }

  if(ret == SIPS_INFEASIBLE) {
    *lb = ub;
  } else if(ret != SIPS_MEMLIMIT) {
    ret = _lag2_get_sol_LR2m(prob, u, ub, sol, lb, o);
  }

  return(ret);
}

#define HAVE_JOB(xjob, xnode)					\
  (xjob == xnode->j[2] || xjob == xnode->j[1] || xjob == xnode->j[0])

#ifdef SIPSI
#define HAVE_REAL_JOB(xjob, xnode)				\
  (IS_REAL_JOB(xjob) && HAVE_JOB(xjob, xnode))

#define HAVE_COMMON_REAL_JOB(xnode1, xnode2)			\
  (HAVE_REAL_JOB(xnode1->j[0], xnode2)				\
   || HAVE_REAL_JOB(xnode1->j[1], xnode2)			\
   || HAVE_REAL_JOB(xnode1->j[2], xnode2))
#else /* SIPSI */
#define HAVE_REAL_JOB(xjob, xnode)				\
  (IS_JOB(xjob) && HAVE_JOB(xjob, xnode))

#define HAVE_COMMON_REAL_JOB(xnode1, xnode2)			\
  (HAVE_JOB(xnode1->j[0], xnode2)				\
   || HAVE_REAL_JOB(xnode1->j[1], xnode2)			\
   || HAVE_REAL_JOB(xnode1->j[2], xnode2))
#endif /* SIPSI */

#define REMOVE_SINGLE_EDGE {			\
    e = (*pe)->next;				\
    free_memory(bedge, *pe);			\
    *pe = e;					\
    n_edges--;					\
  }

#define REMOVE_EDGES(xnode) {				\
    for(e = xnode->e; e != NULL; e = e2) {		\
      e2 = e->next;					\
      free_memory(bedge, e);				\
      n_edges--;					\
    }							\
    xnode->e = NULL;					\
  }

#define UPDATE_WINDOWS {					\
    ptable->window[cnode->j[0]].s				\
      = min(ptable->window[cnode->j[0]].s, t);			\
    ptable->window[cnode->j[0]].e				\
      = max(ptable->window[cnode->j[0]].e, t);			\
    ptable->occ[cnode->j[0]]++;					\
    if(IS_JOB(cnode->j[1])) {					\
      tt = t - prob->sjob[cnode->j[0]]->p;			\
      ptable->window[cnode->j[1]].s				\
	= min(ptable->window[cnode->j[1]].s, tt);		\
      ptable->window[cnode->j[1]].e				\
	= max(ptable->window[cnode->j[1]].e, tt);		\
      ptable->occ[cnode->j[1]]++;				\
      if(IS_JOB(cnode->j[2])) {					\
	tt -= prob->sjob[cnode->j[1]]->p;			\
	ptable->window[cnode->j[2]].s				\
	  = min(ptable->window[cnode->j[2]].s, tt);		\
	ptable->window[cnode->j[2]].e				\
	  = max(ptable->window[cnode->j[2]].e, tt);		\
	ptable->occ[cnode->j[2]]++;				\
      }								\
    }								\
  }

/*
 * _lag2_add_modifiers_LR2m_forward(prob, u, ub, mod, eflag)
 *   adds modifiers and reconstructs the DP states.
 *   It also solves the relaxation (LR2m) by forward DP.
 *        u: Lagrangian multipliers
 *       ub: upper bound
 *      mod: modifiers to be added
 *    eflag: perform state elimination (1)
 *
 */
int _lag2_add_modifiers_LR2m_forward(sips *prob, _real *u, _real ub,
				     _mod_t *mod, unsigned char eflag)
{
  int t, tt;
#ifdef SIPSI
  int i, dlen;
#else /* SIPSI */
  int i;
#endif /* SIPSI */
  int m, mm, mmax;
  char flag, ty;
  _real f, g, g2;
  unsigned int n_nodes, n_edges;
#ifndef WITHOUT_MTABLE
  unsigned char *mtable;
#endif /* !WITHOUT_MTABLE */
  _ptable_t *ptable;
  _benv_t *bnode, *bedge;
  _node2m_t **node, *cnode, *cnode2, *cnode3, *cnode4, **pnode, **nnode;
  _edge2m_t *e, *e2, **pe;

  n_nodes = prob->graph->n_nodes;
  n_edges = prob->graph->n_edges;
  node = (_node2m_t **) prob->graph->node2;
  ptable = prob->graph->ptable;
  bnode = prob->graph->bnode;
  bedge = prob->graph->bedge;

#ifndef WITHOUT_MTABLE
  mtable
    = (unsigned char *) xcalloc(prob->graph->Tmax - prob->graph->Tmin + 1, 1);
  for(i = 0; i < mod->an; i++) {
    m = 1<<i;
    for(t = max(prob->graph->Tmin, ptable->window[mod->jobs[i]].e
		- prob->sjob[mod->jobs[i]]->p + 1);
	t <= prob->graph->Tmax; t++) {
      mtable[t - prob->graph->Tmin] |= m;
    }
  }
#endif /* !WITHOUT_MTABLE */

  memset((void *) ptable->occ, 0, (N_JOBS + 2)*sizeof(unsigned int));
  for(i = 0; i < N_JOBS; i++) {
    ptable->window[i].s = prob->graph->Tmax + 1;
    ptable->window[i].e = prob->graph->Tmin;
  }
  for(i = 0; i < prob->graph->fixed->nhead; i++) {
    ptable->window[prob->graph->fixed->job[i]->no].s
      = ptable->window[prob->graph->fixed->job[i]->no].e
      = prob->graph->fixed->c[i];
  }
  for(i = 0; i < prob->graph->fixed->ntail; i++) {
    ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].s
      = ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].e
      = prob->graph->fixed->c[prob->n - i - 1];
  }

  mmax = (1<<mod->an) - 1;
  nnode = (_node2m_t **) xmalloc((mmax + 1)*sizeof(_node2m_t *));

  node[prob->graph->Tmin]->m = 0;
  node[prob->graph->Tmin]->n[0] = node[prob->graph->Tmin];
  node[prob->graph->Tmin]->ty = 0;
  node[prob->graph->Tmin]->v[0] = ctod(prob->graph->fixed->fhead);
  for(i = 0; i < prob->graph->fixed->nhead; i++) {
    node[prob->graph->Tmin]->v[0] -= u[prob->graph->fixed->job[i]->no];
  }
  CONV_START_NODE(node[prob->graph->Tmin]);

  node[prob->graph->Tmax + 1]->m = mmax;

  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
    pnode = node + t;
    while(*pnode != NULL) {
      cnode = *pnode;
      if(ELIMINATED_NODE(cnode)) {
	      REMOVE_EDGES(cnode);
	      pnode = &(cnode->next);
	      continue;
      }
      if(cnode->e == NULL) {
	      ELIMINATE_NODE(cnode);
	      pnode = &(cnode->next);
	      continue;
      }

      if(IS_REAL_JOB(cnode->j[0]) && prec_get_dom_ti(t, cnode->j[0])) {
	      REMOVE_EDGES(cnode);
	      ELIMINATE_NODE(cnode);
	      pnode = &(cnode->next);
	      continue;
      }

      g = - u[cnode->j[0]] + ctod(prob->sjob[cnode->j[0]]->f[t]);
      mm = mod->m[cnode->j[0]];
      if(IS_JOB(cnode->j[1])) {
        tt = t - prob->sjob[cnode->j[0]]->p;
        if(IS_REAL_JOB(cnode->j[1]) && prec_get_dom_ti(tt, cnode->j[1])) {
          REMOVE_EDGES(cnode);
          ELIMINATE_NODE(cnode);
          pnode = &(cnode->next);
          continue;
        }
        g += - u[cnode->j[1]] + ctod(prob->sjob[cnode->j[1]]->f[tt]);
        mm |= mod->m[cnode->j[1]];

        if(IS_JOB(cnode->j[2])) {
          tt -= prob->sjob[cnode->j[1]]->p;
          if(IS_REAL_JOB(cnode->j[2]) && prec_get_dom_ti(tt, cnode->j[2])) {
            REMOVE_EDGES(cnode);
            ELIMINATE_NODE(cnode);
            pnode = &(cnode->next);
            continue;
          }
          g += - u[cnode->j[2]] + ctod(prob->sjob[cnode->j[2]]->f[tt]);
          mm |= mod->m[cnode->j[2]];
        }
      }

      for(m = mm; m <= mmax; m++) {
        nnode[m] = cnode2 = (_node2m_t *) alloc_memory(bnode);
        n_nodes++;

        cnode2->m = m;
        cnode2->j[0] = cnode->j[0];
        cnode2->j[1] = cnode->j[1];
        cnode2->j[2] = cnode->j[2];
        cnode2->e = NULL;
        cnode2->next = NULL;
        cnode2->v[0] = cnode2->v[1] = LARGE_REAL;
        cnode2->n[0] = cnode2->n[1] = NULL;
        cnode2->ty = 0;
      }

      e = cnode->e;
      while(e != NULL) {
        cnode2 = e->n;
        if(ELIMINATED_NODE(cnode2)) {
          e2 = e->next;
          free_memory(bedge, e);
          n_edges--;
          e = e2;
          continue;
        }

	for(flag = 0; cnode2 != NULL; cnode2 = cnode2->next) {
	  if(CHECK_START_OR_ELIMINATED_NODE(cnode2)) {
	    if(++flag == 2) {
	      break;
	    }
	  }

	  if(cnode2->m & mm) {
	    continue;
	  }

	  m = cnode2->m | mod->m[cnode->j[2]];
	  if(mod->v[cnode->j[2]][m] == 1) {
	    continue;
	  }
	  m |= mod->m[cnode->j[1]];
	  if(mod->v[cnode->j[1]][m] == 1) {
	    continue;
	  }
	  m |= mod->m[cnode->j[0]];
	  if(mod->v[cnode->j[0]][m] == 1) {
	    continue;
	  }
#ifdef WITHOUT_MTABLE
	  if(t == prob->graph->Tmax && m != mmax) {
	    continue;
	  }
#else /* WITHOUT_MTABLE */
	  if((~m)&mtable[t - prob->graph->Tmin]) {
	    continue;
	  }
#endif /* WITHOUT_MTABLE */

	  if(HAVE_COMMON_REAL_JOB(cnode, cnode2)) {
	    continue;
	  }

	  cnode3 = cnode2->n[0];
	  if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
	    if(cnode2->n[1] == NULL) {
	      continue;
	    } else if(cnode2->e->next->next == NULL) {
	      cnode3 = cnode2->n[1];
	      if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
		continue;
	      }
	    }
	    ty = 1;
	    f = cnode2->v[1];
	  } else {
	    ty = 0;
	    f = cnode2->v[0];
	  }

	  if(eflag) {
	    if(ub - (f + cnode->v[0]) < prob->param->lbeps) {
	      continue;
	    } else if(ub - (f + cnode->v[1]) < prob->param->lbeps) {
	      cnode3 = cnode->n[0];
	      if(HAVE_COMMON_REAL_JOB(cnode2, cnode3)) {
		continue;
	      }
	    }
	    if(_check_adj_supernode_forward(prob, cnode2, cnode, t)) {
	      continue;
	    }
	  }

	  f += g;
	  cnode3 = nnode[m];
	  if(f < cnode3->v[0]) {
	    cnode3->v[1] = cnode3->v[0];
	    cnode3->v[0] = f;
	    cnode3->n[1] = cnode3->n[0];
	    cnode3->n[0] = cnode2;
	    cnode3->ty <<= 1;
	    cnode3->ty |= ty;
	  } else if(f < cnode3->v[1]) {
	    cnode3->v[1] = f;
	    cnode3->n[1] = cnode2;
	    cnode3->ty &= 0x1;
	    cnode3->ty |= ty << 1;
	  }

	  e2 = (_edge2m_t *) alloc_memory(bedge);
	  n_edges++;
	  e2->n = cnode2;
	  e2->next = cnode3->e;
	  cnode3->e = e2;
	}

	e2 = e->next;
	free_memory(bedge, e);
	n_edges--;
	e = e2;
      }

      for(m = mm; m <= mmax; m++) {
        if(nnode[m]->e == NULL) {
          free_memory(bnode, nnode[m]);
          n_nodes--;
          nnode[m] = NULL;
          continue;
        }

        cnode2 = nnode[m];
        cnode3 = cnode2->e->n;
#ifdef SIPSI
	if(eflag && cnode2->e->next == NULL && IS_JOB(cnode3->j[0])) {
	  if(IS_JOB(cnode2->j[2])) {
	    if(IS_REAL_JOB(cnode2->j[2]) || IS_REAL_JOB(cnode3->j[0])
	       || IS_JOB(cnode3->j[1])) {
	      continue;
	    }
	    dlen = cnode2->j[2] + cnode3->j[0] + 2 - 2*prob->n;
	    if(dlen > prob->dn) {
	      continue;
	    }
	    cnode2->j[2] = (unsigned short) (prob->n + dlen - 1);
	  } else if(IS_JOB(cnode2->j[1])) {
	    if(IS_JOB(cnode3->j[2])) {
	      continue;
	    } else if(IS_REAL_JOB(cnode2->j[1]) || IS_REAL_JOB(cnode3->j[0])) {
	      if(IS_JOB(cnode3->j[1])) {
		continue;
	      }
	      cnode2->j[2] = cnode3->j[0];
	    } else {
	      dlen = cnode2->j[1] + cnode3->j[0] + 2 - 2*prob->n;
	      if(dlen > prob->dn) {
		if(IS_JOB(cnode3->j[1])) {
		  continue;
		}
		cnode2->j[1] = (unsigned short) (prob->n + prob->dn - 1);
		cnode2->j[2]
		  = (unsigned short) (prob->n + dlen - prob->dn - 1);
	      } else {
		cnode2->j[1] = (unsigned short) (prob->n + dlen - 1);
		cnode2->j[2] = cnode3->j[1];
	      }
	    }
	  } else if(IS_REAL_JOB(cnode2->j[0]) || IS_REAL_JOB(cnode3->j[0])) {
	    if(IS_JOB(cnode3->j[2])) {
	      continue;
	    }
	    cnode2->j[1] = cnode3->j[0];
	    cnode2->j[2] = cnode3->j[1];
	  } else {
	    dlen = cnode2->j[0] + cnode3->j[0] + 2 - 2*prob->n;
	    if(dlen > prob->dn) {
	      if(IS_JOB(cnode3->j[2])) {
		continue;
	      }

	      cnode2->j[0] = (unsigned short) (prob->n + prob->dn - 1);
	      cnode2->j[1]
		= (unsigned short) (prob->n + dlen - prob->dn - 1);
	      cnode2->j[2] = cnode3->j[1];
	    } else {
	      cnode2->j[0] = (unsigned short) (prob->n + dlen - 1);
	      cnode2->j[1] = cnode3->j[1];
	      cnode2->j[2] = cnode3->j[2];
	    }
	  }

	  free_memory(bedge, cnode2->e);
	  n_edges--;

	  if(IS_JOB(cnode2->j[2]) && _check_supernode(prob, cnode2->j, t)) {
	    free_memory(bnode, cnode2);
	    n_nodes--;
	    nnode[m] = NULL;
	    continue;
	  }

	  pe = &(cnode2->e);
	  if(NOT_JOB(cnode2->j[1])) {
	    cnode2->v[0] = cnode3->v[0] + g;
	    cnode2->v[1] = cnode3->v[1] + g;
	    cnode2->n[0] = cnode3->n[0];
	    cnode2->n[1] = cnode3->n[1];
	    cnode2->ty = cnode3->ty;
	    
	    for(e = cnode3->e; e != NULL; e = e->next) {
	      e2 = alloc_memory(bedge);
	      n_edges++;
	      e2->n = e->n;
	      *pe = e2;
	      pe = &(e2->next);
	    }
	    *pe = NULL;
	  } else {
	    g2 = - u[cnode2->j[0]] + ctod(prob->sjob[cnode2->j[0]]->f[t]);
	    if(IS_JOB(cnode2->j[1])) {
	      tt = t - prob->sjob[cnode2->j[0]]->p;
	      g2 += - u[cnode2->j[1]] + ctod(prob->sjob[cnode2->j[1]]->f[tt]);
	      if(IS_JOB(cnode2->j[2])) {
		tt -= prob->sjob[cnode2->j[1]]->p;
		g2 += - u[cnode2->j[2]] + ctod(prob->sjob[cnode2->j[2]]->f[tt]);
	      }
	    }

	    cnode2->v[0] = cnode2->v[1] = LARGE_REAL;
	    cnode2->n[0] = cnode2->n[1] = NULL;
	    cnode2->ty = 0;

	    for(e = cnode3->e; e != NULL; e = e->next) {
	      cnode3 = e->n;
	      if(HAVE_REAL_JOB(cnode2->j[2], cnode3)
		 || HAVE_REAL_JOB(cnode2->j[1], cnode3)
		 || HAVE_REAL_JOB(cnode2->j[0], cnode3)) {
		continue;
	      }

	      cnode4 = cnode3->n[0];
	      if(HAVE_COMMON_REAL_JOB(cnode2, cnode4)) {
		if(cnode3->n[1] == NULL) {
		  continue;
		} else if(cnode3->e->next->next == NULL) {
		  cnode4 = cnode3->n[1];
		  if(HAVE_COMMON_REAL_JOB(cnode2, cnode4)) {
		    continue;
		  }
		}
		ty = 1;
		f = cnode3->v[1] + g2;
	      } else {
		ty = 0;
		f = cnode3->v[0] + g2;
	      }

	      if(f < cnode2->v[0]) {
		cnode2->v[1] = cnode2->v[0];
		cnode2->v[0] = f;
		cnode2->n[1] = cnode2->n[0];
		cnode2->n[0] = cnode3;
		cnode2->ty <<= 1;
		cnode2->ty |= ty;
	      } else if(f < cnode2->v[1]) {
		cnode2->v[1] = f;
		cnode2->n[1] = cnode3;
		cnode2->ty &= 0x1;
		cnode2->ty |= ty << 1;
	      }

	      e2 = alloc_memory(bedge);
	      n_edges++;
	      e2->n = cnode3;
	      *pe = e2;
	      pe = &(e2->next);
	    }
	    *pe = NULL;

	    if(cnode2->e == NULL) {
	      free_memory(bnode, cnode2);
	      n_nodes--;
	      nnode[m] = NULL;
	    }
	  }
	}
#else /* SIPSI */
	if(eflag && cnode2->e->next == NULL && IS_JOB(cnode3->j[0])
	   && NOT_JOB(cnode2->j[2]) && NOT_JOB(cnode3->j[2])) {
	  if(IS_JOB(cnode2->j[1])) {
	    if(NOT_JOB(cnode3->j[1])) {
	      cnode2->j[2] = cnode3->j[0];
	    } else {
	      continue;
	    }
	  } else {
	    cnode2->j[1] = cnode3->j[0];
	    cnode2->j[2] = cnode3->j[1];
	  }

	  free_memory(bedge, cnode2->e);
	  n_edges--;

	  if(IS_JOB(cnode2->j[2]) && _check_supernode(prob, cnode2->j, t)) {
	    free_memory(bnode, cnode2);
	    n_nodes--;
	    nnode[m] = NULL;
	    continue;
	  }

	  g2 = - u[cnode2->j[0]] + ctod(prob->sjob[cnode2->j[0]]->f[t]);
	  if(IS_JOB(cnode2->j[1])) {
	    tt = t - prob->sjob[cnode2->j[0]]->p;
	    g2 += - u[cnode2->j[1]] + ctod(prob->sjob[cnode2->j[1]]->f[tt]);
	    if(IS_JOB(cnode2->j[2])) {
	      tt -= prob->sjob[cnode2->j[1]]->p;
	      g2 += - u[cnode2->j[2]] + ctod(prob->sjob[cnode2->j[2]]->f[tt]);
	    }
	  }

	  cnode2->v[0] = cnode2->v[1] = LARGE_REAL;
	  cnode2->n[0] = cnode2->n[1] = NULL;
	  cnode2->ty = 0;
	  pe = &(cnode2->e);

	  for(e = cnode3->e; e != NULL; e = e->next) {
	    cnode3 = e->n;
	    if(HAVE_REAL_JOB(cnode2->j[2], cnode3)
	       || HAVE_JOB(cnode2->j[1], cnode3)
	       || HAVE_JOB(cnode2->j[0], cnode3)) {
	      continue;
	    }

	    cnode4 = cnode3->n[0];
	    if(HAVE_JOB(cnode2->j[0], cnode4)
	       || HAVE_JOB(cnode2->j[1], cnode4)
	       || HAVE_REAL_JOB(cnode2->j[2], cnode4)) {
	      if(cnode3->n[1] == NULL) {
		continue;
	      } else if(cnode3->e->next->next == NULL) {
		cnode4 = cnode3->n[1];
		if(HAVE_JOB(cnode2->j[0], cnode4)
		   || HAVE_JOB(cnode2->j[1], cnode4)
		   || HAVE_REAL_JOB(cnode2->j[2], cnode4)) {
		  continue;
		}
	      }
	      ty = 1;
	      f = cnode3->v[1] + g2;
	    } else {
	      ty = 0;
	      f = cnode3->v[0] + g2;
	    }

	    if(f < cnode2->v[0]) {
	      cnode2->v[1] = cnode2->v[0];
	      cnode2->v[0] = f;
	      cnode2->n[1] = cnode2->n[0];
	      cnode2->n[0] = cnode3;
	      cnode2->ty <<= 1;
	      cnode2->ty |= ty;
	    } else if(f < cnode2->v[1]) {
	      cnode2->v[1] = f;
	      cnode2->n[1] = cnode3;
	      cnode2->ty &= 0x1;
	      cnode2->ty |= ty << 1;
	    }

	    e2 = alloc_memory(bedge);
	    n_edges++;
	    e2->n = cnode3;
	    *pe = e2;
	    pe = &(e2->next);
	  }
	  *pe = NULL;

	  if(cnode2->e == NULL) {
	    free_memory(bnode, cnode2);
	    n_nodes--;
	    nnode[m] = NULL;
	  }
	}
#endif /* SIPSI */
      }

      for(m = mm; m <= mmax && nnode[m] == NULL; m++);

      if(m <= mmax) {
	cnode2 = cnode->next;
	*cnode = *(nnode[m]);
	RCONV_START_NODE(cnode);
	CONV_START_NODE(cnode);

	UPDATE_WINDOWS;

	pnode = &(cnode->next);
	free_memory(bnode, nnode[m]);
	n_nodes--;

	for(m++; m <= mmax; m++) {
	  if(nnode[m] != NULL) {
	    ptable->occ[nnode[m]->j[0]]++;
	    ptable->occ[nnode[m]->j[1]]++;
	    ptable->occ[nnode[m]->j[2]]++;
	    *pnode = nnode[m];
	    RCONV_START_NODE(*pnode);
	    pnode = &(nnode[m]->next);
	  }
	}

	*pnode = cnode2;
      } else {
	cnode->e = NULL;
	ELIMINATE_NODE(cnode);
	pnode = &(cnode->next);
      }
    }

    if(lag2_get_real_memory_in_MB(prob) > (_real) prob->param->mem) {
      xfree(nnode);
#ifndef WITHOUT_MTABLE
      xfree(mtable);
#endif /* !WITHOUT_MTABLE */
      prob->graph->n_nodes = n_nodes;
      prob->graph->n_edges = n_edges;
      return(SIPS_MEMLIMIT);
    }

    if(t >= prob->graph->Tmin + 3*prob->pmax) {
      pnode = node + (t - 3*prob->pmax);
      while(*pnode != NULL) {
	if(ELIMINATED_NODE(*pnode)) {
	  cnode = (*pnode)->next;
	  free_memory(bnode, *pnode);
	  *pnode = cnode;
	  n_nodes--;
	} else {
	  RCONV_START_NODE(*pnode);
	  pnode = &((*pnode)->next);
	}
      }
    }
  }

  xfree(nnode);
#ifndef WITHOUT_MTABLE
  xfree(mtable);
#endif /* !WITHOUT_MTABLE */

  cnode = node[prob->graph->Tmax + 1];
  cnode->v[0] = cnode->v[1] = LARGE_REAL;
  cnode->n[0] = cnode->n[1] = NULL;
  cnode->ty = 0;
  pe = &(cnode->e);
  while(*pe != NULL) {
    cnode2 = (*pe)->n;
    if(ELIMINATED_NODE(cnode2)) {
      REMOVE_SINGLE_EDGE;
      continue;
    }

    if(cnode2->v[0] < cnode->v[0]) {
      cnode->v[0] = cnode2->v[0];
      cnode->n[0] = cnode2;
    }
    pe = &((*pe)->next);
  }

  if(cnode->e == NULL) {
    ELIMINATE_NODE(cnode);
    prob->graph->n_nodes = n_nodes;
    prob->graph->n_edges = n_edges;
    return(SIPS_INFEASIBLE);
  }

  for(t = max(prob->graph->Tmin, prob->graph->Tmax - 3*prob->pmax + 1);
      t <= prob->graph->Tmax; t++) {
    pnode = node + t;
    while(*pnode != NULL) {
      if(ELIMINATED_NODE(*pnode)) {
	cnode = (*pnode)->next;
	free_memory(bnode, *pnode);
	*pnode = cnode;
	n_nodes--;
      } else {
	RCONV_START_NODE(*pnode);
	pnode = &((*pnode)->next);
      }
    }
  }

  prob->graph->n_nodes = n_nodes;
  prob->graph->n_edges = n_edges;

  return(_lag2_shrink_horizon_tail(prob, u));
}

#undef UPDATE_WINDOWS
#define UPDATE_WINDOWS {					\
    ptable->window[cnode->j[0]].s				\
      = min(ptable->window[cnode->j[0]].s, t);			\
    ptable->window[cnode->j[0]].e				\
      = max(ptable->window[cnode->j[0]].e, t);			\
    ptable->occ[cnode->j[0]]++;					\
    if(IS_JOB(cnode->j[1])) {					\
      ptable->window[cnode->j[1]].s				\
	= min(ptable->window[cnode->j[1]].s, tt[0]);		\
      ptable->window[cnode->j[1]].e				\
	= max(ptable->window[cnode->j[1]].e, tt[0]);		\
      ptable->occ[cnode->j[1]]++;				\
      if(IS_JOB(cnode->j[2])) {					\
	ptable->window[cnode->j[2]].s				\
	  = min(ptable->window[cnode->j[2]].s, tt[1]);		\
	ptable->window[cnode->j[2]].e				\
	  = max(ptable->window[cnode->j[2]].e, tt[1]);		\
	ptable->occ[cnode->j[2]]++;				\
      }								\
    }								\
  }

/*
 * _lag2_add_modifiers_LR2m_backward(prob, u, ub, mod, eflag)
 *   adds modifiers and reconstructs the DP states.
 *   It also solves the relaxation (LR2m) by backward DP.
 *        u: Lagrangian multipliers
 *       ub: upper bound
 *      mod: modifiers to be added
 *    eflag: perform state elimination (1)
 *
 */
int _lag2_add_modifiers_LR2m_backward(sips *prob, _real *u, _real ub,
				      _mod_t *mod, unsigned char eflag)
{
  int t, s, tt[2];
  int i;
  int psum;
  int m, m2, mm, mmax;
  char ty, flag;
  _real f, g;
  unsigned int n_nodes, n_edges;
#ifndef WITHOUT_MTABLE
  unsigned char *mtable;
#endif /* !WITHOUT_MTABLE */
  _ptable_t *ptable;
  _benv_t *bnode, *bedge;
  _node2m_t **node, *cnode, *cnode2, *cnode3, **pnode, **nnode;
  _edge2m_t *e, *e2, **pe;

  n_nodes = prob->graph->n_nodes;
  n_edges = prob->graph->n_edges;
  node = (_node2m_t **) prob->graph->node2;
  ptable = prob->graph->ptable;
  bnode = prob->graph->bnode;
  bedge = prob->graph->bedge;

#ifndef WITHOUT_MTABLE
  mtable
    = (unsigned char *) xcalloc(prob->graph->Tmax - prob->graph->Tmin + 1, 1);
  for(i = 0; i < mod->an; i++) {
    m = 1<<i;
    for(t = prob->graph->Tmin; t <= ptable->window[mod->jobs[i]].s - 1
	  && t <= prob->graph->Tmax; t++) {
      mtable[t - prob->graph->Tmin] |= m;
    }
  }
#endif /* !WITHOUT_MTABLE */

  memset((void *) ptable->occ, 0, (N_JOBS + 2)*sizeof(unsigned int));
  for(i = 0; i < N_JOBS; i++) {
    ptable->window[i].s = prob->graph->Tmax + 1;
    ptable->window[i].e = prob->graph->Tmin;
  }
  for(i = 0; i < prob->graph->fixed->nhead; i++) {
    ptable->window[prob->graph->fixed->job[i]->no].s
      = ptable->window[prob->graph->fixed->job[i]->no].e
      = prob->graph->fixed->c[i];
  }
  for(i = 0; i < prob->graph->fixed->ntail; i++) {
    ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].s
      = ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].e
      = prob->graph->fixed->c[prob->n - i - 1];
  }

  mmax = (1<<mod->an) - 1;
  nnode = (_node2m_t **) xmalloc((mmax + 1)*sizeof(_node2m_t *));

  node[prob->graph->Tmin]->m = 0;
  node[prob->graph->Tmax + 1]->m = mmax;
  node[prob->graph->Tmax + 1]->n[0] = node[prob->graph->Tmax + 1];
  node[prob->graph->Tmax + 1]->ty = 0;
  node[prob->graph->Tmax + 1]->v[0] = ctod(prob->graph->fixed->ftail);
  for(i = 0; i < prob->graph->fixed->ntail; i++) {
    node[prob->graph->Tmax + 1]->v[0]
      -= u[prob->graph->fixed->job[prob->n - i - 1]->no];
  }
  CONV_START_NODE(node[prob->graph->Tmax + 1]);

  for(t = prob->graph->Tmax; t > prob->graph->Tmin; t--) {
    pnode = node + t;
    while(*pnode != NULL) {
      cnode = *pnode;
      if(ELIMINATED_NODE(cnode)) {
	REMOVE_EDGES(cnode);
	pnode = &(cnode->next);
	continue;
      }
      if(cnode->e == NULL) {
	ELIMINATE_NODE(cnode);
	pnode = &(cnode->next);
	continue;
      }

      if(IS_REAL_JOB(cnode->j[0]) && prec_get_dom_ti(t, cnode->j[0])) {
	REMOVE_EDGES(cnode);
	ELIMINATE_NODE(cnode);
	pnode = &(cnode->next);
	continue;
      }

      g = - u[cnode->j[0]] + ctod(prob->sjob[cnode->j[0]]->f[t]);
      psum = prob->sjob[cnode->j[0]]->p;
      mm = mod->m[cnode->j[0]];

      tt[0] = tt[1] = t;
      s = t - prob->sjob[cnode->j[0]]->p;
      if(IS_JOB(cnode->j[1])) {
	if(IS_REAL_JOB(cnode->j[1]) && prec_get_dom_ti(s, cnode->j[1])) {
	  REMOVE_EDGES(cnode);
	  ELIMINATE_NODE(cnode);
	  pnode = &(cnode->next);
	  continue;
	}
	g += - u[cnode->j[1]] + ctod(prob->sjob[cnode->j[1]]->f[s]);
	psum += prob->sjob[cnode->j[1]]->p;
	mm |= mod->m[cnode->j[1]];
	tt[0] = s;
	s -= prob->sjob[cnode->j[1]]->p;

	if(IS_JOB(cnode->j[2])) {
	  if(IS_REAL_JOB(cnode->j[2]) && prec_get_dom_ti(s, cnode->j[2])) {
	    REMOVE_EDGES(cnode);
	    ELIMINATE_NODE(cnode);
	    pnode = &(cnode->next);
	    continue;
	  }
	  g += - u[cnode->j[2]] + ctod(prob->sjob[cnode->j[2]]->f[s]);
	  psum += prob->sjob[cnode->j[2]]->p;
	  mm |= mod->m[cnode->j[2]];
	  tt[1] = s;
	  s -= prob->sjob[cnode->j[2]]->p;
	}
      }

      for(m = mm; m <= mmax; m++) {
	nnode[m] = cnode2 = (_node2m_t *) alloc_memory(bnode);
	n_nodes++;

	cnode2->m = m;
	cnode2->j[0] = cnode->j[0];
	cnode2->j[1] = cnode->j[1];
	cnode2->j[2] = cnode->j[2];
	cnode2->e = NULL;
	cnode2->next = NULL;
	cnode2->v[0] = cnode2->v[1] = LARGE_REAL;
	cnode2->n[0] = cnode2->n[1] = NULL;
	cnode2->ty = 0;
      }

      e = cnode->e;
      while(e != NULL) {
	cnode2 = e->n;
	if(ELIMINATED_NODE(cnode2)) {
	  e2 = e->next;
	  free_memory(bedge, e);
	  n_edges--;
	  e = e2;
	  continue;
	}

	for(flag = 0; cnode2 != NULL; cnode2 = cnode2->next) {
	  if(CHECK_START_OR_ELIMINATED_NODE(cnode2)) {
	    if(++flag == 2) {
	      break;
	    }
	  }

	  m2 = mod->m[cnode2->j[0]] | mod->m[cnode2->j[1]]
	    | mod->m[cnode2->j[2]];
	  m = cnode2->m ^ m2;
	  if((m & mm) != mm) {
	    continue;
	  }
	  if(mod->v[cnode->j[0]][m] == 1) {
	    continue;
	  }
	  m2 = m ^ mod->m[cnode->j[0]];
	  if(mod->v[cnode->j[1]][m2] == 1) {
	    continue;
	  }
	  m2 ^= mod->m[cnode->j[1]];
	  if(mod->v[cnode->j[2]][m2] == 1) {
	    continue;
	  }
#ifdef WITHOUT_MTABLE
	  if(t == prob->graph->Tmin + psum && m2 != mod->m[cnode->j[2]]) {
	    continue;
	  }
#else /* WITHOUT_MTABLE */
	  m2 ^= mod->m[cnode->j[2]];
	  if(m2 & mtable[s - prob->graph->Tmin]) {
	    continue;
	  }
#endif /* WITHOUT_MTABLE */

	  cnode3 = cnode2->n[0];
	  if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
	    if(cnode2->n[1] == NULL) {
	      continue;
	    } else if(cnode2->e->next->next == NULL) {
	      cnode3 = cnode2->n[1];
	      if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
		continue;
	      }
	    }
	    ty = 1;
	    f = cnode2->v[1];
	  } else {
	    ty = 0;
	    f = cnode2->v[0];
	  }

	  if(eflag) {
	    if(ub - (f + cnode->v[0]) < prob->param->lbeps) {
	      continue;
	    } else if(ub - (f + cnode->v[1]) < prob->param->lbeps) {
	      cnode3 = cnode->n[0];
	      if(HAVE_COMMON_REAL_JOB(cnode2, cnode3)) {
		continue;
	      }
	    }
	    if(_check_adj_supernode_backward(prob, cnode, cnode2, t)) {
	      continue;
	    }
	  }

	  f += g;
	  cnode3 = nnode[m];
	  if(f < cnode3->v[0]) {
	    cnode3->v[1] = cnode3->v[0];
	    cnode3->v[0] = f;
	    cnode3->n[1] = cnode3->n[0];
	    cnode3->n[0] = cnode2;
	    cnode3->ty <<= 1;
	    cnode3->ty |= ty;
	  } else if(f < cnode3->v[1]) {
	    cnode3->v[1] = f;
	    cnode3->n[1] = cnode2;
	    cnode3->ty &= 0x1;
	    cnode3->ty |= ty << 1;
	  }

	  e2 = (_edge2m_t *) alloc_memory(bedge);
	  n_edges++;
	  e2->n = cnode2;
	  e2->next = cnode3->e;
	  cnode3->e = e2;
	}

	e2 = e->next;
	free_memory(bedge, e);
	n_edges--;
	e = e2;
      }

      for(m = mm; m <= mmax; m++) {
	if(nnode[m]->e == NULL) {
	  free_memory(bnode, nnode[m]);
	  n_nodes--;
	} else {
	  break;
	}
      }

      if(m <= mmax) {
	cnode2 = cnode->next;
	*cnode = *(nnode[m]);

	UPDATE_WINDOWS;

	RCONV_START_NODE(cnode);
	CONV_START_NODE(cnode);
	pnode = &(cnode->next);
	free_memory(bnode, nnode[m]);
	n_nodes--;

	for(m++; m <= mmax; m++) {
	  if(nnode[m]->e == NULL) {
	    free_memory(bnode, nnode[m]);
	    n_nodes--;
	  } else {
	    ptable->occ[nnode[m]->j[0]]++;
	    ptable->occ[nnode[m]->j[1]]++;
	    ptable->occ[nnode[m]->j[2]]++;
	    *pnode = nnode[m];
	    RCONV_START_NODE(*pnode);
	    pnode = &(nnode[m]->next);
	  }
	}

	*pnode = cnode2;
      } else {
	cnode->e = NULL;
	ELIMINATE_NODE(cnode);
	pnode = &(cnode->next);
      }
    }

    if(lag2_get_real_memory_in_MB(prob) > (_real) prob->param->mem) {
      xfree(nnode);
#ifndef WITHOUT_MTABLE
      xfree(mtable);
#endif /* !WITHOUT_MTABLE */
      prob->graph->n_nodes = n_nodes;
      prob->graph->n_edges = n_edges;
      return(SIPS_MEMLIMIT);
    }

    if(t + 3*prob->pmax <= prob->graph->Tmax + 1) {
      pnode = node + (t + 3*prob->pmax);
      while(*pnode != NULL) {
	if(ELIMINATED_NODE(*pnode)) {
	  cnode = (*pnode)->next;
	  free_memory(bnode, *pnode);
	  *pnode = cnode;
	  n_nodes--;
	} else {
	  RCONV_START_NODE(*pnode);
	  pnode = &((*pnode)->next);
	}
      }
    }
  }

  xfree(nnode);
#ifndef WITHOUT_MTABLE
  xfree(mtable);
#endif /* !WITHOUT_MTABLE */

  cnode = node[prob->graph->Tmin];
  cnode->v[0] = cnode->v[1] = LARGE_REAL;
  cnode->n[0] = cnode->n[1] = NULL;
  cnode->ty = 0;
  pe = &(cnode->e);
  while(*pe != NULL) {
    cnode2 = (*pe)->n;
    if(ELIMINATED_NODE(cnode2)) {
      REMOVE_SINGLE_EDGE;
      continue;
    }

    if(cnode2->v[0] < cnode->v[0]) {
      cnode->v[0] = cnode2->v[0];
      cnode->n[0] = cnode2;
    }
    pe = &((*pe)->next);
  }

  if(cnode->e == NULL) {
    prob->graph->n_nodes = n_nodes;
    prob->graph->n_edges = n_edges;
    return(SIPS_INFEASIBLE);
  }

  for(t = min(prob->graph->Tmax, prob->graph->Tmin + 3*prob->pmax);
      t >= prob->graph->Tmin; t--) {
    pnode = node + t;
    while(*pnode != NULL) {
      if(ELIMINATED_NODE(*pnode)) {
	cnode = (*pnode)->next;
	free_memory(bnode, *pnode);
	*pnode = cnode;
	n_nodes--;
      } else {
	RCONV_START_NODE(*pnode);
	pnode = &((*pnode)->next);
      }
    }
  }

  prob->graph->n_nodes = n_nodes;
  prob->graph->n_edges = n_edges;

  return(_lag2_shrink_horizon_head(prob, u));
}

/*
 * lag2_solve_LR2m(prob, u, ub, sol, lb, o)
 *   solves the relaxation (LR2m) by DP.
 *        u: Lagrangian multipliers
 *       ub: upper bound
 *      sol: solution of the relaxation
 *       lb: lower bound
 *        o: numbers of job occurrences
 *
 */
int lag2_solve_LR2m(sips *prob, _real *u, _real ub, _solution_t *sol,
		    _real *lb, unsigned int *o)
{
  int ret;

  if(prob->graph->direction == SIPS_FORWARD) {
    ret = _lag2_solve_LR2m_forward(prob, u, ub, 1);
  } else {
    ret = _lag2_solve_LR2m_backward(prob, u, ub, 1);
  }

  if(ret == SIPS_INFEASIBLE) {
    *lb = ub;
  }

  return(_lag2_get_sol_LR2m(prob, u, ub, sol, lb, o));
}

/*
 * lag2_solve_LR2m_without_elimination(prob, u, ub, sol, lb, o)
 *   solves the relaxation (LR2m) by DP.  State elimination is not performed.
 *        u: Lagrangian multipliers
 *       ub: upper bound
 *      sol: solution of the relaxation
 *       lb: lower bound
 *        o: numbers of job occurrences
 *
 */
int lag2_solve_LR2m_without_elimination(sips *prob, _real *u, _real ub,
					_solution_t *sol, _real *lb,
					unsigned int *o)
{
  int ret;

  if(prob->graph->direction == SIPS_FORWARD) {
    ret = _lag2_solve_LR2m_forward(prob, u, ub, 0);
  } else {
    ret = _lag2_solve_LR2m_backward(prob, u, ub, 0);
  }

  if(ret == SIPS_INFEASIBLE) {
    *lb = ub;
  }

  return(_lag2_get_sol_LR2m(prob, u, ub, sol, lb, o));
}

/*
 * lag2_reverse(prob)
 *   reverses the direction of the DP solver.
 *
 */
void lag2_reverse(sips *prob)
{
  if(prob->graph->direction == SIPS_FORWARD) {
    _lag2_move_edges_tail(prob);
    prob->graph->direction = SIPS_BACKWARD;
  } else {
    _lag2_move_edges_head(prob);
    prob->graph->direction = SIPS_FORWARD;
  }
}

/*
 * lag2_solve_LR2m_reverse(prob, u, ub)
 *   solves the relaxation (LR2m) by DP in the reverse
 *   direction to the current direction.
 *        u: Lagrangian multipliers
 *       ub: upper bound
 *
 */
int lag2_solve_LR2m_reverse(sips *prob, _real *u, _real ub)
{
  int ret;

  if(prob->graph->direction == SIPS_FORWARD) {
    _lag2_move_edges_tail(prob);
    ret = _lag2_solve_LR2m_backward(prob, u, ub, 1);
    prob->graph->direction = SIPS_BACKWARD;
  } else {
    _lag2_move_edges_head(prob);
    ret = _lag2_solve_LR2m_forward(prob, u, ub, 1);
    prob->graph->direction = SIPS_FORWARD;
  }

  return(ret);
}

/*
 * lag2_solve_LR2m_reverse_without_elimination(prob, u, ub)
 *   solves the relaxation (LR2m) by DP in the reverse direction to
 *   the current direction.  State elimination is not performed.
 *        u: Lagrangian multipliers
 *       ub: upper bound
 *
 */
int lag2_solve_LR2m_reverse_without_elimination(sips *prob, _real *u, _real ub)
{
  int ret;

  if(prob->graph->direction == SIPS_FORWARD) {
    _lag2_move_edges_tail(prob);
    prob->graph->direction = SIPS_BACKWARD;
    ret = _lag2_solve_LR2m_backward(prob, u, ub, 0);
  } else {
    _lag2_move_edges_head(prob);
    prob->graph->direction = SIPS_FORWARD;
    ret = _lag2_solve_LR2m_forward(prob, u, ub, 0);
  }

  return(ret);
}

/*
 * _lag2_solve_LR2m_forward(prob, u, ub, eflag)
 *   solves (LR2m) by forward DP.
 *        u: Lagrangian multipliers
 *       ub: upper bound
 *    eflag: perform state elimination (1)
 *
 */
int _lag2_solve_LR2m_forward(sips *prob, _real *u, _real ub,
			     unsigned char eflag)
{
  int t, tt[2];
  int i;
#ifdef SIPSI
  int dlen;
#endif /* SIPSI */
  char ty, ty1, ty2;
  _real f, g;
  _real v1, v2;
  unsigned int n_edges;
  _ptable_t *ptable;
  _benv_t *bedge;
  _node2m_t **node, *cnode, *cnode2, *cnode3, **pnode, *pr1, *pr2;
  _edge2m_t *e, *e2, **pe;

  n_edges = prob->graph->n_edges;
  node = (_node2m_t **) prob->graph->node2;
  ptable = prob->graph->ptable;
  bedge = prob->graph->bedge;

  memset((void *) ptable->occ, 0, (N_JOBS + 2)*sizeof(unsigned int));
  for(i = 0; i < N_JOBS; i++) {
    ptable->window[i].s = prob->graph->Tmax + 1;
    ptable->window[i].e = prob->graph->Tmin;
  }
  for(i = 0; i < prob->graph->fixed->nhead; i++) {
    ptable->window[prob->graph->fixed->job[i]->no].s
      = ptable->window[prob->graph->fixed->job[i]->no].e
      = prob->graph->fixed->c[i];
  }
  for(i = 0; i < prob->graph->fixed->ntail; i++) {
    ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].s
      = ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].e
      = prob->graph->fixed->c[prob->n - i - 1];
  }

  node[prob->graph->Tmin]->v[0] = ctod(prob->graph->fixed->fhead);
  for(i = 0; i < prob->graph->fixed->nhead; i++) {
    node[prob->graph->Tmin]->v[0] -= u[prob->graph->fixed->job[i]->no];
  }
  node[prob->graph->Tmin]->n[0] = node[prob->graph->Tmin];
  node[prob->graph->Tmin]->ty = 0;

  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
    for(pnode = node + t; *pnode != NULL; pnode = &(cnode->next)) {
      cnode = *pnode;
      if(ELIMINATED_NODE(cnode)) {
      	REMOVE_EDGES(cnode);
	      continue;
      }

      if(IS_REAL_JOB(cnode->j[0]) && prec_get_dom_ti(t, cnode->j[0])) {
	      REMOVE_EDGES(cnode);
	      ELIMINATE_NODE(cnode);
	      continue;
      }
      g = - u[cnode->j[0]] + ctod(prob->sjob[cnode->j[0]]->f[t]);

      tt[0] = tt[1] = t;
      if(IS_JOB(cnode->j[1])) {
	      tt[0] -= prob->sjob[cnode->j[0]]->p;
	      if(IS_REAL_JOB(cnode->j[1]) && prec_get_dom_ti(tt[0], cnode->j[1])) {
	        REMOVE_EDGES(cnode);
	        ELIMINATE_NODE(cnode);
	        continue;
	      }
	      g += - u[cnode->j[1]] + ctod(prob->sjob[cnode->j[1]]->f[tt[0]]);

        if(IS_JOB(cnode->j[2])) {
          tt[1] = tt[0] - prob->sjob[cnode->j[1]]->p;
          if(IS_REAL_JOB(cnode->j[2]) && prec_get_dom_ti(tt[1], cnode->j[2])) {
            REMOVE_EDGES(cnode);
            ELIMINATE_NODE(cnode);
            continue;
          }
          g += - u[cnode->j[2]] + ctod(prob->sjob[cnode->j[2]]->f[tt[1]]);
        }
      }

      v1 = v2 = LARGE_REAL;
      pr1 = pr2 = NULL;
      ty1 = ty2 = 0;

      pe = &(cnode->e);
      while(*pe != NULL) {
        cnode2 = (*pe)->n;
	      if(ELIMINATED_NODE(cnode2) || HAVE_COMMON_REAL_JOB(cnode, cnode2)) {
	        REMOVE_SINGLE_EDGE;
	        continue;
      	}

	      cnode3 = cnode2->n[0];
	      if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
	        if(cnode2->n[1] == NULL) {
	          REMOVE_SINGLE_EDGE;
	          continue;
	        } else if(cnode2->e->next->next == NULL) {
	          cnode3 = cnode2->n[1];
	          if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
	            REMOVE_SINGLE_EDGE;
	            continue;
	          }
	        }
	        ty = 1;
	        f = cnode2->v[1];
	      } else {
	        ty = 0;
	        f = cnode2->v[0];
	      }

	      if(eflag) {
	        if(ub - (f + cnode->v[0]) < prob->param->lbeps) {
	          REMOVE_SINGLE_EDGE;
	          continue;
	        } else if(ub - (f + cnode->v[1]) < prob->param->lbeps) {
	          cnode3 = cnode->n[0];
	          if(HAVE_COMMON_REAL_JOB(cnode2, cnode3)) {
	            REMOVE_SINGLE_EDGE;
	            continue;
	          }
	        }

	        if(_check_adj_supernode_forward(prob, cnode2, cnode, t)) {
	          REMOVE_SINGLE_EDGE;
	          continue;
	        }
	      }

        f += g;
        if(f < v1) {
          v2 = v1;
          v1 = f;
          pr2 = pr1;
          pr1 = cnode2;
          ty2 = ty1;
          ty1 = ty;
        } else if(f < v2) {
          v2 = f;
          pr2 = cnode2;
          ty2 = ty;
        }

        pe = &((*pe)->next);
      }

      if(pr1 == NULL) {
	ELIMINATE_NODE(cnode);
	continue;
      }

      cnode->v[0] = v1;
      cnode->v[1] = v2;
      cnode->ty = CONV_TYPE(ty1, ty2);
      cnode->n[0] = pr1;
      cnode->n[1] = pr2;

      cnode2 = cnode->e->n;
#ifdef SIPSI
      if(eflag && cnode->e->next == NULL && IS_JOB(cnode2->j[0])) {
	if(IS_JOB(cnode->j[2])) {
	  if(IS_REAL_JOB(cnode->j[2]) || IS_REAL_JOB(cnode2->j[0])
	     || IS_JOB(cnode2->j[1])) {
	    goto _forward_next;
	  }
	  dlen = cnode->j[2] + cnode2->j[0] + 2 - 2*prob->n;
	  if(dlen > prob->dn) {
	    goto _forward_next;
	  }
	  cnode->j[2] = (unsigned short) (prob->n + dlen - 1);
	} else if(IS_JOB(cnode->j[1])) {
	  if(IS_JOB(cnode2->j[2])) {
	    goto _forward_next;
	  } else if(IS_REAL_JOB(cnode->j[1]) || IS_REAL_JOB(cnode2->j[0])) {
	    if(IS_JOB(cnode2->j[1])) {
	      goto _forward_next;
	    }
	    cnode->j[2] = cnode2->j[0];
	  } else {
	    dlen = cnode->j[1] + cnode2->j[0] + 2 - 2*prob->n;
	    if(dlen > prob->dn) {
	      if(IS_JOB(cnode2->j[1])) {
		goto _forward_next;
	      }
	      cnode->j[1] = (unsigned short) (prob->n + prob->dn - 1);
	      cnode->j[2]
		= (unsigned short) (prob->n + dlen - prob->dn - 1);
	    } else {
	      cnode->j[1] = (unsigned short) (prob->n + dlen - 1);
	      cnode->j[2] = cnode2->j[1];
	    }
	  }
	} else if(IS_REAL_JOB(cnode->j[0]) || IS_REAL_JOB(cnode2->j[0])) {
	  if(IS_JOB(cnode2->j[2])) {
	    goto _forward_next;
	  }
	  cnode->j[1] = cnode2->j[0];
	  cnode->j[2] = cnode2->j[1];
	} else {
	  dlen = cnode->j[0] + cnode2->j[0] + 2 - 2*prob->n;
	  if(dlen > prob->dn) {
	    if(IS_JOB(cnode2->j[2])) {
	      goto _forward_next;
	    }
	    cnode->j[0] = (unsigned short) (prob->n + prob->dn - 1);
	    cnode->j[1]
	      = (unsigned short) (prob->n + dlen - prob->dn - 1);
	    cnode->j[2] = cnode2->j[1];
	  } else {
	    cnode->j[0] = (unsigned short) (prob->n + dlen - 1);
	    cnode->j[1] = cnode2->j[1];
	    cnode->j[2] = cnode2->j[2];
	  }
	}

	free_memory(bedge, cnode->e);
	n_edges--;

	if(IS_JOB(cnode->j[2]) && _check_supernode(prob, cnode->j, t)) {
	  ELIMINATE_NODE(cnode);
	  continue;
	}

	pe = &(cnode->e);
	if(NOT_JOB(cnode->j[1])) {
	  cnode->v[0] = cnode2->v[0] + g;
	  cnode->v[1] = cnode2->v[1] + g;
	  cnode->n[0] = cnode2->n[0];
	  cnode->n[1] = cnode2->n[1];
	  cnode->ty = cnode2->ty;
	    
	  for(e = cnode2->e; e != NULL; e = e->next) {
	    e2 = alloc_memory(bedge);
	    n_edges++;
	    e2->n = e->n;
	    *pe = e2;
	    pe = &(e2->next);
	  }
	  *pe = NULL;
	} else {
	  g = - u[cnode->j[0]] + ctod(prob->sjob[cnode->j[0]]->f[t]);
	  tt[0] = tt[1] = t;
	  if(IS_JOB(cnode->j[1])) {
	    tt[0] -= prob->sjob[cnode->j[0]]->p;
	    g += - u[cnode->j[1]] + ctod(prob->sjob[cnode->j[1]]->f[tt[0]]);
	    if(IS_JOB(cnode->j[2])) {
	      tt[1] = tt[0] -  prob->sjob[cnode->j[1]]->p;
	      g += - u[cnode->j[2]] + ctod(prob->sjob[cnode->j[2]]->f[tt[1]]);
	    }
	  }

	  v1 = v2 = LARGE_REAL;
	  pr1 = pr2 = NULL;
	  ty1 = ty2 = 0;
	  for(e = cnode2->e; e != NULL; e = e->next) {
	    cnode2 = e->n;
	    if(HAVE_REAL_JOB(cnode->j[2], cnode2)
	       || HAVE_REAL_JOB(cnode->j[1], cnode2)
	       || HAVE_REAL_JOB(cnode->j[0], cnode2)) {
	      continue;
	    }

	    cnode3 = cnode2->n[0];
	    if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
	      if(cnode2->n[1] == NULL) {
		continue;
	      } else if(cnode2->e->next->next == NULL) {
		cnode3 = cnode2->n[1];
		if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
		  continue;
		}
	      }
	      ty = 1;
	      f = cnode2->v[1] + g;
	    } else {
	      ty = 0;
	      f = cnode2->v[0] + g;
	    }

	    if(f < v1) {
	      v2 = v1;
	      v1 = f;
	      pr2 = pr1;
	      pr1 = cnode2;
	      ty2 = ty1;
	      ty1 = ty;
	    } else if(f < v2) {
	      v2 = f;
	      pr2 = cnode2;
	      ty2 = ty;
	    }
	    
	    e2 = alloc_memory(bedge);
	    n_edges++;
	    e2->n = cnode2;
	    *pe = e2;
	    pe = &(e2->next);
	  }
	  *pe = NULL;

	  if(cnode->e == NULL) {
	    ELIMINATE_NODE(cnode);
	    continue;
	  }

	  cnode->v[0] = v1;
	  cnode->v[1] = v2;
	  cnode->ty = CONV_TYPE(ty1, ty2);
	  cnode->n[0] = pr1;
	  cnode->n[1] = pr2;
	}
      }
#else /* SIPSI */
  if(eflag && cnode->e->next == NULL && IS_JOB(cnode2->j[0]) && NOT_JOB(cnode->j[2]) && NOT_JOB(cnode2->j[2])) {
	    
    if(IS_JOB(cnode->j[1])) {
	    if(NOT_JOB(cnode2->j[1])) {
	      cnode->j[2] = cnode2->j[0];
	    } else {
	      goto _forward_next;
	    }
    } else {
      cnode->j[1] = cnode2->j[0];
      cnode->j[2] = cnode2->j[1];
    }

	    free_memory(bedge, cnode->e);
    	n_edges--;

	  if(IS_JOB(cnode->j[2]) && _check_supernode(prob, cnode->j, t)) {
	    ELIMINATE_NODE(cnode);
	    continue;
	  }

	g = - u[cnode->j[0]] + ctod(prob->sjob[cnode->j[0]]->f[t]);
	tt[0] = t - prob->sjob[cnode->j[0]]->p;
	g += - u[cnode->j[1]] + ctod(prob->sjob[cnode->j[1]]->f[tt[0]]);
	if(IS_JOB(cnode->j[2])) {
	  tt[1] = tt[0] - prob->sjob[cnode->j[1]]->p;
	  g += - u[cnode->j[2]] + ctod(prob->sjob[cnode->j[2]]->f[tt[1]]);
	}

	v1 = v2 = LARGE_REAL;
	pr1 = pr2 = NULL;
	ty1 = ty2 = 0;
	pe = &(cnode->e);
	for(e = cnode2->e; e != NULL; e = e->next) {
	  cnode2 = e->n;
	  if(HAVE_REAL_JOB(cnode->j[2], cnode2)
	     || HAVE_JOB(cnode->j[1], cnode2)
	     || HAVE_JOB(cnode->j[0], cnode2)) {
	    continue;
	  }

	  cnode3 = cnode2->n[0];
	  if(HAVE_JOB(cnode->j[0], cnode3)
	     || HAVE_JOB(cnode->j[1], cnode3)
	     || HAVE_REAL_JOB(cnode->j[2], cnode3)) {
	    if(cnode2->n[1] == NULL) {
	      continue;
	    } else if(cnode2->e->next->next == NULL) {
	      cnode3 = cnode2->n[1];
	      if(HAVE_JOB(cnode->j[0], cnode3)
		 || HAVE_JOB(cnode->j[1], cnode3)
		 || HAVE_REAL_JOB(cnode->j[2], cnode3)) {
		continue;
	      }
	    }
	    ty = 1;
	    f = cnode2->v[1] + g;
	  } else {
	    ty = 0;
	    f = cnode2->v[0] + g;
	  }

	  if(f < v1) {
	    v2 = v1;
	    v1 = f;
	    pr2 = pr1;
	    pr1 = cnode2;
	    ty2 = ty1;
	    ty1 = ty;
	  } else if(f < v2) {
	    v2 = f;
	    pr2 = cnode2;
	    ty2 = ty;
	  }

	  e2 = alloc_memory(bedge);
	  n_edges++;
	  e2->n = cnode2;
	  *pe = e2;
	  pe = &(e2->next);
	}
	*pe = NULL;

	if(cnode->e == NULL) {
	  ELIMINATE_NODE(cnode);
	  continue;
	}

        cnode->v[0] = v1;
        cnode->v[1] = v2;
        cnode->ty = CONV_TYPE(ty1, ty2);
        cnode->n[0] = pr1;
        cnode->n[1] = pr2;
      }
#endif /* SIPSI */

    _forward_next:
      UPDATE_WINDOWS;
    }
  }

  cnode = node[prob->graph->Tmax + 1];
  cnode->v[0] = cnode->v[1] = LARGE_REAL;
  cnode->n[0] = cnode->n[1] = NULL;
  cnode->ty = 0;
  pe = &(cnode->e);
  while(*pe != NULL) {
    cnode2 = (*pe)->n;
    if(ELIMINATED_NODE(cnode2)) {
      REMOVE_SINGLE_EDGE;
      continue;
    }

    if(cnode2->v[0] < cnode->v[0]) {
      cnode->v[0] = cnode2->v[0];
      cnode->n[0] = cnode2;
    }
    pe = &((*pe)->next);
  }

  prob->graph->n_edges = n_edges;

  if(cnode->e == NULL) {
    return(SIPS_INFEASIBLE);
  }

  _lag2_free_eliminated_nodes(prob);

  return(_lag2_shrink_horizon_tail(prob, u));
}

/*
 * _lag2_solve_LR2m_backward(prob, u, ub, eflag)
 *   solves (LR2m) by backward DP.
 *        u: Lagrangian multipliers
 *       ub: upper bound
 *    eflag: perform state elimination (1)
 *
 */
int _lag2_solve_LR2m_backward(sips *prob, _real *u, _real ub,
			      unsigned char eflag)
{
  int t, tt[2];
  int i;
  char ty, ty1, ty2;
  _real f, g;
  _real v1, v2;
  unsigned int n_edges;
  _ptable_t *ptable;
  _benv_t *bedge;
  _node2m_t **node, *cnode, *cnode2, *cnode3, **pnode, *pr1, *pr2;
  _edge2m_t *e, *e2, **pe;

  n_edges = prob->graph->n_edges;
  node = (_node2m_t **) prob->graph->node2;
  ptable = prob->graph->ptable;
  bedge = prob->graph->bedge;

  memset((void *) ptable->occ, 0, (N_JOBS + 2)*sizeof(unsigned int));
  for(i = 0; i < N_JOBS; i++) {
    ptable->window[i].s = prob->graph->Tmax + 1;
    ptable->window[i].e = prob->graph->Tmin;
  }
  for(i = 0; i < prob->graph->fixed->nhead; i++) {
    ptable->window[prob->graph->fixed->job[i]->no].s
      = ptable->window[prob->graph->fixed->job[i]->no].e
      = prob->graph->fixed->c[i];
  }
  for(i = 0; i < prob->graph->fixed->ntail; i++) {
    ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].s
      = ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].e
      = prob->graph->fixed->c[prob->n - i - 1];
  }

  node[prob->graph->Tmax + 1]->v[0]  = ctod(prob->graph->fixed->ftail);
  for(i = 0; i < prob->graph->fixed->ntail; i++) {
    node[prob->graph->Tmax + 1]->v[0]
      -= u[prob->graph->fixed->job[prob->n - i - 1]->no];
  }
  node[prob->graph->Tmax + 1]->n[0] = node[prob->graph->Tmax + 1];
  node[prob->graph->Tmax + 1]->ty = 0;

  for(t = prob->graph->Tmax; t > prob->graph->Tmin; t--) {
    for(pnode = node + t; *pnode != NULL; pnode = &(cnode->next)) {
      cnode = *pnode;
      if(ELIMINATED_NODE(cnode)) {
	      REMOVE_EDGES(cnode);
	      continue;
      }

      if(IS_REAL_JOB(cnode->j[0]) && prec_get_dom_ti(t, cnode->j[0])) {
	      REMOVE_EDGES(cnode);
	      ELIMINATE_NODE(cnode);
	      continue;
      }
      g = - u[cnode->j[0]] + ctod(prob->sjob[cnode->j[0]]->f[t]);

      tt[0] = tt[1] = t;
      if(IS_JOB(cnode->j[1])) {
	      tt[0] -= prob->sjob[cnode->j[0]]->p;
	      if(IS_REAL_JOB(cnode->j[1]) && prec_get_dom_ti(tt[0], cnode->j[1])) {
	        REMOVE_EDGES(cnode);
	        ELIMINATE_NODE(cnode);
	        continue;
	      }
	      g += - u[cnode->j[1]] + ctod(prob->sjob[cnode->j[1]]->f[tt[0]]);

        if(IS_JOB(cnode->j[2])) {
          tt[1] = tt[0] - prob->sjob[cnode->j[1]]->p;
          if(IS_REAL_JOB(cnode->j[2]) && prec_get_dom_ti(tt[1], cnode->j[2])) {
            REMOVE_EDGES(cnode);
            ELIMINATE_NODE(cnode);
            continue;
          }
          g += - u[cnode->j[2]] + ctod(prob->sjob[cnode->j[2]]->f[tt[1]]);
        }
      }

      v1 = v2 = LARGE_REAL;
      pr1 = pr2 = NULL;
      ty1 = ty2 = 0;

      pe = &(cnode->e);
      while(*pe != NULL) {
        cnode2 = (*pe)->n;
	      if(ELIMINATED_NODE(cnode2)) {
	        REMOVE_SINGLE_EDGE;
	        continue;
	      }

	cnode3 = cnode2->n[0];
        if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
          if(cnode2->n[1] == NULL) {
            REMOVE_SINGLE_EDGE;
            continue;
          } else if(cnode2->e->next->next == NULL) {
            cnode3 = cnode2->n[1];
            if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
              REMOVE_SINGLE_EDGE;
              continue;
            }
          }
          ty = 1;
          f = cnode2->v[1];
        } else {
          ty = 0;
          f = cnode2->v[0];
        }

        if(eflag) {
          if(ub - (f + cnode->v[0]) < prob->param->lbeps) {
            REMOVE_SINGLE_EDGE;
            continue;
          } else if(ub - (f + cnode->v[1]) < prob->param->lbeps) {
            cnode3 = cnode->n[0];
            if(HAVE_COMMON_REAL_JOB(cnode2, cnode3)) {
              REMOVE_SINGLE_EDGE;
              continue;
            }
          }

          if(_check_adj_supernode_backward(prob, cnode, cnode2, t)) {
            REMOVE_SINGLE_EDGE;
            continue;
          }
        }

        f += g;
        if(f < v1) {
          v2 = v1;
          v1 = f;
          pr2 = pr1;
          pr1 = cnode2;
          ty2 = ty1;
          ty1 = ty;
        } else if(f < v2) {
          v2 = f;
          pr2 = cnode2;
          ty2 = ty;
        }

	      pe = &((*pe)->next);
      }

      if(pr1 == NULL) {
	      ELIMINATE_NODE(cnode);
	      continue;
      }

      cnode->v[0] = v1;
      cnode->v[1] = v2;
      cnode->ty = CONV_TYPE(ty1, ty2);
      cnode->n[0] = pr1;
      cnode->n[1] = pr2;

      UPDATE_WINDOWS;
    }
  }

  cnode = node[prob->graph->Tmin];
  cnode->v[0] = cnode->v[1] = LARGE_REAL;
  cnode->n[0] = cnode->n[1] = NULL;
  cnode->ty = 0;
  pe = &(cnode->e);
  while(*pe != NULL) {
    cnode2 = (*pe)->n;
    if(ELIMINATED_NODE(cnode2)) {
      REMOVE_SINGLE_EDGE;
      continue;
    }

    if(cnode2->v[0] < cnode->v[0]) {
      cnode->v[0] = cnode2->v[0];
      cnode->n[0] = cnode2;
    }
    pe = &((*pe)->next);
  }

  prob->graph->n_edges = n_edges;

  if(cnode->e == NULL) {
    return(SIPS_INFEASIBLE);
  }

  _lag2_free_eliminated_nodes(prob);

  return(_lag2_shrink_horizon_head(prob, u));
}

/*
 * _lag2_get_sol_LR2m(prob, u, ub, sol, lb, o)
 *   returns the current solution of (LR2m).
 *        u: Lagrangian multipliers
 *       ub: upper bound
 *      sol: solution of the relaxation
 *       lb: lower bound
 *        o: numbers of job occurrences
 *
 */
int _lag2_get_sol_LR2m(sips *prob, _real *u, _real ub, _solution_t *sol,
		       _real *lb, unsigned int *o)
{
  int ret;

  if(prob->graph->direction == SIPS_FORWARD) { /* forward */
    ret = _lag2_get_sol_LR2m_forward(prob, u, sol, lb, o);
  } else { /* backward */
    ret = _lag2_get_sol_LR2m_backward(prob, u, sol, lb, o);
  }

  if(*lb > ub) {
    *lb = ub;
  }

  return(ret);
}

/*
 * _lag2_get_sol_LR2m_forward(prob, u, sol, lb, o)
 *   returns the current solution of (LR2m) from forward DP states.
 *        u: Lagrangian multipliers
 *      sol: solution
 *       lb: objective value
 *        o: numbers of job occurrences
 *
 */
int _lag2_get_sol_LR2m_forward(sips *prob, _real *u, _solution_t *sol,
			       _real *lb, unsigned int *o) {
  int t;
  int i;
  int ty, pty;
  int ret;
  _node2m_t **node, *cnode;
  _fixed_t *fixed;

  node = (_node2m_t **) prob->graph->node2;
  fixed = prob->graph->fixed;

  cnode = node[prob->graph->Tmax + 1];
  if(cnode == NULL) {
    *lb = LARGE_REAL;
    if(sol != NULL) {
      sol->f = LARGE_COST;
    }
    return(SIPS_INFEASIBLE);
  }

  *lb = cnode->v[0] + ctod(fixed->ftail);
  for(i = 0; i < fixed->ntail; i++) {
    *lb -= u[fixed->job[prob->n - i - 1]->no];
  }

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

    for(i = 0; i < fixed->nhead; i++) {
      o[fixed->job[i]->no]++;
    }
    for(i = 0; i < fixed->ntail; i++) {
      o[fixed->job[prob->n - i - 1]->no]++;
    }

    while(IS_JOB(cnode->j[0])) {
#ifdef SIPSI
      if(IS_REAL_JOB(cnode->j[0])) {
	o[cnode->j[0]]++;
      }
      if(IS_REAL_JOB(cnode->j[1])) {
	o[cnode->j[1]]++;
      }
      if(IS_REAL_JOB(cnode->j[2])) {
	o[cnode->j[2]]++;
      }
#else /* SIPSI */
      o[cnode->j[0]]++;
      if(IS_JOB(cnode->j[1])) {
	o[cnode->j[1]]++;
      }
      if(IS_JOB(cnode->j[2])) {
	o[cnode->j[2]]++;
      }
#endif /* SIPSI */

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
  sol->f = fixed->ftail;
  for(i = 0; i < fixed->ntail; i++) {
    sol->job[sol->n] = fixed->job[prob->n - i - 1];
    sol->c[sol->n++] = fixed->c[prob->n - i - 1];
  }

  if(o != NULL) {
    memset((void *) o, 0, prob->n*sizeof(unsigned int));

    for(i = 0; i < fixed->nhead; i++) {
      o[fixed->job[i]->no]++;
    }
    for(i = 0; i < fixed->ntail; i++) {
      o[fixed->job[prob->n - i - 1]->no]++;
    }

    while(IS_JOB(cnode->j[0])) {
      sol->f += prob->sjob[cnode->j[0]]->f[t];
      if(IS_REAL_JOB(cnode->j[0])) {
	o[cnode->j[0]]++;
	sol->job[sol->n] = prob->sjob[cnode->j[0]];
	sol->c[sol->n++] = t;
      }
      t -= prob->sjob[cnode->j[0]]->p;

      if(IS_JOB(cnode->j[1])) {
	sol->f += prob->sjob[cnode->j[1]]->f[t];
	if(IS_REAL_JOB(cnode->j[1])) {
	  o[cnode->j[1]]++;
	  sol->job[sol->n] = prob->sjob[cnode->j[1]];
	  sol->c[sol->n++] = t;
	}
	t -= prob->sjob[cnode->j[1]]->p;

	if(IS_JOB(cnode->j[2])) {
	  sol->f += prob->sjob[cnode->j[2]]->f[t];
	  if(IS_REAL_JOB(cnode->j[2])) {
	    o[cnode->j[2]]++;
	    sol->job[sol->n] = prob->sjob[cnode->j[2]];
	    sol->c[sol->n++] = t;
	  }
	  t -= prob->sjob[cnode->j[2]]->p;
	}
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
    while(IS_JOB(cnode->j[0])) {
      if(IS_REAL_JOB(cnode->j[0])) {
        sol->f += prob->sjob[cnode->j[0]]->f[t];
        sol->job[sol->n] = prob->sjob[cnode->j[0]];
        sol->c[sol->n++] = t;
      }
      t -= prob->sjob[cnode->j[0]]->p;

      if(IS_JOB(cnode->j[1])) {
        if(IS_REAL_JOB(cnode->j[1])) {
          sol->f += prob->sjob[cnode->j[1]]->f[t];
          sol->job[sol->n] = prob->sjob[cnode->j[1]];
          sol->c[sol->n++] = t;
        }
	      t -= prob->sjob[cnode->j[1]]->p;

        if(IS_JOB(cnode->j[2])) {
          if(IS_REAL_JOB(cnode->j[2])) {
            sol->f += prob->sjob[cnode->j[2]]->f[t];
            sol->job[sol->n] = prob->sjob[cnode->j[2]];
            sol->c[sol->n++] = t;
          }
          t -= prob->sjob[cnode->j[2]]->p;
        }
      }

      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    ret = SIPS_NORMAL;
  }

  sol->f += fixed->fhead;
  for(i = fixed->nhead - 1; i >= 0; i--) {
    sol->job[sol->n] = fixed->job[i];
    sol->c[sol->n++] = fixed->c[i];
  }

  reverse_solution(sol);

  if(fixed->nhead + fixed->ntail == prob->n) {
    return(SIPS_OPTIMAL);
  }

  return(ret);
}

/*
 * _lag2_get_sol_LR2m_backward(prob, u, sol, lb, o)
 *   returns the current solution of (LR2m) from backward DP states.
 *        u: Lagrangian multipliers
 *      sol: solution
 *       lb: objective value
 *        o: numbers of job occurrences
 *
 */
int _lag2_get_sol_LR2m_backward(sips *prob, _real *u, _solution_t *sol,
				_real *lb, unsigned int *o)
{
  int t;
  int i;
  int ty, pty;
  int ret;
  _node2m_t **node, *cnode;
  _fixed_t *fixed;

  node = (_node2m_t **) prob->graph->node2;
  fixed = prob->graph->fixed;

  cnode = node[prob->graph->Tmin];
  if(cnode == NULL) {
    *lb = LARGE_REAL;
    if(sol != NULL) {
      sol->f = LARGE_COST;
    }
    return(SIPS_INFEASIBLE);
  }

  *lb = cnode->v[0] + ctod(fixed->fhead);
  for(i = 0; i < fixed->nhead; i++) {
    *lb -= u[fixed->job[i]->no];
  }

  if(*lb > LARGE_REAL2) {
    if(sol != NULL) {
      sol->f = LARGE_COST;
    }
    return(SIPS_INFEASIBLE);
  }
  ty = 0;
  cnode = cnode->n[0];
  t = prob->graph->Tmin;

  ret = SIPS_OPTIMAL;
  if(sol == NULL) {
    if(o == NULL) {
      return(SIPS_NORMAL);
    }

    memset((void *) o, 0, prob->n*sizeof(unsigned int));

    for(i = 0; i < fixed->nhead; i++) {
      o[fixed->job[i]->no]++;
    }
    for(i = 0; i < fixed->ntail; i++) {
      o[fixed->job[prob->n - i - 1]->no]++;
    }

    while(IS_JOB(cnode->j[0])) {
#ifdef SIPSI
      if(IS_REAL_JOB(cnode->j[0])) {
	o[cnode->j[0]]++;
      }
      if(IS_REAL_JOB(cnode->j[1])) {
	o[cnode->j[1]]++;
      }
      if(IS_REAL_JOB(cnode->j[2])) {
	o[cnode->j[2]]++;
      }
#else /* SIPSI */
      o[cnode->j[0]]++;
      if(IS_JOB(cnode->j[1])) {
	o[cnode->j[1]]++;
      }
      if(IS_JOB(cnode->j[2])) {
	o[cnode->j[2]]++;
      }
#endif /* SIPSI */

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
  sol->f = fixed->fhead;
  for(i = 0; i < fixed->nhead; i++) {
    sol->job[sol->n] = fixed->job[i];
    sol->c[sol->n++] = fixed->c[i];
  }

  if(o != NULL) {
    memset((void *) o, 0, prob->n*sizeof(unsigned int));

    for(i = 0; i < fixed->nhead; i++) {
      o[fixed->job[i]->no]++;
    }
    for(i = 0; i < fixed->ntail; i++) {
      o[fixed->job[prob->n - i - 1]->no]++;
    }

    while(IS_JOB(cnode->j[0])) {
      if(IS_JOB(cnode->j[2])) {
	t += prob->sjob[cnode->j[2]]->p;
	if(IS_REAL_JOB(cnode->j[2])) {
	  o[cnode->j[2]]++;
	  sol->f += prob->sjob[cnode->j[2]]->f[t];
	  sol->job[sol->n] = prob->sjob[cnode->j[2]];
	  sol->c[sol->n++] = t;
	}
      }
      if(IS_JOB(cnode->j[1])) {
	t += prob->sjob[cnode->j[1]]->p;
	if(IS_REAL_JOB(cnode->j[1])) {
	  o[cnode->j[1]]++;
	  sol->f += prob->sjob[cnode->j[1]]->f[t];
	  sol->job[sol->n] = prob->sjob[cnode->j[1]];
	  sol->c[sol->n++] = t;
	}
      }
      t += prob->sjob[cnode->j[0]]->p;
      if(IS_REAL_JOB(cnode->j[0])) {
	o[cnode->j[0]]++;
	sol->f += prob->sjob[cnode->j[0]]->f[t];
	sol->job[sol->n] = prob->sjob[cnode->j[0]];
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
    while(IS_JOB(cnode->j[0])) {
      if(IS_JOB(cnode->j[2])) {
	t += prob->sjob[cnode->j[2]]->p;
	sol->f += prob->sjob[cnode->j[2]]->f[t];
	if(IS_REAL_JOB(cnode->j[2])) {
	  sol->job[sol->n] = prob->sjob[cnode->j[2]];
	  sol->c[sol->n++] = t;
	}
      }
      if(IS_JOB(cnode->j[1])) {
	t += prob->sjob[cnode->j[1]]->p;
	sol->f += prob->sjob[cnode->j[1]]->f[t];
	if(IS_REAL_JOB(cnode->j[1])) {
	  sol->job[sol->n] = prob->sjob[cnode->j[1]];
	  sol->c[sol->n++] = t;
	}
      }
      t += prob->sjob[cnode->j[0]]->p;
      sol->f += prob->sjob[cnode->j[0]]->f[t];
      if(IS_REAL_JOB(cnode->j[0])) {
	sol->job[sol->n] = prob->sjob[cnode->j[0]];
	sol->c[sol->n++] = t;
      }

      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    ret = SIPS_NORMAL;
  }

  sol->f += fixed->ftail;
  for(i = fixed->ntail - 1; i >= 0; i--) {
    sol->job[sol->n] = fixed->job[prob->n - i - 1];
    sol->c[sol->n++] = fixed->c[prob->n - i - 1];
  }

  if(fixed->nhead + fixed->ntail == prob->n) {
    return(SIPS_OPTIMAL);
  }

  return(ret);
}

/*
 * _lag2_check_time_window(prob)
 *   checks and updates time windows of jobs.
 *
 */
int _lag2_check_time_window(sips *prob)
{
  int t;
  int i;
  _ptable_t *ptable;
  _fixed_t *fixed;
  _window_t *w;

  ptable = prob->graph->ptable;
  fixed = prob->graph->fixed;

  for(i = 0; i < prob->n; i++) {
    if(!is_fixed(i) && (ptable->window[i].s > prob->graph->Tmax
			|| ptable->window[i].e < prob->graph->Tmin)) {
      return(SIPS_INFEASIBLE);
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

  return(SIPS_NORMAL);
}

/*
 * lag2_assign_modifiers(prob, type, nmax, mod)
 *   returns the jobs to which nonzero modifiers are assigned.
 *   Two strategies are switched by "type."
 *     (1) The jobs that never occur in the current DP solution are selected.
 *         Ties are broken by the number of occurrences in the DP states.
 *         If the number of jobs is less than nmax, the jobs that
 *         occur more than once are selected.
 *     (2) The jobs whose numbers of dominated or dominating jobs in the
 *         DP states are smaller.
 *
 *     nmax: maximal number of jobs to which nonzero modifiers are assigned
 *      mod: modifier information
 *
 */
int lag2_assign_modifiers(sips *prob, unsigned char type, int nmax,
			  _mod_t *mod)
{
  int i, j;
  int ty, pty;
  int n, nn, m;
  _ptable_t *ptable;
  _fixed_t *fixed;
  _node2m_t **node, *cnode;
  _occur_t *occ;

  if(_lag2_check_time_window(prob) == SIPS_INFEASIBLE) {
    return(SIPS_INFEASIBLE);
  }

  node = (_node2m_t **) prob->graph->node2;
  fixed = prob->graph->fixed;

  occ = (_occur_t *) xcalloc(sizeof(_occur_t), N_JOBS + 2);

  ptable = prob->graph->ptable;
  for(i = 0; i < prob->n; i++) {
    occ[i].j = i;
    occ[i].nd = ptable->occ[i];
  }

  for(i = 0; i < fixed->nhead; i++) {
    mod->fl[fixed->job[i]->no] = 1;
  }
  for(i = 0; i < fixed->ntail; i++) {
    mod->fl[fixed->job[prob->n - i - 1]->no] = 1;
  }

  if(prob->graph->direction == SIPS_FORWARD) {
    ty = 0;
    cnode = node[prob->graph->Tmax + 1]->n[0];
    while(IS_JOB(cnode->j[0])) {
      occ[cnode->j[0]].no++;
      occ[cnode->j[1]].no++;
      occ[cnode->j[2]].no++;
      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }
  } else {
    ty = 0;
    cnode = node[prob->graph->Tmin]->n[0];
    while(IS_JOB(cnode->j[0])) {
      occ[cnode->j[2]].no++;
      occ[cnode->j[1]].no++;
      occ[cnode->j[0]].no++;
      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }
  }

  for(i = nn = 0; i < prob->n; i++) {
    if(mod->fl[occ[i].j] == 0) {
      occ[nn++] = occ[i];
    }
  }

  if(type == 0) {
    qsort((void *) occ, nn, sizeof(_occur_t), _compare_occur);

    for(i = 0; i < nn && occ[i].no == 0; i++);
    for(n = 0, i--; n < nmax && i >= 0; n++, i--) {
      mod->jobs[n] = occ[i].j;
      mod->fl[occ[i].j] = 1;
    }

    if(n < nmax) {
      for(i = nn - 1; n < nmax && i >= 0 && occ[i].no > 1; n++, i--) {
	      mod->jobs[n] = occ[i].j;
	      mod->fl[occ[i].j] = 1;
      }
    }
  } else {
    qsort((void *) occ, nn, sizeof(_occur_t), _compare_occur2);

    for(n = 0;  n < nn && n < nmax; n++) {
      mod->jobs[n] = occ[n].j;
      mod->fl[occ[n].j] = 1;
    }
  }

  xfree(occ);

  mod->an = n;
  memset((void *) mod->v[0], 0, (N_JOBS + 2)*(1<<prob->param->mod));
  memset((void *) mod->m, 0, N_JOBS + 2);
  for(i = 0; i < mod->an; i++) {
    mod->m[mod->jobs[i]] = 1<<i;
  }
  //las mascaras validas 
  for(j = 0; j < prob->n; j++) {
    for(m = 0; m < 1<<mod->an; m++) {
      if(mod->m[j] && !(m & mod->m[j])) {
	      mod->v[j][m] = 1;
      }
    }
  }

  for(i = 0; i < mod->an; i++) {
    for(j = 0; j < prob->n; j++) {
      if(ptable->inc[mod->jobs[i]][j] == 1) {
        /* mod->job[i] -> j */
        for(m = 0; m < 1<<mod->an; m++) {
          if(!(m & (1<<i))) {
            mod->v[j][m] = 1;
          }
        }
      } else if(ptable->inc[mod->jobs[i]][j] == -1) {
        /* j -> mod->job[i] */
        for(m = 0; m < 1<<mod->an; m++) {
          if(m & (1<<i)) {
            mod->v[j][m] = 1;
          }
        }
      }
    }
  }
  mod->n += mod->an;

  return(SIPS_OK);
}

/*
 * lag2_get_memory_in_MB(prob)
 *   returns memory usage of DP states computed by the current graph size.
 *
 */
_real lag2_get_memory_in_MB(sips *prob)
{
  _real mem;

  mem = (_real) (prob->T + 2)*sizeof(_node2m_t *);
  mem += sizeof(_edge2m_t)*(_real) prob->graph->n_edges
    + sizeof(_node2m_t)*(_real) prob->graph->n_nodes;
  mem /= (_real) (1<<20);
  mem += prec_get_memory_in_MB(prob);
  mem += prob->graph->copy->mem;

  return(mem);
}

/*
 * lag2_get_real_memory_in_MB(prob)
 *   returns real memory usage of DP states.
 *
 */
_real lag2_get_real_memory_in_MB(sips *prob)
{
  _real mem;

  mem = (_real) (prob->T + 2)*sizeof(_node2m_t *);
  if(prob->graph->bnode != NULL) {
    mem += DP_BLOCKSIZE
      *(_real) (prob->graph->bnode->nb + prob->graph->bedge->nb);
  }
  mem /= (_real) (1<<20);
  mem += prec_get_memory_in_MB(prob);
  mem += prob->graph->copy->rmem;

  return(mem);
}

/*
 * _lag2_move_edges_head(prob)
 *   moves edges from their source nodes to their destination nodes.
 *
 */
void _lag2_move_edges_head(sips *prob)
{
  int t;
  _node2m_t **node;
  _node2m_t *cnode;
  _edge2m_t *e, *e2, *e3;

  node = (_node2m_t **) prob->graph->node2;
  for(t = prob->graph->Tmax; t >= prob->graph->Tmin; t--) {
    for(cnode = node[t]; cnode != NULL; cnode = cnode->next) {
      for(e = cnode->e; e != NULL;) {
	e3 = e->next;
	e2 = e->n->e;
	e->n->e = e;
	e->n = cnode;
	e->next = e2;
	e = e3;
      }

      cnode->e = NULL;
    }
  }
}

/*
 * _lag2_move_edges_tail(prob)
 *   moves edges from their destination nodes to their source nodes.
 *
 */
void _lag2_move_edges_tail(sips *prob)
{
  int t;
  _node2m_t **node;
  _node2m_t *cnode;
  _edge2m_t *e, *e2, *e3;

  node = (_node2m_t **) prob->graph->node2;
  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax + 1; t++) {
    for(cnode = node[t]; cnode != NULL; cnode = cnode->next) {
      for(e = cnode->e; e != NULL;) {
	e3 = e->next;
	e2 = e->n->e;
	e->n->e = e;
	e->n = cnode;
	e->next = e2;
	e = e3;
      }
      cnode->e = NULL;
    }
  }
}

/*
 * _lag2_free_eliminated_nodes(prob)
 *   frees nodes marked "eliminated."
 *
 */
void _lag2_free_eliminated_nodes(sips *prob)
{
  int t;
  unsigned int n_nodes;
  _benv_t *bnode;
  _node2m_t **node;
  _node2m_t *cnode, **pnode;

  n_nodes = prob->graph->n_nodes;
  bnode = prob->graph->bnode;
  node = (_node2m_t **) prob->graph->node2;
  for(t = prob->graph->Tmin; t <= prob->graph->Tmax + 1; t++) {
    pnode = node + t;
    while(*pnode != NULL) {
      if(ELIMINATED_NODE(*pnode)) {
	cnode = (*pnode)->next;
	free_memory(bnode, *pnode);
	*pnode = cnode;
	n_nodes--;
      } else {
	pnode = &((*pnode)->next);
      }
    }
  }
  prob->graph->n_nodes = n_nodes;
}

/*
 * _lag2_shrink_horizon_head(prob, u)
 *   increases the start time of the scheduling horizon to shrink
 *   its length.
 *        u: Lagrangian multipliers
 *
 */
int _lag2_shrink_horizon_head(sips *prob, _real *u)
{
  int t, tt;
  int i, j;
  int nhead;
  _real f;
  char **inc;
  unsigned int n_nodes, n_edges;
  _ptable_t *ptable;
  _fixed_t *fixed;
  _benv_t *bnode, *bedge;
  _node2m_t **node, *cnode, *cnode2;
  _edge2m_t *e, *e2;

  node = (_node2m_t **) prob->graph->node2;
  cnode = node[prob->graph->Tmin];
  if(cnode == NULL) {
    return(SIPS_INFEASIBLE);
  }

  n_nodes = prob->graph->n_nodes;
  n_edges = prob->graph->n_edges;
  bnode = prob->graph->bnode;
  bedge = prob->graph->bedge;
  ptable = prob->graph->ptable;
  inc = ptable->inc;
  fixed = prob->graph->fixed;
  nhead = fixed->nhead;

  e = cnode->e;
  f = cnode->v[0];
  t = prob->graph->Tmin;
  while(e->next == NULL && t < prob->graph->Tmax) {
    cnode = e->n;

    if(IS_JOB(cnode->j[2])) {
      t += prob->sjob[cnode->j[2]]->p;
      if(IS_REAL_JOB(cnode->j[2])) {
#ifdef DEBUG
	printf("FIXED no%d c=%d\n", prob->sjob[cnode->j[2]]->rno, t);
#endif /* DEBUG */
	if(is_fixed(cnode->j[2])) {
	  prob->graph->n_nodes = n_nodes;
	  prob->graph->n_edges = n_edges;
	  return(SIPS_INFEASIBLE);
	}
	for(tt = prob->graph->Tmin; tt <= prob->graph->Tmax; tt++) {
	  prec_set_dom_ti(tt, cnode->j[2]);
	}
	prec_rev_dom_ti(t, cnode->j[2]);
	ptable->window[cnode->j[2]].s = t;
	ptable->window[cnode->j[2]].e = t;
	ptable->occ[cnode->j[2]] = 1;
	fixed->job[fixed->nhead] = prob->sjob[cnode->j[2]];
	fixed->c[fixed->nhead++] = t;
	fixed->fhead += prob->sjob[cnode->j[2]]->f[t];
	f -= ctod(prob->sjob[cnode->j[2]]->f[t]) - u[cnode->j[2]];
	set_fixed(cnode->j[2]);
      }
    }

    if(IS_JOB(cnode->j[1])) {
      t += prob->sjob[cnode->j[1]]->p;
      if(IS_REAL_JOB(cnode->j[1])) {
#ifdef DEBUG
	printf("FIXED no%d c=%d\n", prob->sjob[cnode->j[1]]->rno, t);
#endif /* DEBUG */
	if(is_fixed(cnode->j[1])) {
	  prob->graph->n_nodes = n_nodes;
	  prob->graph->n_edges = n_edges;
	  return(SIPS_INFEASIBLE);
	}
	for(tt = prob->graph->Tmin; tt <= prob->graph->Tmax; tt++) {
	  prec_set_dom_ti(tt, cnode->j[1]);
	}
	prec_rev_dom_ti(t, cnode->j[1]);
	ptable->window[cnode->j[1]].s = t;
	ptable->window[cnode->j[1]].e = t;
	ptable->occ[cnode->j[1]] = 1;
	fixed->job[fixed->nhead] = prob->sjob[cnode->j[1]];
	fixed->c[fixed->nhead++] = t;
	fixed->fhead += prob->sjob[cnode->j[1]]->f[t];
	f -= ctod(prob->sjob[cnode->j[1]]->f[t]) - u[cnode->j[1]];
	set_fixed(cnode->j[1]);
      }
    }

    t += prob->sjob[cnode->j[0]]->p;

    if(IS_REAL_JOB(cnode->j[0])) {
#ifdef DEBUG
      printf("FIXED no%d c=%d\n", prob->sjob[cnode->j[0]]->rno, t);
#endif /* DEBUG */
      if(is_fixed(cnode->j[0])) {
	prob->graph->n_nodes = n_nodes;
	prob->graph->n_edges = n_edges;
	return(SIPS_INFEASIBLE);
      }
      for(tt = prob->graph->Tmin; tt <= prob->graph->Tmax; tt++) {
	prec_set_dom_ti(tt, cnode->j[0]);
      }
      prec_rev_dom_ti(t, cnode->j[0]);
      ptable->window[cnode->j[0]].s = t;
      ptable->window[cnode->j[0]].e = t;
      ptable->occ[cnode->j[0]] = 1;
      fixed->job[fixed->nhead] = prob->sjob[cnode->j[0]];
      fixed->c[fixed->nhead++] = t;
      fixed->fhead += prob->sjob[cnode->j[0]]->f[t];
      f -= ctod(prob->sjob[cnode->j[0]]->f[t]) - u[cnode->j[0]];
      set_fixed(cnode->j[0]);
    }

    e = cnode->e;
  }

  if(t > prob->graph->Tmin) {
    free_memory(bedge, node[prob->graph->Tmin]->e);
    n_edges--;
    node[prob->graph->Tmin]->e = e;
    node[prob->graph->Tmin]->v[0] = f;
    node[prob->graph->Tmin]->n[0] = cnode->n[0];
    cnode->e = NULL;
    for(tt = t; tt > prob->graph->Tmin; tt--) {
      for(cnode = node[tt]; cnode != NULL; cnode = cnode2) {
	REMOVE_EDGES(cnode);
	cnode2 = cnode->next;
	free_memory(bnode, cnode);
	n_nodes--;
      }
      node[tt] = NULL;
    }
    node[t] = node[prob->graph->Tmin];
    node[prob->graph->Tmin] = NULL;
    prob->graph->Tmin = t;
  }

  prob->graph->n_nodes = n_nodes;
  prob->graph->n_edges = n_edges;

  if(t == prob->graph->Tmax && fixed->nhead + fixed->ntail != prob->n) {
    return(SIPS_INFEASIBLE);
  }

  if(nhead < fixed->nhead) {
    for(i = nhead; i < fixed->nhead; i++) {
      for(j = 0; j < i; j++) {
	if(inc[fixed->job[j]->no][fixed->job[i]->no] == -1) {
	  return(SIPS_INFEASIBLE);
	} else if(inc[fixed->job[j]->no][fixed->job[i]->no] != 1) {
	  _set_inc(fixed->job[j]->no, fixed->job[i]->no);
	}
      }
      for(j = 0; j < prob->n; j++) {
	if(j != fixed->job[i]->no && inc[fixed->job[i]->no][j] == 0) {
	  _set_inc(fixed->job[i]->no, j);
	}
      }
    }
  }

  if(fixed->nhead + fixed->ntail == prob->n
     && prob->graph->Tmin == prob->graph->Tmax) {
    return(SIPS_OPTIMAL);
  }

  return(SIPS_NORMAL);
}

/*
 * _lag2_shrink_horizon_tail(prob, u)
 *   decreases the end time of the scheduling horizon to shrink
 *   its length.
 *        u: Lagrangian multipliers
 *
 */
int _lag2_shrink_horizon_tail(sips *prob, _real *u)
{
  int t, tt;
  int i, j;
  int ntail;
  _real f;
  char **inc;
  unsigned int n_nodes, n_edges;
  _ptable_t *ptable;
  _fixed_t *fixed;
  _benv_t *bnode, *bedge;
  _node2m_t **node, *cnode, *cnode2;
  _edge2m_t *e, *e2;

  node = (_node2m_t **) prob->graph->node2;
  cnode = node[prob->graph->Tmax + 1];
  if(cnode == NULL) {
    if(prob->graph->fixed->nhead + prob->graph->fixed->ntail != prob->n) {
      return(SIPS_INFEASIBLE);
    } else {
      return(SIPS_OPTIMAL);
    }
  }

  n_nodes = prob->graph->n_nodes;
  n_edges = prob->graph->n_edges;
  bnode = prob->graph->bnode;
  bedge = prob->graph->bedge;
  ptable = prob->graph->ptable;
  inc = ptable->inc;
  fixed = prob->graph->fixed;
  ntail = fixed->ntail;

  e = cnode->e;
  f = cnode->v[0];
  t = prob->graph->Tmax;
  while(e->next == NULL && t > prob->graph->Tmin) {
    cnode = e->n;

    if(IS_REAL_JOB(cnode->j[0])) {
      if(is_fixed(cnode->j[0])) {
        prob->graph->n_nodes = n_nodes;
        prob->graph->n_edges = n_edges;
        return(SIPS_INFEASIBLE);
      }
      for(tt = prob->graph->Tmin; tt <= prob->graph->Tmax; tt++) {
	      prec_set_dom_ti(tt, cnode->j[0]);
      }
      prec_rev_dom_ti(t, cnode->j[0]);
      ptable->window[cnode->j[0]].s = t;
      ptable->window[cnode->j[0]].e = t;
      ptable->occ[cnode->j[0]] = 1;
      fixed->job[prob->n - fixed->ntail - 1] = prob->sjob[cnode->j[0]];
      fixed->c[prob->n - fixed->ntail - 1] = t;
      fixed->ntail++;
      fixed->ftail += prob->sjob[cnode->j[0]]->f[t];
      f -= ctod(prob->sjob[cnode->j[0]]->f[t]) - u[cnode->j[0]];
      set_fixed(cnode->j[0]);
    }

    t -= prob->sjob[cnode->j[0]]->p;

    if(IS_JOB(cnode->j[1])) {
      if(IS_REAL_JOB(cnode->j[1])) {
        if(is_fixed(cnode->j[1])) {
          prob->graph->n_nodes = n_nodes;
          prob->graph->n_edges = n_edges;
          return(SIPS_INFEASIBLE);
        }
        for(tt = prob->graph->Tmin; tt <= prob->graph->Tmax; tt++) {
          prec_set_dom_ti(tt, cnode->j[1]);
        }
        prec_rev_dom_ti(t, cnode->j[1]);
        ptable->window[cnode->j[1]].s = t;
        ptable->window[cnode->j[1]].e = t;
        ptable->occ[cnode->j[1]] = 1;
        fixed->job[prob->n - fixed->ntail - 1] = prob->sjob[cnode->j[1]];
        fixed->c[prob->n - fixed->ntail - 1] = t;
        fixed->ntail++;
        fixed->ftail += prob->sjob[cnode->j[1]]->f[t];
        f -= ctod(prob->sjob[cnode->j[1]]->f[t]) - u[cnode->j[1]];
        set_fixed(cnode->j[1]);
      }
      t -= prob->sjob[cnode->j[1]]->p;
    }

    if(IS_JOB(cnode->j[2])) {
      if(IS_REAL_JOB(cnode->j[2])) {
        if(is_fixed(cnode->j[2])) {
          prob->graph->n_nodes = n_nodes;
          prob->graph->n_edges = n_edges;
          return(SIPS_INFEASIBLE);
        }
        for(tt = prob->graph->Tmin; tt <= prob->graph->Tmax; tt++) {
          prec_set_dom_ti(tt, cnode->j[2]);
        }
        prec_rev_dom_ti(t, cnode->j[2]);
        ptable->window[cnode->j[2]].s = t;
        ptable->window[cnode->j[2]].e = t;
        ptable->occ[cnode->j[2]] = 1;
        fixed->job[prob->n - fixed->ntail - 1] = prob->sjob[cnode->j[2]];
        fixed->c[prob->n - fixed->ntail - 1] = t;
        fixed->ntail++;
        fixed->ftail += prob->sjob[cnode->j[2]]->f[t];
        f -= ctod(prob->sjob[cnode->j[2]]->f[t]) - u[cnode->j[2]];
        set_fixed(cnode->j[2]);
      }
      t -= prob->sjob[cnode->j[2]]->p;
    }

    e = cnode->e;
  }

  if(t < prob->graph->Tmax) {
    free_memory(bedge, node[prob->graph->Tmax + 1]->e);
    n_edges--;
    node[prob->graph->Tmax + 1]->e = e;
    node[prob->graph->Tmax + 1]->n[0] = cnode->n[0];
    node[prob->graph->Tmax + 1]->v[0] = f;
    cnode->e = NULL;
    for(tt = t + 1; tt <= prob->graph->Tmax; tt++) {
      for(cnode = node[tt]; cnode != NULL; cnode = cnode2) {
        REMOVE_EDGES(cnode);
        cnode2 = cnode->next;
        free_memory(bnode, cnode);
        n_nodes--;
      }
      node[tt] = NULL;
    }
    node[t + 1] = node[prob->graph->Tmax + 1];
    node[prob->graph->Tmax + 1] = NULL;
    prob->graph->Tmax = t;
  }

  prob->graph->n_nodes = n_nodes;
  prob->graph->n_edges = n_edges;

  if(t == prob->graph->Tmin && fixed->nhead + fixed->ntail != prob->n) {
    return(SIPS_INFEASIBLE);
  }

  if(ntail < fixed->ntail) {
    for(i = prob->n - ntail - 1; i > prob->n - fixed->ntail - 1; i--) {
      for(j = prob->n - 1; j > i; j--) {
        if(inc[fixed->job[i]->no][fixed->job[j]->no] == -1) {
          return(SIPS_INFEASIBLE);
        } else if(inc[fixed->job[i]->no][fixed->job[j]->no] != 1) {
          _set_inc(fixed->job[i]->no, fixed->job[j]->no);
        }
      }
      for(j = 0; j < prob->n; j++) {
        if(j != fixed->job[i]->no && inc[j][fixed->job[i]->no] == 0) {
          _set_inc(j, fixed->job[i]->no);
        }
      }
    }
  }

  if(fixed->nhead + fixed->ntail == prob->n && prob->graph->Tmin == prob->graph->Tmax) {
    return(SIPS_OPTIMAL);
  }

  return(SIPS_NORMAL);
}

#define THREE_COST(j0, j1, j2)						\
  prob->sjob[j[j0]]->f[c]						\
  + prob->sjob[j[j1]]->f[c - prob->sjob[j[j0]]->p]			\
  + prob->sjob[j[j2]]->f[c - prob->sjob[j[j0]]->p - prob->sjob[j[j1]]->p]

#define THREE_COST_CHECK(j0, j1, j2) {					\
    g = THREE_COST(j0, j1, j2);						\
    if(obj_lesser(g, f)							\
       || (obj_equal(g, f)						\
	   && prob->sjob[j[j2]]->tno < prob->sjob[j[2]]->tno)) {	\
      return(1);							\
    }									\
  }

/*
 * _check_supernode(prob, j, c)
 *   checks whether there exists a three-job sequence which dominates
 *   the specified sequence.
 *        j: job list
 *        c: completion time
 *
 */
char _check_supernode(sips *prob, unsigned short *j, int c)
{
  cost_t f, g;

  // 2->1->0
  f = THREE_COST(0, 1, 2);
  // 0->2->1
  THREE_COST_CHECK(1, 2, 0);
  // 1->0->2
  THREE_COST_CHECK(2, 0, 1);
  // 0->1->2
  THREE_COST_CHECK(2, 1, 0);

  return(0);
}

/*
 * _check_adj_supernode_forward(prob, n1, n2, c)
 *   checks whether the job sequence represented by the specified nodes 
 *   is dominated by another sequence (in the forward manner).
 *       n1: predecessor node 
 *       n2: successor node
 *        c: completion time of n2
 *
 */
char _check_adj_supernode_forward(sips *prob, _node2m_t *n1, _node2m_t *n2,
				  int c)
{
#if 1
  int k, l;
  int s, s2;
  int j[6];
  _edge2m_t *e;
  _node2m_t *n0;
#else /* 1 */
  int k;
  int s;
  int j[6];
#endif /* 1 */

  if(IS_SOURCE_OR_SINK(n1->j[0]) || IS_SOURCE_OR_SINK(n2->j[0])) {
    return(0);
  }

  k = 5;
  s = c - prob->sjob[n1->j[0]]->p - prob->sjob[n2->j[0]]->p;
  j[k--] = n2->j[0];
  if(IS_JOB(n2->j[2])) {
    j[k--] = n2->j[1];
    j[k--] = n2->j[2];
    s -= prob->sjob[n2->j[2]]->p + prob->sjob[n2->j[1]]->p;
  } else if(IS_JOB(n2->j[1])) {
    j[k--] = n2->j[1];
    s -= prob->sjob[n2->j[1]]->p;
  }

  j[k--] = n1->j[0];
  if(IS_JOB(n1->j[2])) {
    j[k--] = n1->j[1];
    j[k--] = n1->j[2];
    s -= prob->sjob[n1->j[2]]->p + prob->sjob[n1->j[1]]->p;
  } else if(IS_JOB(n1->j[1])) {
    j[k--] = n1->j[1];
    s -= prob->sjob[n1->j[1]]->p;
  }

  switch(k) {
  case -1:
    if(check_four_cost_forward(prob, j + 2,
			       s + prob->sjob[j[0]]->p + prob->sjob[j[1]]->p,
			       c)) {
      return(1);
    }
    if(check_five_cost_forward(prob, j + 1, s + prob->sjob[j[0]]->p, c)) {
      return(1);
    }
    return(check_six_cost_forward(prob, j, s, c));
  case 0:
    if(check_three_cost(prob, j + 3,
			s + prob->sjob[j[1]]->p + prob->sjob[j[2]]->p, c)) {
      return(1);
    }
    if(check_four_cost_forward(prob, j + 2, s + prob->sjob[j[1]]->p, c)) {
      return(1);
    }
    return(check_five_cost_forward(prob, j + 1, s, c));
  case 1:
    if(check_three_cost(prob, j + 3, s + prob->sjob[j[2]]->p, c)) {
      return(1);
    }
    return(check_four_cost_forward(prob, j + 2, s, c));
  case 2:
#if 1
    return(check_three_cost(prob, j + 3, s, c));
#else /* 1 */
    if(check_three_cost(prob, j + 3, s, c)) {
      return(1);
    }
    for(e = n1->e; e != NULL; e = e->next) {
      n0 = e->n;
      if(IS_SOURCE_OR_SINK(n0->j[0])) {
	return(0);
      }
      l = k;
      if(IS_REAL_JOB(n0->j[0])
	 && (n0->j[0] == j[5] || n0->j[0] == j[4] || n0->j[0] == j[3])) {
	continue;
      }
      j[l--] = n0->j[0];
      s2 = s - prob->sjob[n0->j[0]]->p;
      if(IS_JOB(n0->j[2])) {
	j[l--] = n0->j[1];
	j[l--] = n0->j[2];
	if(IS_REAL_JOB(j[2])
	   && (j[2] == j[5] || j[2] == j[4] || j[2] == j[3])) {
	  continue;
	}
	if(IS_REAL_JOB(j[1])
	   && (j[1] == j[5] || j[1] == j[4] || j[1] == j[3])) {
	  continue;
	}
	s2 -= prob->sjob[n0->j[2]]->p + prob->sjob[n0->j[1]]->p;
      } else if(IS_JOB(n0->j[1])) {
	j[l--] = n0->j[1];
	if(IS_REAL_JOB(j[2])
	   && (j[2] == j[5] || j[2] == j[4] || j[2] == j[3])) {
	  continue;
	}
	s2 -= prob->sjob[n0->j[1]]->p;
      }

      switch(l) {
      case -1:
	if(check_four_cost_forward(prob, j + 2,
				   s2 + prob->sjob[j[0]]->p
				   + prob->sjob[j[1]]->p, c)) {
	  continue;
	}
	if(check_five_cost_forward(prob, j + 1, s2 + prob->sjob[j[0]]->p, c)) {
	  continue;
	}
	if(check_six_cost_forward(prob, j, s2, c)) {
	  continue;
	}
	return(0);
      case 0:
	if(check_four_cost_forward(prob, j + 2, s2 + prob->sjob[j[1]]->p, c)) {
	  continue;
	}
	if(check_five_cost_forward(prob, j + 1, s2, c)) {
	  continue;
	}
	return(0);
      case 1:
      default:
	if(check_four_cost_forward(prob, j + 2, s2, c)) {
	  continue;
	}
	return(0);
      }
    }

    return(1);
#endif /* 1 */
  default:
    break;
  }

#if 1
  for(e = n1->e; e != NULL; e = e->next) {
    n0 = e->n;
    if(IS_SOURCE_OR_SINK(n0->j[0])) {
      return(0);
    }
    l = k;
    if(IS_REAL_JOB(n0->j[0]) && (n0->j[0] == j[5] || n0->j[0] == j[4])) {
      continue;
    }
    j[l--] = n0->j[0];
    s2 = s - prob->sjob[n0->j[0]]->p;
    if(IS_JOB(n0->j[2])) {
      j[l--] = n0->j[1];
      j[l--] = n0->j[2];
      if(IS_REAL_JOB(j[2]) && (j[2] == j[5] || j[2] == j[4])) {
	continue;
      }
      if(IS_REAL_JOB(j[1]) && (j[1] == j[5] || j[1] == j[4])) {
	continue;
      }
      s2 -= prob->sjob[n0->j[2]]->p + prob->sjob[n0->j[1]]->p;
    } else if(IS_JOB(n0->j[1])) {
      j[l--] = n0->j[1];
      if(IS_REAL_JOB(j[2]) && (j[2] == j[5] || j[2] == j[4])) {
	continue;
      }
      s2 -= prob->sjob[n0->j[1]]->p;
    }

    switch(l) {
    case 0:
      if(check_three_cost(prob, j + 3,
			  s2 + prob->sjob[j[1]]->p + prob->sjob[j[2]]->p, c)) {
	continue;
      }
      if(check_four_cost_forward(prob, j + 2, s2 + prob->sjob[j[1]]->p, c)) {
	continue;
      }
      if(check_five_cost_forward(prob, j + 1, s2, c)) {
	continue;
      }
      return(0);
    case 1:
      if(check_three_cost(prob, j + 3, s2 + prob->sjob[j[2]]->p, c)) {
	continue;
      }
      if(check_four_cost_forward(prob, j + 2, s2, c)) {
	continue;
      }
      return(0);
    case 2:
    default:
      if(check_three_cost(prob, j + 3, s2, c)) {
	continue;
      }
      return(0);
    }
  }

  return(1);
#else /* 1 */
  return(0);
#endif /* 1 */
}

/*
 * _check_adj_supernode_backward(prob, n1, n2, c)
 *   checks whether the job sequence represented by the specified nodes 
 *   is dominated by another sequence (in the backward manner).
 *       n1: predecessor node 
 *       n2: successor node
 *        c: completion time of n1
 *
 */
char _check_adj_supernode_backward(sips *prob, _node2m_t *n1, _node2m_t *n2,
				   int c)
{
#if 1
  int k, l;
  int s, c2;
  int j[6];
  _edge2m_t *e;
  _node2m_t *n3;
#else /* 1 */
  int k;
  int s;
  int j[6];
#endif /* 1 */

  if(IS_SOURCE_OR_SINK(n1->j[0]) || IS_SOURCE_OR_SINK(n2->j[0])) {
    return(0);
  }

  k = 0;
  s = c - prob->sjob[n1->j[0]]->p;
  c += prob->sjob[n2->j[0]]->p;
  if(IS_JOB(n1->j[2])) {
    j[k++] = n1->j[2];
    j[k++] = n1->j[1];
    s -= prob->sjob[n1->j[2]]->p + prob->sjob[n1->j[1]]->p;
  } else if(IS_JOB(n1->j[1])) {
    j[k++] = n1->j[1];
    s -= prob->sjob[n1->j[1]]->p;
  }
  j[k++] = n1->j[0];

  if(IS_JOB(n2->j[2])) {
    j[k++] = n2->j[2];
    j[k++] = n2->j[1];
    c += prob->sjob[n2->j[2]]->p + prob->sjob[n2->j[1]]->p;
  } else if(IS_JOB(n2->j[1])) {
    j[k++] = n2->j[1];
    c += prob->sjob[n2->j[1]]->p;
  }
  j[k++] = n2->j[0];

  switch(k) {
  case 6:
    if(check_four_cost_backward(prob, j, s,
				c - prob->sjob[j[5]]->p
				- prob->sjob[j[4]]->p)) {
      return(1);
    }
    if(check_five_cost_backward(prob, j, s, c -  prob->sjob[j[5]]->p)) {
      return(1);
    }
    return(check_six_cost_backward(prob, j, s, c));
  case 5:
    if(check_three_cost(prob, j, s,
			c - prob->sjob[j[4]]->p - prob->sjob[j[3]]->p)) {
      return(1);
    }
    if(check_four_cost_backward(prob, j, s, c - prob->sjob[j[4]]->p)) {
      return(1);
    }
    return(check_five_cost_backward(prob, j, s, c));
  case 4:
    if(check_three_cost(prob, j, s, c - prob->sjob[j[3]]->p)) {
      return(1);
    }
    return(check_four_cost_backward(prob, j, s, c));
  case 3:
#if 1
    return(check_three_cost(prob, j, s, c));
#else /* 1 */
    if(check_three_cost(prob, j, s, c)) {
      return(1);
    }
    for(e = n2->e; e != NULL; e = e->next) {
      n3 = e->n;
      if(IS_SOURCE_OR_SINK(n3->j[0])) {
	return(0);
      }

      l = k;
      c2 = c + prob->sjob[n3->j[0]]->p;
      if(IS_JOB(n3->j[2])) {
	j[l++] = n3->j[2];
	j[l++] = n3->j[1];
	if(IS_REAL_JOB(j[3])
	   && (j[3] == j[0] || j[3] == j[1] || j[3] == j[2])) {
	  continue;
	}
	if(IS_REAL_JOB(j[4])
	   && (j[4] == j[0] || j[4] == j[1] || j[4] == j[2])) {
	  continue;
	}
	c2 += prob->sjob[n3->j[2]]->p + prob->sjob[n3->j[1]]->p;
      } else if(IS_JOB(n3->j[1])) {
	j[l++] = n3->j[1];
	if(IS_REAL_JOB(j[3])
	   && (j[3] == j[0] || j[3] == j[1] || j[3] == j[2])) {
	  continue;
	}
	c2 += prob->sjob[n3->j[1]]->p;
      }
      if(IS_REAL_JOB(n3->j[0])
	 && (n3->j[0] == j[0] || n3->j[0] == j[1] || n3->j[0] == j[2])) {
	continue;
      }
      j[l++] = n3->j[0];

      switch(l) {
      case 6:
	if(check_four_cost_backward(prob, j, s,
				    c2 - prob->sjob[j[5]]->p
				    - prob->sjob[j[4]]->p)) {
	  continue;
	}
	if(check_five_cost_backward(prob, j, s,
				    c2 - prob->sjob[j[5]]->p)) {
	  continue;
	}
	if(check_six_cost_backward(prob, j, s, c2)) {
	  continue;
	}
	return(0);
      case 5:
	if(check_four_cost_backward(prob, j, s, c2 - prob->sjob[j[4]]->p)) {
	  continue;
	}
	if(check_five_cost_backward(prob, j, s, c2)) {
	  continue;
	}
	return(0);
      case 4:
      default:
	if(check_four_cost_backward(prob, j, s, c2)) {
	  continue;
	}
	return(0);
      }
    }

    return(1);
#endif /* 1 */
  default:
    break;
  }

#if 1
  for(e = n2->e; e != NULL; e = e->next) {
    n3 = e->n;
    if(IS_SOURCE_OR_SINK(n3->j[0])) {
      return(0);
    }
    l = k;
    c2 = c + prob->sjob[n3->j[0]]->p;
    if(IS_JOB(n3->j[2])) {
      j[l++] = n3->j[2];
      j[l++] = n3->j[1];
      if(IS_REAL_JOB(j[2]) && (j[2] == j[0] || j[2] == j[1])) {
	continue;
      }
      if(IS_REAL_JOB(j[3]) && (j[3] == j[0] || j[3] == j[1])) {
	continue;
      }
      c2 += prob->sjob[n3->j[2]]->p + prob->sjob[n3->j[1]]->p;
    } else if(IS_JOB(n3->j[1])) {
      j[l++] = n3->j[1];
      if(IS_REAL_JOB(j[2]) && (j[2] == j[0] || j[2] == j[1])) {
	continue;
      }
      c2 += prob->sjob[n3->j[1]]->p;
    }
    if(IS_REAL_JOB(n3->j[0]) && (n3->j[0] == j[0] || n3->j[0] == j[1])) {
      continue;
    }
    j[l++] = n3->j[0];
    
    switch(l) {
    case 5:
      if(check_three_cost(prob, j, s,
			  c2 - prob->sjob[j[4]]->p - prob->sjob[j[3]]->p)) {
	continue;
      }
      if(check_four_cost_backward(prob, j, s, c2 - prob->sjob[j[4]]->p)) {
	continue;
      }
      if(check_five_cost_backward(prob, j, s, c2)) {
	continue;
      }
      return(0);
    case 4:
      if(check_three_cost(prob, j, s, c2 - prob->sjob[j[3]]->p)) {
	continue;
      }
      if(check_four_cost_backward(prob, j, s, c2)) {
	continue;
      }
      return(0);
    case 3:
    default:
      if(check_three_cost(prob, j, s, c2)) {
	continue;
      }
      return(0);
    }
  }

  return(1);
#else /* 1 */
  return(0);
#endif /* 1 */
}

/*
 * _check_none(prob, c, jn, n)
 *   dummy function.
 *        c: time
 *       jn: job completed at time c
 *        n: node connected to job jn
 *
 */
char _check_none(sips *prob, int c, int jn, _node2m_t *n)
{
  return(0);
}

/*
 * _check_three_forward(prob, c, jn, n)
 *   checks whether the specified arc can be eliminated by the dominance
 *   of three successive jobs (in the forward manner).
 *        c: time
 *       jn: job completed at time c
 *        n: node connected to job jn
 *
 */
char _check_three_forward(sips *prob, int c, int jn, _node2m_t *n)
{
  int s;
  int j[3];
  _edge2m_t *e;

#if 0
  if(IS_SOURCE_OR_SINK(jn)) {
    return(0);
  }
#endif /* 0 */
  if((j[1] = n->j[0]) == N_JOBS) {
    return(0);
  }

  j[2] = jn;
  s = c - prob->sjob[j[2]]->p - prob->sjob[j[1]]->p;
  for(e = n->e; e != NULL; e = e->next) {
    if(ELIMINATED_NODE(e->n)) {
      continue;
    }
    if((j[0] = e->n->j[0]) == N_JOBS) {
      return(0);
    } else if(IS_REAL_JOB(j[0]) && j[0] == j[2]) {
      continue;
    }
    if(!check_three_cost(prob, j, s - prob->sjob[j[0]]->p, c)) {
      return(0);
    }
  }

  return(1);
}

/*
 * _check_four_forward(prob, c, jn, n)
 *   checks whether the specified arc can be eliminated by the dominance
 *   of four successive jobs (in the forward manner).
 *        c: time
 *       jn: job completed at time c
 *        n: node connected to job jn
 *
 */
char _check_four_forward(sips *prob, int c, int jn, _node2m_t *n)
{
  int s;
  int j[4];
  _edge2m_t *e, *e2;

#if 0
  if(IS_SOURCE_OR_SINK(jn)) {
    return(0);
  }
#endif /* 0 */
  if((j[2] = n->j[0]) == N_JOBS) {
    return(0);
  }

  j[3] = jn;
  for(e = n->e; e != NULL; e = e->next) {
    if(ELIMINATED_NODE(e->n)) {
      continue;
    }
    if((j[1] = e->n->j[0]) == N_JOBS) {
      return(0);
    } else if(IS_REAL_JOB(j[1]) && j[1] == j[3]) {
      continue;
    }

    s = c - prob->sjob[j[3]]->p - prob->sjob[j[2]]->p - prob->sjob[j[1]]->p;
    if(!check_three_cost(prob, j + 1, s, c)) {
      for(e2 = e->n->e; e2 != NULL; e2 = e2->next) {
        if(ELIMINATED_NODE(e2->n)) {
          continue;
        }
        if((j[0] = e2->n->j[0]) == N_JOBS) {
          return(0);
        } else if(IS_REAL_JOB(j[0]) && (j[0] == j[3] || j[0] == j[2])) {
          continue;
        }
        if(!check_four_cost_forward(prob, j, s - prob->sjob[j[0]]->p, c)) {
          return(0);
        }
      }
    }
  }

  return(1);
}

/*
 * _check_five_forward(prob, c, jn, n)
 *   checks whether the specified arc can be eliminated by the dominance
 *   of five successive jobs (in the forward manner).
 *        c: time
 *       jn: job completed at time c
 *        n: node connected to job jn
 *
 */
char _check_five_forward(sips *prob, int c, int jn, _node2m_t *n)
{
  int s1, s2;
  int j[5];
  _edge2m_t *e, *e2, *e3;

#if 0
  if(IS_SOURCE_OR_SINK(jn)) {
    return(0);
  }
#endif /* 0 */
  if((j[3] = n->j[0]) == N_JOBS) {
    return(0);
  }

  j[4] = jn;
  for(e = n->e; e != NULL; e = e->next) {
    if(ELIMINATED_NODE(e->n)) {
      continue;
    }
    if((j[2] = e->n->j[0]) == N_JOBS) {
      return(0);
    } else if(IS_REAL_JOB(j[2]) && j[2] == j[4]) {
      continue;
    }

    s1 = c - prob->sjob[j[4]]->p - prob->sjob[j[3]]->p - prob->sjob[j[2]]->p;
    if(!check_three_cost(prob, j + 2, s1, c)) {
      for(e2 = e->n->e; e2 != NULL; e2 = e2->next) {
	if(ELIMINATED_NODE(e2->n)) {
	  continue;
	}
	if((j[1] = e2->n->j[0]) == N_JOBS) {
	  return(0);
	} else if(IS_REAL_JOB(j[1]) && (j[1] == j[4] || j[1] == j[3])) {
	  continue;
	}

	s2 = s1 - prob->sjob[j[1]]->p;
	if(!check_four_cost_forward(prob, j + 1, s2, c)) {
	  for(e3 = e2->n->e; e3 != NULL; e3 = e3->next) {
	    if(ELIMINATED_NODE(e3->n)) {
	      continue;
	    }
	    if((j[0] = e3->n->j[0]) == N_JOBS) {
	      return(0);
	    } else if(IS_REAL_JOB(j[0])
		      && (j[0] == j[4] || j[0] == j[3] || j[0] == j[2])) {
	      continue;
	    }
	    if(!check_five_cost_forward(prob, j,
					s2 - prob->sjob[j[0]]->p, c)) {
	      return(0);
	    }
	  }
	}
      }
    }
  }

  return(1);
}

/*
 * _check_six_forward(prob, c, jn, n)
 *   checks whether the specified arc can be eliminated by the dominance
 *   of six successive jobs (in the forward manner).
 *        c: time
 *       jn: job completed at time c
 *        n: node connected to job jn
 *
 */
char _check_six_forward(sips *prob, int c, int jn, _node2m_t *n)
{
  int s1, s2, s3;
  int j[6];
  _edge2m_t *e, *e2, *e3, *e4;

#if 0
  if(IS_SOURCE_OR_SINK(jn)) {
    return(0);
  }
#endif /* 0 */
  if((j[4] = n->j[0]) == N_JOBS) {
    return(0);
  }

  j[5] = jn;
  for(e = n->e; e != NULL; e = e->next) {
    if(ELIMINATED_NODE(e->n)) {
      continue;
    }
    if((j[3] = e->n->j[0]) == N_JOBS) {
      return(0);
    } else if(IS_REAL_JOB(j[3]) && j[3] == j[5]) {
      continue;
    }

    s1 = c - prob->sjob[j[5]]->p - prob->sjob[j[4]]->p - prob->sjob[j[3]]->p;
    if(!check_three_cost(prob, j + 3, s1, c)) {
      for(e2 = e->n->e; e2 != NULL; e2 = e2->next) {
	if(ELIMINATED_NODE(e2->n)) {
	  continue;
	}
	if((j[2] = e2->n->j[0]) == N_JOBS) {
	  return(0);
	} else if(IS_REAL_JOB(j[2]) && (j[2] == j[5] || j[2] == j[4])) {
	  continue;
	}

	s2 = s1 - prob->sjob[j[2]]->p;
	if(!check_four_cost_forward(prob, j + 2, s2, c)) {
	  for(e3 = e2->n->e; e3 != NULL; e3 = e3->next) {
	    if(ELIMINATED_NODE(e3->n)) {
	      continue;
	    }
	    if((j[1] = e3->n->j[0]) == N_JOBS) {
	      return(0);
	    } else if(IS_REAL_JOB(j[1])
		      && (j[1] == j[5] || j[1] == j[4] || j[1] == j[3])) {
	      continue;
	    }

	    s3 = s2 - prob->sjob[j[1]]->p;
	    if(!check_five_cost_forward(prob, j + 1, s3, c)) {
	      for(e4 = e3->n->e; e4 != NULL; e4 = e4->next) {
		if(ELIMINATED_NODE(e4->n)) {
		  continue;
		}
		if((j[0] = e4->n->j[0]) == N_JOBS) {
		  return(0);
		} else if(IS_REAL_JOB(j[0])
			  && (j[0] == j[5] || j[0] == j[4] || j[0] == j[3]
			      || j[0] == j[2])) {
		  continue;
		}
		if(!check_six_cost_forward(prob, j,
					   s3 - prob->sjob[j[0]]->p, c)) {
		  return(0);
		}
	      }
	    }
	  }
	}
      }
    }
  }

  return(1);
}

/*
 * _compare_occur(a, b)
 *   compares the occurrences of two jobs in the current solution.
 *   Ties are broken by the occurences in the DP states.
 *
 */
int _compare_occur(const void *a, const void *b)
{
  _occur_t *x = (_occur_t *) a;
  _occur_t *y = (_occur_t *) b;

  if(x->no > y->no) {
    return(1);
  } else if(x->no < y->no) {
    return(-1);
  } else if(x->nd < y->nd) {
    return(1);
  } else if(x->nd > y->nd) {
    return(-1);
 }

  return(0);
}

/*
 * _compare_occur2(a, b)
 *   compares occurrences of two jobs in the DP states.
 *   Ties are broken by the occurences in the current solution.
 *
 */
int _compare_occur2(const void *a, const void *b)
{
  _occur_t *x = (_occur_t *) a;
  _occur_t *y = (_occur_t *) b;

  if(x->nd > y->nd) {
    return(1);
  } else if(x->nd < y->nd) {
    return(-1);
  } else if(x->no > y->no) {
    return(1);
  } else if(x->no < y->no) {
    return(-1);
  }

  return(0);
}
