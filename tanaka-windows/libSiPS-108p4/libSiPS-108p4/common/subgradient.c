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
 *  $Id: subgradient.c,v 1.39 2015/10/27 22:23:20 tanaka Rel $
 *  $Revision: 1.39 $
 *  $Date: 2015/10/27 22:23:20 $
 *  $Author: tanaka $
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "define.h"
#include "sips_common.h"
#include "dynasearch.h"
#include "heuristics.h"
#include "lag.h"
#include "lag2.h"
#include "memory.h"
#include "print.h"
#include "ptable.h"
#include "sol.h"
#include "subgradient.h"
#include "timer.h"

#define MAXNSITER (N_JOBS)

#define PRINT_ITER_INFO(fp) {						\
    if(prob->param->verbose >= 2) {					\
      fprintf(fp, "[%4u] LB=", iter);					\
      print_real(fp, 11.4, lmax + ctod(prob->off));			\
      fprintf(fp, " UB=");						\
      print_current_objective(fp);					\
      fprintf(fp, " lk=");						\
      print_real(fp, .4, lk);						\
      fprintf(fp, "\n");						\
    }									\
  }
#define PRINT_ITER_INFO2(fp) {						\
    if(prob->param->verbose >= 2) {					\
      fprintf(fp, "[%4u] LB=", iter);					\
      print_real(fp, 11.4, lmax + ctod(prob->off));			\
      fprintf(fp, " UB=");						\
      print_current_objective(fp);					\
      fprintf(fp, " lk=");						\
      print_real(fp, .4, lk);						\
      fprintf(fp, " nodes=%u\n", prob->graph->n_nodes);			\
    }									\
  }
#define PRINT_UBINFO(fp) {						\
    fprintf(fp, "      Lower bound: ");					\
    print_real(fp, .4, lmax + ctod(prob->off));				\
    fprintf(fp, "\n      Upper bound: ");				\
    print_current_objective(fp);					\
    fprintf(fp, "\n");							\
  }
#define PRINT_UBINFO_SHORT(fp)	{					\
    fprintf(fp, "UB=");							\
    print_current_objective(fp);					\
    fprintf(fp, " LB=");						\
    print_real(fp, .4, lmax + ctod(prob->off));				\
    fprintf(fp, "\n");							\
  }
#define UPDATE_UB(fp, tsol) {						\
    if(obj_greater(prob->sol->f, tsol->f)) {				\
      if(prob->param->verbose == 1) {					\
	fprintf(fp, "UB=");						\
	print_cost(fp, tsol->f + prob->off);				\
	fprintf(fp, "\n");						\
      }									\
      copy_solution(prob, prob->sol, tsol);				\
    }									\
  }
#define PRINT_TIMELIMIT_SHORT(fp) {					\
    fprintf(fp, "Reached time limit (");				\
    print_real2(fp, prob->param->tlimit);				\
    fprintf(fp, "s).\n");						\
  }
#define PRINT_TIMELIMIT(fp, i)	{					\
    fprintf(fp, "[%4u] ", iter);					\
    PRINT_TIMELIMIT_SHORT(fp);						\
  }

/*
 * subgradient_LR1(prob, lb)
 *   optimizes the Lagrangian dual corresponding to (LR1)
 *   by subgradient optimization.
 *       lb: best lower bound
 *
 */
int subgradient_LR1(sips *prob, _real *lb)
{
  int i;
  int ititer, titer, iter;
  int updated, nupdated, nsiter, pos;
  int ret, ret2;
#if defined(COST_LONGLONG) || defined(COST_REAL) || defined(REAL_LONG)
  unsigned long long int osq;
#else /* COST_LONGLONG || COST_REAL || REAL_LONG */
  unsigned int osq;
#endif /* COST_LONGLONG || COST_REAL || REAL_LONG */
  char pflag;
  _real g;
  _real ksi, dnorm;
  _real tk, lk, cl, off, lmax, lmax2;
  unsigned int *o;
  _real *u, *ubest, *d;
  _real *gap_table;
  _solution_t *tmpsol, *csol;

  if(prob->param->titer1 > prob->param->eps) {
    if((_real) prob->n < prob->param->titer1) {
      titer = 1;
    } else {
      g = ((_real) prob->n)/prob->param->titer1;
      titer = (int) g;
      if(((_real) (titer + 1)) - g < prob->param->eps) {
	titer++;
      }
    }
  } else if(prob->param->titer1 < - 1.0 + prob->param->eps) {
    titer = (int) (- prob->param->titer1);
    if(prob->param->titer1 + (_real) (titer + 1) < prob->param->eps) {
      titer++;
    }
  } else {
    titer = 0;
  }

  if(prob->param->ititer1 > prob->param->eps) {
    if((_real) prob->n < prob->param->ititer1) {
      ititer = 1;
    } else {
      g = ((_real) prob->n)/prob->param->ititer1;
      ititer = (int) g;
      if(((_real) (ititer + 1)) - g < prob->param->eps) {
	ititer++;
      }
    }
  } else if(prob->param->ititer1 < - 1.0 + prob->param->eps) {
    ititer = (int) (- prob->param->titer1);
    if(prob->param->ititer1 + (_real) (ititer + 1) < prob->param->eps) {
      ititer++;
    }
  } else {
    ititer = 0;
  }

  if(ititer < titer) {
    ititer = titer;
  }

  o = (unsigned int *) xmalloc(prob->n*sizeof(unsigned int));
  u = (_real *) xcalloc(2*(N_JOBS + 1), sizeof(_real));
  ubest = u + (N_JOBS + 1);
  d = (_real *) xcalloc(prob->n, sizeof(_real));

  if(titer > 0) {
    gap_table = (_real *) xmalloc(titer*sizeof(_real));
  } else {
    gap_table = NULL;
  }

  csol = create_solution(prob);
  tmpsol = create_solution(prob);

  if(is_true(prob->param->warm)) {
    for(i = 0; i < prob->n; i++) {
      u[i] = ubest[i] = prob->sjob[i]->u;
    }
  } else {
    for(i = 0; i < prob->n; i++) {
      u[i] = ubest[i] = prob->param->u0;
    }
  }

  for(i = 0; i < titer; i++) {
    gap_table[i] = - LARGE_REAL;
  }

  iter = updated = nupdated = nsiter = 0;
  pflag = 0;
  lmax = *lb;
  lmax2 = - LARGE_REAL;
  lk = prob->param->initstep1;
  pos = 0;
  ret = SIPS_UNSOLVED;
  dnorm = 1.0;

  while(++iter) {
    pflag = 0;
    off = 0.0;
    for(i = 0; i < prob->n; i++) {
      off += u[i];
    }

    ret2 = lag_solve_LR1(prob, u, ctod(prob->sol->f) - off, tmpsol, &cl, o);
    if(ret2 == SIPS_FAIL || ret2 == SIPS_MEMLIMIT) {
      ret = ret2;
      break;
    }

    cl += off;
    if(cl - lmax > prob->param->eps) {
      updated = nsiter = 0;
      nupdated++;
      pflag = 1;
      lmax = cl;
      memcpy((void *) ubest, (void *) u, prob->n*sizeof(_real));

      if(iter > 1) {
	lk *= prob->param->expand1;
	if(lk > prob->param->maxstep1 && lk > prob->param->initstep1) {
	  lk = max(prob->param->maxstep1, prob->param->initstep1);
	}
      }

      _get_time(prob);
      if(prob->param->tlimit > 0 && prob->time > prob->param->tlimit) {
	if(prob->param->verbose >= 2) {
	  PRINT_TIMELIMIT(stdout, n);
	} else if(prob->param->verbose >= 1) {
	  PRINT_TIMELIMIT_SHORT(stdout);
	}

	ret = SIPS_TIMELIMIT;
	break;
      }
    } else if(lmax2 >= cl - prob->param->lbeps) {
      updated++;
    }

    if(iter > 1) {
      lmax2 = max(lmax2, cl);
    }

#ifndef LB_DEBUG
    if(ctod(prob->sol->f) - lmax < prob->param->lbeps) {
      PRINT_ITER_INFO(stdout);
      ret = SIPS_SOLVED;
      break;
    }
#endif /* LB_DEBUG */

    if(ret2 != SIPS_NORMAL) {
      if(ret2 == SIPS_OPTIMAL) {
#ifdef LB_DEBUG
	fprintf(stdout, "Feasible.\n");
#endif /* LB_DEBUG */
	UPDATE_UB(stdout, tmpsol);
#ifdef LB_DEBUG
      } else {
	fprintf(stderr, "Feasibility error.\n");
	exit(1);
#endif /* LB_DEBUG */
      }
      PRINT_ITER_INFO(stdout);
      ret = SIPS_SOLVED;
      break;
    }

    if(pflag) {
      PRINT_ITER_INFO(stdout);
    }

    if(!titer || is_true(prob->param->skip1)) {
      break;
    }

    if(nupdated <= 1) {
      if(iter > ititer) {
	break;
      }
    } else if(prob->param->tratio1*(ctod(prob->sol->f) - gap_table[pos])
	      > lmax - gap_table[pos]
	      && prob->param->tratio1*lmax + prob->param->eps
	      > lmax - gap_table[pos]
	      && (nupdated*prob->param->minupdate >= prob->n
		  || nsiter >= MAXNSITER)) {
      break;
    }
#ifdef LB_DEBUG
    if(iter >= 5000) {
      break;
    }
#endif /* LB_DEBUG */

    if(iter == 1) {
      lag_solve_LR1_backward(prob, u, ctod(prob->sol->f) - off);
    }

    if(updated >= prob->param->siter1) {
      nsiter += updated;
      updated = 0;
      lk *= prob->param->shrink1;
      if(lk < 1.0e-4) {
	break;
      }
    }

    gap_table[pos++] = lmax;
    pos %= titer;

    osq = 0;
    for(i = 0; i < prob->n; i++) {
#if defined(COST_LONGLONG) || defined(COST_REAL) || defined(REAL_LONG)
      osq += (1LL - (long long int) o[i])*(1LL - (long long int) o[i]);
#else /* COST_LONGLONG || COST_REAL || REAL_LONG */
      osq += (1 - (int) o[i])*(1 - (int) o[i]);
#endif /* COST_LONGLONG || COST_REAL || REAL_LONG */
    }

#ifdef REAL_LONG
    ksi = sqrtl((_real) osq/dnorm);
#else /* REAL_LONG */
    ksi = sqrt((_real) osq/dnorm);
#endif /* REAL_LONG */
    for(i = 0; i < prob->n; i++) {
#if defined(COST_LONGLONG) || defined(COST_REAL) || defined(REAL_LONG)
      d[i] = (_real) (1LL - (long long int) o[i]) + ksi*d[i];
#else /* COST_LONGLONG || COST_REAL || REAL_LONG */
      d[i] = (_real) (1 - (int) o[i]) + ksi*d[i];
#endif /* COST_LONGLONG || COST_REAL || REAL_LONG */
    }

    dnorm = 0.0;
    for(i = 0; i < prob->n; i++) {
      dnorm += d[i]*d[i];
    }

    if(dnorm < prob->param->eps) {
      dnorm = (_real) osq;
      for(i = 0; i < prob->n; i++) {
#if defined(COST_LONGLONG) || defined(COST_REAL) || defined(REAL_LONG)
	d[i] = (_real) (1LL - (long long int) o[i]);
#else /* COST_LONGLONG || COST_REAL || REAL_LONG */
	d[i] = (_real) (1 - (int) o[i]);
#endif /* COST_LONGLONG || COST_REAL || REAL_LONG */
      }
    }

    tk = lk*(ctod(prob->sol->f) - cl)/dnorm;
    for(i = 0; i < prob->n; i++) {
      u[i] += tk*d[i];
    }
  }

  if(ret == SIPS_UNSOLVED) {
    if(is_false(prob->param->skip1)) {
      off = 0.0;
      for(i = 0; i < prob->n; i++) {
	off += ubest[i];
      }

      lag_solve_LR1(prob, ubest, ctod(prob->sol->f) - off, tmpsol, &cl, o);
      cl += off;
    }

#ifdef LB_DEBUG
    if(is_true(prob->param->ubupd1)) {
      copy_solution(prob, csol, tmpsol);
      partialdp(prob, u, csol);

      if(prob->param->ls == SIPS_LS_EDYNA) {
	edynasearch(prob, csol);
      } else if(prob->param->ls == SIPS_LS_DYNA
		|| prob->param->ls == SIPS_LS_COMBINED_A
		|| prob->param->ls == SIPS_LS_COMBINED_B) {
	dynasearch(prob, csol);
      }

      UPDATE_UB(stdout, csol);
    }

    lag_solve_LR1_backward(prob, ubest, ctod(prob->sol->f) - off);
#else /* LB_DEBUG */
    if(ctod(prob->sol->f) - cl < prob->param->lbeps) {
      ret = SIPS_SOLVED;
    } else if(is_true(prob->param->ubupd1)) {
      copy_solution(prob, csol, tmpsol);
      partialdp(prob, u, csol);

      if(prob->param->ls == SIPS_LS_EDYNA) {
	edynasearch(prob, csol);
      } else if(prob->param->ls == SIPS_LS_DYNA
		|| prob->param->ls == SIPS_LS_COMBINED_A
		|| prob->param->ls == SIPS_LS_COMBINED_B) {
	dynasearch(prob, csol);
      }

      UPDATE_UB(stdout, csol);

      if(ctod(prob->sol->f) - cl < prob->param->lbeps) {
	ret = SIPS_SOLVED;
      } else {
	lag_solve_LR1_backward(prob, ubest, ctod(prob->sol->f) - off);
      }
    } else {
      lag_solve_LR1_backward(prob, ubest, ctod(prob->sol->f) - off);
    }
#endif /* LB_DEBUG */

    lmax = cl;
  }

  for(i = 0; i < prob->n; i++) {
    prob->sjob[i]->u = ubest[i];
  }

  xfree(d);
  xfree(u);
  xfree(o);
  xfree(gap_table);

  free_solution(tmpsol);
  free_solution(csol);

  if(prob->param->verbose == 2) {
    fprintf(stdout, "       Iterations: %u\n", iter);
    PRINT_UBINFO(stdout);
    fprintf(stdout, "             Time: ");
    _print_time(prob, stdout);
    fflush(stdout);
  } else if(prob->param->verbose >= 1) {
    PRINT_UBINFO_SHORT(stdout);
    fflush(stdout);
  }

  *lb = lmax;

  return(ret);
}

/*
 * subgradient_LR2adj(prob, lb)
 *   optimizes the Lagrangian dual corresponding to (LR2adj)
 *   by subgradient optimization.
 *       lb: best lower bound
 *
 */
int subgradient_LR2adj(sips *prob, _real *lb)
{
  int i;
  int ititer, titer, iter;
  int updated, nupdated, nsiter, pos;
  int ret, ret2;
#if defined(COST_LONGLONG) || defined(COST_REAL) || defined(REAL_LONG)
  unsigned long long int osq, bosq;
#else /* COST_LONGLONG || COST_REAL || REAL_LONG */
  unsigned int osq, bosq;
#endif /* COST_LONGLONG || COST_REAL || REAL_LONG */
  char pflag;
  _real g;
  _real ksi, dnorm;
  _real tk, lk, cl, off, lmax, lmax2;
  unsigned int *o;
  _real *u, *ubest, *d;
  _real *gap_table;
  _solution_t *tmpsol, *csol;

  if(prob->param->titer2 > prob->param->eps) {
    if((_real) prob->n < prob->param->titer2) {
      titer = 1;
    } else {
      g = ((_real) prob->n)/prob->param->titer2;
      titer = (int) g;
      if(((_real) (titer + 1)) - g < prob->param->eps) {
	titer++;
      }
    }
  } else if(prob->param->titer2 < - 1.0 + prob->param->eps) {
    titer = (int) (- prob->param->titer2);
    if(prob->param->titer2 + (_real) (titer + 1) < prob->param->eps) {
      titer++;
    }
  } else {
    titer = 0;
  }

  if(prob->param->ititer2 > prob->param->eps) {
    if((_real) prob->n < prob->param->ititer2) {
      ititer = 1;
    } else {
      g = ((_real) prob->n)/prob->param->ititer2;
      ititer = (int) g;
      if(((_real) (ititer + 1)) - g < prob->param->eps) {
	ititer++;
      }
    }
  } else if(prob->param->ititer2 < - 1.0 + prob->param->eps) {
    ititer = (int) (- prob->param->titer2);
    if(prob->param->ititer2 + (_real) (ititer + 1) < prob->param->eps) {
      ititer++;
    }
  } else {
    ititer = 0;
  }

  if(ititer < titer) {
    ititer = titer;
  }

  o = (unsigned int *) xmalloc(prob->n*sizeof(unsigned int));
  u = (_real *) xcalloc(2*(N_JOBS + 1), sizeof(_real));
  ubest = u + (N_JOBS + 1);
  d = (_real *) xcalloc(prob->n, sizeof(_real));

  if(titer > 0) {
    gap_table = (_real *) xmalloc(titer*sizeof(_real));
  } else {
    gap_table = NULL;
  }

  csol = create_solution(prob);
  tmpsol = create_solution(prob);

  for(i = 0; i < prob->n; i++) {
    u[i] = ubest[i] = prob->sjob[i]->u;
  }

  for(i = 0; i < titer; i++) {
    gap_table[i] = - LARGE_REAL;
  }

  iter = updated = nupdated = nsiter = 0;
  pflag = 0;
  lmax = *lb;
  lmax2 = - LARGE_REAL;
  lk = prob->param->initstep2;
  pos = 0;
  ret = SIPS_UNSOLVED;
#if defined(COST_LONGLONG) || defined(COST_REAL) || defined(REAL_LONG)
  bosq = 1152921504606846975LL;
#else /* COST_LONGLONG || COST_REAL || REAL_LONG */
  bosq = LARGE_INTEGER;
#endif /* COST_LONGLONG || COST_REAL || REAL_LONG */
  dnorm = 1.0;

  while(++iter) {
    pflag = 0;
    off = 0.0;
    for(i = 0; i < prob->n; i++) {
      off += u[i];
    }

    ret2 = lag_solve_LR2adj(prob, u, ctod(prob->sol->f) - off, tmpsol, &cl, o);
    cl += off;

    if(iter == 1) {
      pflag = 1;
      if(ret2 == SIPS_MEMLIMIT) {
	if(prob->param->verbose >= 2) {
	  fprintf(stdout, "[%4u] Reached maximum memory (", 0);
	  print_real(stdout, .2, lag_get_memory_in_MB(prob)
		     + prec_get_memory_in_MB(prob));
	  fprintf(stdout, "MB > %dMB).\n", prob->param->mem);
	} else {
	  fprintf(stdout, "Reached maximum memory (");
	  print_real(stdout, .2, lag_get_memory_in_MB(prob)
		     + prec_get_memory_in_MB(prob));
	  fprintf(stdout, "MB > %dMB).\n", prob->param->mem);
	}

	ret = SIPS_MEMLIMIT;
	lmax = *lb;
	break;
      } else if(prob->param->verbose >= 2) {
	fprintf(stdout, "[%4u] Memory ", 0);
	print_real(stdout, .2, lag_get_memory_in_MB(prob)
		   + prec_get_memory_in_MB(prob));
	fprintf(stdout, "MB\n");
      }
    }

    if(cl - lmax > prob->param->eps) {
      updated = nsiter = 0;
      nupdated++;
      pflag = 1;
      lmax = cl;
      memcpy((void *) ubest, (void *) u, prob->n*sizeof(_real));

      if(iter > 1) {
	lk *= prob->param->expand2;
	if(lk > prob->param->maxstep2 && lk > prob->param->initstep2) {
	  lk = max(prob->param->maxstep2, prob->param->initstep2);
	}
      }

      _get_time(prob);
      if(prob->param->tlimit > 0 && prob->time > prob->param->tlimit) {
	if(prob->param->verbose >= 2) {
	  PRINT_TIMELIMIT(stdout, n);
	} else if(prob->param->verbose >= 1) {
	  PRINT_TIMELIMIT_SHORT(stdout);
	}

	ret = SIPS_TIMELIMIT;
	break;
      }
    } else if(lmax2 >= cl - prob->param->lbeps) {
      updated++;
    }

    if(iter > 1) {
      lmax2 = max(lmax2, cl);
    }

    if(ret2 != SIPS_NORMAL) {
      if(ret2 == SIPS_OPTIMAL) {
#ifdef LB_DEBUG
	fprintf(stdout, "Feasible.\n");
#endif /* LB_DEBUG */
	UPDATE_UB(stdout, tmpsol);
#ifdef LB_DEBUG
      } else {
	fprintf(stderr, "Feasibility error.\n");
	exit(1);
#endif /* LB_DEBUG */
      }

      PRINT_ITER_INFO2(stdout);
      ret = SIPS_SOLVED;
      break;
    }

#ifndef LB_DEBUG
    if(ctod(prob->sol->f) - lmax < prob->param->lbeps) {
      PRINT_ITER_INFO2(stdout);
      ret = SIPS_SOLVED;
      break;
    }
#endif /* LB_DEBUG */

    osq = 0;
    for(i = 0; i < prob->n; i++) {
#if defined(COST_LONGLONG) || defined(COST_REAL) || defined(REAL_LONG)
      osq += (1LL - (long long int) o[i])*(1LL - (long long int) o[i]);
#else /* COST_LONGLONG || COST_REAL || REAL_LONG */
      osq += (1 - (int) o[i])*(1 - (int) o[i]);
#endif /* COST_LONGLONG || COST_REAL || REAL_LONG */
    }

    if(bosq > osq) {
      bosq = osq;
      copy_solution(prob, csol, tmpsol);
    }

    if(prob->param->ubiter != 0 && iter%prob->param->ubiter == 1) {
      partialdp(prob, u, csol);
      if(prob->param->ls == SIPS_LS_EDYNA) {
	edynasearch(prob, csol);
      } else if(prob->param->ls == SIPS_LS_DYNA
		|| prob->param->ls == SIPS_LS_COMBINED_A
		|| prob->param->ls == SIPS_LS_COMBINED_B) {
	dynasearch(prob, csol);
      }

#if defined(COST_LONGLONG) || defined(COST_REAL) || defined(REAL_LONG)
      bosq = 1152921504606846975LL;
#else /* COST_LONGLONG || COST_REAL || REAL_LONG */
      bosq = LARGE_INTEGER;
#endif /* COST_LONGLONG || COST_REAL || REAL_LONG */
    } else {
      csol->f = LARGE_COST;
    }

    if(obj_greater(prob->sol->f, csol->f) || pflag) {
      if(iter > 1 && obj_greater(prob->sol->f, csol->f)
	 && ctod(csol->f) > lmax) {
	lk *= (prob->sol->f - lmax)/(csol->f - lmax);
	if(lk > prob->param->maxstep2) {
	  lk = prob->param->maxstep2;
	}
      }

      UPDATE_UB(stdout, csol);
#ifndef LB_DEBUG
      if(ctod(prob->sol->f) - lmax < prob->param->lbeps) {
	PRINT_ITER_INFO2(stdout);
	ret = SIPS_SOLVED;
	break;
      }
#endif /* LB_DEBUG */

      ret = lag_solve_LR2adj_backward(prob, u, ctod(prob->sol->f) - off);
#ifdef LB_DEBUG
      PRINT_ITER_INFO2(stdout);
      if(ret == SIPS_INFEASIBLE) {
	fprintf(stderr, "Feasibility error.\n");
	exit(1);
      } else {
	ret = SIPS_UNSOLVED;
      }
#else /* LB_DEBUG */
      if(ret == SIPS_INFEASIBLE) {
	lmax = *lb = ctod(prob->sol->f);
	PRINT_ITER_INFO2(stdout);
	ret = SIPS_SOLVED;
	break;
      } else {
	ret = SIPS_UNSOLVED;
      }
      PRINT_ITER_INFO2(stdout);
#endif /* LB_DEBUG */
    }

    if(!titer) {
      break;
    }

    if(nupdated <= 1) {
      if(iter > ititer) {
	break;
      }
    } else if(prob->param->tratio2*(ctod(prob->sol->f) - gap_table[pos])
	      > lmax - gap_table[pos]
	      && prob->param->tratio2*lmax + prob->param->eps
	      > lmax - gap_table[pos]
	      && (nupdated*prob->param->minupdate >= prob->n
		  || nsiter >= MAXNSITER)) {
      break;
    }
#ifdef LB_DEBUG
    if(iter >= 5000) {
      break;
    }
#endif /* LB_DEBUG */

    if(updated >= prob->param->siter2) {
      nsiter += updated;
      updated = 0;
      lk *= prob->param->shrink2;
      if(lk < 1.0e-4) {
	break;
      }
    }

    gap_table[pos++] = lmax;
    pos %= titer;

#ifdef REAL_LONG
    ksi = sqrtl((_real) osq/dnorm);
#else /* REAL_LONG */
    ksi = sqrt((_real) osq/dnorm);
#endif /* REAL_LONG */
    for(i = 0; i < prob->n; i++) {
#if defined(COST_LONGLONG) || defined(COST_REAL) || defined(REAL_LONG)
      d[i] = (_real) (1LL - (long long int) o[i]) + ksi*d[i];
#else /* COST_LONGLONG || COST_REAL || REAL_LONG */
      d[i] = (_real) (1 - (int) o[i]) + ksi*d[i];
#endif /* COST_LONGLONG || COST_REAL || REAL_LONG */
    }

    dnorm = 0.0;
    for(i = 0; i < prob->n; i++) {
      dnorm += d[i]*d[i];
    }
    if(dnorm < prob->param->eps) {
      dnorm = (_real) osq;
      for(i = 0; i < prob->n; i++) {
#if defined(COST_LONGLONG) || defined(COST_REAL) || defined(REAL_LONG)
	d[i] = (_real) (1LL - (long long int) o[i]);
#else /* COST_LONGLONG || COST_REAL || REAL_LONG */
	d[i] = (_real) (1 - (int) o[i]);
#endif /* COST_LONGLONG || COST_REAL || REAL_LONG */
      }
    }

    tk = lk*(ctod(prob->sol->f) - cl)/dnorm;
    for(i = 0; i < prob->n; i++) {
      u[i] += tk*d[i];
    }
  }

  if(ret == SIPS_UNSOLVED
     && (prob->param->ls == SIPS_LS_COMBINED_A
	 || prob->param->ls == SIPS_LS_COMBINED_B)) {
    tmpsol->f = prob->sol->f;
    edynasearch(prob, prob->sol);
#ifndef LB_DEBUG
    if(ctod(prob->sol->f) - lmax < prob->param->lbeps) {
      ret = SIPS_SOLVED;
    }
#endif /* !LB_DEBUG */
    if(ret == SIPS_UNSOLVED && obj_lesser(prob->sol->f, tmpsol->f)) {
      off = 0.0;
      for(i = 0; i < prob->n; i++) {
	off += ubest[i];
      }

      ret2 = lag_solve_LR2adj(prob, ubest, ctod(prob->sol->f) - off,
			      tmpsol, &cl, o);
      cl += off;

      if(ret2 != SIPS_NORMAL) {
	if(ret2 == SIPS_OPTIMAL) {
#ifdef LB_DEBUG
	  fprintf(stdout, "Feasible.\n");
	} else {
	  fprintf(stderr, "Feasibility error.\n");
	  exit(1);
#endif /* LB_DEBUG */
	}

	PRINT_ITER_INFO2(stdout);
	ret = SIPS_SOLVED;
      } else {
	ret2 = lag_solve_LR2adj_backward(prob, ubest,
					 ctod(prob->sol->f) - off);
	PRINT_ITER_INFO2(stdout);
#ifdef LB_DEBUG
	if(ret2 == SIPS_INFEASIBLE) {
	  fprintf(stderr, "Feasibility error.\n");
	  exit(1);
	}
#else /* LB_DEBUG */
	if(ret2 == SIPS_INFEASIBLE) {
	  lmax = *lb = ctod(prob->sol->f);
	  PRINT_ITER_INFO2(stdout);
	  ret = SIPS_SOLVED;
	}
#endif /* LB_DEBUG */
      }
    }
  }

  for(i = 0; i < prob->n; i++) {
    prob->sjob[i]->u = ubest[i];
  }

  xfree(d);
  xfree(u);
  xfree(o);
  xfree(gap_table);

  free_solution(tmpsol);
  free_solution(csol);

  if(prob->param->verbose >= 2) {
    fprintf(stdout, "       Iterations: %u\n", iter);
    PRINT_UBINFO(stdout);
    fprintf(stdout, "            Nodes: %u\n", prob->graph->n_nodes);
    fprintf(stdout, "             Time: ");
    _print_time(prob, stdout);
    fflush(stdout);
  } else if(prob->param->verbose >= 1) {
    PRINT_UBINFO_SHORT(stdout);
    fflush(stdout);
  }

  *lb = lmax;
  return(ret);
}
