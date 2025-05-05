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
 *  $Id: objective.c,v 1.15 2015/01/12 06:50:31 tanaka Rel $
 *  $Revision: 1.15 $
 *  $Date: 2015/01/12 06:50:31 $
 *  $Author: tanaka $
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "define.h"
#include "sips_common.h"
#include "memory.h"
#include "objective.h"

/*
 * objective_initialize(prob)
 *   initializes the job cost function.
 *   It is called only once after the instance is loaded from a data file.
 *
 */
void objective_initialize(sips *prob)
{
  int t;
  int i, j;
  int np;
  int tard;
  int sign, scount;
#if 1
  int d, dmin, toff;
#else /* 1 */
  int d, dmin;
#endif /* 1 */
  cost_t f, fpr, slope, fmin;
  char ptype;
#if 1
  _point_t *p, *p2;
#else /* 1 */
  _point_t *p;
#endif /* 1 */

  if(prob->cfunc == NULL && prob->cfuncl == NULL) {
    for(i = 0; i < prob->n; i++) {
      for(t = 0; t < prob->job[i].r + prob->job[i].p; t++) {
	prob->job[i].f[t] = LARGE_COST;
      }

      tard = prob->T - prob->job[i].d;
      fmin = prob->job[i].ew*(cost_t) max(-tard, 0)
	+ prob->job[i].tw*(cost_t) max(tard, 0);
      tard = prob->job[i].r + prob->job[i].p - prob->job[i].d;
      f = prob->job[i].ew*(cost_t) max(-tard, 0)
	+ prob->job[i].tw*(cost_t) max(tard, 0);
      if(fmin > f) {
	fmin = f;
      }
      if(fobj_greater(fmin, ZERO_COST)) {
	fmin = ZERO_COST;
      }
      prob->off += fmin;

      tard = prob->job[i].r + prob->job[i].p - prob->job[i].d;
      for(t = prob->job[i].r + prob->job[i].p; t <= prob->T; t++, tard++) {
	prob->job[i].f[t]
	  = prob->job[i].ew*(cost_t) max(-tard, 0)
	  + prob->job[i].tw*(cost_t) max(tard, 0) - fmin;
      }

      t = prob->job[i].r + prob->job[i].p;
      if(prob->job[i].d <= t || prob->T <= prob->job[i].d) {
	prob->job[i].func = p = (_point_t *) xmalloc(2*sizeof(_point_t));

	p[0].t = t;
	p[0].v = prob->job[i].f[t];
	p[0].s = 0;
	p[0].prev = &(p[1]);
	p[0].next = &(p[1]);

	p[1].t = prob->T;
	p[1].v = prob->job[i].f[prob->T];
	p[1].s = 0;
	p[1].prev = &(p[0]);
	p[1].next = NULL;
      } else {
	prob->job[i].func = p = (_point_t *) xmalloc(3*sizeof(_point_t));

	p[0].t = t;
	p[0].v = prob->job[i].f[t];
	p[0].s = 0;
	p[0].prev = &(p[2]);
	p[0].next = &(p[1]);

	p[1].t = prob->job[i].d;
	p[1].v = prob->job[i].f[prob->job[i].d];
	p[1].s = 0;
	p[1].prev = &(p[0]);
	p[1].next = &(p[2]);

	p[2].t = prob->T;
	p[2].v = prob->job[i].f[prob->T];
	p[2].s = 0;
	p[2].prev = &(p[1]);
	p[2].next = NULL;
      }
    }
  } else {
    prob->off = ZERO_COST;
    ptype = SIPS_PROB_REGULAR | SIPS_PROB_VSHAPE;
    dmin = prob->T;
    for(i = 0; i < prob->n; i++) {
      for(t = 0; t < prob->job[i].r + prob->job[i].p; t++) {
	prob->job[i].f[t] = LARGE_COST;
      }

      prob->job[i].func = p = (_point_t *) xmalloc(prob->T*sizeof(_point_t));

      t = prob->job[i].r + prob->job[i].p;
      if(prob->cfunc != NULL) {
	fpr = prob->job[i].f[t] = (prob->cfunc)(i, t + prob->Toff);
      } else {
	fpr = prob->job[i].f[t] = (prob->cfuncl)(prob, i, t + prob->Toff);
      }
      fmin = fpr;

      p[0].t = t;
      p[0].v = fpr;
      p[0].s = 0;
      np = 1;
      d = 0;
      sign = 0;
      scount = 0;
      if(t < prob->T) {
	t++;
	if(prob->cfunc != NULL) {
	  f = prob->job[i].f[t] = (prob->cfunc)(i, t + prob->Toff);
	} else {
	  f = prob->job[i].f[t] = (prob->cfuncl)(prob, i, t + prob->Toff);
	}
	if(fmin > f) {
	  fmin = f;
	  d = t;
	}

	slope = f - fpr;
	if(slope < 0) {
	  scount++;
	  sign = -1;
	} else if(slope > 0) {
	  scount++;
	  sign = 1;
	}
	fpr = f;
	for(t++; t <= prob->T; fpr = f, t++) {
	  if(prob->cfunc != NULL) {
	    f = prob->job[i].f[t] = (prob->cfunc)(i, t + prob->Toff);
	  } else {
	    f = prob->job[i].f[t] = (prob->cfuncl)(prob, i, t + prob->Toff);
	  }
	  if(fmin > f) {
	    fmin = f;
	    d = t;
	  }
	  if(fobj_lesser(f, fpr) && sign >= 0) {
	    scount++;
	    sign = -1;
	  } else if(fobj_greater(f, fpr) && sign <= 0) {
	    scount++;
	    sign = 1;
	  }

	  if(!fobj_equal(f - slope, fpr)) {
	    p[np].t = t - 1;
	    p[np].v = fpr;
	    p[np++].s = 0;
	    slope = f - fpr;
	  }
	}

	p[np].t = prob->T;
	p[np].v = f;
	p[np++].s = 0;
      }

      dmin = min(d, dmin);

      prob->job[i].func = p = (_point_t *) xrealloc(p, np*sizeof(_point_t));

      prob->off += fmin;
      for(j = 0; j < np; j++) {
	p[j].v -= fmin;
      }
      for(t = prob->job[i].r + prob->job[i].p; t <= prob->T; t++) {
	prob->job[i].f[t] -= fmin;
      }

      if(scount > 2 || (scount == 2 && sign < 0)) {
	ptype &= ~(SIPS_PROB_REGULAR | SIPS_PROB_VSHAPE);
      } else if((scount == 2 && sign > 0) || sign < 0) {
	ptype &= ~SIPS_PROB_REGULAR;
      }

      if(np == 1) {
	p[0].next = NULL;
	p[0].prev = &(p[0]);
      } else {
	p[0].next = &(p[1]);
	p[0].prev = &(p[np - 1]);
	for(j = 1; j < np - 1; j++) {
	  p[j].next = &(p[j + 1]);
	  p[j].prev = &(p[j - 1]);
	}
	p[np - 1].prev = &(p[np - 2]);
	p[np - 1].next = NULL;
	p[0].prev = &(p[np - 1]);
      }
    }

    if(ptype & SIPS_PROB_REGULAR) {
      prob->param->ptype |= SIPS_PROB_REGULAR;
    } else if(ptype & SIPS_PROB_VSHAPE) {
      prob->param->ptype |= SIPS_PROB_VSHAPE;
#if 1
      if(dmin - prob->psum > 0) {
	toff = dmin - prob->psum;
	prob->rmin = max(prob->rmin - toff, 0);
	prob->rmax = max(prob->rmax - toff, 0);
	prob->T -= toff;
	prob->Toff += toff;
	for(i = 0; i < prob->n; i++) {
	  if(prob->job[i].r < toff) {
	    prob->job[i].r = 0;

	    for(p = (prob->job[i].func)->next; p != NULL; p = p->next) {
	      if(p->t >= prob->job[i].p + toff) {
		break;
	      }
	    }

	    if(p->t == prob->job[i].p + toff) {
	      for(p2 = prob->job[i].func; p != NULL;
		  p = p->next, p2 = p2->next) {
		p2->v = p->v;
		p2->t = p->t - toff;
	      }
	      p2->prev->next = NULL;
	      (prob->job[i].func)->prev = p2->prev;
	    } else {
	      (prob->job[i].func)->t = prob->job[i].p;
	      (prob->job[i].func)->v = prob->job[i].f[prob->job[i].p + toff];
	      for(p2 = (prob->job[i].func)->next; p != NULL;
		  p = p->next, p2 = p2->next) {
		p2->v = p->v;
		p2->t = p->t - toff;
	      }
	      if(p2 != NULL) {
		p2->prev->next = NULL;
		(prob->job[i].func)->prev = p2->prev;
	      }	      
	    }
	  } else {
	    prob->job[i].r -= toff;
	    for(p = prob->job[i].func; p != NULL; p = p->next) {
	      p->t -= toff;
	    }
	  }

	  memmove((void *) prob->job[i].f, (void *) (prob->job[i].f + toff),
		  (prob->T + 1)*sizeof(cost_t));
	  for(t = 0; t < prob->job[i].r + prob->job[i].p; t++) {
	    prob->job[i].f[t] = LARGE_COST;
	  }
	}
      }
#endif /* 1 */
    }
  }

  memset((void *) prob->job[prob->n].f, 0, (prob->T + 1)*sizeof(cost_t));

  for(i = prob->n; i <= prob->tn + 1; i++) {
    prob->job[i].func = NULL;
  }
}

/*
 * objective(sol)
 *   sets and returns the objective function value of sol.
 *
 */
void objective(_solution_t *sol)
{
  int i;

  sol->f = ZERO_COST;
  for(i = 0; i < sol->n; i++) {
    sol->f += sol->job[i]->f[sol->c[i]];
  }
}
