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
 *  $Id: job_sort.c,v 1.7 2013/05/28 13:27:09 tanaka Rel $
 *  $Revision: 1.7 $
 *  $Date: 2013/05/28 13:27:09 $
 *  $Author: tanaka $
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "define.h"
#include "sips_common.h"
#include "job_sort.h"

static int _job_compare_edd(const void *, const void *);
static int _job_compare_spt(const void *, const void *);
static int _job_compare_ldd(const void *, const void *);
static int _job_compare_lpt(const void *, const void *);
static int _job_compare_wspt(const void *, const void *);
static int _job_compare_wlpt(const void *, const void *);
#ifdef SIPSI
static int _job_compare_release(const void *, const void *);
#endif /* SIPSI */

/*
 * sort_jobs(n, job, type)
 *   sorts the job sequence by quick sort.
 *        n: number of jobs
 *      job: job sequence
 *     type: sort type
 *             0: EDD (+SPT)
 *             1: SPT (+EDD)
 *             2: LDD (+LPT)
 *             3: LPT (+LDD)
 *             4: WSPT (+EDD)
 *             5: WLPT (+LDD)
 *             6: release (+EDD)
 *
 */
void sort_jobs(int n, _job_t **job, unsigned char type)
{
  switch(type) {
  case 0:
  default:
    qsort((void *) job, n, sizeof(_job_t *), _job_compare_edd);
    break;
  case 1:
    qsort((void *) job, n, sizeof(_job_t *), _job_compare_spt);
    break;
  case 2:
    qsort((void *) job, n, sizeof(_job_t *), _job_compare_ldd);
    break;
  case 3:
    qsort((void *) job, n, sizeof(_job_t *), _job_compare_lpt);
    break;
  case 4:
    qsort((void *) job, n, sizeof(_job_t *), _job_compare_wspt);
    break;
  case 5:
    qsort((void *) job, n, sizeof(_job_t *), _job_compare_wlpt);
    break;
#ifdef SIPSI
  case 6:
    qsort((void *) job, n, sizeof(_job_t *), _job_compare_release);
    break;
#endif /* SIPSI */
  }
}

/*
 * _job_compare_edd(a, b)
 *   compares two jobs and returns which should be sequenced first (EDD).
 *        a: job x
 *        b: job y
 *
 */
int _job_compare_edd(const void *a, const void *b)
{
  _job_t *x = *((_job_t **) a);
  _job_t *y = *((_job_t **) b);

  if(x->d > y->d) {
    return(1);
  } else if(x->d < y->d) {
    return(-1);
  } else if(x->p > y->p) {
    return(1);
  } else if(x->p < y->p) {
    return(-1);
  } else if(x->tw > y->tw) {
    return(1);
  } else if(x->tw < y->tw) {
    return(-1);
  } else if(x->ew > y->ew) {
    return(1);
  } else if(x->ew < y->ew) {
    return(-1);
  } else if(x->no > y->no) {
    return(1);
  } else if(x->no < y->no) {
    return(-1);
  }

  return(0);
}

/*
 * _job_compare_spt(a, b)
 *   compares two jobs and returns which should be sequenced first (SPT).
 *        a: job x
 *        b: job y
 *
 */
int _job_compare_spt(const void *a, const void *b)
{
  _job_t *x = *((_job_t **) a);
  _job_t *y = *((_job_t **) b);

  if(x->p > y->p) {
    return(1);
  } else if(x->p < y->p) {
    return(-1);
  } else if(x->d > y->d) {
    return(1);
  } else if(x->d < y->d) {
    return(-1);
  } else if(x->tw > y->tw) {
    return(1);
  } else if(x->tw < y->tw) {
    return(-1);
  } else if(x->ew > y->ew) {
    return(1);
  } else if(x->ew < y->ew) {
    return(-1);
  } else if(x->no > y->no) {
    return(1);
  } else if(x->no < y->no) {
    return(-1);
  }

  return(0);
}

/*
 * _job_compare_ldd(a, b)
 *   compares two jobs and returns which should be sequenced first (LDD).
 *        a: job x
 *        b: job y
 *
 */
int _job_compare_ldd(const void *a, const void *b)
{
  _job_t *x = *((_job_t **) a);
  _job_t *y = *((_job_t **) b);

  if(x->d < y->d) {
    return(1);
  } else if(x->d > y->d) {
    return(-1);
  } else if(x->p < y->p) {
    return(1);
  } else if(x->p > y->p) {
    return(-1);
  } else if(x->tw < y->tw) {
    return(1);
  } else if(x->tw > y->tw) {
    return(-1);
  } else if(x->ew < y->ew) {
    return(1);
  } else if(x->ew > y->ew) {
    return(-1);
  } else if(x->no < y->no) {
    return(1);
  } else if(x->no > y->no) {
    return(-1);
  }

  return(0);
}

/*
 * _job_compare_lpt(a, b)
 *   compares two jobs and returns which should be sequenced first (LPT).
 *        a: job x
 *        b: job y
 *
 */
int _job_compare_lpt(const void *a, const void *b)
{
  _job_t *x = *((_job_t **) a);
  _job_t *y = *((_job_t **) b);

  if(x->p < y->p) {
    return(1);
  } else if(x->p > y->p) {
    return(-1);
  } else if(x->d < y->d) {
    return(1);
  } else if(x->d > y->d) {
    return(-1);
  } else if(x->tw < y->tw) {
    return(1);
  } else if(x->tw > y->tw) {
    return(-1);
  } else if(x->ew < y->ew) {
    return(1);
  } else if(x->ew > y->ew) {
    return(-1);
  } else if(x->no < y->no) {
    return(1);
  } else if(x->no > y->no) {
    return(-1);
  }

  return(0);
}

/*
 * _job_compare_wspt(a, b)
 *   compares two jobs and returns which should be sequenced first (WSPT).
 *        a: job x
 *        b: job y
 *
 */
int _job_compare_wspt(const void *a, const void *b)
{
  _job_t *x = *((_job_t **) a);
  _job_t *y = *((_job_t **) b);

  if(y->tw*x->p > x->tw*y->p) {
    return(1);
  } else if(y->tw*x->p < x->tw*y->p) {
    return(-1);
  } else if(x->d > y->d) {
    return(1);
  } else if(x->d < y->d) {
    return(-1);
  } else if(x->p > y->p) {
    return(1);
  } else if(x->p < y->p) {
    return(-1);
  } else if(x->tw > y->tw) {
    return(1);
  } else if(x->tw < y->tw) {
    return(-1);
  } else if(x->ew > y->ew) {
    return(1);
  } else if(x->ew < y->ew) {
    return(-1);
  } else if(x->no > y->no) {
    return(1);
  } else if(x->no < y->no) {
    return(-1);
  }

  return(0);
}

/*
 * _job_compare_wlpt(a, b)
 *   compares two jobs and returns which should be sequenced first (WLPT).
 *        a: job x
 *        b: job y
 *
 */
int _job_compare_wlpt(const void *a, const void *b)
{
  _job_t *x = *((_job_t **) a);
  _job_t *y = *((_job_t **) b);

  if(y->tw*x->p < x->tw*y->p) {
    return(1);
  } else if(y->tw*x->p > x->tw*y->p) {
    return(-1);
  } else if(x->d < y->d) {
    return(1);
  } else if(x->d > y->d) {
    return(-1);
  } else if(x->p < y->p) {
    return(1);
  } else if(x->p > y->p) {
    return(-1);
  } else if(x->tw < y->tw) {
    return(1);
  } else if(x->tw > y->tw) {
    return(-1);
  } else if(x->ew < y->ew) {
    return(1);
  } else if(x->ew > y->ew) {
    return(-1);
  } else if(x->no < y->no) {
    return(1);
  } else if(x->no > y->no) {
    return(-1);
  }

  return(0);
}

#ifdef SIPSI
/*
 * _job_compare_release(a, b)
 *   compares two jobs and returns which should be sequenced first (release).
 *        a: job x
 *        b: job y
 *
 */
int _job_compare_release(const void *a, const void *b)
{
  _job_t *x = *((_job_t **) a);
  _job_t *y = *((_job_t **) b);

  if(x->r > y->r) {
    return(1);
  } else if(x->r < y->r) {
    return(-1);
  } else if(x->d > y->d) {
    return(1);
  } else if(x->d < y->d) {
    return(-1);
  } else if(x->p > y->p) {
    return(1);
  } else if(x->p < y->p) {
    return(-1);
  } else if(x->no > y->no) {
    return(1);
  } else if(x->no < y->no) {
    return(-1);
  }

  return(0);
}
#endif /* SIPSI */
