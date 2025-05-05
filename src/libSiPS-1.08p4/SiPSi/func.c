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
 *  $Id: func.c,v 1.13 2015/01/12 06:50:29 tanaka Rel $
 *  $Revision: 1.13 $
 *  $Date: 2015/01/12 06:50:29 $
 *  $Author: tanaka $
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "define.h"
#include "sips_common.h"
#include "bmemory.h"
#include "memory.h"
#include "func.h"

#ifdef FUNC_DEBUG
#define CHECK_FUNCTION(x) _check_function(x)
static _point_t *_check_function(_point_t *);
#else /* FUNC_DEBUG */
#define CHECK_FUNCTION(x)
#endif /* FUNC_DEBUG */

#define CREATE_POINT2(name, time, value) {		\
    name = alloc_memory(benv);				\
    (name)->t = time;					\
    (name)->v = value;					\
  }

#define CREATE_POINT(name, time, value, state) {	\
    CREATE_POINT2(name, time, value);			\
    (name)->s = state;					\
  }

#define APPEND(s, a) {				\
    (a)->prev = (s)->prev;			\
    (a)->next = NULL;				\
    (s)->prev->next = a;			\
    (s)->prev = a;				\
  }

#define INSERT_TOP(s, a) {			\
    (a)->prev = (s)->prev;			\
    (a)->next = s;				\
    (s)->prev = a;				\
    s = a;					\
  }

#define INSERT_NEXT(s, a) {			\
    if((s)->next == NULL) {			\
      APPEND(s, a);				\
    } else {					\
      (a)->next = (s)->next;			\
      (a)->prev = s;				\
      (s)->next->prev = a;			\
      (s)->next = a;				\
    }						\
  }

#define INSERT_NEXT2(s, a) {			\
    (a)->prev = s;				\
    (a)->next = NULL;				\
    (s)->next = a;				\
    s = a;					\
  }

#define INSERT_BEFORE(s, a) {			\
    (a)->next = (s);				\
    (a)->prev = (s)->prev;			\
    (s)->prev->next = a;			\
    (s)->prev = a;				\
  }

/*
 * duplicate_function(benv, src)
 *   returns a copy of the specified function.
 *
 */
_point_t *duplicate_function(_benv_t *benv, _point_t *src)
{
  _point_t *dest, *p;

  if(src == NULL) {
    return(NULL);
  }

  dest = p = (_point_t *) alloc_memory(benv);
  for(; src->next != NULL; src = src->next, p = p->next) {
    p->t = src->t;
    p->v = src->v;
    p->s = src->s;
    p->next = (_point_t *) alloc_memory(benv);
    p->next->prev = p;
  }

  p->t = src->t;
  p->v = src->v;
  p->s = src->s;
  p->next = NULL;
  dest->prev = p;

  CHECK_FUNCTION(dest);

  return(dest);
}

/*
 * free_function(benv, src)
 *   frees the specified function.
 *
 */
void free_function(_benv_t *benv, _point_t *src)
{
  _point_t *p;

  for(; src != NULL;) {
    p = src->next;
    free_memory(benv, src);
    src = p;
  }  
}

/*
 * zero_function(benv, tmax)
 *   creates a zero function.
 *      tmax: time domain [0, tmax]
 *
 */
_point_t *zero_function(_benv_t *benv, int tmax)
{
  _point_t *p0, *p;

  CREATE_POINT(p0, 0, 0, 0);
  CREATE_POINT(p, tmax, 0, 0);

  p0->next = p;
  p0->prev = p;
  p->prev = p0;
  p->next = NULL;

  CHECK_FUNCTION(p0);

  return(p0);
}

/*
 * null_function(benv)
 *   creates a null function.
 *
 */
_point_t *null_function(_benv_t *benv)
{
  return(NULL);
}

#ifdef COST_REAL
/*
 * _convert_function(prob, benv, src)
 *   applies the following mapping.
 *      f(t) ->  min f(s)
 *              s<=t
 *
 */
_point_t *_convert_function(sips *prob, _benv_t *benv, _point_t *src)
#else /* COST_REAL */
/*
 * _convert_function(benv, src)
 *   applies the following mapping.
 *      f(t) ->  min f(s)
 *              s<=t
 *
 */
_point_t *_convert_function(_benv_t *benv, _point_t *src)
#endif /* COST_REAL */
{
  int t;
  cost_t k, vdash;
  _point_t *p, *p0, *p1, *p2;
  
  if(src == NULL) {
    return(NULL);
  }

  for(p = src; p->next != NULL;) {
    if(fobj_lesser_equal(p->v, p->next->v)) {
      for(p1 = p->next->next;
	  p1 != NULL && fobj_lesser_equal(p->v, p1->v); p1 = p1->next) {
	free_memory(benv, p1->prev);
      }
      if(p1 != NULL) {
	p2 = p1->prev;
#ifdef COST_REAL
	if(fobj_equal(p->v, p2->v)) {
	  k = ZERO_COST;
	  t = 0;
	  vdash = p->v;
	} else {
	  k = (p1->v - p2->v)/itoc(p1->t - p2->t);
	  t = ctoi((p->v - p2->v)/k);
	  if(t >= p1->t - p2->t) {
	    t = p1->t - p2->t - 1;
	  }
	  p2->t += t;
	  vdash = p2->v + k*itoc(t);
	}

	p2->v = p->v;
	p->next = p2;
	p2->prev = p;
#else  /* COST_REAL */
	k = (p1->v - p2->v)/itoc(p1->t - p2->t);
	t = ctoi((p->v - p2->v)/k);
	vdash = p2->v + k*itoc(t);
	p2->t += t;
	p2->v = p->v;
	p->next = p2;
	p2->prev = p;
#endif /* COST_REAL */

	if(fobj_lesser(p->v, vdash) && p2->t + 1 < p1->t) {
	  CREATE_POINT2(p0, p2->t + 1, vdash + k);

	  p2->next = p0;
	  p0->prev = p2;
	  p0->next = p1;
	  p1->prev = p0;
	}

	p = p1;
      } else {
	p2 = src->prev;
	p2->v = p->v;
	p->next = p2;
	p2->prev = p;
	break;
      }
    } else {
      p = p->next;
    }
  }

  CHECK_FUNCTION(src);

  return(src);
}

#ifdef COST_REAL
/*
 * _rconvert_function(prob, benv, src)
 *   applies the following mapping.
 *      f(t) ->  min f(s)
 *              t<=s
 *
 */
_point_t *_rconvert_function(sips *prob, _benv_t *benv, _point_t **src)
#else /* COST_REAL */
/*
 * _rconvert_function(benv, src)
 *   applies the following mapping.
 *      f(t) ->  min f(s)
 *              t<=s
 *
 */
_point_t *_rconvert_function(_benv_t *benv, _point_t **src)
#endif /* COST_REAL */
{
  int t;
  cost_t k, vdash;
  _point_t *p, *p0, *p1, *p2;
  
  if(*src == NULL) {
    return(NULL);
  }

  for(p = (*src)->prev; p != *src;) {
    if(fobj_lesser_equal(p->v, p->prev->v)) {
      for(p1 = p->prev->prev;
	  p1 != (*src)->prev && fobj_lesser_equal(p->v, p1->v); p1 = p1->prev) {
	free_memory(benv, p1->next);
      }

      if(p1 != (*src)->prev) {
	p2 = p1->next;
#ifdef COST_REAL
	if(fobj_equal(p1->v, p2->v)) {
	  k = ZERO_COST;
	  t = 0;
	  vdash = p->v;
	} else {
	  k = (p2->v - p1->v)/itoc(p2->t - p1->t);
	  t = ctoi((p2->v - p->v)/k);
	  if(t >= p2->t - p1->t) {
	    t = p2->t - p1->t - 1;
	  }
	  p2->t -= t;
	  vdash = p2->v - k*itoc(t);
	}

	p2->v = p->v;
	p->prev = p2;
	p2->next = p;
#else /* COST_REAL */
	k = (p2->v - p1->v)/itoc(p2->t - p1->t);
	t = ctoi((p2->v - p->v)/k);
	vdash = p2->v - k*itoc(t);
	p2->t -= t;
	p2->v = p->v;
	p->prev = p2;
	p2->next = p;
#endif /* COST_REAL */
	if(fobj_lesser(p->v, vdash) && p2->t - 1 > p1->t) {
	  CREATE_POINT2(p0, p2->t - 1, vdash - k);
	  p1->next = p0;
	  p0->prev = p1;
	  p0->next = p2;
	  p2->prev = p0;
	}

	p = p1;
      } else {
	(*src)->v = p->v;
	p->prev = *src;
	(*src)->next = p;
	break;
      }
    } else {
      p = p->prev;
    }
  }

  if((*src)->next != NULL && (*src)->v == (*src)->next->v) {
    (*src)->t = 0;
  } else if((*src)->t != 0) {
    p = alloc_memory(benv);
    p->t = 0;
    p->v = (*src)->v;
    INSERT_TOP(*src, p);
  }

  CHECK_FUNCTION(*src);

  return(*src);
}

/*
 * shift_function(benv, src, tmax, off)
 *   applies the following mapping.
 *      f(t) ->  f(t - off) (off >= 0, 0 <= t <= tmax)
 *
 */
_point_t *shift_function(_benv_t *benv, _point_t **src, int tmax, int off)
{
  cost_t k;
  _point_t *p, *p0;
  
  if(*src == NULL) {
    return(NULL);
  } else if((*src)->t + off > tmax) {
    free_function(benv, *src);
    *src = NULL;
    return(NULL);
  }

  (*src)->t += off;
  for(p = (*src)->next; p != NULL; p = p->next) {
    p->t += off;
    if(p->t > tmax) {
      p0 = p->prev;
      if(p0->t != tmax) {
	k = (p->v - p0->v)/itoc(p->t - p0->t);
	p->v = p0->v + k*itoc(tmax - p0->t);
	p->t = tmax;
	p0 = p->next;
	p->next = NULL;
	(*src)->prev = p;
      } else {
	p0->next = NULL;
	(*src)->prev = p0;
	p0 = p;
      }
      while(p0 != NULL) {
	p = p0->next;
	free_memory(benv, p0);
	p0 = p;
      }
      break;
    }
  }

  CHECK_FUNCTION(*src);

  return(*src);
}

/*
 * rshift_function(benv, src, off)
 *   applies the following mapping.
 *      f(t) ->  f(t + off) (off >= 0)
 *
 */
_point_t *rshift_function(_benv_t *benv, _point_t **src, int off)
{
  cost_t k;
  _point_t *p, *p0, *p1, *p2;
  
  if(*src == NULL) {
    return(NULL);
  } else if((*src)->prev->t - off < 0) {
    free_function(benv, *src);
    *src = NULL;
    return(NULL);
  }

  p0 = (*src)->prev;
  p0->t -= off;
  for(p = p0->prev; p != p0; p = p->prev) {
    p->t -= off;
    if(p->t < 0) {
      p2 = p->next;
      if(p2->t != 0) {
	k = (p2->v - p->v)/itoc(p2->t - p->t);
	p->v -= k*itoc(p->t);
	p->t = 0;
	p2 = p->prev;
	p->prev = p0;
      } else {
	p2->prev = p0;
	p2 = p;
	p = p->next;
      }
      while(p2 != p0) {
	p1 = p2->prev;
	free_memory(benv, p2);
	p2 = p1;
      }
      *src = p;
      break;
    }
  }

  CHECK_FUNCTION(*src);

  return(*src);
}

/*
 * duplicate_function_with_shift(benv, src, tmax, off)
 *   copies the specified function and then shifts it.
 *   States are resetted.
 *
 */
_point_t *duplicate_function_with_shift(_benv_t *benv, _point_t *src,
					int tmax, int off)
{
  cost_t k;
  _point_t *fp, *p, *p0, *p1;

  if(src == NULL || src->t + off > tmax) {
    return(NULL);
  }
  
  CREATE_POINT(fp, src->t + off, src->v, 0);
  fp->next = NULL;
  p0 = fp;

  for(src = src->next; src != NULL; src = src->next) {
    p0->next = p = alloc_memory(benv);
    p->s = 0;
    p->next = NULL;
    p->prev = p0;
    p0 = p;

    if(src->t + off > tmax) {
      p1 = src->prev;
      k = (src->v - p1->v)/itoc(src->t - p1->t);
      p0->v = p1->v + k*itoc(tmax - off - p1->t);
      p0->t = tmax;
      break;
    } else {
      p0->v = src->v;
      p0->t = src->t + off;
      if(p0->t == tmax) {
	break;
      }
    }
  }

  fp->prev = p0;

  if(fp->t == p0->t) {
    free_memory(benv, p0);
    fp->next = NULL;
    fp->prev = fp;
  }

  CHECK_FUNCTION(fp);

  return(fp);
}

/*
 * duplicate_function_with_tmax(benv, src, tmax)
 *   copies the specified function with the upper limit of t restricted.
 *   States are resetted.
 *
 */
_point_t *duplicate_function_with_tmax(_benv_t *benv, _point_t *src, int tmax)
{
  cost_t k;
  _point_t *fp, *p, *p0, *p1;

  if(src == NULL || src->t > tmax) {
    return(NULL);
  }
  
  CREATE_POINT(fp, src->t, src->v, 0);
  fp->next = NULL;
  p0 = fp;

  for(src = src->next; src != NULL; src = src->next) {
    p0->next = p = alloc_memory(benv);
    p->s = 0;
    p->next = NULL;
    p->prev = p0;
    p0 = p;

    if(src->t > tmax) {
      p1 = src->prev;
      k = (src->v - p1->v)/itoc(src->t - p1->t);
      p0->v = p1->v + k*itoc(tmax - p1->t);
      p0->t = tmax;
      break;
    } else {
      p0->v = src->v;
      p0->t = src->t;
      if(p0->t == tmax) {
	break;
      }
    }
  }

  fp->prev = p0;

  if(fp->t == p0->t) {
    free_memory(benv, p0);
    fp->next = NULL;
    fp->prev = fp;
  }

  CHECK_FUNCTION(fp);

  return(fp);
}

/*
 * add_functions(benv, src, src2)
 *   adds two functions.
 *
 */
_point_t *add_functions(_benv_t *benv, _point_t **src, _point_t *src2)
{
  int t2;
  cost_t v1, v2, k;
  _point_t p0;
  _point_t *p1, *p2, *p3, *p4;

  if(*src == NULL || src2 == NULL) {
    free_function(benv, *src);
    *src = NULL;
    return(NULL);
  }

  p1 = *src;
  p2 = src2;
  if(p2->t < p1->t) {
    for(p2 = p2->next; p2 != NULL && p2->t < p1->t; p2 = p2->next);
    if(p2 == NULL) {
      free_function(benv, *src);
      *src = NULL;
      return(NULL);
    }

    if(p2->t == p1->t) {
      p0 = *p2;
    } else {
      p0.t = p1->t;
      k = (p2->v - p2->prev->v)/itoc(p2->t - p2->prev->t);
      p0.v = p2->prev->v + k*itoc(p1->t - p2->prev->t);
      p0.next = p2;
    }
  } else if(p1->t < p2->t) {
    p3 = p1->prev;
    for(; p1->next != NULL && p1->next->t < p2->t;) {
      p4 = p1->next;
      free_memory(benv, p1);
      p1 = p4;
    }
    if(p1->next == NULL) {
      free_memory(benv, p1);
      *src = NULL;
      return(NULL);
    }
    if(p1->next->t == p2->t) {
      *src = p1->next;
      free_memory(benv, p1);
      p1 = *src;
      p1->prev = p3;
    } else {
      k = (p1->next->v - p1->v)/itoc(p1->next->t - p1->t);
      p1->v += k*itoc(p2->t - p1->t);
      p1->t = p2->t;
      p1->prev = p3;
      *src = p1;
    }
    p0 = *p2;
  } else {
    p0 = *p2;
  }

  p2 = &p0;
  t2 = p0.t;
  v2 = p0.v;
  v1 = p1->v;
  while(p1 != NULL && p2 != NULL) {
    if(p1->t < p2->t) {
      for(p1 = p1->next; p1 != NULL && p1->t < p2->t; p1 = p1->next) {
	v1 = p1->v;
	k = (p2->v - v2)/itoc(p2->t - t2);
	p1->v += v2 + k*itoc(p1->t - t2);
      }

      if(p1 == NULL) {
	break;
      } else if(p1->t > p2->t) {
	k = (p1->v - v1)/itoc(p1->t - p1->prev->t);
	CREATE_POINT2(p3, p2->t, v1 + k*itoc(p2->t - p1->prev->t));
	INSERT_BEFORE(p1, p3);
	p1 = p3;
      }
    } else {
      v1 = p1->v;
      p1->v += p2->v;
      t2 = p2->t;
      v2 = p2->v;
      p2 = p2->next;
    }
  }

  CHECK_FUNCTION(*src);

  return(*src);
}

#ifdef COST_REAL
/*
 * _minimum_of_functions(prob, benv, src1, src2)
 *   takes the minimum of two functions.
 *         f(t) = min{f1(t), f2(t)}
 *
 */
_point_t *_minimum_of_functions(sips *prob, _benv_t *benv, _point_t **src,
				_point_t *src2)
#else /* COST_REAL */
/*
 * _minimum_of_functions(benv, src1, src2)
 *   takes the minimum of two functions.
 *         f(t) = min{f1(t), f2(t)}
 *
 */
_point_t *_minimum_of_functions(_benv_t *benv, _point_t **src, _point_t *src2)
#endif /* COST_REAL */
{
  int t, t0, t1, t2, t3;
  int s1, s2;
  char m;
  cost_t v1, v2, v3, v4, v5, v6;
  cost_t k1, k2;
  _point_t *p, *p0, *p1, *p2;
  _point_t *fp, *fp1, *fp2;

  if(src2 == NULL) {
    return(*src);
  } else if(*src == NULL) {
    *src = duplicate_function(benv, src2);
    return(*src);
  }

  p1 = *src;
  if(p1->prev->t < src2->t || p1->t > src2->prev->t) {
    fprintf(stderr, "minimum_of_functions(): Two function domains should ");
    fprintf(stderr, " overlap.\n");
    exit(1);
  }

  if(p1->next == NULL && src2->next == NULL) {
    if(fobj_greater(p1->v, src2->v)) {
      p1->v = src2->v;
      p1->s = src2->s;
    }
    return(*src);
  }

  fp1 = *src;
  fp2 = src2;
  if(fp1->t < src2->t || fp1->prev->t > src2->prev->t) {
    fp2 = duplicate_function(benv, src2);
  }

  p1 = fp1;
  p2 = fp2;
  s1 = p1->s;
  s2 = p2->s;
  if(p1->t < p2->t) {
    v1 = p1->v;
    for(p0 = p1->next; p0 != NULL && p0->prev->t < p2->t; p0 = p0->next) {
      v1 = max(v1, p0->v);
    }
    v1 += itoc(1);

    CREATE_POINT(p, p1->t, v1, s2);
    INSERT_TOP(fp2, p);

    t2 = p2->t;
    p2 = fp2;
    if(p1->t + 1 < t2) {
      CREATE_POINT(p, t2 - 1, v1, s2);
      INSERT_NEXT(fp2, p);
    }
  } else if(p1->t > p2->t) {
    v2 = p2->v;
    for(p0 = p2->next; p0 != NULL && p0->prev->t < p1->t; p0 = p0->next) {
      v2 = max(v2, p0->v);
    }
    v2 += itoc(1);

    CREATE_POINT(p, p2->t, v2, s1);
    INSERT_TOP(fp1, p);

    t1 = p1->t;
    p1 = fp1;
    if(p2->t + 1 < t1) {
      CREATE_POINT(p, t1 - 1, v2, s1);
      INSERT_NEXT(fp1, p);
    }
  }

  s1 = p1->prev->s;
  s2 = p2->prev->s;
  if(p1->prev->t < p2->prev->t) {
    if(p1->prev->t + 1 < p2->prev->t) {
      CREATE_POINT(p, p1->prev->t + 1, LARGE_COST, s1);
      APPEND(p1, p);
    }

    CREATE_POINT(p, p2->prev->t, LARGE_COST, s1);
    APPEND(p1, p);
  } else if(p1->prev->t > p2->prev->t) {
    if(p1->prev->t > p2->prev->t + 1) {
      CREATE_POINT(p, p2->prev->t + 1, LARGE_COST, s2);
      APPEND(p2, p);
    }

    CREATE_POINT(p, p1->prev->t, LARGE_COST, s2);
    APPEND(p2, p);
  }

#ifdef DEBUG
  printf("----\n");
  print_function(fp1, stdout);
  print_function(fp2, stdout);
  printf("----\n");
#endif

  if(fobj_lesser_equal(p1->v, p2->v)) {
    CREATE_POINT(fp, p1->t, p1->v, p1->s);
    m = 0;
    s1 = p1->s;
  } else {
    CREATE_POINT(fp, p2->t, p2->v, p2->s);
    m = 1;
    s2 = p2->s;
  }
  fp->next = NULL;
  p = fp;

  t = p1->t;
  v1 = p1->v;
  v2 = p2->v;
  p1 = p1->next;
  p2 = p2->next;
  while(p1 != NULL && p2 != NULL) {
    t1 = p1->t;
    t2 = p2->t;
    if(t1 == p1->prev->t) {
      print_function(fp1, stdout);
      exit(1);
    }
    k1 = (p1->v - p1->prev->v)/itoc(t1 - p1->prev->t);
    k2 = (p2->v - p2->prev->v)/itoc(t2 - p2->prev->t);
    if(t1 < t2) {
      t3 = t1;
      v3 = p1->v;
      v4 = v2 + k2*(t3 - t);
    } else if(t2 < t1) {
      t3 = t2;
      v3 = v1 + k1*(t3 - t);
      v4 = p2->v;
    } else {
      t3 = t1;
      v3 = p1->v;
      v4 = p2->v;
    }
    s1 = p1->s;
    s2 = p2->s;

#ifdef DEBUG
#ifdef COST_REAL
#ifdef REAL_LONG
    printf("t=%d, t1=%d, t2=%d, t3=%d, k1=%Lg, k2=%Lg, s1=%d, s2=%d\n",
	   t, t1, t2, t3, k1, k2, s1, s2);
    printf("m=%d, p1->t=%d, p1->v=%Lg, p2->t=%d, p2->v=%Lg\n",
	   m, p1->t, p1->v, p2->t, p2->v);
    printf("v1=%Lg, v2=%Lg, v3=%Lg, v4=%Lg\n", v1, v2, v3, v4);
#else /* REAL_LONG */
    printf("t=%d, t1=%d, t2=%d, t3=%d, k1=%g, k2=%g, s1=%d, s2=%d\n",
	   t, t1, t2, t3, k1, k2, s1, s2);
    printf("m=%d, p1->t=%d, p1->v=%g, p2->t=%d, p2->v=%g\n",
	   m, p1->t, p1->v, p2->t, p2->v);
    printf("v1=%g, v2=%g, v3=%g, v4=%g\n", v1, v2, v3, v4);
#endif /* REAL_LONG */
#else /* COST_REAL */
    printf("t=%d, t1=%d, t2=%d, t3=%d, k1=%d, k2=%d, s1=%d, s2=%d\n",
	   t, t1, t2, t3, k1, k2, s1, s2);
    printf("m=%d, p1->t=%d, p1->v=%d, p2->t=%d, p2->v=%d\n",
	   m, p1->t, p1->v, p2->t, p2->v);
    printf("v1=%d, v2=%d, v3=%d, v4=%d\n", v1, v2, v3, v4);
#endif /* COST_REAL */
#endif

    if((m == 0 && fobj_lesser_equal(v3, v4))
       || (m == 1 && fobj_greater(v3, v4))) {
      if(m == 0 && t3 == t1) {
	CREATE_POINT(p0, t3, v3, s1);
	INSERT_NEXT2(p, p0);
      } else if(m == 1 && t3 == t2) {
	CREATE_POINT(p0, t3, v4, s2);
	INSERT_NEXT2(p, p0);
      }
      if(t3 == t1) {
	p1 = p1->next;
      }
      if(t3 == t2) {
	p2 = p2->next;
      }
      t = t3;
      v1 = v3;
      v2 = v4;

#ifdef DEBUG
      print_function(fp, stdout);
      fflush(stdout);
#endif /* DEBUG */

      continue;
    }

    t0 = t - (int) ((v1 - v2)/(k1 - k2));
    t0 = max(t0, t);
    t0 = min(t0, t3 - 1);
#ifdef COST_REAL
    do {
      t0++;
      v5 = v1 + k1*(t0 - t);
      v6 = v2 + k2*(t0 - t);

      if(t0 > t3) {
	fprintf(stderr,
		"_minimum_of_functions(): Could not find a cross point\n");
	exit(1);
      }
    } while((m == 0 && fobj_lesser_equal(v5, v6))
	    || (m == 1 && fobj_greater(v5, v6)));
    t0--;
    v5 = v1 + k1*(t0 - t);
    v6 = v2 + k2*(t0 - t);
#else /* COST_REAL */
    v5 = v1 + k1*(t0 - t);
    v6 = v2 + k2*(t0 - t);
    while((m == 0 && fobj_lesser_equal(v5 + k1, v6 + k2))
	  || (m == 1 && fobj_greater(v5 + k1, v6 + k2))) {
      t0++;
      v5 += k1;
      v6 += k2;
    }
#endif /* COST_REAL */

#ifdef DEBUG
#ifdef COST_REAL
#ifdef REAL_LONG
    printf("t0=%d, v5=%Lg, v6=%Lg\n", t0, v5, v6);
#else /* REAL_LONG */
    printf("t0=%d, v5=%g, v6=%g\n", t0, v5, v6);
#endif /* REAL_LONG */
#else /* COST_REAL */
    printf("t0=%d, v5=%d, v6=%d\n", t0, v5, v6);
#endif /* COST_REAL */
#endif

    if(t0 == t) {
      if(p->t != t) {
	if(m == 0) {
	  CREATE_POINT(p0, t0, v1, p1->prev->s);
	} else {
	  CREATE_POINT(p0, t0, v2, p2->prev->s);
	}
	INSERT_NEXT2(p, p0);
      }
    } else {
      if(m == 0) {
	CREATE_POINT(p0, t0, v5, s1);
      } else {
	CREATE_POINT(p0, t0, v6, s2);
      }
      INSERT_NEXT2(p, p0);
    }

    if(m == 0) {
      if(t0 + 1 == t2) {
	CREATE_POINT(p0, t2, v4, s2);
      } else {
	CREATE_POINT(p0, t0 + 1, v6 + k2, s2);
      }
      INSERT_NEXT2(p, p0);
      if(t3 == t2) {
	if(p->t != t3) {
	  CREATE_POINT(p0, t3, v4, s2);
	  INSERT_NEXT2(p, p0);
	}
	p2 = p2->next;
      }
      if(t3 == t1) {
	p1 = p1->next;
      }
    } else {
      if(t0 + 1 == t1) {
	CREATE_POINT(p0, t1, v3, s1);
      } else {
	CREATE_POINT(p0, t0 + 1, v5 + k1, s1);
      }
      INSERT_NEXT2(p, p0);
      if(t3 == t1) {
	if(p->t != t3) {
	  CREATE_POINT(p0, t3, v3, s1);
	  INSERT_NEXT2(p, p0);
	}
	p1 = p1->next;
      }
      if(t3 == t2) {
	p2 = p2->next;
      }
    }

    t = t3;
    v1 = v3;
    v2 = v4;
    m = 1 - m;

#ifdef DEBUG
    print_function(fp, stdout);
    fflush(stdout);
#endif /* DEBUG */
  }

  fp->prev = p;

  free_function(benv, fp1);
  if(fp2 != src2) {
    free_function(benv, fp2);
  }
  *src = fp;

  CHECK_FUNCTION(*src);

  return(*src);
}

#ifdef COST_REAL
/*
 * _minimize_function(prob, src, tmax, s)
 *   minimizes the specified nonincreasing function.
 *   It returns t = argmin f(s) and the state of f(t).
 *                  s<=tmax
 *
 */
int _minimize_function(sips *prob, _point_t *src, int tmax, int *s)
#else /* COST_REAL */
/*
 * _minimize_function(src, tmax, s)
 *   minimizes the specified nonincreasing function.
 *   It returns t = argmin f(s) and the state of f(t).
 *                  s<=tmax
 *
 */
int _minimize_function(_point_t *src, int tmax, int *s)
#endif /* COST_REAL */
{
  _point_t *p;

  if(src == NULL || src->t > tmax) {
    return(-1);
  } else if(src->next == NULL || src->t == tmax) {
    if(s != NULL) {
      *s = src->s;
    }
    return(src->t);
  }

  p = src->prev;  
  if(p->t <= tmax) {
    for(; fobj_lesser_equal(p->prev->v, p->v) && p != src; p = p->prev);
    if(s != NULL) {
      *s = p->s;
    }

    return(p->t);
  }

  for(; p->t > tmax; p = p->prev);
  if(p->t == tmax || fobj_lesser_equal(p->v, p->next->v)) {
    for(; fobj_lesser_equal(p->prev->v, p->v) && p != src; p = p->prev);
    tmax = p->t;
  }

  if(s != NULL) {
    *s = p->s;
  }

  return(tmax);
}

#ifdef COST_REAL
/*
 * _rminimize_function(prob, src, tmin, s)
 *   minimizes the specified nondecreasing function.
 *   It returns t = argmin f(s) and the state of f(t).
 *                  tmin<=s
 *
 */
int _rminimize_function(sips *prob, _point_t *src, int tmin, int *s)
#else /* COST_REAL */
/*
 * _rminimize_function(src, tmin, s)
 *   minimizes the specified nondecreasing function.
 *   It returns t = argmin f(s) and the state of f(t).
 *                  tmin<=s
 *
 */
int _rminimize_function(_point_t *src, int tmin, int *s)
#endif /* COST_REAL */
{
  _point_t *p;

  if(src == NULL || src->prev->t < tmin) {
    return(-1);
  } else if(src->next == NULL) {
    if(s != NULL) {
      *s = src->s;
    }
    return(src->t);
  } else if(src->prev->t == tmin) {
    if(s != NULL) {
      *s = src->prev->s;
    }
    return(tmin);
  }

  for(p = src; p->t < tmin; p = p->next);
  if(p != src && p->t != tmin && fobj_lesser(p->prev->v, p->v)) {
    if(s != NULL) {
      *s = p->s;
    }
    return(tmin);
  }

  for(; p->next != NULL && fobj_greater_equal(p->v, p->next->v); p = p->next);
  if(s != NULL) {
    *s = p->s;
  }

  return(p->t);
}

/*
 * minimize_functions_when_added(src1, src2, v)
 *   minimizes the sum of the specified nonincreasing
 *   and nondecreasing functions.
 *   It returns t = argmin f1(s)+f2(s) and f1(t)+f2(t) as v.
 *                     s
 *
 */
int minimize_functions_when_added(_point_t *src1, _point_t *src2, cost_t *v)
{
  int t, t1, t2, mint;
  cost_t v0, minv;
  cost_t k1, k2;

  if(src1 == NULL || src2 == NULL) {
    return(-1);
  }

  t1 = src1->t;
  t2 = src2->t;

  if(src1->next != NULL) {
    k1 = (src1->next->v - src1->v)/itoc(src1->next->t - t1);
  } else {
    k1 = LARGE_COST;
  }
  if(src2->next != NULL) {
    k2 = (src2->next->v - src2->v)/itoc(src2->next->t - t2);
  } else {
    k2 = LARGE_COST;
  }

  if(t1 < t2) {
    for(src1 = src1->next; src1 != NULL && src1->t < t2;
	src1 = src1->next);
    if(src1 == NULL) {
      return(-1);
    }

    src1 = src1->prev;
    k1 = (src1->next->v - src1->v)/(src1->next->t - src1->t);
  } else if(t1 > t2) {
    for(src2 = src2->next; src2 != NULL && src2->t < t1;
	src2 = src2->next);
    if(src2 == NULL) {
      return(-1);
    }

    src2 = src2->prev;
    k2 = (src2->next->v - src2->v)/(src2->next->t - src2->t);
  }

  mint = t = max(src1->t, src2->t);
  minv = LARGE_COST;
  while(1) {
    v0 = src1->v + k1*itoc(t - src1->t) + src2->v + k2*itoc(t - src2->t);
    if(v0 < minv) {
      mint = t;
      minv = v0;
    }
    if(src1->next == NULL || src2->next == NULL) {
      break;
    }

    if(src1->next->t < src2->next->t) {
      src1 = src1->next;
      t = src1->t;
      if(src1->next != NULL) {
	k1 = (src1->next->v - src1->v)/itoc(src1->next->t - src1->t);
      }
    } else if(src1->next->t > src2->next->t) {
      src2 = src2->next;
      t = src2->t;
      if(src2->next != NULL) {
	k2 = (src2->next->v - src2->v)/itoc(src2->next->t - src2->t);
      }
    } else {
      src1 = src1->next;
      src2 = src2->next;
      t = src1->t;
      if(src1->next != NULL) {
	k1 = (src1->next->v - src1->v)/itoc(src1->next->t - src1->t);
      }
      if(src2->next != NULL) {
	k2 = (src2->next->v - src2->v)/itoc(src2->next->t - src2->t);
      }
    }
  } 

  if(v != NULL) {
    *v = minv;
  }

  return(mint); 
}

/*
 * set_function_state(src, s)
 *   assigns the state s value to the function.
 *
 */
void set_function_state(_point_t *src, int s)
{
  for(; src != NULL; src = src->next) {
    src->s = s;
  }
}

/*
 * print_function(src, fp)
 *   print out the function.
 *
 */
void print_function(_point_t *src, FILE *fp)
{
#ifdef DYNA_DEBUG
  int s;
#endif /* DYNA_DEBUG */

  if(src == NULL) {
    return;
  }

#ifdef DYNA_DEBUG
#ifdef COST_REAL
  for(; src->next != NULL; src = src->next) {
    if(src->s > 600) {
      s = 0;
    } else {
      s = src->s;
    }

    if(large(src->v)) {
      printf("(%d,*){%d}-", src->t, s);
    } else {
#ifdef REAL_LONG
      printf("(%d,%Lg){%d}-", src->t, src->v, s);
#else /* REAL_LONG */
      printf("(%d,%g){%d}-", src->t, src->v, s);
#endif /* REAL_LONG */
    }
  }
  if(src->s > 600) {
    s = 0;
  } else {
    s = src->s;
  }

  if(large(src->v)) {
    printf("(%d,*){%d}\n", src->t, s);
  } else {
#ifdef REAL_LONG
    printf("(%d,%Lg){%d}\n", src->t, src->v, s);
#else /* REAL_LONG */
    printf("(%d,%g){%d}\n", src->t, src->v, s);
#endif /* REAL_LONG */
  }
#else /* COST_REAL */
  for(; src->next != NULL; src = src->next) {
    if(src->s > 600) {
      s = 0;
    } else {
      s = src->s;
    }
    if(large(src->v)) {
      printf("(%d,*){%d}-", src->t, s);
    } else {
#ifdef COST_LONGLONG
      printf("(%d,%Ld){%d}-", src->t, src->v, s);
#else /* COST_LONGLONG */
      printf("(%d,%d){%d}-", src->t, src->v, s);
#endif /* COST_LONGLONG */
    }
  }

  if(src->s > 600) {
    s = 0;
  } else {
    s = src->s;
  }
  if(large(src->v)) {
    printf("(%d,*){%d}\n", src->t, s);
  } else {
#ifdef COST_LONGLONG
    printf("(%d,%Ld){%d}\n", src->t, src->v, s);
#else /* COST_LONGLONG */
    printf("(%d,%d){%d}\n", src->t, src->v, s);
#endif /* COST_LONGLONG */
  }
#endif /* COST_REAL */
#else /* DYNA_DEBUG */
#ifdef COST_REAL
  for(; src->next != NULL; src = src->next) {
    if(large(src->v)) {
      printf("(%d,*){%d}-", src->t, src->s);
    } else {
#ifdef REAL_LONG
      printf("(%d,%Lg){%d}-", src->t, src->v, src->s);
#else /* REAL_LONG */
      printf("(%d,%g){%d}-", src->t, src->v, src->s);
#endif /* REAL_LONG */
    }
  }
  if(large(src->v)) {
    printf("(%d,*){%d}\n", src->t, src->s);
  } else {
#ifdef REAL_LONG
    printf("(%d,%Lg){%d}\n", src->t, src->v, src->s);
#else /* REAL_LONG */
    printf("(%d,%g){%d}\n", src->t, src->v, src->s);
#endif /* REAL_LONG */
  }
#else /* COST_REAL */
  for(; src->next != NULL; src = src->next) {
    if(large(src->v)) {
      printf("(%d,*){%d}-", src->t, src->s);
    } else {
#ifdef COST_LONGLONG
      printf("(%d,%Ld){%d}-", src->t, src->v, src->s);
#else /* COST_LONGLONG */
      printf("(%d,%d){%d}-", src->t, src->v, src->s);
#endif /* COST_LONGLONG */
    }
  }
  if(large(src->v)) {
    printf("(%d,*){%d}\n", src->t, src->s);
  } else {
#ifdef COST_LONGLONG
    printf("(%d,%Ld){%d}\n", src->t, src->v, src->s);
#else /* COST_LONGLONG */
    printf("(%d,%d){%d}\n", src->t, src->v, src->s);
#endif /* COST_LONGLONG */
  }
#endif /* COST_REAL */
#endif /* DYNA_DEBUG */

}

#ifdef FUNC_DEBUG
/*
 * _check_function(src)
 *   checks the correctness of function.
 *
 */
_point_t *_check_function(_point_t *src)
{
  _point_t *p;

  if(src == NULL) {
    return(NULL);
  }

#ifdef COST_REAL
  if(src->next == NULL) {
    if(src->prev != src) {
#ifdef REAL_LONG
      printf("(%d, %Lg) <- (%d, %Lg) != (%d, %Lg)\n",
	     src->t, src->v, src->prev->t, src->prev->v, src->t, src->v);
#else /* REAL_LONG */
      printf("(%d, %g) <- (%d, %g) != (%d, %g)\n",
	     src->t, src->v, src->prev->t, src->prev->v, src->t, src->v);
#endif /* REAL_LONG */
      exit(1);
    }
    return(src);
  }

  if(src != src->next->prev) {
#ifdef REAL_LONG
    printf("(%d, %Lg) -> (%d, %Lg) <- (%d, %Lg)\n",
	   src->t, src->v, src->next->t, src->next->v,
	   src->next->prev->t, src->next->prev->v);
#else /* REAL_LONG */
    printf("(%d, %g) -> (%d, %g) <- (%d, %g)\n",
	   src->t, src->v, src->next->t, src->next->v,
	   src->next->prev->t, src->next->prev->v);
#endif /* REAL_LONG */
    exit(1);
  }

  for(p = src->next; p->next != NULL; p = p->next) {
    if(p != p->next->prev) {
#ifdef REAL_LONG
      printf("(%d, %Lg) -> (%d, %Lg) <- (%d, %Lg)\n",
	     p->t, p->v, p->next->t, p->next->v,
	     p->next->prev->t, p->next->prev->v);
#else /* REAL_LONG */
      printf("(%d, %g) -> (%d, %g) <- (%d, %g)\n",
	     p->t, p->v, p->next->t, p->next->v,
	     p->next->prev->t, p->next->prev->v);
#endif /* REAL_LONG */
      exit(1);
    }

    if(p->prev->next != p) {
#ifdef REAL_LONG
      printf("(%d, %Lg) <- (%d, %Lg) -> (%d, %Lg)\n",
	     p->t, p->v, p->prev->t, p->prev->v,
	     p->prev->next->t, p->prev->next->v);
#else /* REAL_LONG */
      printf("(%d, %g) <- (%d, %g) -> (%d, %g)\n",
	     p->t, p->v, p->prev->t, p->prev->v,
	     p->prev->next->t, p->prev->next->v);
#endif /* REAL_LONG */
      exit(1);
    }
  }

  if(src->prev != p) {
#ifdef REAL_LONG
    printf("(%d, %Lg) <- (%d, %Lg) != (%d, %Lg)\n",
	   src->t, src->v, src->prev->t, src->prev->v, p->t, p->v);
#else /* REAL_LONG */
    printf("(%d, %g) <- (%d, %g) != (%d, %g)\n",
	   src->t, src->v, src->prev->t, src->prev->v, p->t, p->v);
#endif /* REAL_LONG */
    exit(1);
  }
#else /* COST_REAL */
  if(src->next == NULL) {
    if(src->prev != src) {
#ifdef COST_LONGLONG
      printf("(%d, %Ld) <- (%d, %Ld) != (%d, %Ld)\n",
	     src->t, src->v, src->prev->t, src->prev->v, src->t, src->v);
#else /* COST_LONGLONG */
      printf("(%d, %d) <- (%d, %d) != (%d, %d)\n",
	     src->t, src->v, src->prev->t, src->prev->v, src->t, src->v);
#endif /* COST_LONGLONG */
      exit(1);
    }
    return(src);
  }

  if(src != src->next->prev) {
#ifdef COST_LONGLONG
    printf("(%d, %Ld) -> (%d, %Ld) <- (%d, %Ld)\n",
	   src->t, src->v, src->next->t, src->next->v,
	   src->next->prev->t, src->next->prev->v);
#else /* COST_LONGLONG */
    printf("(%d, %d) -> (%d, %d) <- (%d, %d)\n",
	   src->t, src->v, src->next->t, src->next->v,
	   src->next->prev->t, src->next->prev->v);
#endif /* COST_LONGLONG */
    exit(1);
  }

  for(p = src->next; p->next != NULL; p = p->next) {
    if(p != p->next->prev) {
#ifdef COST_LONGLONG
      printf("(%d, %Ld) -> (%d, %Ld) <- (%d, %Ld)\n",
	     p->t, p->v, p->next->t, p->next->v,
	     p->next->prev->t, p->next->prev->v);
#else /* COST_LONGLONG */
      printf("(%d, %d) -> (%d, %d) <- (%d, %d)\n",
	     p->t, p->v, p->next->t, p->next->v,
	     p->next->prev->t, p->next->prev->v);
#endif /* COST_LONGLONG */
      exit(1);
    }

    if(p->prev->next != p) {
#ifdef COST_LONGLONG
      printf("(%d, %Ld) <- (%d, %Ld) -> (%d, %Ld)\n",
	     p->t, p->v, p->prev->t, p->prev->v,
	     p->prev->next->t, p->prev->next->v);
#else /* COST_LONGLONG */
      printf("(%d, %d) <- (%d, %d) -> (%d, %d)\n",
	     p->t, p->v, p->prev->t, p->prev->v,
	     p->prev->next->t, p->prev->next->v);
#endif /* COST_LONGLONG */
      exit(1);
    }
  }

  if(src->prev != p) {
#ifdef COST_LONGLONG
    printf("(%d, %Ld) <- (%d, %Ld) != (%d, %Ld)\n",
	   src->t, src->v, src->prev->t, src->prev->v, p->t, p->v);
#else /* COST_LONGLONG */
    printf("(%d, %d) <- (%d, %d) != (%d, %d)\n",
	   src->t, src->v, src->prev->t, src->prev->v, p->t, p->v);
#endif /* COST_LONGLONG */
    exit(1);
  }
#endif /* COST_REAL */

  return(src);
}
#endif /* FUNC_DEBUG */
