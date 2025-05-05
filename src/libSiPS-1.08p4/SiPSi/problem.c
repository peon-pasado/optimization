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
 *  $Id: problem.c,v 1.36 2015/10/27 22:22:38 tanaka Rel $
 *  $Revision: 1.36 $
 *  $Date: 2015/10/27 22:22:38 $
 *  $Author: tanaka $
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "define.h"
#include "sips_common.h"
#include "job_sort.h"
#include "memory.h"
#include "objective.h"
#include "prob_common.h"
#include "problem.h"

#ifndef _MSC_VER
#define sscanf_s sscanf
#endif /* _MSC_VER */

/*
 * SiPS_read_problem(prob, fname)
 *   reads an instance from the data file.
 *     fname: file name (read from the standard input if NULL)
 *
 */
int SiPS_read_problem(sips *prob, char *fname)
{
  int i, j;
  int i1, i2, i3;
#ifdef COST_REAL
  double i4, i5;
#else /* COST_REAL */
  int i4, i5;
#endif /* COST_REAL */
  char buf[MAXBUFLEN];
  FILE *fp;

  null_check(prob);

  if(fname == NULL) {
    fp = stdin;
#ifdef _MSC_VER
  } else if(fopen_s(&fp, fname, "r") != 0) {
#else /* _MSC_VER */
  } else if((fp = fopen(fname, "r")) == NULL) {
#endif /* _MSC_VER */
    fprintf(stderr, "Cannot open file: %s.\n", fname);
    return(SIPS_FAIL);
  }

  i = 0;
  while(fgets(buf, MAXBUFLEN, fp) != NULL) {
    remove_comments(buf);
    j = sscanf_s(buf, "%d", &i1);
    if(j == 1 && i1 > 0) {
      if(problem_set_size(prob, i1) == SIPS_FAIL) {
	fclose(fp);
	return(SIPS_FAIL);
      }

      break;
    }
  }

  if(prob->n == 0) {
    return(SIPS_FAIL);
  }

  while(fgets(buf, MAXBUFLEN, fp) != NULL) {
    remove_comments(buf);
#ifdef COST_REAL
    j = sscanf_s(buf, "%d %d %d %lf %lf", &i1, &i2, &i3, &i4, &i5);
#else /* COST_REAL */
    j = sscanf_s(buf, "%d %d %d %d %d", &i1, &i2, &i3, &i4, &i5);
#endif /* COST_REAL */
    if(j >= 3) {
      if(i == prob->n) {
	i++;
	break;
      }

      switch(j) {
      case 5:
      default:
	prob->job[i].p = i1;
	prob->job[i].rr = i2;
	prob->job[i].rd = i3;
	prob->job[i].ew = (cost_t) i4;
	prob->job[i].tw = (cost_t) i5;

	break;

      case 4:
	prob->job[i].p = i1;
	prob->job[i].rr = i2;
	prob->job[i].rd = i3;
	prob->job[i].ew = ZERO_COST;
	prob->job[i].tw = (cost_t) i4;

	break;

      case 3:
	prob->job[i].p = i1;
	prob->job[i].rr = i2;
	prob->job[i].rd = i3;
	prob->job[i].ew = ZERO_COST;
	prob->job[i].tw = (cost_t) 1;

	break;
      }

      i++;
    }
  }

  fclose(fp);

  if(i != prob->n) {
    return(SIPS_FAIL);
  }

  return(SIPS_OK);
}

/*
 * SiPS_TWT_set_job(prob, no, name, p, r, d, w)
 *   specifies data of job n for the total weighted tardiness problem.
 *      no: job number (indexed from 0)
 *    name: job name (optional)
 *       p: processing time
 *       r: release date
 *       d: duedate
 *       w: tardiness weight
 *
 */
int SiPS_TWT_set_job(sips *prob, int no, char *name, int p, int r, int d,
		     cost_t w)
{
  int ret;
  size_t length;

  null_check(prob);

  if(no < 0) {
    return(SIPS_FAIL);
  }

  if(no >= prob->n) {
    ret = problem_set_size(prob, no + 1);
    if(ret != SIPS_OK) {
      return(ret);
    }
  }

  if(name != NULL) {
    length = min(strlen(name), MAXNAMELEN) + 1;
    prob->job[no].name
      = (char *) xrealloc((void *) prob->job[no].name, length);
    memcpy((void *) prob->job[no].name, (void *) name, length - 1);
    *(prob->job[no].name + length - 1) = '\0';
  }

  prob->job[no].p = p;
  prob->job[no].rd = d;
  prob->job[no].rr = r;
  prob->job[no].ew = 0;
  prob->job[no].tw = w;

  return(SIPS_OK);
}

/*
 * SiPS_TWT_set_jobs(prob, n, no, name, p, r, d, w)
 *   specifies data of jobs for the total weighted tardiness problem.
 *       n: number of specified jobs
 *      no: array of job numbers
 *    name: array of job names (optional)
 *       p: array of processing times
 *       r: array of release date
 *       d: array of duedates
 *       w: array of tardiness weights
 *
 */
int SiPS_TWT_set_jobs(sips *prob, int n, int *no, char **name, int *p,
		      int *r, int *d, cost_t *w)
{
  int i;
  int max_number;
  size_t length;

  null_check(prob);

  if(n <= 0 || no == NULL) {
    return(SIPS_FAIL);
  }
  if(name == NULL && p == NULL && r == NULL && d == NULL && w == NULL) {
    return(SIPS_FAIL);
  }

  max_number = 0;
  for(i = 0; i < n; i++) {
    if(no[i] < 0) {
      return(SIPS_FAIL);
    }
    max_number = max(no[i], max_number);
  }

  problem_set_size(prob, max(prob->n, max_number + 1));

  if(name != NULL) {
    for(i = 0; i < n; i++) {
      length = min(strlen(name[i]), MAXNAMELEN) + 1;
      prob->job[no[i]].name
	= (char *) xrealloc((void *) prob->job[no[i]].name, length);
      memcpy((void *) prob->job[no[i]].name, (void *) name[i], length - 1);
      *(prob->job[no[i]].name + length - 1) = '\0';
    }
  }

  if(p != NULL) {
    for(i = 0; i < n; i++) {
      if(p[i] <= 0) {
	return(SIPS_FAIL);
      }
      prob->job[no[i]].p = p[i];
    }
  }

  if(d != NULL) {
    for(i = 0; i < n; i++) {
      prob->job[no[i]].rd = d[i];
    }
  }

  if(r != NULL) {
    for(i = 0; i < n; i++) {
      prob->job[no[i]].rr = r[i];
    }
  }

  if(w != NULL) {
    for(i = 0; i < n; i++) {
      prob->job[no[i]].ew = 0;
      prob->job[no[i]].tw = w[i];
    }
  }

  return(SIPS_OK);
}

/*
 * SiPS_TWET_set_job(prob, no, name, p, r, d, ew, tw)
 *   specifies data of job n in the total weighted earliness-tardiness
 *   problem.
 *      no: job number (indexed from 0)
 *    name: job name (optional)
 *       p: processing time
 *       r: release date
 *       d: duedate
 *      ew: earliness weight
 *      tw: tardiness weight
 *
 */
int SiPS_TWET_set_job(sips *prob, int no, char *name, int p, int r, int d,
		      cost_t ew, cost_t tw)
{
  int ret;
  size_t length;

  null_check(prob);

  if(no < 0) {
    return(SIPS_FAIL);
  }

  if(no >= prob->n) {
    ret = problem_set_size(prob, no + 1);
    if(ret != SIPS_OK) {
      return(ret);
    }
  }

  if(name != NULL) {
    length = min(strlen(name), MAXNAMELEN) + 1;
    prob->job[no].name
      = (char *) xrealloc((void *) prob->job[no].name, length);
    memcpy((void *) prob->job[no].name, (void *) name, length - 1);
    *(prob->job[no].name + length - 1) = '\0';
  }

  prob->job[no].p = p;
  prob->job[no].rd = d;
  prob->job[no].rr = r;
  prob->job[no].ew = ew;
  prob->job[no].tw = tw;

  return(SIPS_OK);
}

/*
 * SiPS_TWET_set_jobs(prob, n, no, name, p, r, d, ew, tw)
 *   specifies data of jobs for the total weighted earliness-tardiness
 *   problem.
 *       n: number of specified jobs
 *      no: array of job numbers
 *    name: array of job names (optional)
 *       p: array of processing times
 *       r: array of release dates
 *       d: array of duedates
 *      ew: array of earliness weights
 *      tw: array of tardiness weights
 *
 */
int SiPS_TWET_set_jobs(sips *prob, int n, int *no, char **name, int *p, int *r,
		       int *d, cost_t *ew, cost_t *tw)
{
  int i;
  int max_number;
  size_t length;

  null_check(prob);

  if(n <= 0 || no == NULL) {
    return(SIPS_FAIL);
  }
  if(name == NULL && p == NULL && r == NULL && d == NULL
     && ew == NULL && tw == NULL) {
    return(SIPS_FAIL);
  }

  max_number = 0;
  for(i = 0; i < n; i++) {
    if(no[i] < 0) {
      return(SIPS_FAIL);
    }
    max_number = max(no[i], max_number);
  }

  problem_set_size(prob, max(prob->n, max_number + 1));

  if(name != NULL) {
    for(i = 0; i < n; i++) {
      length = min(strlen(name[i]), MAXNAMELEN) + 1;
      prob->job[no[i]].name
	= (char *) xrealloc((void *) prob->job[no[i]].name, length);
      memcpy((void *) prob->job[no[i]].name, (void *) name[i], length - 1);
      *(prob->job[no[i]].name + length - 1) = '\0';
    }
  }

  if(p != NULL) {
    for(i = 0; i < n; i++) {
      if(p[i] <= 0) {
	return(SIPS_FAIL);
      }
      prob->job[no[i]].p = p[i];
    }
  }

  if(d != NULL) {
    for(i = 0; i < n; i++) {
      prob->job[no[i]].rd = d[i];
    }
  }

  if(r != NULL) {
    for(i = 0; i < n; i++) {
      prob->job[no[i]].rr = r[i];
    }
  }

  if(ew != NULL) {
    for(i = 0; i < n; i++) {
      prob->job[no[i]].ew = ew[i];
    }
  }

  if(tw != NULL) {
    for(i = 0; i < n; i++) {
      prob->job[no[i]].tw = tw[i];
    }
  }

  return(SIPS_OK);
}

/*
 * SiPS_get_job(prob, no, p, r, d, ew, tw)
 *   retrieve data of a job.
 *      no: job number (indexed from 0)
 *       p: processing time
 *       r: release date
 *       d: duedate
 *      ew: earliness weight
 *      tw: tardiness weight
 *
 */
int SiPS_get_job(sips *prob, int no, int *p, int *r, int *d,
		 cost_t *ew, cost_t *tw)
{
  null_check(prob);

  if(no < 0 || no >= prob->n) {
    return(SIPS_FAIL);
  }
  if(p == NULL && r == NULL && d == NULL && ew == NULL && tw == NULL) {
    return(SIPS_FAIL);
  }

  if(p != NULL) {
    *p = prob->job[no].p;
  }

  if(r != NULL) {
    *r = prob->job[no].rr;
  }

  if(d != NULL) {
    *d = prob->job[no].rd;
  }

  if(ew != NULL) {
    *ew = prob->job[no].ew;
  }

  if(tw != NULL) {
    *tw = prob->job[no].tw;
  }

  return(SIPS_OK);
}

/*
 * SiPS_get_jobs(prob, n, no, p, r, d, ew, tw)
 *   retrieve job data.
 *       n: number of jobs to be retrieved
 *      no: array of job numbers
 *       p: array of processing times
 *       r: array of release dates
 *       d: array of duedates
 *      ew: array of earliness weights
 *      tw: array of tardiness weights
 *
 */
int SiPS_get_jobs(sips *prob, int n, int *no, int *p, int *r, int *d,
		  cost_t *ew, cost_t *tw)
{
  int i;

  null_check(prob);

  if(n <= 0 || n > prob->n || no == NULL) {
    return(SIPS_FAIL);
  }
  if(p == NULL && d == NULL && ew == NULL && tw == NULL) {
    return(SIPS_FAIL);
  }

  if(p != NULL) {
    for(i = 0; i < n; i++) {
      p[i] = prob->job[no[i]].p;
    }
  }

  if(r != NULL) {
    for(i = 0; i < n; i++) {
      r[i] = prob->job[no[i]].rr;
    }
  }

  if(d != NULL) {
    for(i = 0; i < n; i++) {
      d[i] = prob->job[no[i]].rd;
    }
  }

  if(ew != NULL) {
    for(i = 0; i < n; i++) {
      ew[i] = prob->job[no[i]].ew;
    }
  }

  if(tw != NULL) {
    for(i = 0; i < n; i++) {
      tw[i] = prob->job[no[i]].tw;
    }
  }

  return(SIPS_OK);
}

/*
 * SiPS_set_horizon(prob, Tmax)
 *   specifies the end of the scheduling horizon.
 *       Tmax: end of the scheduling horizon
 */
int SiPS_set_horizon(sips *prob, int Tmax)
{
  null_check(prob);

  prob->usrT = Tmax;

  return(SIPS_OK);
}

/*
 * problem_set_size(prob, n)
 *   sets the number of jobs.
 *       n: number of jobs
 *
 */
int problem_set_size(sips *prob, int n)
{
  int i, k;
  size_t length;

  null_check(prob);

  if(n < 0) {
    return(SIPS_FAIL);
  } else if(n == 0) {
    problem_free_jobs(prob);
    prob->n = 0;
    return(SIPS_OK);
  } else if(n == prob->n && prob->dn == 0) {
    return(SIPS_OK);
  }

  if(prob->job != NULL) {
    for(i = min(n, prob->n); i <= prob->tn + 1; i++) {
      xfree(prob->job[i].name);
      prob->job[i].name = NULL;
    }

    prob->job = (_job_t *) xrealloc((void *) prob->job,
				    (n + prob->dn + 2)*sizeof(_job_t));
    k = prob->n;
  } else {
    prob->job = (_job_t *) xcalloc(n + prob->dn + 2, sizeof(_job_t));
    k = 0;
  }

  for(i = k; i < n; i++) {
    prob->job[i].p = 0;
    prob->job[i].rno = prob->job[i].no = i;
    prob->job[i].sno = -1;
    prob->job[i].rd = prob->job[i].d = 0;
    prob->job[i].rr = prob->job[i].r = 0;
    prob->job[i].ew = prob->job[i].tw = 0;
    prob->job[i].name = NULL;
    prob->job[i].func = NULL;
    prob->job[i].u = 0.0;
  }

  prob->n = n;
  prob->tn = n + prob->dn;

  for(i = prob->n; i < prob->tn; i++) {
    prob->job[i].p = i - prob->n + 1;
    prob->job[i].rno = prob->job[i].no = i;
    prob->job[i].sno = -1;
    prob->job[i].rd = prob->job[i].d = 0;
    prob->job[i].rr = prob->job[i].r = 0;
    prob->job[i].ew = prob->job[i].tw = 0;
    length = ndigits(i - prob->n) + 2;
    prob->job[i].name = (char *) xmalloc(length);
#ifdef _MSC_VER
    _snprintf_s(prob->job[i].name, length, _TRUNCATE, "d%d", i - prob->n);
#else /* _MSC_VER */
    snprintf(prob->job[i].name, length, "d%d", i - prob->n);
    *(prob->job[i].name + length - 1) = '\0';
#endif /* _MSC_VER */
    prob->job[i].func = NULL;
    prob->job[i].u = 0.0;
  }

  n = prob->tn;

  prob->job[n].p = 0;
  prob->job[n].rno = prob->job[n].no = n;
  prob->job[n].sno = -1;
  prob->job[n].rd = prob->job[n].d = 0;
  prob->job[n].rr = prob->job[n].r = 0;
  prob->job[n].ew = prob->job[n].tw = 0;
  prob->job[n].name = (char *) xmalloc(2);
  prob->job[n].name[0] = 's';
  prob->job[n].name[1] = '\0';
  prob->job[n].func = NULL;
  prob->job[n].u = 0.0;

  prob->job[n + 1].p = 0;
  prob->job[n + 1].rno = prob->job[n + 1].no = n + 1;
  prob->job[n + 1].sno = -1;
  prob->job[n + 1].rd = prob->job[n + 1].d = 0;
  prob->job[n + 1].rr = prob->job[n + 1].r = 0;
  prob->job[n + 1].ew = prob->job[n + 1].tw = 0;
  prob->job[n + 1].name = (char *) xmalloc(2);
  prob->job[n + 1].name[0] = 'n';
  prob->job[n + 1].name[1] = '\0';
  prob->job[n + 1].func = NULL;
  prob->job[n + 1].u = 0.0;

  return(SIPS_OK);
}

/*
 * problem_set_probdata(prob)
 *   computes and sets the following constants.
 *    - total processing time
 *    - maximum and minimum processing times
 *    - maximum and minimum duedates
 *    - maximum and minimum release dates
 *    - scheduling horizon
 *    - objective function values
 *    - offset for objective function value
 *
 */
int problem_set_probdata(sips *prob)
{
  int i;
  int dmax;
  int rtmp;
  cost_t v;

  null_check(prob);

  prob->psum = 0;
  prob->pmin = LARGE_INTEGER;
  prob->pmax = 0;
  prob->rmin = LARGE_INTEGER;
  prob->rmax = 0;
  for(i = 0; i < prob->n; i++) {
    prob->psum += prob->job[i].p;
    prob->pmax = max(prob->pmax, prob->job[i].p);
    prob->pmin = min(prob->pmin, prob->job[i].p);
    rtmp = max(prob->job[i].rr, 0);
    prob->rmax = max(prob->rmax, rtmp);
    prob->rmin = min(prob->rmin, rtmp);
  }

  prob->param->ptype = SIPS_PROB_IDLETIME;

  if(prob->cfunc == NULL && prob->cfuncl == NULL) {
    for(i = 0; i < prob->n && obj_equal(prob->job[i].ew, ZERO_COST); i++);
    if(i == prob->n) {
      prob->param->ptype |= SIPS_PROB_REGULAR;
      if(prob->rmin == prob->rmax) {
	prob->param->ptype ^= SIPS_PROB_IDLETIME;
      }
    } else {
      prob->param->ptype |= SIPS_PROB_VSHAPE;
    }
  }

  prob->Toff = prob->rmin;
  for(i = 0; i < prob->n; i++) {
    prob->job[i].r = max(prob->job[i].rr, 0) - prob->Toff;
    prob->job[i].d = prob->job[i].rd - prob->Toff;
  }
  prob->rmax -= prob->Toff;
  prob->rmin = 0;

  prob->off = ZERO_COST;
  prob->dmin = LARGE_INTEGER;
  prob->dmax = dmax = - LARGE_INTEGER;

  if(prob->cfunc == NULL && prob->cfuncl == NULL) {
    for(i = 0; i < prob->n; i++) {
      dmax = max(dmax, prob->job[i].d);
      if(prob->job[i].d < prob->job[i].r + prob->job[i].p) {
	v = prob->job[i].tw
	  *(cost_t) (prob->job[i].r + prob->job[i].p - prob->job[i].d);
	prob->off += v;
	prob->job[i].d = prob->job[i].r + prob->job[i].p;
      }
      prob->dmax = max(prob->dmax, prob->job[i].d);
      prob->dmin = min(prob->dmin, prob->job[i].d);
    }
  }

  if(prob->param->ptype & SIPS_PROB_IDLETIME) {
    if(prob->param->dummy == 0) {
      prob->dn = prob->pmax;
    } else {
      prob->dn = min(prob->param->dummy, prob->pmax);
    }
    problem_set_size(prob, prob->n);
    if(prob->cfunc == NULL && prob->cfuncl == NULL) {
      prob->T = max(dmax, prob->rmax) + prob->psum;
#if 1
      if((prob->param->ptype & SIPS_PROB_VSHAPE) && prob->dmin > prob->psum) {
	int t = prob->dmin - prob->psum;
	prob->Toff += t;
	prob->T -= t;
	prob->dmax -= t;
	prob->dmin = prob->psum;
	prob->rmax -= t;
	if(prob->rmax < 0) {
	  prob->rmax = 0;
	}
	prob->rmin -= t;
	if(prob->rmin < 0) {
	  prob->rmin = 0;
	}

	for(i = 0; i < prob->n; i++) {
	  prob->job[i].d -= t;
	  if(prob->job[i].r < t) {
	    prob->job[i].r = 0;
	  } else {
	    prob->job[i].r -= t;
	  }
	}
      }
#endif /* 1 */
    } else {
#if 1
      prob->T = max(prob->rmax + prob->psum, prob->usrT - prob->Toff);
#else /* 0 */
      prob->T = prob->usrT - prob->Toff;
#endif /* 0 */
    }
  } else {
    prob->T = prob->psum;
    prob->dn = 1;
    problem_set_size(prob, prob->n);
    prob->dn = 0;
  }

  problem_free_objective(prob);
  prob->job[0].f
    = (cost_t *) xmalloc((prob->T + 1)*(prob->n + 1)*sizeof(cost_t));
  for(i = 1; i <= prob->n; i++) {
    prob->job[i].f = prob->job[i - 1].f + prob->T + 1;
  }
  for(i = prob->n + 1; i <= prob->tn + 1; i++) {
    prob->job[i].f = prob->job[prob->n].f;
  }
  objective_initialize(prob);

  prob->sjob = (_job_t **) xmalloc((prob->tn + 2)*sizeof(_job_t *));
  for(i = 0; i <= prob->tn + 1; i++) {
    prob->sjob[i] = &(prob->job[i]);
  }

  sort_jobs(prob->n, prob->sjob, 1); /* spt */
  for(i = 0; i < prob->n; i++) {
    prob->sjob[i]->no = i;
  }

  return(SIPS_OK);
}
