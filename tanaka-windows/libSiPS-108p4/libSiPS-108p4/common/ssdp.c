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
 *  $Id: ssdp.c,v 1.43 2022/12/05 05:45:43 tanaka Rel $
 *  $Revision: 1.43 $
 *  $Date: 2022/12/05 05:45:43 $
 *  $Author: tanaka $
 *
 */

/*
 * [NOTE]
 *   The SSDP (successive sublimation dynamic programing) method
 *   was originally proposed in:
 *
 *     T. Ibaraki. 1987.
 *     Enumerative approaches to combinatorial optimization,
 *     Annals of Operations Resarch 10 and 11.
 *
 *     T. Ibaraki and Y. Nakamura. 1994.
 *     A Dynamic Programming Method for Single Machine Scheduling,
 *     European Journal of Operational Research 76, 72/82.
 *
 */
#include <stdio.h>
#include <string.h>

#include "define.h"
#include "sips_common.h"
#include "dynasearch.h"
#include "heuristics.h"
#include "job_sort.h"
#include "lag_common.h"
#include "lag.h"
#include "lag2.h"
#include "memory.h"
#include "sol.h"
#include "ssdp.h"
#include "subgradient.h"
#include "timer.h"

static int _stage0(sips *);
static int _stage1(sips *, _real *);
static int _stage2(sips *, _real *);
static int _stage3(sips *);
static int _stage3_loop(sips *, cost_t *, _real, _real *);

/*
 * ssdp(prob)
 *   applies the SSDP method.
 *
 */
int ssdp(sips *prob)
{
  int ret;
  _real lb;

  ret = _stage0(prob);
  if(ret == SIPS_SOLVED) {
    if(prob->param->verbose >= 2) {
      fprintf(stdout, "Solved in Stage 1.\n");
    }

    return(ret);
  } else if(is_true(prob->param->ub) || large(prob->sol->f)) {
    return(ret);
  }

  lb = - LARGE_REAL;
  ret = _stage1(prob, &lb);
  if(ret != SIPS_UNSOLVED) {
    if(ret == SIPS_SOLVED) {
      if(prob->param->verbose >= 2) {
	fprintf(stdout, "Solved in Stage 1.\n");
      }
    }

    return(ret);
  }

  ret = _stage2(prob, &lb);
  if(ret != SIPS_UNSOLVED || is_true(prob->param->lb)) {
    if(ret == SIPS_SOLVED) {
      if(prob->param->verbose >= 2) {
	fprintf(stdout, "Solved in Stage 2.\n");
      }
    }

    return(ret);
  }

  ret = _stage3(prob);

  return(ret);
}

/*
 * _stage0(prob)
 *   computes the initial upper bound.
 *
 */
int _stage0(sips *prob)
{
  int i, j;
  int *no;
  char *e;
  _solution_t *sol2;
  _job_t **job, *pjob;

  job = (_job_t **) xmalloc(prob->n*sizeof(_job_t *));

  prob->stage = 0;
  prob->sol->f = LARGE_COST;
  if(prob->niseq > 0) {
    prob->sol->n = 0;
    e = (char *) xcalloc(prob->n, 1);
    for(i = 0; i < prob->niseq; i++) {
      if(prob->iseq[i] >= 0 && prob->iseq[i] < prob->n
	 && e[prob->job[prob->iseq[i]].no] == 0) {
	prob->sol->job[prob->sol->n] = &(prob->job[prob->iseq[i]]);
	e[prob->sol->job[prob->sol->n++]->no] = 1;
      }
    }

    if(prob->sol->n > 0 && prob->sol->n != prob->n) {
      for(i = j = 0; i < prob->n; i++) {
	if(!e[i]) {
	  job[j++] = prob->sjob[i];
	}
      }

      insert_ordered_jobs_greedily(prob, prob->sol, j, job);
    } else {
      solution_set_c(prob, prob->sol);
    }

    xfree(e);
  } else if(is_true(prob->param->warm)) {
    j = -1;
    for(i = 0; i < prob->n; i++) {
      j = max(prob->sjob[i]->sno, j);
    }
    if(j >= 0) {
      no = (int *) xcalloc(j + 1, sizeof(int));
      e = (char *) xcalloc(prob->n, 1);

      for(i = 0; i < prob->n; i++) {
	if(prob->sjob[i]->sno >= 0) {
	  no[prob->sjob[i]->sno] = i + 1;
	}
      }

      for(i = 0; i <= j; i++) {
	if(no[i] > 0) {
	  prob->sol->job[prob->sol->n++] = prob->sjob[no[i] - 1];
	  e[no[i] - 1] = 1;
	}
      }

      if(prob->sol->n < prob->n) {
	for(i = j = 0; i < prob->n; i++) {
	  if(!e[i]) {
	    job[j++] = prob->sjob[i];
	  }
	}

	insert_ordered_jobs_greedily(prob, prob->sol, j, job);
      } else {
	solution_set_c(prob, prob->sol);
      }

      xfree(e);
      xfree(no);
    }
  }

  if(obj_lesser_equal(prob->sol->f, ZERO_COST)) {
    xfree(job);
    return(SIPS_SOLVED);
  }

  sol2 = create_solution(prob);

  /* spt */
  sol2->n = 0;
  insert_ordered_jobs_greedily(prob, sol2, prob->n, prob->sjob);
  if(prob->sol->n < prob->n || obj_lesser(sol2->f, prob->sol->f)) {
    copy_solution(prob, prob->sol, sol2);

    if(obj_lesser_equal(prob->sol->f, ZERO_COST)) {
      free_solution(sol2);
      xfree(job);
      return(SIPS_SOLVED);
    }
  }

  /* lpt */
  for(i = 0; i < prob->n; i++) {
    job[i] = prob->sjob[prob->n - i - 1];
  }

  sol2->n = 0;
  insert_ordered_jobs_greedily(prob, sol2, prob->n, job);
  if(obj_lesser(sol2->f, prob->sol->f)) {
    copy_solution(prob, prob->sol, sol2);

    if(obj_lesser_equal(prob->sol->f, ZERO_COST)) {
      free_solution(sol2);
      xfree(job);
      return(SIPS_SOLVED);
    }
  }

  /* edd */
  sort_jobs(prob->n, job, 0);

  sol2->n = 0;
  insert_ordered_jobs_greedily(prob, sol2, prob->n, job);
  if(obj_lesser(sol2->f, prob->sol->f)) {
    copy_solution(prob, prob->sol, sol2);

    if(obj_lesser_equal(prob->sol->f, ZERO_COST)) {
      free_solution(sol2);
      xfree(job);
      return(SIPS_SOLVED);
    }
  }

  /* ldd */
  for(i = 0, j = prob->n - 1; i < j; i++, j--) {
    pjob = job[i];
    job[i] = job[j];
    job[j] = pjob;
  }

  sol2->n = 0;
  insert_ordered_jobs_greedily(prob, sol2, prob->n, job);
  if(obj_lesser(sol2->f, prob->sol->f)) {
    copy_solution(prob, prob->sol, sol2);

    if(obj_lesser_equal(prob->sol->f, ZERO_COST)) {
      free_solution(sol2);
      xfree(job);
      return(SIPS_SOLVED);
    }
  }

  free_solution(sol2);
  xfree(job);

  if(prob->param->ls == SIPS_LS_EDYNA) {
    edynasearch(prob, prob->sol);
  } else if(prob->param->ls == SIPS_LS_DYNA
	    || prob->param->ls == SIPS_LS_COMBINED_A
	    || prob->param->ls == SIPS_LS_COMBINED_B) {
    dynasearch(prob, prob->sol);
  }

  if(obj_lesser_equal(prob->sol->f, ZERO_COST)) {
    return(SIPS_SOLVED);
  }

  return(SIPS_UNSOLVED);
}

/*
 * _stage1(prob, lb)
 *   SSDP stage 1.
 *       lb: lower bound
 *
 */
int _stage1(sips *prob, _real *lb)
{
  int ret;

  if(prob->param->verbose >= 1) {
    fprintf(stdout, "Stage 1\n");
  }
  prob->stage = 1;
  
  ret = subgradient_LR1(prob, lb);
  if(ret != SIPS_UNSOLVED) {
    lag_free(prob);
    return(ret);
  }

  return(SIPS_UNSOLVED);
}

/*
 * _stage2(prob, lb)
 *   SSDP stage 2.
 *       lb: lower bound
 *
 */
int _stage2(sips *prob, _real *lb)
{
  int ret;

  if(prob->param->verbose >= 1) {
    fprintf(stdout, "Stage 2\n");
  }
  prob->stage = 2;

  ret = subgradient_LR2adj(prob, lb);
  if(ret != SIPS_UNSOLVED) {
    lag_free(prob);
    return(ret);
  }

  lag_free_LR2adj_solver(prob);

  return(SIPS_UNSOLVED);
}

#define PRINT_LBUB(fp, nn) {						\
    fprintf(fp, "[%3u] LB=", nn);					\
    print_real(fp, .4, lb + ctod(prob->off));				\
    fprintf(fp, ", tentative UB=");					\
    print_cost(fp, *ub + prob->off);					\
    fprintf(fp, ", true UB=");						\
    print_current_objective(fp);					\
    fprintf(fp, "\n");							\
  }
#define PRINT_LBUB2(fp, nn) {						\
    fprintf(fp, "[%3u] LB=", nn);					\
    print_real(fp, .4, lb2 + ctod(prob->off));				\
    fprintf(fp, ", tentative UB=");					\
    print_cost(fp, *ub + prob->off);					\
    fprintf(fp, ", true UB=");						\
    print_current_objective(fp);					\
    fprintf(fp, "\n");							\
  }
#define PRINT_UBUPDATED(fp, nn, xsol) {					\
    if(prob->param->verbose >= 2) {					\
      fprintf(fp, "[%3u] UB updated. ", nn);				\
      print_current_objective(fp);					\
      fprintf(fp, "->");						\
      print_cost(fp, xsol->f + prob->off);				\
      fprintf(fp, "\n");						\
    } else if(prob->param->verbose >= 1) {				\
      fprintf(fp, "UB=");						\
      print_cost(fp, xsol->f + prob->off);				\
      fprintf(fp, "\n");						\
    }									\
}
#define PRINT_TUBUPDATED(fp, nn, xsol) {				\
    if(prob->param->verbose >= 2) {					\
      fprintf(fp, "[%3u] true UB updated. ", nn);			\
      print_current_objective(fp);					\
      fprintf(fp, "->");						\
      print_cost(fp, xsol->f + prob->off);				\
      fprintf(fp, "\n");						\
    } else if(prob->param->verbose >= 1) {				\
      fprintf(fp, "UB=");						\
      print_cost(fp, xsol->f + prob->off);				\
      fprintf(fp, "\n");						\
    }									\
}
#define PRINT_NETWORKSIZE(fp, nn) {					\
    fprintf(fp, "[%3u] (N, A)=(%u, %u), ",				\
	    nn, prob->graph->n_nodes, prob->graph->n_edges);		\
    print_real(fp, .2, lag2_get_memory_in_MB(prob));			\
    fprintf(fp, "MB (");						\
    print_real(fp, .2, lag2_get_real_memory_in_MB(prob));		\
    fprintf(fp, "MB), time=");						\
    _print_time(prob, fp);						\
}
#define PRINT_TIMELIMIT_SHORT(fp) {					\
    fprintf(fp, "Reached time limit (");				\
    print_real2(fp, prob->param->tlimit);				\
    fprintf(fp, "s).\n");						\
  }
#define PRINT_TIMELIMIT(fp, nn)	{					\
    fprintf(fp, "[%3u] ", nn);						\
    PRINT_TIMELIMIT_SHORT(fp);						\
  }
#define PRINT_MAXMEMORY(fp, nn)						\
    fprintf(fp, "[%3u] Reached maximum memory (%uMB). Terminating.\n",	\
	    nn, prob->param->mem);
#define PRINT_MAXMEMORY2(fp) 						\
    fprintf(fp, "Reached maximum memory (%uMB).\n", prob->param->mem);

/*
 * _stage3(prob)
 *   SSDP stage 3.  The tentative upper bound is increased while Stage 3 
 *   of the original algorithm is applied iteratively.
 *
 */
int _stage3(sips *prob)
{
  int i;
  int iter;
  int ret;
  cost_t ub, f;
  _real lb, lboff;
  _real r, lbprev, lbbest;
  _real *u;
  _solution_t *tmpsol;

  if(prob->param->verbose >= 1) {
    fprintf(stdout, "Stage 3\n");
  }
  prob->stage = 3;

  u = (_real *) xcalloc(N_JOBS + 1, sizeof(_real));

  lboff = 0.0;
  for(i = 0; i < prob->n; i++) {
    u[i] = prob->sjob[i]->u;
    lboff += u[i];
  }

  tmpsol = create_solution(prob);

  ret = lag2_initialize(prob, u, ctod(prob->sol->f) - lboff,
			tmpsol, &lb, NULL);
  lb += lboff;

  _get_time(prob);

  if(ret == SIPS_OPTIMAL) {
    if(obj_lesser(tmpsol->f, prob->sol->f)) {
      PRINT_UBUPDATED(stdout, 0, tmpsol);
      copy_solution(prob, prob->sol, tmpsol);
    }
  }

  free_solution(tmpsol);

  if(ret == SIPS_OPTIMAL || ret == SIPS_INFEASIBLE) {
#ifdef LB_DEBUG
    if(ret != SIPS_OPTIMAL) {
      fprintf(stderr, "Feasibility error.\n");
      exit(1);
    } else {
      fprintf(stdout, "[%3u] Feasible.\n", 0);
    }
#endif /* LB_DEBUG */
    if(prob->param->verbose >= 2) {
      fprintf(stdout, "[%3u] Solved.\n", 0);
    }
    ret = SIPS_SOLVED;
  } else if(ret == SIPS_MEMLIMIT) {
    if(prob->param->verbose >= 2) {
      PRINT_NETWORKSIZE(stdout, 0);
      PRINT_MAXMEMORY(stdout, 0);
    } else if(prob->param->verbose >= 1) {
      PRINT_MAXMEMORY2(stdout);
    }
  }

  if(ret != SIPS_NORMAL) {
    xfree(u);
    lag2_free(prob);
    return(ret);
  }

  lag2_push_nodes(prob);

  if(prob->param->secsw >= 0
     && prob->graph->copy->mem >= (_real) prob->param->secsw) {
    if(prob->param->secratio <= prob->param->eps) {
      lbprev = (ctod(prob->sol->f) + lb)/2.0;
      r = (ctod(prob->sol->f) - lbprev)/2.0;
    } else {
      r = ctod(prob->sol->f)*prob->param->secratio;
      if(prob->param->seciniratio > prob->param->eps) {
	lbprev = lb + ctod(prob->sol->f)*prob->param->seciniratio;
      } else {
	lbprev = lb + r;
      }

      if(lbprev >= ctod(prob->sol->f)) {
	lbprev = ctod(prob->sol->f);
      }
    }
  } else {
    lbprev = ctod(prob->sol->f);
    r = ctod(prob->sol->f);
  }

  lbbest = lb;
  f = prob->sol->f;
  for(iter = 1;; iter++) {
#ifdef COST_REAL
    ub = lbprev;
    if(obj_lesser(prob->sol->f, ub)) {
      ub = prob->sol->f;
      if(prob->param->bisec) {
	ret = lag2_recover_nodes(prob);
      } else {
	ret = lag2_pop_nodes(prob);
      }
    } else {
      ret = lag2_recover_nodes(prob);
    }
#else /* COST_REAL */
    ub = (cost_t) (lbprev + 0.5 + prob->param->eps);
    while(ub <= (cost_t) (lbbest + prob->param->eps)) {
      ub++;
    }

    if(obj_lesser_equal(prob->sol->f, ub)) {
      ub = prob->sol->f;
      if(prob->param->bisec) {
	ret = lag2_recover_nodes(prob);
      } else {
	ret = lag2_pop_nodes(prob);
      }
    } else {
      ret = lag2_recover_nodes(prob);
    }
#endif /* COST_REAL */

    if(ret == SIPS_MEMLIMIT) {
      if(prob->param->verbose >= 1) {
	PRINT_MAXMEMORY2(stdout);
      }
      break;
    }

    if(prob->param->ls == SIPS_LS_COMBINED_A && iter >= 3
       && obj_equal(prob->sol->f, ub) && obj_lesser_equal(prob->sol->f, f)) {
      edynasearch(prob, prob->sol);
      if(obj_lesser(prob->sol->f, ub)) {
	if(prob->param->verbose == 1) {
	  fprintf(stdout, "UB=");
	  print_cost(stdout, prob->sol->f + prob->off);
	  fprintf(stdout, "\n");
	}
	ub = prob->sol->f;
      }
    }

    if(prob->param->verbose >= 2) {
      fprintf(stdout, "Iteration %d: tentative UB=", iter);
      print_cost(stdout, ub + prob->off);
      fprintf(stdout, ", true UB=");
      print_current_objective(stdout);
      fprintf(stdout, "\n");
    }

    ret = _stage3_loop(prob, &ub, lboff, u);

    if(ret == SIPS_SOLVED) {
      if(obj_lesser_equal(prob->sol->f, ub)) {
	break;
      }
      lbbest = ctod(ub);
      lbprev += r;
    } else if(ret == SIPS_MEMLIMIT) {
      if(!prob->param->bisec) {
	break;
      }
      if(obj_lesser_equal(prob->sol->f, ub)) {
	lbprev = ctod(ub);
      }
      if(lbprev - lbbest <= 1.0) {
	break;
      }
      r = (lbprev - lbbest)/2.0;
      lbprev = lbbest + r;
    } else {
      break;
    }
  }

  xfree(u);

  lag2_free_copy(prob);
  lag2_free(prob);

  if(ret != SIPS_SOLVED) {
    if(prob->param->verbose >= 2) {
      fprintf(stdout, "      Lower bound: ");
      print_real(stdout, .4, lbbest + (_real) prob->off);
      fprintf(stdout, "\n");
    }
  } else {
    if(prob->param->verbose >= 2) {
      fprintf(stdout, "    UB Iterations: %d\n", iter);
    }
  }

  return(ret);
}

/*
 * _stage3_loop(prob, ub, lboff, u)
 *   SSDP stage 3.
 *       ub: tentative upper bound
 *    lboff: sum of Lagrangian multipliers
 *        u: Lagrangian multipliers
 *
 */
int _stage3_loop(sips *prob, cost_t *ub, _real lboff, _real *u)
{
  int i;
  unsigned int max_nedges, max_nnodes;
  unsigned char type;
  unsigned char nmod;
  int ret, heavy;
  _real lb, lb2, lbprev;
  _real mb;
  unsigned int *o;
  _real *ud;
  _solution_t *tmpsol, *tmpsol2;
  _mod_t *mod;

  o = (unsigned int *) xmalloc(prob->n*sizeof(unsigned int));
  mod = (_mod_t *) xmalloc(sizeof(_mod_t));
  mod->m = (unsigned char *) xmalloc(N_JOBS + 2);
  mod->fl = (unsigned char *) xcalloc(prob->n, 1);
  mod->jobs = (int *) xmalloc(prob->n*sizeof(int));
  mod->v = (unsigned char **) xmalloc((N_JOBS + 2)*sizeof(unsigned char *));
  mod->v[0] = (unsigned char *) xmalloc((N_JOBS + 2)*(1<<prob->param->mod));
  ud = (_real *) xcalloc(N_JOBS + 1, sizeof(_real));
  for(i = 1; i <= N_JOBS + 1; i++) {
    mod->v[i] = mod->v[i - 1] + (1<<prob->param->mod);
  }

  tmpsol = create_solution(prob);
  tmpsol2 = create_solution(prob);

  type = 0;
  mod->an = 0;
  mod->n = 0;

  max_nnodes = prob->graph->n_nodes;
  max_nedges = prob->graph->n_edges;

  lag2_reverse(prob);

  lb2 = - LARGE_REAL;
  ret = lag2_solve_LR2m(prob, u, ctod(*ub) - lboff, tmpsol, &lb, o);
  lb += lboff;

  if(ret == SIPS_OPTIMAL) {
    if(prob->param->verbose >= 2) {
      fprintf(stdout, "[%3u] Feasible.\n", mod->n);
    }
    if(obj_lesser(tmpsol->f, prob->sol->f)) {
      PRINT_UBUPDATED(stdout, 0, tmpsol);
      copy_solution(prob, prob->sol, tmpsol);
      if(obj_lesser(tmpsol->f, *ub)) {
	*ub = tmpsol->f;
      }
    }
    ret = SIPS_SOLVED;
  } else if(ret == SIPS_INFEASIBLE) {
#ifdef LB_DEBUG
    if(ret == SIPS_INFEASIBLE) {
      if(obj_equal(prob->sol->f, *ub)) {
	fprintf(stderr, "Feasibility error.\n");
	exit(1);
      }
    }
#endif /* LB_DEBUG */
    ret = SIPS_SOLVED;
  }

  _get_time(prob);
  if(prob->param->verbose >= 2) {
    PRINT_LBUB(stdout, mod->n);
    PRINT_NETWORKSIZE(stdout, mod->n);
  }

  mb = lag2_get_memory_in_MB(prob) - prob->graph->copy->mem;
  if(prob->param->modsw >= 0 && mb >= (_real) prob->param->modsw) {
    type = 1;
  } else {
    type = 0;
  }

  lbprev = lb;
  nmod = prob->param->mod;
  heavy = SIPS_FALSE;
  while(ret == SIPS_NORMAL) {
    lbprev = max(lbprev, lb);
    mb = ((_real) prob->param->mem - prob->graph->copy->mem
	  - prob->graph->ptable->mem)
      /(lag2_get_memory_in_MB(prob) - prob->graph->copy->mem
	- prob->graph->ptable->mem);
    for(nmod = 0; mb >= 1.0 && nmod <= prob->param->mod; mb /= 2.0, nmod++);
    nmod--;
    if(nmod == 0) {
      nmod = 1;
    }
#ifdef SIPSI
    if(mod->n + nmod > prob->n) {
      nmod = prob->n - mod->n;
    }
#else /* SIPSI */
    if(mod->n + nmod >= prob->n) {
      nmod = prob->n - mod->n - 1;
    }
#endif /* SIPSI */

    ret = lag2_assign_modifiers(prob, type, nmod, mod);

    if(ret == SIPS_INFEASIBLE) {
      if(prob->param->verbose >= 2) {
	fprintf(stdout,
		"[%3u] Time window constraints made the problem infeasible.\n",
		mod->n);
      }
#ifdef LB_DEBUG
      if(obj_equal(prob->sol->f, *ub)) {
	fprintf(stderr, "Feasibility error.\n");
	exit(1);
      }
#endif /* LB_DEBUG */
      goto _loop_end;
    }

    if(prob->param->verbose >= 2) {
      if(mod->an > 0) {
	if(mod->an == 1) {
	  fprintf(stdout, "[%3u] Recovering the constraint for", mod->n);
	} else {
	  fprintf(stdout, "[%3u] Recovering the constraints for", mod->n);
	}
	for(i = 0; i < mod->an; i++) {
	  if(i > 0) {
	    fprintf(stdout, ",");
	  }
	  if(prob->sjob[mod->jobs[i]]->name == NULL) {
	    fprintf(stdout, " %d", prob->sjob[mod->jobs[i]]->rno);
	  } else {
	    fprintf(stdout, " %s", prob->sjob[mod->jobs[i]]->name);
	  }
	}
	fprintf(stdout, "\n");
	fflush(stdout);
      }
    }

    if(is_true(prob->param->zmplier)) {
      if(mod->an == 0) {
	ret = lag2_solve_LR2m_without_elimination(prob, u, ctod(*ub) - lboff,
						  tmpsol, &lb, o);
      } else {
	ret = lag2_add_modifiers_LR2m_without_elimination(prob, u,
							  ctod(*ub) - lboff,
							  tmpsol, &lb, o, mod);
      }
    } else {
      lag2_reverse(prob);
      if(mod->an == 0) {
	ret = lag2_solve_LR2m(prob, u, ctod(*ub) - lboff, tmpsol, &lb, o);
      } else {
	ret = lag2_add_modifiers_LR2m(prob, u, ctod(*ub) - lboff,
				      tmpsol, &lb, o, mod);
      }
    }
    lb += lboff;

    max_nnodes = max(max_nnodes, prob->graph->n_nodes);
    max_nedges = max(max_nedges, prob->graph->n_edges);

    if(ret == SIPS_MEMLIMIT) {
      if(prob->param->verbose >= 2) {
	PRINT_MAXMEMORY(stdout, mod->n);
      } else if(prob->param->verbose >= 1) {
	PRINT_MAXMEMORY2(stdout);
      }
      break;
    }

    mb = (lag2_get_memory_in_MB(prob) - prob->graph->copy->mem
	  - prob->graph->ptable->mem)
      /((_real) prob->param->mem - prob->graph->copy->mem
	- prob->graph->ptable->mem);
    if(nmod == 1 && mb >= 0.75) {
      heavy = SIPS_TRUE;
    } else if(nmod > 1 || mb < 0.5) {
      heavy = SIPS_FALSE;
    }

    if((is_true(heavy) || is_true(prob->param->zmplier))
       && ret == SIPS_NORMAL) {
      lag2_reverse(prob);
      ret = lag2_solve_LR2m(prob, u, ctod(*ub) - lboff, tmpsol, &lb2, o);
      lb2 += lboff;
      lb = max(lb, lb2);
      max_nnodes = max(max_nnodes, prob->graph->n_nodes);
      max_nedges = max(max_nedges, prob->graph->n_edges);
    }

    _get_time(prob);
    if(prob->param->verbose >= 2) {
      PRINT_LBUB(stdout, mod->n);
      PRINT_NETWORKSIZE(stdout, mod->n);
    }

#ifdef LB_DEBUG
    if(ret != SIPS_NORMAL || (ctod(*ub) - lb < prob->param->lbeps
			      && !obj_equal(prob->sol->f, *ub))) {
      goto _loop_end;
    }
#else /* LB_DEBUG */
    if(ret != SIPS_NORMAL || ctod(*ub) - lb < prob->param->lbeps) {
      goto _loop_end;
    }
#endif /* LB_DEBUG */

    if(is_true(prob->param->zmplier)) {
      ret = lag2_solve_LR2m_without_elimination(prob, ud, ctod(*ub),
						tmpsol2, &lb2, o);
      max_nnodes = max(max_nnodes, prob->graph->n_nodes);
      max_nedges = max(max_nedges, prob->graph->n_edges);

#ifdef LB_DEBUG
      if(ret != SIPS_NORMAL || (ctod(*ub) - lb2 < prob->param->lbeps
				&& !obj_equal(prob->sol->f, *ub))) {
	copy_solution(prob, tmpsol, tmpsol2);
	lb = lb2;

	_get_time(prob);
	if(prob->param->verbose >= 2) {
	  PRINT_LBUB2(stdout, mod->n);
	  PRINT_NETWORKSIZE(stdout, mod->n);
	}

	goto _loop_end;
      }
#else /* LB_DEBUG */
      if(ret != SIPS_NORMAL || ctod(*ub) - lb2 < prob->param->lbeps) {
	copy_solution(prob, tmpsol, tmpsol2);
	lb = lb2;

	_get_time(prob);
	if(prob->param->verbose >= 2) {
	  PRINT_LBUB2(stdout, mod->n);
	  PRINT_NETWORKSIZE(stdout, mod->n);
	}

	goto _loop_end;
      }
#endif /* LB_DEBUG */

      lag2_reverse(prob);
      ret = lag2_solve_LR2m(prob, ud, ctod(*ub), tmpsol2, &lb2, o);
      max_nnodes = max(max_nnodes, prob->graph->n_nodes);
      max_nedges = max(max_nedges, prob->graph->n_edges);

      _get_time(prob);
      if(prob->param->verbose >= 2) {
	PRINT_LBUB2(stdout, mod->n);
	PRINT_NETWORKSIZE(stdout, mod->n);
      }

#ifdef LB_DEBUG
      if(ret != SIPS_NORMAL || (ctod(*ub) - lb2 < prob->param->lbeps
				&& !obj_equal(prob->sol->f, *ub))) {
	copy_solution(prob, tmpsol, tmpsol2);
	lb = lb2;
	goto _loop_end;
      }
#else /* LB_DEBUG */
      if(ret != SIPS_NORMAL || ctod(*ub) - lb2 < prob->param->lbeps) {
	copy_solution(prob, tmpsol, tmpsol2);
	lb = lb2;
	goto _loop_end;
      }
#endif /* LB_DEBUG */
    }

    if(lb2 > lb) {
      partialdp(prob, u, tmpsol2);
      if(prob->param->ls == SIPS_LS_EDYNA
	 || prob->param->ls == SIPS_LS_COMBINED_A
	 || prob->param->ls == SIPS_LS_COMBINED_B) {
	edynasearch(prob, tmpsol2);
      } else if(prob->param->ls == SIPS_LS_DYNA) {
	dynasearch(prob, tmpsol2);
      }

      if(obj_lesser(tmpsol2->f, *ub)) {
	PRINT_UBUPDATED(stdout, mod->n, tmpsol2);
	copy_solution(prob, prob->sol, tmpsol2);
	*ub = tmpsol2->f;
	if(!prob->param->bisec) {
	  lag2_free_copy(prob);
	}
      } else if(obj_lesser(tmpsol2->f, prob->sol->f)) {
	PRINT_TUBUPDATED(stdout, mod->n, tmpsol2);
	copy_solution(prob, prob->sol, tmpsol2);
      }
    } else if(lb - lbprev > prob->param->eps
	      || is_true(prob->param->ubupd3)) {
      partialdp(prob, u, tmpsol);
      if(prob->param->ls == SIPS_LS_EDYNA
	 || prob->param->ls == SIPS_LS_COMBINED_B) {
	edynasearch(prob, tmpsol);
      } else if(prob->param->ls == SIPS_LS_DYNA
		|| prob->param->ls == SIPS_LS_COMBINED_A) {
	dynasearch(prob, tmpsol);
      }

      if(obj_lesser(tmpsol->f, *ub)) {
	PRINT_UBUPDATED(stdout, mod->n, tmpsol);
	copy_solution(prob, prob->sol, tmpsol);
	*ub = tmpsol->f;
	if(!prob->param->bisec) {
	  lag2_free_copy(prob);
	}
      } else if(obj_lesser(tmpsol->f, prob->sol->f)) {
	PRINT_TUBUPDATED(stdout, mod->n, tmpsol);
	copy_solution(prob, prob->sol, tmpsol);
      }
    }
#ifdef LB_DEBUG
    if(ctod(*ub) - lb2 < prob->param->lbeps
       && !obj_equal(prob->sol->f, *ub)) {
      lb = lb2;
    }
#else /* LB_DEBUG */
    if(ctod(*ub) - lb2 < prob->param->lbeps) {
      lb = lb2;
    }
#endif /* LB_DEBUG */

_loop_end:
    if(ret == SIPS_OPTIMAL) {
#ifdef LB_DEBUG
      fprintf(stdout, "[%3u] Feasible.\n", mod->n);
#endif /* LB_DEBUG */
      if(obj_lesser(tmpsol->f, *ub)) {
	PRINT_UBUPDATED(stdout, mod->n, tmpsol);
	copy_solution(prob, prob->sol, tmpsol);
	*ub = tmpsol->f;
      } else if(obj_lesser(tmpsol->f, prob->sol->f)) {
	PRINT_TUBUPDATED(stdout, mod->n, tmpsol);
	copy_solution(prob, prob->sol, tmpsol);
      }
      ret = SIPS_SOLVED;
      break;
    } else if(ret == SIPS_INFEASIBLE
	      || ctod(*ub) - lb < prob->param->lbeps) {
      lb = ctod(*ub);
#ifdef LB_DEBUG
      if(!obj_equal(prob->sol->f, *ub)) {
	ret = SIPS_SOLVED;
	break;
      } else {
	fprintf(stderr, "Feasibility error.\n");
	exit(1);
      }
#else /* LB_DEBUG */
      ret = SIPS_SOLVED;
      break;
#endif /* LB_DEBUG */
    }

    if(prob->param->tlimit > 0 && prob->time > prob->param->tlimit) {
      if(prob->param->verbose >= 2) {
	PRINT_TIMELIMIT(stdout, mod->n);
      } else if(prob->param->verbose >= 1) {
	PRINT_TIMELIMIT_SHORT(stdout);
      }

      ret = SIPS_TIMELIMIT;
      break;
    }
  }

  if(ret == SIPS_SOLVED) {
    if(prob->param->verbose >= 2) {
      if(ctod(prob->sol->f) - lb < prob->param->lbeps) {
	fprintf(stdout, "[%3u] Solved.\n", mod->n);
      } else {
	fprintf(stdout, "[%3u] End of iteration.\n", mod->n);
      }
    }
  }

  if(prob->param->verbose >= 2) {
    fprintf(stdout, "      Constraints: %u\n", mod->n);
    fprintf(stdout, "            Nodes: %u\n", max_nnodes);
    fprintf(stdout, "             Arcs: %u\n", max_nedges);
  }

  lag2_free(prob);

  free_solution(tmpsol2);
  free_solution(tmpsol);

  xfree(ud);
  xfree(mod->v[0]);
  xfree(mod->v);
  xfree(mod->jobs);
  xfree(mod->fl);
  xfree(mod->m);
  xfree(mod);
  xfree(o);

  if(prob->param->verbose >= 1) {
    fflush(stdout);
  }

  return(ret);
}
