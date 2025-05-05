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
 *  $Id: constraint.c,v 1.14 2013/05/28 13:27:01 tanaka Rel $
 *  $Revision: 1.14 $
 *  $Date: 2013/05/28 13:27:01 $
 *  $Author: tanaka $
 *
 */

/*
 * [NOTE]
 *   The algorithms for constraint propagation derives from:
 *     
 *     J. Carlier and E. Pinson. 1989.
 *     An algorithm for solving the job-shop problem,
 *     Management Science 35, 164/176.
 *
 *     J. Carlier and E. Pinson. 1990.
 *     A practical use of Jackson's preemptive scheduling for
 *     solving the job-shop problem,
 *     Annals of Operations Research 26, 268/287.
 *
 *     J. Carlier and E. Pinson. 1994.
 *     Adjustment of heads and tails for the job-shop problem,
 *     European Journal of Operational Research 78, 146/161.
 *
 *     P. Brucker, B. Jurisch and A. Krarner. 1994.
 *     The job-shop problem and immediate selection,
 *     Annals of Operations Research 50, 73/114.
 *
 *     P. Baptiste, C. Le Pape and W. Nuijten. 2001.
 *     Constraint-Based Scheduling:  Applying Constraint Programming
 *     to Scheduling Problems.
 *     Kluwer Academic Publishers.
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "define.h"
#include "sips_common.h"
#include "bmemory.h"
#include "constraint.h"
#include "fixed.h"
#include "lag2.h"
#include "lag_common.h"
#include "memory.h"
#include "ptable.h"
#include "print.h"

#ifdef NLOGN_EDGE_FINDING
typedef struct {
  unsigned short j;
  unsigned short no;
  int sigma;
  int tau;
  int pplus;
  int ksi;
  int q;
  int r;
} _btree_t;
#endif /* NLOGN_EDGE_FINDING */

typedef struct {
  unsigned short j;
  int s;
  int e;
} _list_t;

typedef struct {
  unsigned short j;
  int bi;
  int key;
} _list2_t;

static int _forbidden_time_window(sips *, _window_t *);
static int _update_dominance(sips *, _window_t *);
static int _adjust_by_psum(sips *, _window_t *, _list_t **, _list_t **);
static int _adjust_by_not_first_not_last(sips *, _window_t *, _list_t **,
					 _list_t **);
static int _not_first(sips *, _window_t *, _list_t **);
static int _not_last(sips *, _window_t *, _list_t **);
static int _adjust_by_edge_finding(sips *, _window_t *, _list_t **,
				   _list_t **);
#ifdef NLOGN_EDGE_FINDING
static int _Adjust(sips *, _window_t *, _btree_t *, _btree_t **, _list2_t *,
		   char, char *);
static int _Find(sips *, _btree_t **, int);
static void _Update(sips *, _btree_t **, int, int);
static void _update_heap_root(int, _list2_t *);
static void _update_heap_leaf(int, _list2_t *);
#ifdef DEBUG
static void _print_bt(sips *, _btree_t **);
#endif /* DEBUG */
#else /* NLOGN_EDGE_FINDING */
static int _edge_finding_head(sips *, _window_t *, _list_t **);
static int _edge_finding_tail(sips *, _window_t *, _list_t **);
#endif /* NLOGN_EDGE_FINDING */
static int _compare_list2(const void *, const void *);
static int _compare_list_by_s(const void *, const void *);
static int _compare_list_by_e(const void *, const void *);

/*
 * constraint_propagation(prob, w)
 *   updates job time windows by contraint propagation.
 *       w: list of job time window
 *
 */
int constraint_propagation(sips *prob, _window_t *w)
{
  int ret;
  _list_t *r, *d;

  r = d = NULL;

  ret = SIPS_NORMAL;
  do {
    if((ret = _update_dominance(prob, w)) == SIPS_INFEASIBLE) {
      break;
    }

    if((ret = _forbidden_time_window(prob, w)) == SIPS_INFEASIBLE) {
      break;
    }

    if((ret = _adjust_by_psum(prob, w, &r, &d)) == SIPS_INFEASIBLE) {
      break;
    }

    if((ret = _adjust_by_edge_finding(prob, w, &r, &d)) == SIPS_INFEASIBLE) {
      break;
    }

    if((ret = _adjust_by_not_first_not_last(prob, w, &r, &d))
       == SIPS_INFEASIBLE) {
      break;
    }
  } while(0);

  xfree(r);
  xfree(d);

  return(ret);
}

#define ELIMINATE_FORBIDDEN(x, y) {					\
    for(t = max(w[(x)].s, w[(y)].e + 1 - prob->sjob[(y)]->p);		\
	t < w[(y)].s + prob->sjob[(x)]->p && t <= w[(x)].e;		\
	t++) {								\
      prec_set_dom_ti(t, x);						\
    }									\
  }

/*
 * constraint_propagation_simple(prob, w)
 *    only eliminates nodes in forbidden time window for disjunctive pairs.
 *       w: list of job time window
 *
 */
int constraint_propagation_simple(sips *prob, _window_t *w)
{
  int t;
  int i, j;
#ifdef AGGRESSIVE
  char **inc;
#endif /* AGGRESSIVE */
  _ptable_t *ptable;

  ptable = prob->graph->ptable;
#ifdef AGGRESSIVE
  inc = ptable->inc;
#endif /* AGGRESSIVE */

  for(i = 0; i < prob->n - 1; i++) {
    for(j = i + 1; j < prob->n; j++) {
#ifdef DEBUG
      if(w[i].e < w[i].s + prob->sjob[i]->p + prob->sjob[j]->p - 1
	 && w[j].s < w[i].s + prob->sjob[j]->p
	 && w[j].e + prob->sjob[i]->p > w[i].e) {
	printf("no%d: [%d %d] of [%d %d] is forbidden from no%d [%d %d]\n",
	       prob->sjob[j]->rno,
	       w[i].e - prob->sjob[i]->p + 1,
	       w[i].s + prob->sjob[j]->p - 1,
	       w[j].s, w[j].e,
	       prob->sjob[i]->rno,
	       w[i].s, w[i].e);
	ELIMINATE_FORBIDDEN(j, i);
      }

      if(w[j].e < w[j].s + prob->sjob[i]->p + prob->sjob[j]->p - 1
	 && w[i].s < w[j].s + prob->sjob[i]->p  
	 && w[i].e + prob->sjob[j]->p > w[j].e) {
	printf("no%d: [%d %d] of [%d %d] is forbidden from no%d [%d %d]\n",
	       prob->sjob[i]->rno,
	       w[j].e - prob->sjob[j]->p + 1,
	       w[j].s + prob->sjob[i]->p - 1,
	       w[i].s, w[i].e,
	       prob->sjob[j]->rno,
	       w[j].s, w[j].e);
	ELIMINATE_FORBIDDEN(i, j);
      }
#else /* DEBUG */
      ELIMINATE_FORBIDDEN(j, i);
      ELIMINATE_FORBIDDEN(i, j);
#endif /* DEBUG */

#ifdef AGGRESSIVE
      if(inc[i][j] == 0) {
#ifdef SIPSI
	if((prob->param->ptype & SIPS_PROB_REGULAR)
	   && prob->sjob[i]->p == prob->sjob[j]->p
	   && prob->sjob[i]->r + prob->sjob[j]->p <= w[j].s 
	   && prob->sjob[j]->r + prob->sjob[i]->p <= w[i].s) {
	  if(prob->sjob[i]->d <= prob->sjob[j]->d
	     && prob->sjob[i]->tno < prob->sjob[j]->tno
	     && obj_greater_equal(prob->sjob[i]->tw, prob->sjob[j]->tw)) {
	    _set_inc(i, j);
	  } else if(prob->sjob[i]->d >= prob->sjob[j]->d
		    && prob->sjob[i]->tno > prob->sjob[j]->tno
		    && obj_lesser_equal(prob->sjob[i]->tw,
					prob->sjob[j]->tw)) {
	    _set_inc(j, i);
	  } else if(max(w[i].e, w[j].e)
		    <= min(prob->sjob[i]->d, prob->sjob[j]->d)) {
	    if(prob->sjob[i]->tno < prob->sjob[j]->tno) {
	      _set_inc(i, j);
	    } else {
	      _set_inc(j, i);
	    }
	  }
	}
#else /* SIPSI */
	if((prob->param->ptype & SIPS_PROB_REGULAR)
	   && max(w[i].e, w[j].e) <= min(prob->sjob[i]->d, prob->sjob[j]->d)) {
	  if(prob->sjob[i]->p <= prob->sjob[j]->p
	     && prob->sjob[i]->tno < prob->sjob[j]->tno) {
	    _set_inc(i, j);
	  } else if(prob->sjob[i]->p >= prob->sjob[j]->p
		    && prob->sjob[i]->tno > prob->sjob[j]->tno) {
	    _set_inc(j, i);
	  }
	}
#endif /* SIPSI */
      }
#endif /* AGGRESSIVE */
    }
  }

  return(SIPS_NORMAL);
}

/*
 * _update_dominance(prob, w)
 *    updates dominance of jobs by the current job time windows.
 *       w: list of job time window
 *
 *    This algorithm derives from
 *      P. Brucker, B. Jurisch and A. Krarner. 1994.
 *      The job-shop problem and immediate selection,
 *      Annals of Operations Research 50, 73/114.
 *
 */
int _update_dominance(sips *prob, _window_t *w)
{
  int i, j, k;
  char **inc;
  _ptable_t *ptable;
  _list2_t *list, *list2;

  ptable = prob->graph->ptable;
  inc = ptable->inc;

  list = (_list2_t *) xmalloc(2*prob->n*sizeof(_list2_t));
  list2 = list + prob->n;

  for(i = 0; i < prob->n; i++) {
    list[i].j = list2[i].j = i;
    list[i].key = - w[i].s;
    list2[i].key = - w[i].e + prob->sjob[i]->p;
  }
  qsort((void *) list, prob->n, sizeof(_list2_t), _compare_list2); 
  qsort((void *) list2, prob->n, sizeof(_list2_t), _compare_list2); 

  j = 0;
  for(i = 0; i < prob->n; i++) {
    for(; j < prob->n && list[i].key >= list2[j].key; j++);
    if(j < prob->n) {
      for(k = j; k < prob->n; k++) {
        if(list2[k].j != list[i].j) {
          if(inc[list2[k].j][list[i].j] == -1) {
            xfree(list);
            return(SIPS_INFEASIBLE);
          } else if(inc[list2[k].j][list[i].j] != 1) {
            _set_inc(list2[k].j, list[i].j);
          }
        }
      }
    }
  }

  xfree(list);

  return(SIPS_NORMAL);
}

/*
 * _forbidden_time_window(prob, w)
 *    (1) update time windows by the dominance of a pair of jobs
 *    (2) forbidden time window for disjunctive pairs
 *       w: list of job time window
 *
 */
int _forbidden_time_window(sips *prob, _window_t *w)
{
  int t;
  int i, j;
  int ns, ne;
  char **inc;
  _ptable_t *ptable;

  ptable = prob->graph->ptable;
  inc = ptable->inc;

  for(i = 0; i < prob->n - 1; i++) {
    for(j = i + 1; j < prob->n; j++) {
      if(inc[i][j] == 0) {
#ifdef DEBUG
	if(w[i].e < w[i].s + prob->sjob[i]->p + prob->sjob[j]->p - 1
	   && w[j].s < w[i].s + prob->sjob[j]->p
	   && w[j].e + prob->sjob[i]->p > w[i].e) {
	  printf("no%d: [%d %d] of [%d %d] is forbidden from no%d [%d %d]\n",
		 prob->sjob[j]->rno,
		 w[i].e - prob->sjob[i]->p + 1,
		 w[i].s + prob->sjob[j]->p - 1,
		 w[j].s, w[j].e,
		 prob->sjob[i]->rno,
		 w[i].s, w[i].e);
	  ELIMINATE_FORBIDDEN(j, i);
	}

	if(w[j].e < w[j].s + prob->sjob[i]->p + prob->sjob[j]->p - 1
	   && w[i].s < w[j].s + prob->sjob[i]->p  
	   && w[i].e + prob->sjob[j]->p > w[j].e) {
	  printf("no%d: [%d %d] of [%d %d] is forbidden from no%d [%d %d]\n",
		 prob->sjob[i]->rno,
		 w[j].e - prob->sjob[j]->p + 1,
		 w[j].s + prob->sjob[i]->p - 1,
		 w[i].s, w[i].e,
		 prob->sjob[j]->rno,
		 w[j].s, w[j].e);
	  ELIMINATE_FORBIDDEN(i, j);
	}
#else /* DEBUG */
	ELIMINATE_FORBIDDEN(j, i);
	ELIMINATE_FORBIDDEN(i, j);
#endif /* DEBUG */

#ifdef AGGRESSIVE
#ifdef SIPSI
	if((prob->param->ptype & SIPS_PROB_REGULAR)
	   && prob->sjob[i]->p == prob->sjob[j]->p
	   && prob->sjob[i]->r + prob->sjob[j]->p <= w[j].s 
	   && prob->sjob[j]->r + prob->sjob[i]->p <= w[i].s) {
	  if(prob->sjob[i]->d <= prob->sjob[j]->d
	     && prob->sjob[i]->tno < prob->sjob[j]->tno
	     && obj_greater_equal(prob->sjob[i]->tw, prob->sjob[j]->tw)) {
	    _set_inc(i, j);
	  } else if(prob->sjob[i]->d >= prob->sjob[j]->d
		    && prob->sjob[i]->tno > prob->sjob[j]->tno
		    && obj_lesser_equal(prob->sjob[i]->tw,
					prob->sjob[j]->tw)) {
	    _set_inc(j, i);
	  } else if(max(w[i].e, w[j].e)
		    <= min(prob->sjob[i]->d, prob->sjob[j]->d)) {
	    if(prob->sjob[i]->tno < prob->sjob[j]->tno) {
	      _set_inc(i, j);
	    } else {
	      _set_inc(j, i);
	    }
	  } else {
	    continue;
	  }
	} else {
	  continue;
	}
#else /* SIPSI */
	if((prob->param->ptype & SIPS_PROB_REGULAR)
	   && max(w[i].e, w[j].e) <= min(prob->sjob[i]->d, prob->sjob[j]->d)) {
	  if(prob->sjob[i]->p <= prob->sjob[j]->p
	     && prob->sjob[i]->tno < prob->sjob[j]->tno) {
	    _set_inc(i, j);
	  } else if(prob->sjob[i]->p >= prob->sjob[j]->p
		    && prob->sjob[i]->tno > prob->sjob[j]->tno) {
	    _set_inc(j, i);
	  } else {
	    continue;
	  }
	} else {
	  continue;
	}
#endif /* SIPSI */
#else /* AGGRESSIVE */
	continue;
#endif /* AGGRESSIVE */
      }

      if(inc[i][j] == 1) {
	ns = w[i].s + prob->sjob[j]->p;
	if(w[j].s < ns) {
#ifdef DEBUG
	  printf(">No%d: [%d %d]=>[%d %d]\n",
		 prob->sjob[j]->rno,
		 w[j].s, w[j].e,
		 ns, w[j].e);
#endif /* DEBUG */
	  w[j].s = ns;
	  if(w[j].s > w[j].e) {
	    return(SIPS_INFEASIBLE);
	  }
	}
	ne = w[j].e - prob->sjob[j]->p;
	if(w[i].e > ne) {
#ifdef DEBUG
	  printf(">No%d: [%d %d]=>[%d %d]\n",
		 prob->sjob[i]->rno,
		 w[i].s, w[i].e,
		 w[i].s, ne);
#endif /* DEBUG */
	  w[i].e = ne;
	  if(w[i].s > w[i].e) {
	    return(SIPS_INFEASIBLE);
	  }
	}
      } else if(inc[j][i] == 1) {
	ns = w[j].s + prob->sjob[i]->p;
	if(w[i].s < ns) {
#ifdef DEBUG
	  printf(">No%d: [%d %d]=>[%d %d]\n",
		 prob->sjob[i]->rno,
		 w[i].s, w[i].e,
		 ns, w[i].e);
#endif /* DEBUG */
	  w[i].s = ns;
	  if(w[i].s > w[i].e) {
	    return(SIPS_INFEASIBLE);
	  }
	}
	ne = w[i].e - prob->sjob[i]->p;
	if(w[j].e > ne) {
#ifdef DEBUG
	  printf(">No%d: [%d %d]=>[%d %d]\n",
		 prob->sjob[j]->rno,
		 w[j].s, w[j].e,
		 w[j].s, ne);
#endif /* DEBUG */
	  w[j].e = ne;
	  if(w[j].s > w[j].e) {
	    return(SIPS_INFEASIBLE);
	  }
	}
      }
    }
  }

  return(SIPS_NORMAL);
}

/*
 * _adjust_by_psum(prob, w, r, d)
 *   updates job time windows by total processing times of 
 *   dominating/dominated jobs.
 *       w: list of job time window
 *       r: list of jobs sorted in the nondecreasing order of heads
 *       d: list of jobs sorted in the nondecreasing order of tails
 *
 */
int _adjust_by_psum(sips *prob, _window_t *w, _list_t **r, _list_t **d)
{
  int i, j;
  int p, rmax;
  char updated;
  _list_t *list;
  char **inc;
  _ptable_t *ptable;

  ptable = prob->graph->ptable;
  inc = ptable->inc;

#ifdef DEBUG
  printf("Pre-Psum\n");
#endif /* DEBUG */

  if(*r == NULL) {
    *r = list = (_list_t *) xmalloc(prob->n*sizeof(_list_t));
    for(i = 0; i < prob->n; i++) {
      list[i].j = i;
      list[i].s = w[i].s - prob->sjob[i]->p;
      list[i].e = w[i].e;
    }
    qsort((void *) list, prob->n, sizeof(_list_t), _compare_list_by_s);
  } else {
    list = *r;
  }

  updated = SIPS_FALSE;
  for(i = 0; i < prob->n; i++) {
    p = ptable->pre_psum[list[i].j];
    rmax = list[i].s;
    for(j = 0; j < i; j++) {
      if(inc[list[j].j][list[i].j] == 1) {
	rmax = max(rmax, list[j].s + p);
	p -= prob->sjob[list[j].j]->p;
      }
    }

    rmax += prob->sjob[list[i].j]->p;
    if(w[list[i].j].s < rmax) {
#ifdef DEBUG
      printf("-No%d: [%d %d]=>[%d %d]\n",
	     prob->sjob[list[i].j]->rno,
	     w[list[i].j].s, w[list[i].j].e,
	     rmax, w[list[i].j].e);
#endif /* DEBUG */
      updated = SIPS_TRUE;
      w[list[i].j].s = rmax;
      if(w[list[i].j].s > w[list[i].j].e) {
	return(SIPS_INFEASIBLE);
      }
    }
  }

  if(is_true(updated)) {
    xfree(*r);
    *r = NULL;
  }

#ifdef DEBUG
  printf("Suc-Psum\n");
#endif /* DEBUG */

  if(*d == NULL) {
    *d = list = (_list_t *) xmalloc(prob->n*sizeof(_list_t));
    for(i = 0; i < prob->n; i++) {
      list[i].j = i;
      list[i].s = w[i].s - prob->sjob[i]->p;
      list[i].e = w[i].e;
    }
    qsort((void *) list, prob->n, sizeof(_list_t), _compare_list_by_e);
  } else {
    list = *d;
  }

  updated = SIPS_FALSE;
  for(i = prob->n - 1; i >= 0; i--) {
    p = ptable->suc_psum[list[i].j];
    rmax = list[i].e;
    for(j = prob->n - 1; j > i; j--) {
      if(inc[list[i].j][list[j].j] == 1) {
	rmax = min(rmax, list[j].e - p);
	p -= prob->sjob[list[j].j]->p;
      }
    }

    if(w[list[i].j].e > rmax) {
#ifdef DEBUG
      printf("-No%d: [%d %d]=>[%d %d]\n",
	     prob->sjob[list[i].j]->rno,
	     w[list[i].j].s, w[list[i].j].e,
	     w[list[i].j].s, rmax);
#endif /* DEBUG */
      updated = SIPS_TRUE;
      w[list[i].j].e = rmax;
      if(w[list[i].j].s > w[list[i].j].e) {
	return(SIPS_INFEASIBLE);
      }
    }
  }

  if(is_true(updated)) {
    xfree(*d);
    *d = NULL;
  }

  return(SIPS_NORMAL);
}

/*
 * _adjust_by_not_first_not_last(prob, w, r, d)
 *   updates job time windows by Not-First and Not-Last.
 *       w: list of job time window
 *       r: list of jobs sorted in the nondecreasing order of heads
 *       d: list of jobs sorted in the nondecreasing order of tails
 *
 *   This algorithm derives from
 *     P. Baptiste, C. Le Pape and W. Nuijten. 2001.
 *     Constraint-Based Scheduling:  Applying Constraint Programming
 *     to Scheduling Problems.
 *     Kluwer Academic Publishers.
 *
 */
int _adjust_by_not_first_not_last(sips *prob, _window_t *w,
				  _list_t **r, _list_t **d)
{
#ifdef DEBUG
  printf("Not-First\n");
#endif /* DEBUG */
  if(_not_first(prob, w, d) == SIPS_INFEASIBLE) {
    return(SIPS_INFEASIBLE);
  }

#ifdef DEBUG
  printf("Not-Last\n");
#endif /* DEBUG */
  if(_not_last(prob, w, r) == SIPS_INFEASIBLE) {
    return(SIPS_INFEASIBLE);
  }

  return(SIPS_NORMAL);
}

/*
 * _not_first(prob, w, d)
 *   updates heads by Not-First procedure in Baptiste, Le Pape and
 *   Nuijten (2001).
 *       w: list of job time window
 *       d: list of jobs sorted in the nonincreasing order of tails
 *
 */
int _not_first(sips *prob, _window_t *w, _list_t **d)
{
  int i, j;
  int Sji;
  int rpi, rpj;
  char updated;
  int *Deltaj;
  _list_t *list;

  if(*d == NULL) {
    *d = list = (_list_t *) xmalloc(prob->n*sizeof(_list_t));
    for(i = 0; i < prob->n; i++) {
      list[i].j = i;
      list[i].s = w[i].s - prob->sjob[i]->p;
      list[i].e = w[i].e;
    }
    qsort((void *) list, prob->n, sizeof(_list_t), _compare_list_by_e);
  } else {
    list = *d;
  }

  Deltaj = (int *) xmalloc((prob->n + 1)*sizeof(int));

  updated = SIPS_FALSE;
  for(j = 0; j < prob->n; j++) {
    memset((void *) Deltaj, 0, (prob->n + 1)*sizeof(int));
    Sji = 0;
    Deltaj[0] = LARGE_INTEGER;
    for(i = 0; i < prob->n; i++) {
      if(list[j].s + prob->sjob[list[j].j]->p
	 <= list[i].s + prob->sjob[list[i].j]->p) {
	Sji += prob->sjob[list[i].j]->p;
	Deltaj[i + 1] = min(Deltaj[i], list[i].e - Sji);
      } else {
	Deltaj[i + 1] = Deltaj[i];
      }
    }

    for(i = 0; i < prob->n; i++) {
      rpi = list[i].s + prob->sjob[list[i].j]->p;
      rpj = list[j].s + prob->sjob[list[j].j]->p;
      if(rpi < rpj) {
	if(rpi > Deltaj[prob->n]
	   && w[list[i].j].s < rpj + prob->sjob[list[i].j]->p) {
#ifdef DEBUG
	  printf("=No%d: [%d %d]=>[%d %d]\n",
		 prob->sjob[list[i].j]->rno,
		 w[list[i].j].s, w[list[i].j].e,
		 rpj + prob->sjob[list[i].j]->p, w[list[i].j].e);
#endif /* DEBUG */
	  updated = SIPS_TRUE;
	  w[list[i].j].s = rpj + prob->sjob[list[i].j]->p;
	  if(w[list[i].j].s > w[list[i].j].e) {
	    xfree(Deltaj);
	    return(SIPS_INFEASIBLE);
	  }
	}
      } else if((rpi > Deltaj[i] || list[i].s > Deltaj[prob->n])
		&& w[list[i].j].s < rpj + prob->sjob[list[i].j]->p) {
#ifdef DEBUG
	printf("=No%d: [%d %d]=>[%d %d]\n",
	       prob->sjob[list[i].j]->rno,
	       w[list[i].j].s, w[list[i].j].e,
	       rpj + prob->sjob[list[i].j]->p, w[list[i].j].e);
#endif /* DEBUG */
	w[list[i].j].s = rpj + prob->sjob[list[i].j]->p;
	if(w[list[i].j].s > w[list[i].j].e) {
	  updated = SIPS_TRUE;
	  xfree(Deltaj);
	  return(SIPS_INFEASIBLE);
	}
      }
    }
  }

  if(is_true(updated)) {
    xfree(*d);
    *d = NULL;
  }
  xfree(Deltaj);

  return(SIPS_NORMAL);
}

/*
 * _not_last(prob, w, r)
 *   updates tails by Not-First procedure in Baptiste, Le Pape and
 *   Nuijten (2001).
 *       w: list of job time window
 *       r: list of jobs sorted in the nondecreasing order of heads
 *
 */
int _not_last(sips *prob, _window_t *w, _list_t **r)
{
  int i, j;
  int Sji;
  int rpi, rpj;
  char updated;
  int *Deltaj;
  _list_t *list;

  if(*r == NULL) {
    *r = list = (_list_t *) xmalloc(prob->n*sizeof(_list_t));
    for(i = 0; i < prob->n; i++) {
      list[i].j = i;
      list[i].s = w[i].s - prob->sjob[i]->p;
      list[i].e = w[i].e;
    }
    qsort((void *) list, prob->n, sizeof(_list_t), _compare_list_by_s);
  } else {
    list = *r;
  }

  Deltaj = (int *) xmalloc((prob->n + 1)*sizeof(int));

  updated = SIPS_FALSE;
  for(j = prob->n - 1; j >= 0; j--) {
    memset((void *) Deltaj, 0, (prob->n + 1)*sizeof(int));
    Sji = 0;
    Deltaj[prob->n] = - LARGE_INTEGER;
    for(i = prob->n - 1; i >= 0; i--) {
      if(list[j].e + prob->sjob[list[i].j]->p
	 >= list[i].e + prob->sjob[list[j].j]->p) {
	Sji += prob->sjob[list[i].j]->p;
	Deltaj[i] = max(Deltaj[i + 1], list[i].s + Sji);
      } else {
	Deltaj[i] = Deltaj[i + 1];
      }
    }

    for(i = prob->n - 1; i >= 0; i--) {
      rpi = list[i].e - prob->sjob[list[i].j]->p;
      rpj = list[j].e - prob->sjob[list[j].j]->p;
      if(rpi > rpj) {
	if(rpi < Deltaj[0] && w[list[i].j].e > rpj) {
#ifdef DEBUG
	  printf("=No%d: [%d %d]=>[%d %d]\n",
		 prob->sjob[list[i].j]->rno,
		 w[list[i].j].s, w[list[i].j].e,
		 w[list[i].j].s, rpj);
#endif /* DEBUG */
	  updated = SIPS_TRUE;
	  w[list[i].j].e = rpj;
	  if(w[list[i].j].s > w[list[i].j].e) {
	    xfree(Deltaj);
	    return(SIPS_INFEASIBLE);
	  }
	}
      } else if((rpi < Deltaj[i + 1] || list[i].e < Deltaj[0])
		&& w[list[i].j].e > rpj) {
#ifdef DEBUG
	printf("=No%d: [%d %d]=>[%d %d]\n",
	       prob->sjob[list[i].j]->rno,
	       w[list[i].j].s, w[list[i].j].e,
	       w[list[i].j].s, rpj);
#endif /* DEBUG */
	updated = SIPS_TRUE;
	w[list[i].j].e = rpj;
	if(w[list[i].j].s > w[list[i].j].e) {
	  xfree(Deltaj);
	  return(SIPS_INFEASIBLE);
	}
      }
    }
  }

  if(is_true(updated)) {
    xfree(*r);
    *r = NULL;
  }
  xfree(Deltaj);

  return(SIPS_NORMAL);
}

#ifdef NLOGN_EDGE_FINDING
/*
 * _adjust_by_edge_finding(prob, w, r, d)
 *   updates job time windows by applying Edge-Finding in
 *       w: list of job time window
 *       r: list of jobs sorted in the nondecreasing order of heads
 *       d: list of jobs sorted in the nonincreasing order of tails
 *
 *   This algorithm derives from
 *     J. Carlier and E. Pinson. 1994.
 *     Adjustment of heads and tails for the job-shop problem,
 *     European Journal of Operational Research 78, 146/161.
 *
 *
 */
int _adjust_by_edge_finding(sips *prob, _window_t *w, _list_t **r, _list_t **d)
{
  int i;
  int ret;
  char updated;
  _btree_t *btl, **bt;
  _list2_t *U;
  int *bi, *rbi;

  bi = prob->graph->ptable->btctable;

  if(*r == NULL) {
    *r = (_list_t *) xmalloc(prob->n*sizeof(_list_t));
    for(i = 0; i < prob->n; i++) {
      (*r)[i].j = i;
      (*r)[i].s = w[i].s - prob->sjob[i]->p;
      (*r)[i].e = w[i].e;
    }
    qsort((void *) *d, prob->n, sizeof(_list_t), _compare_list_by_s);
  }
  if(*d == NULL) {
    *d = (_list_t *) xmalloc(prob->n*sizeof(_list_t));
    for(i = 0; i < prob->n; i++) {
      (*d)[i].j = i;
      (*d)[i].s = w[i].s - prob->sjob[i]->p;
      (*d)[i].e = w[i].e;
    }
    qsort((void *) *d, prob->n, sizeof(_list_t), _compare_list_by_e);
  }

  btl = (_btree_t *) xcalloc(prob->n, sizeof(_btree_t));
  bt = (_btree_t **) xmalloc(prob->n*sizeof(_btree_t *));
  U = (_list2_t *) xmalloc(prob->n*sizeof(_list2_t));
  rbi = (int *) xmalloc(prob->n*sizeof(int));

  for(i = 0; i < prob->n; i++) {
    btl[i].no = i;
    btl[i].j = (*d)[prob->n - i - 1].j;
    btl[i].q = prob->graph->Tmax - (*d)[prob->n - i - 1].e;
    btl[i].r = (*d)[prob->n - i - 1].s;
    bt[i] = btl + bi[i];

    rbi[(*r)[i].j] = i;
  }

  for(i = 0; i < prob->n; i++) {
    U[i].j = (*r)[i].j;
    U[i].key = (*r)[i].s;
    U[rbi[bt[i]->j]].bi = i;
  }

#ifdef DEBUG
  printf("Edge-Finding head\n");
#endif /* DEBUG */
  if((ret = _Adjust(prob, w, btl, bt, U, 1, &updated)) == SIPS_INFEASIBLE) {
    xfree(rbi);
    xfree(U);
    xfree(bt);
    xfree(btl);
    return(ret);
  }

  if(updated) {
    xfree(*r);
    *r = (_list_t *) xmalloc(prob->n*sizeof(_list_t));
    for(i = 0; i < prob->n; i++) {
      (*r)[i].j = i;
      (*r)[i].s = w[i].s - prob->sjob[i]->p;
      (*r)[i].e = w[i].e;
    }
    qsort((void *) *r, prob->n, sizeof(_list_t), _compare_list_by_s);
  }

  memset((void *) btl, 0, prob->n*sizeof(_btree_t));

  for(i = 0; i < prob->n; i++) {
    btl[i].no = i;
    btl[i].j = (*r)[i].j;
    btl[i].q = (*r)[i].s;
    btl[i].r = prob->graph->Tmax - (*r)[i].e;

    rbi[(*d)[prob->n - i - 1].j] = i;
  }

  for(i = 0; i < prob->n; i++) {
    U[i].j = (*d)[prob->n - i - 1].j;
    U[i].key = prob->graph->Tmax - (*d)[prob->n - i - 1].e;
    U[rbi[bt[i]->j]].bi = i;
  }

#ifdef DEBUG
  printf("Edge-Finding tail\n");
#endif /* DEBUG */
  ret = _Adjust(prob, w, btl, bt, U, 0, &updated);

  xfree(rbi);
  xfree(U);
  xfree(bt);
  xfree(btl);

  if(ret == SIPS_INFEASIBLE) {
    return(ret);
  }

  if(updated) {
    xfree(*d);
    *d = NULL;
  }

  return(ret);
}

/*
 * _Adjust(prob, w, btl, bt, U, head)
 *   updates heads/tails by "Adjust" in Carlier and Pinson (1994).
 *       w: list of job time window
 *     btl: work
 *      bt: work
 *       U: work
 *    head: 1 if head adjustment, 0 if tail adjustment
 * updated: 1 if heads or tails are adjusted
 *
 */
int _Adjust(sips *prob, _window_t *w, _btree_t *btl, _btree_t **bt,
	    _list2_t *U, char head, char *updated)
{
  int i;
  int t, tdash;
  int sc, nu;
  int cU, nA, nD;
  int epsilon;
  int UB;
  _list2_t *A, *D;
  char **inc;

  inc = prob->graph->ptable->inc;

  UB = prob->graph->Tmax;
#ifdef DEBUG
  printf("UB=%d\n", UB);
#endif /* DEBUG */

  A = (_list2_t *) xmalloc(2*prob->n*sizeof(_list2_t));
  D = A + prob->n;

#ifdef DEBUG
  for(i = 0; i < prob->n; i++) {
    printf("(no%d,%d)", prob->sjob[btl[i].j]->rno, btl[i].q);
  }
  printf("\n");
#endif /* DEBUG */

  /* initialize */
  for(i = prob->n - 1; i >= 0; i--) {
    bt[i]->sigma = prob->sjob[bt[i]->j]->p;
    bt[i]->ksi = - LARGE_INTEGER;
    bt[i]->pplus = prob->sjob[bt[i]->j]->p;

    if(2*i + 2 < prob->n) {
      bt[i]->sigma += bt[2*i + 2]->tau;
      bt[i]->ksi = max(bt[i]->ksi, bt[2*i + 2]->ksi);
    }

    bt[i]->ksi = max(bt[i]->ksi, bt[i]->q + bt[i]->sigma);
    bt[i]->tau = bt[i]->sigma;
    if(2*i + 1 < prob->n) {
      bt[i]->tau += bt[2*i + 1]->tau;
      bt[i]->ksi = max(bt[i]->ksi, bt[2*i + 1]->ksi + bt[i]->sigma);
    }
  }

#ifdef DEBUG
  _print_bt(prob, bt);
#endif /* DEBUG */

  cU = 0;
  nA = nD = 0;
  nu = prob->n - 1;
  t = U[0].key;
  *updated = SIPS_FALSE;
  while(cU < prob->n || nA > 0 || nD > 0) {
#ifdef DEBUG
    printf("t=%d cU=%d nA=%d nD=%d\n", t, cU, nA, nD);
#endif /* DEBUG */
    for(; cU < prob->n && U[cU].key == t; cU++) {
      /* find */
      if((sc = _Find(prob, bt, U[cU].bi)) >= 0) {
	/* D */
#ifdef DEBUG
	printf("found no%d sc=%d (no%d) \n", prob->sjob[U[cU].j]->rno,
	       sc, prob->sjob[btl[sc].j]->rno);
#endif /* DEBUG */
	_Update(prob, bt, U[cU].bi, - prob->sjob[bt[U[cU].bi]->j]->p);
#ifdef DEBUG
	_print_bt(prob, bt);
#endif /* DEBUG */
	_Update(prob, bt, U[cU].bi, prob->sjob[bt[U[cU].bi]->j]->p);
	
	D[nD].j = U[cU].j;
	D[nD].bi = U[cU].bi;
	D[nD++].key = - sc;
	_update_heap_leaf(nD, D);
      } else {
	/* A */
	A[nA].j = U[cU].j;
	A[nA].bi = U[cU].bi;
	A[nA++].key = - bt[U[cU].bi]->no;
	_update_heap_leaf(nA, A);
      }
    }

    if(cU == prob->n) {
      tdash = LARGE_INTEGER;
    } else {
      tdash = U[cU].key;
    }

    if(nA > 0) {
      epsilon = min(bt[A[0].bi]->pplus, tdash - t);
      t += epsilon;
      bt[A[0].bi]->pplus -= epsilon;
      _Update(prob, bt, A[0].bi, - epsilon);
#ifdef DEBUG
      _print_bt(prob, bt);
      printf("no%d (bi=%d) %d->%d\n", prob->sjob[bt[A[0].bi]->j]->rno,
	     A[0].bi, bt[A[0].bi]->pplus + epsilon, bt[A[0].bi]->pplus);
#endif /* DEBUG */
      if(bt[A[0].bi]->pplus == 0) {
	A[0] = A[--nA];
	_update_heap_root(nA, A);
      }
    }

    for(; nu >= 0 && btl[nu].pplus == 0; nu--);
#ifdef DEBUG
    if(nu >= 0) {
      printf("nu=%d (no%d)\n", nu, prob->sjob[btl[nu].j]->rno);
    } else {
      printf("nu=%d\n", nu);
    }
#endif /* DEBUG */
    while(nD > 0 && - D[0].key > nu) {
      /* A */
#ifdef DEBUG
      printf("no%d in D -> A\n", prob->sjob[D[0].j]->rno);
#endif /* DEBUG */
      A[nA].j = D[0].j;
      A[nA].bi = D[0].bi;
      A[nA++].key = - bt[D[0].bi]->no;
      _update_heap_leaf(nA, A);

      if(head == 1 && w[D[0].j].s < t + prob->sjob[D[0].j]->p) {
#ifdef DEBUG
	printf("+No%d: [%d %d]=>[%d %d]\n",
	       prob->sjob[D[0].j]->rno,
	       w[D[0].j].s, w[D[0].j].e,
	       t + prob->sjob[D[0].j]->p, w[D[0].j].e);
#endif /* DEBUG */
	*updated = SIPS_TRUE;
	w[D[0].j].s = t + prob->sjob[D[0].j]->p;
	if(w[D[0].j].s > w[D[0].j].e) {
	  xfree(A);
	  return(SIPS_INFEASIBLE);
	}
      } else if(head == 0 && w[D[0].j].e > prob->graph->Tmax - t) {
#ifdef DEBUG
	printf("+No%d: [%d %d]=>[%d %d]\n",
	       prob->sjob[D[0].j]->rno,
	       w[D[0].j].s, w[D[0].j].e,
	       w[D[0].j].s, prob->graph->Tmax - t);
#endif /* DEBUG */
	*updated = SIPS_TRUE;
	w[D[0].j].e = prob->graph->Tmax - t;
	if(w[D[0].j].s > w[D[0].j].e) {
	  xfree(A);
	  return(SIPS_INFEASIBLE);
	}
      }

      D[0] = D[--nD];
      _update_heap_root(nD, D);
    }

    if(nA == 0 && cU < prob->n) {
      t = U[cU].key;
    }
  }

  xfree(A);

  return(SIPS_OK);
}

/*
 * _Update(prob, bt, k, epsilon)
 *   "Update" in Carlier and Pinson (1994).
 *          bt: binary tree
 *           k: target job index in bi
 *     epsilon: modification for p+
 *
 */
void _Update(sips *prob, _btree_t **bt, int k, int epsilon)
{
  int i;

  bt[k]->sigma += epsilon;
  bt[k]->ksi = - LARGE_INTEGER;
  if(2*k + 1 < prob->n) {
    bt[k]->ksi = max(bt[k]->ksi, bt[2*k + 1]->ksi + bt[k]->sigma);
  }
  if(2*k + 2 < prob->n) {
    bt[k]->ksi = max(bt[k]->ksi, bt[2*k + 2]->ksi);
  }
  if(bt[k]->pplus > 0) {
    bt[k]->ksi = max(bt[k]->ksi, bt[k]->q + bt[k]->sigma);
  }

  for(i = k; i > 0;) {
    i = (i - 1)/2;
    if(bt[i]->no < bt[k]->no) {
      bt[i]->sigma += epsilon;
    }
    bt[i]->ksi = - LARGE_INTEGER;
    if(2*i + 1 < prob->n) {
      bt[i]->ksi = max(bt[i]->ksi, bt[2*i + 1]->ksi + bt[i]->sigma);
    }
    if(2*i + 2 < prob->n) {
      bt[i]->ksi = max(bt[i]->ksi, bt[2*i + 2]->ksi);
    }
    if(bt[i]->pplus > 0) {
      bt[i]->ksi = max(bt[i]->ksi, bt[i]->q + bt[i]->sigma);
    }
  }
}

/*
 * _Find(prob, bt, k)
 *   "Find" in Carlier and Pinson (1994).  The original algorithm
 *   is modified so that backtrack is allowed to cope with the
 *   identical "q" values.
 *      bt: binary tree
 *       k: target job index in bi
 *
 */
int _Find(sips *prob, _btree_t **bt, int k)
{
  int i, j;
  int sc;
  int delta;

  _Update(prob, bt, k, - prob->sjob[bt[k]->j]->p);

  delta = prob->graph->Tmax - bt[k]->r - prob->sjob[bt[k]->j]->p;
#ifdef DEBUG
  printf("_Find: k=%d(no%d) delta=%d\n", k, prob->sjob[bt[k]->j]->rno, delta);
#endif /* DEBUG */

  sc = -1;
  i = 0;
  while(1) {
    while(i < prob->n && bt[i]->ksi > delta) {
#ifdef DEBUG
      printf("i=%d (no%d l=%d), ksi=%d, delta=%d\n", i,
	     prob->sjob[bt[i]->j]->rno, bt[i]->no, bt[i]->ksi, delta);
#endif /* DEBUG */

      if(2*i + 1 < prob->n && bt[2*i + 1]->ksi + bt[i]->sigma > delta) {
	for(j = 2*i + 1; j < prob->n; j = 2*j + 2);
	j = j/2 - 1; /* rightmost leaf of tree of root=(2*i + 1) */
	if(bt[j]->no > bt[k]->no) {
	  delta -= bt[i]->sigma;
	  i = 2*i + 1;
	  continue;
	}
      }

      if(bt[i]->q + bt[i]->sigma > delta && bt[i]->pplus > 0
	 && bt[i]->no > bt[k]->no) {
	sc = bt[i]->no;
	break;
      } else {
	i = 2*i + 2;
      }
    }

    if(sc == -1) {
      for(; i%2 == 0 && i > 0; i = i/2 - 1);
      if(i == 0) {
	break;
      }

      delta += bt[(i - 1)/2]->sigma;
      i++;
    } else {
      break;
    }
  }

  _Update(prob, bt, k, prob->sjob[bt[k]->j]->p);

  return(sc);
}

/*
 * _update_heap_root(n, h)
 *    updates the heap.  (The new element is added to its root.)
 *       n: number of elements
 *       h: heap
 *
 */
void _update_heap_root(int n, _list2_t *h)
{
  int i, j;
  _list2_t x;

  if(n <= 1) {
    return;
  }

  x = h[0];
  i = 0;
  while((j = 2*i + 1) < n) {
    if(j < n - 1 && _compare_list2(&(h[j]), &(h[j + 1])) > 0) {
      j++;
    }
    if(_compare_list2(&x, &(h[j])) <= 0) {
      break;
    }
    h[i] = h[j];
    i = j;
  }
  h[i] = x;
}

/*
 * _update_heap_leaf(n, h)
 *    updates the heap.  (The new element is added to its leaf.)
 *       n: number of elements
 *       h: heap
 *
 */
void _update_heap_leaf(int n, _list2_t *h)
{
  int i, j;
  _list2_t x;

  if(n <= 1) {
    return;
  }

  x = h[n - 1];
  i = n - 1;
  for(j = (i - 1)/2; i > 0 && _compare_list2(&x, &(h[j])) < 0; j = (i - 1)/2) {
    h[i] = h[j];
    i = j;
  }
  h[i] = x;
}
#else /* NLOGN_EDGE_FINDING */

/*
 * _adjust_by_edge_finding(prob, w, r, d)
 *   updates job time windows by applying Edge-Finding.
 *       w: list of job time window
 *       r: list of jobs sorted in the nondecreasing order of heads
 *       d: list of jobs sorted in the nonincreasing order of tails
 *
 *   This algorithm derives from
 *     P. Baptiste, C. Le Pape and W. Nuijten. 2001.
 *     Constraint-Based Scheduling:  Applying Constraint Programming
 *     to Scheduling Problems.
 *     Kluwer Academic Publishers.
 *
 */
int _adjust_by_edge_finding(sips *prob, _window_t *w, _list_t **r, _list_t **d)
{
#ifdef DEBUG
  printf("Edge-Finding head\n");
#endif /* DEBUG */
  if(_edge_finding_head(prob, w, r) == SIPS_INFEASIBLE) {
    return(SIPS_INFEASIBLE);
  }

#ifdef DEBUG
  printf("Edge-Finding tail\n");
#endif /* DEBUG */
  if(_edge_finding_tail(prob, w, d) == SIPS_INFEASIBLE) {
    return(SIPS_INFEASIBLE);
  }

  return(SIPS_NORMAL);
}

/*
 * _edge_finding_head(prob, w, r)
 *   updates heads by applying Edge-Finding procedure in
 *   Baptiste, Le Pape and Nuijten (2001).
 *       w: list of job time window
 *       r: list of jobs sorted in the nondecreasing order of heads.
 *
 */
int _edge_finding_head(sips *prob, _window_t *w, _list_t **r)
{
  int i, k;
  int P, C, H;
  int rnew;
  char updated;
  int *Ci;
  _list_t *list;

  if(*r == NULL) {
    *r = list = (_list_t *) xmalloc(prob->n*sizeof(_list_t));
    for(i = 0; i < prob->n; i++) {
      list[i].j = i;
      list[i].s = w[i].s - prob->sjob[i]->p;
      list[i].e = w[i].e;
    }
    qsort((void *) list, prob->n, sizeof(_list_t), _compare_list_by_s);
  } else {
    list = *r;
  }

  Ci = (int *) xcalloc(prob->n, sizeof(int));

  updated = SIPS_FALSE;
  for(k = 0; k < prob->n; k++) {
    P = 0;
    C = - LARGE_INTEGER;
    for(i = prob->n - 1; i >= 0; i--) {
      if(list[i].e <= list[k].e) {
        P += prob->sjob[list[i].j]->p;
        C = max(C, list[i].s + P);
        if(C > list[k].e) {
          xfree(Ci);
          return(SIPS_INFEASIBLE);
        }
      }
      Ci[i] = C;
    }

    H = - LARGE_INTEGER;
    for(i = 0; i < prob->n; i++) {
      if(list[i].e <= list[k].e) {
	H = max(H, list[i].s + P);
	P -= prob->sjob[list[i].j]->p;
      } else {
	rnew = w[list[i].j].s - prob->sjob[list[i].j]->p;
	if(list[i].s + P + prob->sjob[list[i].j]->p > list[k].e) {
	  rnew = max(rnew, Ci[i]);
	}
	if(H + prob->sjob[list[i].j]->p > list[k].e) {
	  rnew = max(rnew, C);
	}
	rnew += prob->sjob[list[i].j]->p;
	if(rnew > w[list[i].j].s) {
#ifdef DEBUG
	  printf("+No%d: [%d %d]=>[%d %d]\n",
		 prob->sjob[list[i].j]->rno,
		 w[list[i].j].s, w[list[i].j].e,
		 rnew, w[list[i].j].e);
#endif /* DEBUG */
	  updated = SIPS_TRUE;
	  w[list[i].j].s = rnew;
	  if(w[list[i].j].s > w[list[i].j].e) {
	    xfree(Ci);
	    return(SIPS_INFEASIBLE);
	  }
	}
      }
    }
  }

  if(is_true(updated)) {
    xfree(*r);
    *r = NULL;
  }
  xfree(Ci);

  return(SIPS_NORMAL);
}

/*
 * _edge_finding_tail(prob, w, d)
 *   updates tails by applying Edge-Finding procedure in
 *   Baptiste, Le Pape and Nuijten (2001).
 *       w: list of job time window
 *       d: list of jobs sorted in the nonincreasing order of tails.
 *
 */
int _edge_finding_tail(sips *prob, _window_t *w, _list_t **d)
{
  int i, k;
  int P, C, H;
  int dnew;
  char updated;
  int *Ci;
  _list_t *list;

  if(*d == NULL) {
    *d = list = (_list_t *) xmalloc(prob->n*sizeof(_list_t));
    for(i = 0; i < prob->n; i++) {
      list[i].j = i;
      list[i].s = w[i].s - prob->sjob[i]->p;
      list[i].e = w[i].e;
    }
    qsort((void *) list, prob->n, sizeof(_list_t), _compare_list_by_e);
  } else {
    list = *d;
  }

  Ci = (int *) xcalloc(prob->n, sizeof(int));

  updated = SIPS_FALSE;
  for(k = prob->n - 1; k >= 0; k--) {
    P = 0;
    C = LARGE_INTEGER;
    for(i = 0; i < prob->n; i++) {
      if(list[i].s >= list[k].s) {
	P += prob->sjob[list[i].j]->p;
	C = min(C, list[i].e - P);
	if(C < list[k].s) {
	  xfree(Ci);
	  return(SIPS_INFEASIBLE);
	}
      }
      Ci[i] = C;
    }

    H = LARGE_INTEGER;
    for(i = prob->n - 1; i >= 0; i--) {
      if(list[i].s >= list[k].s) {
	H = min(H, list[i].e - P);
	P -= prob->sjob[list[i].j]->p;
      } else {
	dnew = w[list[i].j].e;
	if(list[i].e - P - prob->sjob[list[i].j]->p < list[k].s) {
	  dnew = min(dnew, Ci[i]);
	}
	if(H - prob->sjob[list[i].j]->p < list[k].s) {
	  dnew = min(dnew, C);
	}
	if(dnew < w[list[i].j].e) {
#ifdef DEBUG
	  printf("+No%d: [%d %d]=>[%d %d]\n",
		 prob->sjob[list[i].j]->rno,
		 w[list[i].j].s, w[list[i].j].e,
		 w[list[i].j].s, dnew);
#endif /* DEBUG */
	  updated = SIPS_TRUE;
	  w[list[i].j].e = dnew;
	  if(w[list[i].j].s > w[list[i].j].e) {
	    xfree(Ci);
	    return(SIPS_INFEASIBLE);
	  }
	}
      }
    }
  }

  if(is_true(updated)) {
    xfree(*d);
    *d = NULL;
  }
  xfree(Ci);

  return(SIPS_NORMAL);
}
#endif /* NLOGN_EDGE_FINDING */

/*
 * _compare_list_by_s(a, b)
 *   compares elements of a list by the key "s".
 *   Ties are broken by job numbers.
 *
 */
int _compare_list_by_s(const void *a, const void *b)
{
  _list_t *x = (_list_t *) a;
  _list_t *y = (_list_t *) b;

  if(x->s > y->s) {
    return(1);
  } else if(x->s < y->s) {
    return(-1);
  } else if(x->e > y->e) {
    return(1);
  } else if(x->e < y->e) {
    return(-1);
  } else if(x->j > y->j) {
    return(1);
  } else if(x->j < y->j) {
    return(-1);
  }

  return(0);
}

/*
 * _compare_list_by_e(a, b)
 *   compares elements of a list by the key "e".
 *   Ties are broken by reverse order of job numbers.
 *
 */
int _compare_list_by_e(const void *a, const void *b)
{
  _list_t *x = (_list_t *) a;
  _list_t *y = (_list_t *) b;

  if(x->e > y->e) {
    return(1);
  } else if(x->e < y->e) {
    return(-1);
  } else if(x->s < y->s) {
    return(1);
  } else if(x->s > y->s) {
    return(-1);
  } else if(x->j < y->j) {
    return(1);
  } else if(x->j > y->j) {
    return(-1);
  }

  return(0);
}

/*
 * _compare_list2(a, b)
 *   compares elements of a list2 by their keys.
 *   Ties are broken by job numbers.
 *
 */
int _compare_list2(const void *a, const void *b)
{
  _list2_t *x = (_list2_t *) a;
  _list2_t *y = (_list2_t *) b;

  if(x->key > y->key) {
    return(1);
  } else if(x->key < y->key) {
    return(-1);
  } else if(x->j > y->j) {
    return(1);
  } else if(x->j < y->j) {
    return(-1);
  }

  return(0);
}

#ifdef NLOGN_EDGE_FINDING
#ifdef DEBUG
void _print_bt(sips *prob, _btree_t **bt)
{
  int i, j, k;
  int s;
  int lv;

  for(lv = 0, i = prob->n; i > 0; i >>= 1, lv++);

  for(i = 0; i < lv; i++) {
    s = (1<<(lv - i - 1)) - 1;
    for(k = 0; k < 6*s/2; k++) {
      printf(" ");
    }
    printf("no%-5d", prob->sjob[bt[(1<<i) - 1]->j]->rno);

    for(j = (1<<i); j < (2<<i) - 1 && j < prob->n; j++) {
      for(k = 0; k < 6*s; k++) {
	printf(" ");
      }
      printf("no%-5d", prob->sjob[bt[j]->j]->rno);
    }
    printf("\n");

    for(k = 0; k < 6*s/2; k++) {
      printf(" ");
    }
    printf("p%-5d", bt[(1<<i) - 1]->pplus);
    for(j = (1<<i); j < (2<<i) - 1 && j < prob->n; j++) {
      for(k = 0; k < 6*s; k++) {
	printf(" ");
      }
      printf("p%-5d", bt[j]->pplus);
    }
    printf("\n");

    for(k = 0; k < 6*s/2; k++) {
      printf(" ");
    }
    printf("q%-5d", bt[(1<<i) - 1]->q);
    for(j = (1<<i); j < (2<<i) - 1 && j < prob->n; j++) {
      for(k = 0; k < 6*s; k++) {
	printf(" ");
      }
      printf("q%-5d", bt[j]->q);
    }
    printf("\n");

    for(k = 0; k < 6*s/2; k++) {
      printf(" ");
    }
    printf("r%-5d", bt[(1<<i) - 1]->r);
    for(j = (1<<i); j < (2<<i) - 1 && j < prob->n; j++) {
      for(k = 0; k < 6*s; k++) {
	printf(" ");
      }
      printf("r%-5d", bt[j]->r);
    }
    printf("\n");

    for(k = 0; k < 6*s/2; k++) {
      printf(" ");
    }
    printf("s%-5d", bt[(1<<i) - 1]->sigma);
    for(j = (1<<i); j < (2<<i) - 1 && j < prob->n; j++) {
      for(k = 0; k < 6*s; k++) {
	printf(" ");
      }
      printf("s%-5d", bt[j]->sigma);
    }
    printf("\n");

    for(k = 0; k < 6*s/2; k++) {
      printf(" ");
    }
    if(bt[(1<<i) - 1]->ksi < - LARGE_INTEGER/2) {
      printf("k-Inf ");
    } else {
      printf("k%-5d", bt[(1<<i) - 1]->ksi);
    }
    for(j = (1<<i); j < (2<<i) - 1 && j < prob->n; j++) {
      for(k = 0; k < 6*s; k++) {
	printf(" ");
      }
      if(bt[j]->ksi < - LARGE_INTEGER/2) {
	printf("k-Inf ");
      } else {
	printf("k%-5d", bt[j]->ksi);
      }
    }
    printf("\n");
  }
}
#endif /* DEBUG */
#endif /* NLOGN_EDGE_FINDING */
