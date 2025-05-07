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
 *  $Id: dominance.c,v 1.12 2013/05/28 13:27:03 tanaka Rel $
 *  $Revision: 1.12 $
 *  $Date: 2013/05/28 13:27:03 $
 *  $Author: tanaka $
 *
 */
#include "define.h"
#include "sips_common.h"
#include "dominance.h"

#define THREE_COST(j0, j1, j2)					\
  prob->sjob[j[j0]]->f[s + prob->sjob[j[j0]]->p]		\
  + prob->sjob[j[j1]]->f[c - prob->sjob[j[j2]]->p]		\
  + prob->sjob[j[j2]]->f[c]

#define THREE_COST_CHECK(j0, j1, j2) {					\
    g = THREE_COST(j0, j1, j2);						\
    if(obj_lesser(g, f)							\
       || (obj_equal(g, f)						\
	   && prob->sjob[j[j0]]->tno < prob->sjob[j[0]]->tno)) {	\
      return(1);							\
    }									\
  }

/*
 * check_three_cost(prob, j, s, c)
 *   checks whether there exists a three-job sequence which dominates
 *   the specified sequence.
 *        j: job list
 *        s: starting time
 *        c: completion time
 *
 */
char check_three_cost(sips *prob, int *j, int s, int c)
{
  cost_t f, g;

  /* 0->1->2 */
  f = THREE_COST(0, 1, 2);
  /* 1->2->0 */
  THREE_COST_CHECK(1, 2, 0);
  /* 2->0->1 */
  THREE_COST_CHECK(2, 0, 1);
  /* 2->1->0 */
  THREE_COST_CHECK(2, 1, 0);

  return(0);
}

#define FOUR_COST(j0, j1, j2, j3)					\
  prob->sjob[j[j0]]->f[s + prob->sjob[j[j0]]->p]			\
  + prob->sjob[j[j1]]->f[s + prob->sjob[j[j0]]->p + prob->sjob[j[j1]]->p] \
  + prob->sjob[j[j2]]->f[c - prob->sjob[j[j3]]->p]			\
  + prob->sjob[j[j3]]->f[c]

#define FOUR_COST_CHECK1(j0, j1, j2, j3) {				\
    g = FOUR_COST(j0, j1, j2, j3);					\
    if(obj_lesser(g, f)							\
       || (obj_equal(g, f)						\
	   && prob->sjob[j[j1]]->tno < prob->sjob[j[1]]->tno)) {	\
      return(1);							\
    }									\
  }
#define FOUR_COST_CHECK(j0, j1, j2, j3) {		\
    g = FOUR_COST(j0, j1, j2, j3);			\
    if(obj_lesser_equal(g, f)) {			\
      return(1);					\
    }							\
  }
#define FOUR_COST_STRICT_CHECK(j0, j1, j2, j3) {	\
    g = FOUR_COST(j0, j1, j2, j3);			\
    if(obj_lesser(g, f)) {				\
      return(1);					\
    }							\
  }

/*
 * check_four_cost_forward(prob, j, s, c)
 *   checks whether there exists a four-job sequence which dominates
 *   the specified sequence (in the forward manner).
 *        j: job list
 *        s: starting time
 *        c: completion time
 *
 */
char check_four_cost_forward(sips *prob, int *j, int s, int c)
{
  cost_t f, g;

  /* 0->1->2->3 */
  f = FOUR_COST(0, 1, 2, 3);

  if(prob->sjob[j[1]]->tno < prob->sjob[j[0]]->tno) {
    /* 1->2->0->3 */
    FOUR_COST_CHECK(1, 2, 0, 3);
    /* 1->2->3->0 */
    FOUR_COST_CHECK(1, 2, 3, 0);
    /* 1->3->0->2 */
    FOUR_COST_CHECK(1, 3, 0, 2);
    /* 1->3->2->0 */
    FOUR_COST_CHECK(1, 3, 2, 0);
  } else {
    /* 1->2->0->3 */
    FOUR_COST_STRICT_CHECK(1, 2, 0, 3);
    /* 1->2->3->0 */
    FOUR_COST_STRICT_CHECK(1, 2, 3, 0);
    /* 1->3->0->2 */
    FOUR_COST_STRICT_CHECK(1, 3, 0, 2);
    /* 1->3->2->0 */
    FOUR_COST_STRICT_CHECK(1, 3, 2, 0);
  }

  if(prob->sjob[j[2]]->tno < prob->sjob[j[0]]->tno) {
    /* 2->0->1->3 */
    FOUR_COST_CHECK(2, 0, 1, 3);
    /* 2->0->3->1 */
    FOUR_COST_CHECK(2, 0, 3, 1);
    /* 2->1->0->3 */
    FOUR_COST_CHECK(2, 1, 0, 3);
    /* 2->1->3->0 */
    FOUR_COST_CHECK(2, 1, 3, 0);
    /* 2->3->0->1 */
    FOUR_COST_CHECK(2, 3, 0, 1);
    /* 2->3->1->0 */
    FOUR_COST_CHECK(2, 3, 1, 0);
  } else {
    /* 2->0->1->3 */
    FOUR_COST_STRICT_CHECK(2, 0, 1, 3);
    /* 2->0->3->1 */
    FOUR_COST_STRICT_CHECK(2, 0, 3, 1);
    /* 2->1->0->3 */
    FOUR_COST_STRICT_CHECK(2, 1, 0, 3);
    /* 2->1->3->0 */
    FOUR_COST_STRICT_CHECK(2, 1, 3, 0);
    /* 2->3->0->1 */
    FOUR_COST_STRICT_CHECK(2, 3, 0, 1);
    /* 2->3->1->0 */
    FOUR_COST_STRICT_CHECK(2, 3, 1, 0);
  }

  if(prob->sjob[j[3]]->tno < prob->sjob[j[0]]->tno) {
    /* 3->0->1->2 */
    FOUR_COST_CHECK(3, 0, 1, 2);
    /* 3->0->2->1 */
    FOUR_COST_CHECK(3, 0, 2, 1);
    /* 3->1->0->2 */
    FOUR_COST_CHECK(3, 1, 0, 2);
    /* 3->1->2->0 */
    FOUR_COST_CHECK(3, 1, 2, 0);
    /* 3->2->0->1 */
    FOUR_COST_CHECK(3, 2, 0, 1);
    /* 3->2->1->0 */
    FOUR_COST_CHECK(3, 2, 1, 0);
  } else {
    /* 3->0->1->2 */
    FOUR_COST_STRICT_CHECK(3, 0, 1, 2);
    /* 3->0->2->1 */
    FOUR_COST_STRICT_CHECK(3, 0, 2, 1);
    /* 3->1->0->2 */
    FOUR_COST_STRICT_CHECK(3, 1, 0, 2);
    /* 3->1->2->0 */
    FOUR_COST_STRICT_CHECK(3, 1, 2, 0);
    /* 3->2->0->1 */
    FOUR_COST_STRICT_CHECK(3, 2, 0, 1);
    /* 3->2->1->0 */
    FOUR_COST_STRICT_CHECK(3, 2, 1, 0);
  }

  return(0);
}

/*
 * check_four_cost_backward(prob, j, s, c)
 *   checks whether there exists a four-job sequence which dominates
 *   the specified sequence (in the backward manner).
 *        j: job list
 *        s: starting time
 *        c: completion time
 *
 */
char check_four_cost_backward(sips *prob, int *j, int s, int c)
{
  cost_t f, g;

  /* 0->1->2->3 */
  f = FOUR_COST(0, 1, 2, 3);

  /* 0->2->3->1 */
  FOUR_COST_CHECK1(0, 2, 3, 1);
  /* 0->3->1->2 */
  FOUR_COST_CHECK1(0, 3, 1, 2);
  /* 0->3->2->1 */
  FOUR_COST_CHECK1(0, 3, 2, 1);

  if(prob->sjob[j[1]]->tno < prob->sjob[j[0]]->tno) {
    /* 1->2->3->0 */
    FOUR_COST_CHECK(1, 2, 3, 0);
    /* 1->3->0->2 */
    FOUR_COST_CHECK(1, 3, 0, 2);
    /* 1->3->2->0 */
    FOUR_COST_CHECK(1, 3, 2, 0);
  } else {
    /* 1->2->3->0 */
    FOUR_COST_STRICT_CHECK(1, 2, 3, 0);
    /* 1->3->0->2 */
    FOUR_COST_STRICT_CHECK(1, 3, 0, 2);
    /* 1->3->2->0 */
    FOUR_COST_STRICT_CHECK(1, 3, 2, 0);
  }

  if(prob->sjob[j[2]]->tno < prob->sjob[j[0]]->tno) {
    /* 2->0->3->1 */
    FOUR_COST_CHECK(2, 0, 3, 1);
    /* 2->1->3->0 */
    FOUR_COST_CHECK(2, 1, 3, 0);
    /* 2->3->0->1 */
    FOUR_COST_CHECK(2, 3, 0, 1);
    /* 2->3->1->0 */
    FOUR_COST_CHECK(2, 3, 1, 0);
  } else {
    /* 2->0->3->1 */
    FOUR_COST_STRICT_CHECK(2, 0, 3, 1);
    /* 2->1->3->0 */
    FOUR_COST_STRICT_CHECK(2, 1, 3, 0);
    /* 2->3->0->1 */
    FOUR_COST_STRICT_CHECK(2, 3, 0, 1);
    /* 2->3->1->0 */
    FOUR_COST_STRICT_CHECK(2, 3, 1, 0);
  }

  if(prob->sjob[j[3]]->tno < prob->sjob[j[0]]->tno) {
    /* 3->0->1->2 */
    FOUR_COST_CHECK(3, 0, 1, 2);
    /* 3->0->2->1 */
    FOUR_COST_CHECK(3, 0, 2, 1);
    /* 3->1->0->2 */
    FOUR_COST_CHECK(3, 1, 0, 2);
    /* 3->1->2->0 */
    FOUR_COST_CHECK(3, 1, 2, 0);
    /* 3->2->0->1 */
    FOUR_COST_CHECK(3, 2, 0, 1);
    /* 3->2->1->0 */
    FOUR_COST_CHECK(3, 2, 1, 0);
  } else {
    /* 3->0->1->2 */
    FOUR_COST_STRICT_CHECK(3, 0, 1, 2);
    /* 3->0->2->1 */
    FOUR_COST_STRICT_CHECK(3, 0, 2, 1);
    /* 3->1->0->2 */
    FOUR_COST_STRICT_CHECK(3, 1, 0, 2);
    /* 3->1->2->0 */
    FOUR_COST_STRICT_CHECK(3, 1, 2, 0);
    /* 3->2->0->1 */
    FOUR_COST_STRICT_CHECK(3, 2, 0, 1);
    /* 3->2->1->0 */
    FOUR_COST_STRICT_CHECK(3, 2, 1, 0);
  }

  return(0);
}

#if defined(SIPSI) && !defined(COST_REAL) && !defined(COST_LONGLONG)
#define FIVE_COST(j0, j1, j2, j3, j4)					\
  (unsigned int)							\
  (prob->sjob[j[j0]]->f[s + prob->sjob[j[j0]]->p]			\
   + prob->sjob[j[j1]]->f[s + prob->sjob[j[j0]]->p + prob->sjob[j[j1]]->p] \
   + prob->sjob[j[j2]]->f[c - prob->sjob[j[j3]]->p - prob->sjob[j[j4]]->p]) \
  + (unsigned int) (prob->sjob[j[j3]]->f[c - prob->sjob[j[j4]]->p]	\
		    + prob->sjob[j[j4]]->f[c])
#else /* SIPSI && !COST_REAL && !COST_LONGLONG  */
#define FIVE_COST(j0, j1, j2, j3, j4)					\
  prob->sjob[j[j0]]->f[s + prob->sjob[j[j0]]->p]			\
  + prob->sjob[j[j1]]->f[s + prob->sjob[j[j0]]->p + prob->sjob[j[j1]]->p] \
  + prob->sjob[j[j2]]->f[c - prob->sjob[j[j3]]->p - prob->sjob[j[j4]]->p] \
  + prob->sjob[j[j3]]->f[c - prob->sjob[j[j4]]->p]			\
  + prob->sjob[j[j4]]->f[c]
#endif /* SIPSI && !COST_REAL && !COST_LONGLONG  */

#define FIVE_COST_CHECK2(j0, j1, j2, j3, j4) {				\
    g = FIVE_COST(j0, j1, j2, j3, j4);					\
    if(obj_lesser(g, f)							\
       || (obj_equal(g, f)						\
	   && prob->sjob[j[j2]]->tno < prob->sjob[j[2]]->tno)) {	\
      return(1);							\
    }									\
  }
#define FIVE_COST_CHECK(j0, j1, j2, j3, j4) {		\
    g = FIVE_COST(j0, j1, j2, j3, j4);			\
    if(obj_lesser_equal(g, f)) {			\
      return(1);					\
    }							\
  }
#define FIVE_COST_STRICT_CHECK(j0, j1, j2, j3, j4) {	\
    g = FIVE_COST(j0, j1, j2, j3, j4);			\
    if(obj_lesser(g, f)) {				\
      return(1);					\
    }							\
  }

/*
 * check_five_cost_forward(prob, j, s, c)
 *   checks whether there exists a five-job sequence which dominates
 *   the specified sequence (in the forward manner).
 *        j: job list
 *        s: starting time
 *        c: completion time
 *
 */
char check_five_cost_forward(sips *prob, int *j, int s, int c)
{
#if defined(SIPSI) && !defined(COST_REAL) && !defined(COST_LONGLONG)
  unsigned int f, g;
#else /* SIPSI && !COST_REAL && !COST_LONGLONG */
  cost_t f, g;
#endif /* SIPSI && !COST_REAL && !COST_LONGLONG */

  /* 0->1->2->3->4 */
  f = FIVE_COST(0, 1, 2, 3, 4);

  if(prob->sjob[j[1]]->tno < prob->sjob[j[0]]->tno) {
    /* 1->2->0->3->4 */
    FIVE_COST_CHECK(1, 2, 0, 3, 4);
    /* 1->2->0->4->3 */
    FIVE_COST_CHECK(1, 2, 0, 4, 3);
    /* 1->2->3->0->4 */
    FIVE_COST_CHECK(1, 2, 3, 0, 4);
    /* 1->2->3->4->0 */
    FIVE_COST_CHECK(1, 2, 3, 4, 0);
    /* 1->2->4->0->3 */
    FIVE_COST_CHECK(1, 2, 4, 0, 3);
    /* 1->2->4->3->0 */
    FIVE_COST_CHECK(1, 2, 4, 3, 0);
    /* 1->3->0->2->4 */
    FIVE_COST_CHECK(1, 3, 0, 2, 4);
    /* 1->3->0->4->2 */
    FIVE_COST_CHECK(1, 3, 0, 4, 2);
    /* 1->3->2->0->4 */
    FIVE_COST_CHECK(1, 3, 2, 0, 4);
    /* 1->3->2->4->0 */
    FIVE_COST_CHECK(1, 3, 2, 4, 0);
    /* 1->3->4->0->2 */
    FIVE_COST_CHECK(1, 3, 4, 0, 2);
    /* 1->3->4->2->0 */
    FIVE_COST_CHECK(1, 3, 4, 2, 0);
    /* 1->4->0->2->3 */
    FIVE_COST_CHECK(1, 4, 0, 2, 3);
    /* 1->4->0->3->2 */
    FIVE_COST_CHECK(1, 4, 0, 3, 2);
    /* 1->4->2->0->3 */
    FIVE_COST_CHECK(1, 4, 2, 0, 3);
    /* 1->4->2->3->0 */
    FIVE_COST_CHECK(1, 4, 2, 3, 0);
    /* 1->4->3->0->2 */
    FIVE_COST_CHECK(1, 4, 3, 0, 2);
    /* 1->4->3->2->0 */
    FIVE_COST_CHECK(1, 4, 3, 2, 0);
  } else {
    /* 1->2->0->3->4 */
    FIVE_COST_STRICT_CHECK(1, 2, 0, 3, 4);
    /* 1->2->0->4->3 */
    FIVE_COST_STRICT_CHECK(1, 2, 0, 4, 3);
    /* 1->2->3->0->4 */
    FIVE_COST_STRICT_CHECK(1, 2, 3, 0, 4);
    /* 1->2->3->4->0 */
    FIVE_COST_STRICT_CHECK(1, 2, 3, 4, 0);
    /* 1->2->4->0->3 */
    FIVE_COST_STRICT_CHECK(1, 2, 4, 0, 3);
    /* 1->2->4->3->0 */
    FIVE_COST_STRICT_CHECK(1, 2, 4, 3, 0);
    /* 1->3->0->2->4 */
    FIVE_COST_STRICT_CHECK(1, 3, 0, 2, 4);
    /* 1->3->0->4->2 */
    FIVE_COST_STRICT_CHECK(1, 3, 0, 4, 2);
    /* 1->3->2->0->4 */
    FIVE_COST_STRICT_CHECK(1, 3, 2, 0, 4);
    /* 1->3->2->4->0 */
    FIVE_COST_STRICT_CHECK(1, 3, 2, 4, 0);
    /* 1->3->4->0->2 */
    FIVE_COST_STRICT_CHECK(1, 3, 4, 0, 2);
    /* 1->3->4->2->0 */
    FIVE_COST_STRICT_CHECK(1, 3, 4, 2, 0);
    /* 1->4->0->2->3 */
    FIVE_COST_STRICT_CHECK(1, 4, 0, 2, 3);
    /* 1->4->0->3->2 */
    FIVE_COST_STRICT_CHECK(1, 4, 0, 3, 2);
    /* 1->4->2->0->3 */
    FIVE_COST_STRICT_CHECK(1, 4, 2, 0, 3);
    /* 1->4->2->3->0 */
    FIVE_COST_STRICT_CHECK(1, 4, 2, 3, 0);
    /* 1->4->3->0->2 */
    FIVE_COST_STRICT_CHECK(1, 4, 3, 0, 2);
    /* 1->4->3->2->0 */
    FIVE_COST_STRICT_CHECK(1, 4, 3, 2, 0);
  }

  if(prob->sjob[j[2]]->tno < prob->sjob[j[0]]->tno) {
    /* 2->0->1->3->4 */
    FIVE_COST_CHECK(2, 0, 1, 3, 4);
    /* 2->0->1->4->3 */
    FIVE_COST_CHECK(2, 0, 1, 4, 3);
    /* 2->0->3->1->4 */
    FIVE_COST_CHECK(2, 0, 3, 1, 4);
    /* 2->0->3->4->1 */
    FIVE_COST_CHECK(2, 0, 3, 4, 1);
    /* 2->0->4->1->3 */
    FIVE_COST_CHECK(2, 0, 4, 1, 3);
    /* 2->0->4->3->1 */
    FIVE_COST_CHECK(2, 0, 4, 3, 1);
    /* 2->1->0->3->4 */
    FIVE_COST_CHECK(2, 1, 0, 3, 4);
    /* 2->1->0->4->3 */
    FIVE_COST_CHECK(2, 1, 0, 4, 3);
    /* 2->1->3->0->4 */
    FIVE_COST_CHECK(2, 1, 3, 0, 4);
    /* 2->1->3->4->0 */
    FIVE_COST_CHECK(2, 1, 3, 4, 0);
    /* 2->1->4->0->3 */
    FIVE_COST_CHECK(2, 1, 4, 0, 3);
    /* 2->1->4->3->0 */
    FIVE_COST_CHECK(2, 1, 4, 3, 0);
    /* 2->3->0->1->4 */
    FIVE_COST_CHECK(2, 3, 0, 1, 4);
    /* 2->3->0->4->1 */
    FIVE_COST_CHECK(2, 3, 0, 4, 1);
    /* 2->3->1->0->4 */
    FIVE_COST_CHECK(2, 3, 1, 0, 4);
    /* 2->3->1->4->0 */
    FIVE_COST_CHECK(2, 3, 1, 4, 0);
    /* 2->3->4->0->1 */
    FIVE_COST_CHECK(2, 3, 4, 0, 1);
    /* 2->3->4->1->0 */
    FIVE_COST_CHECK(2, 3, 4, 1, 0);
    /* 2->4->0->1->3 */
    FIVE_COST_CHECK(2, 4, 0, 1, 3);
    /* 2->4->0->3->1 */
    FIVE_COST_CHECK(2, 4, 0, 3, 1);
    /* 2->4->1->0->3 */
    FIVE_COST_CHECK(2, 4, 1, 0, 3);
    /* 2->4->1->3->0 */
    FIVE_COST_CHECK(2, 4, 1, 3, 0);
    /* 2->4->3->0->1 */
    FIVE_COST_CHECK(2, 4, 3, 0, 1);
    /* 2->4->3->1->0 */
    FIVE_COST_CHECK(2, 4, 3, 1, 0);
  } else {
    /* 2->0->1->3->4 */
    FIVE_COST_STRICT_CHECK(2, 0, 1, 3, 4);
    /* 2->0->1->4->3 */
    FIVE_COST_STRICT_CHECK(2, 0, 1, 4, 3);
    /* 2->0->3->1->4 */
    FIVE_COST_STRICT_CHECK(2, 0, 3, 1, 4);
    /* 2->0->3->4->1 */
    FIVE_COST_STRICT_CHECK(2, 0, 3, 4, 1);
    /* 2->0->4->1->3 */
    FIVE_COST_STRICT_CHECK(2, 0, 4, 1, 3);
    /* 2->0->4->3->1 */
    FIVE_COST_STRICT_CHECK(2, 0, 4, 3, 1);
    /* 2->1->0->3->4 */
    FIVE_COST_STRICT_CHECK(2, 1, 0, 3, 4);
    /* 2->1->0->4->3 */
    FIVE_COST_STRICT_CHECK(2, 1, 0, 4, 3);
    /* 2->1->3->0->4 */
    FIVE_COST_STRICT_CHECK(2, 1, 3, 0, 4);
    /* 2->1->3->4->0 */
    FIVE_COST_STRICT_CHECK(2, 1, 3, 4, 0);
    /* 2->1->4->0->3 */
    FIVE_COST_STRICT_CHECK(2, 1, 4, 0, 3);
    /* 2->1->4->3->0 */
    FIVE_COST_STRICT_CHECK(2, 1, 4, 3, 0);
    /* 2->3->0->1->4 */
    FIVE_COST_STRICT_CHECK(2, 3, 0, 1, 4);
    /* 2->3->0->4->1 */
    FIVE_COST_STRICT_CHECK(2, 3, 0, 4, 1);
    /* 2->3->1->0->4 */
    FIVE_COST_STRICT_CHECK(2, 3, 1, 0, 4);
    /* 2->3->1->4->0 */
    FIVE_COST_STRICT_CHECK(2, 3, 1, 4, 0);
    /* 2->3->4->0->1 */
    FIVE_COST_STRICT_CHECK(2, 3, 4, 0, 1);
    /* 2->3->4->1->0 */
    FIVE_COST_STRICT_CHECK(2, 3, 4, 1, 0);
    /* 2->4->0->1->3 */
    FIVE_COST_STRICT_CHECK(2, 4, 0, 1, 3);
    /* 2->4->0->3->1 */
    FIVE_COST_STRICT_CHECK(2, 4, 0, 3, 1);
    /* 2->4->1->0->3 */
    FIVE_COST_STRICT_CHECK(2, 4, 1, 0, 3);
    /* 2->4->1->3->0 */
    FIVE_COST_STRICT_CHECK(2, 4, 1, 3, 0);
    /* 2->4->3->0->1 */
    FIVE_COST_STRICT_CHECK(2, 4, 3, 0, 1);
    /* 2->4->3->1->0 */
    FIVE_COST_STRICT_CHECK(2, 4, 3, 1, 0);
  }

  if(prob->sjob[j[3]]->tno < prob->sjob[j[0]]->tno) {
    /* 3->0->1->2->4 */
    FIVE_COST_CHECK(3, 0, 1, 2, 4);
    /* 3->0->1->4->2 */
    FIVE_COST_CHECK(3, 0, 1, 4, 2);
    /* 3->0->2->1->4 */
    FIVE_COST_CHECK(3, 0, 2, 1, 4);
    /* 3->0->2->4->1 */
    FIVE_COST_CHECK(3, 0, 2, 4, 1);
    /* 3->0->4->1->2 */
    FIVE_COST_CHECK(3, 0, 4, 1, 2);
    /* 3->0->4->2->1 */
    FIVE_COST_CHECK(3, 0, 4, 2, 1);
    /* 3->1->0->2->4 */
    FIVE_COST_CHECK(3, 1, 0, 2, 4);
    /* 3->1->0->4->2 */
    FIVE_COST_CHECK(3, 1, 0, 4, 2);
    /* 3->1->2->0->4 */
    FIVE_COST_CHECK(3, 1, 2, 0, 4);
    /* 3->1->2->4->0 */
    FIVE_COST_CHECK(3, 1, 2, 4, 0);
    /* 3->1->4->0->2 */
    FIVE_COST_CHECK(3, 1, 4, 0, 2);
    /* 3->1->4->2->0 */
    FIVE_COST_CHECK(3, 1, 4, 2, 0);
    /* 3->2->0->1->4 */
    FIVE_COST_CHECK(3, 2, 0, 1, 4);
    /* 3->2->0->4->1 */
    FIVE_COST_CHECK(3, 2, 0, 4, 1);
    /* 3->2->1->0->4 */
    FIVE_COST_CHECK(3, 2, 1, 0, 4);
    /* 3->2->1->4->0 */
    FIVE_COST_CHECK(3, 2, 1, 4, 0);
    /* 3->2->4->0->1 */
    FIVE_COST_CHECK(3, 2, 4, 0, 1);
    /* 3->2->4->1->0 */
    FIVE_COST_CHECK(3, 2, 4, 1, 0);
    /* 3->4->0->1->2 */
    FIVE_COST_CHECK(3, 4, 0, 1, 2);
    /* 3->4->0->2->1 */
    FIVE_COST_CHECK(3, 4, 0, 2, 1);
    /* 3->4->1->0->2 */
    FIVE_COST_CHECK(3, 4, 1, 0, 2);
    /* 3->4->1->2->0 */
    FIVE_COST_CHECK(3, 4, 1, 2, 0);
    /* 3->4->2->0->1 */
    FIVE_COST_CHECK(3, 4, 2, 0, 1);
    /* 3->4->2->1->0 */
    FIVE_COST_CHECK(3, 4, 2, 1, 0);
  } else {
    /* 3->0->1->2->4 */
    FIVE_COST_STRICT_CHECK(3, 0, 1, 2, 4);
    /* 3->0->1->4->2 */
    FIVE_COST_STRICT_CHECK(3, 0, 1, 4, 2);
    /* 3->0->2->1->4 */
    FIVE_COST_STRICT_CHECK(3, 0, 2, 1, 4);
    /* 3->0->2->4->1 */
    FIVE_COST_STRICT_CHECK(3, 0, 2, 4, 1);
    /* 3->0->4->1->2 */
    FIVE_COST_STRICT_CHECK(3, 0, 4, 1, 2);
    /* 3->0->4->2->1 */
    FIVE_COST_STRICT_CHECK(3, 0, 4, 2, 1);
    /* 3->1->0->2->4 */
    FIVE_COST_STRICT_CHECK(3, 1, 0, 2, 4);
    /* 3->1->0->4->2 */
    FIVE_COST_STRICT_CHECK(3, 1, 0, 4, 2);
    /* 3->1->2->0->4 */
    FIVE_COST_STRICT_CHECK(3, 1, 2, 0, 4);
    /* 3->1->2->4->0 */
    FIVE_COST_STRICT_CHECK(3, 1, 2, 4, 0);
    /* 3->1->4->0->2 */
    FIVE_COST_STRICT_CHECK(3, 1, 4, 0, 2);
    /* 3->1->4->2->0 */
    FIVE_COST_STRICT_CHECK(3, 1, 4, 2, 0);
    /* 3->2->0->1->4 */
    FIVE_COST_STRICT_CHECK(3, 2, 0, 1, 4);
    /* 3->2->0->4->1 */
    FIVE_COST_STRICT_CHECK(3, 2, 0, 4, 1);
    /* 3->2->1->0->4 */
    FIVE_COST_STRICT_CHECK(3, 2, 1, 0, 4);
    /* 3->2->1->4->0 */
    FIVE_COST_STRICT_CHECK(3, 2, 1, 4, 0);
    /* 3->2->4->0->1 */
    FIVE_COST_STRICT_CHECK(3, 2, 4, 0, 1);
    /* 3->2->4->1->0 */
    FIVE_COST_STRICT_CHECK(3, 2, 4, 1, 0);
    /* 3->4->0->1->2 */
    FIVE_COST_STRICT_CHECK(3, 4, 0, 1, 2);
    /* 3->4->0->2->1 */
    FIVE_COST_STRICT_CHECK(3, 4, 0, 2, 1);
    /* 3->4->1->0->2 */
    FIVE_COST_STRICT_CHECK(3, 4, 1, 0, 2);
    /* 3->4->1->2->0 */
    FIVE_COST_STRICT_CHECK(3, 4, 1, 2, 0);
    /* 3->4->2->0->1 */
    FIVE_COST_STRICT_CHECK(3, 4, 2, 0, 1);
    /* 3->4->2->1->0 */
    FIVE_COST_STRICT_CHECK(3, 4, 2, 1, 0);
  }

  if(prob->sjob[j[4]]->tno < prob->sjob[j[0]]->tno) {
    /* 4->0->1->2->3 */
    FIVE_COST_CHECK(4, 0, 1, 2, 3);
    /* 4->0->1->3->2 */
    FIVE_COST_CHECK(4, 0, 1, 3, 2);
    /* 4->0->2->1->3 */
    FIVE_COST_CHECK(4, 0, 2, 1, 3);
    /* 4->0->2->3->1 */
    FIVE_COST_CHECK(4, 0, 2, 3, 1);
    /* 4->0->3->1->2 */
    FIVE_COST_CHECK(4, 0, 3, 1, 2);
    /* 4->0->3->2->1 */
    FIVE_COST_CHECK(4, 0, 3, 2, 1);
    /* 4->1->0->2->3 */
    FIVE_COST_CHECK(4, 1, 0, 2, 3);
    /* 4->1->0->3->2 */
    FIVE_COST_CHECK(4, 1, 0, 3, 2);
    /* 4->1->2->0->3 */
    FIVE_COST_CHECK(4, 1, 2, 0, 3);
    /* 4->1->2->3->0 */
    FIVE_COST_CHECK(4, 1, 2, 3, 0);
    /* 4->1->3->0->2 */
    FIVE_COST_CHECK(4, 1, 3, 0, 2);
    /* 4->1->3->2->0 */
    FIVE_COST_CHECK(4, 1, 3, 2, 0);
    /* 4->2->0->1->3 */
    FIVE_COST_CHECK(4, 2, 0, 1, 3);
    /* 4->2->0->3->1 */
    FIVE_COST_CHECK(4, 2, 0, 3, 1);
    /* 4->2->1->0->3 */
    FIVE_COST_CHECK(4, 2, 1, 0, 3);
    /* 4->2->1->3->0 */
    FIVE_COST_CHECK(4, 2, 1, 3, 0);
    /* 4->2->3->0->1 */
    FIVE_COST_CHECK(4, 2, 3, 0, 1);
    /* 4->2->3->1->0 */
    FIVE_COST_CHECK(4, 2, 3, 1, 0);
    /* 4->3->0->1->2 */
    FIVE_COST_CHECK(4, 3, 0, 1, 2);
    /* 4->3->0->2->1 */
    FIVE_COST_CHECK(4, 3, 0, 2, 1);
    /* 4->3->1->0->2 */
    FIVE_COST_CHECK(4, 3, 1, 0, 2);
    /* 4->3->1->2->0 */
    FIVE_COST_CHECK(4, 3, 1, 2, 0);
    /* 4->3->2->0->1 */
    FIVE_COST_CHECK(4, 3, 2, 0, 1);
    /* 4->3->2->1->0 */
    FIVE_COST_CHECK(4, 3, 2, 1, 0);
  } else {
    /* 4->0->1->2->3 */
    FIVE_COST_STRICT_CHECK(4, 0, 1, 2, 3);
    /* 4->0->1->3->2 */
    FIVE_COST_STRICT_CHECK(4, 0, 1, 3, 2);
    /* 4->0->2->1->3 */
    FIVE_COST_STRICT_CHECK(4, 0, 2, 1, 3);
    /* 4->0->2->3->1 */
    FIVE_COST_STRICT_CHECK(4, 0, 2, 3, 1);
    /* 4->0->3->1->2 */
    FIVE_COST_STRICT_CHECK(4, 0, 3, 1, 2);
    /* 4->0->3->2->1 */
    FIVE_COST_STRICT_CHECK(4, 0, 3, 2, 1);
    /* 4->1->0->2->3 */
    FIVE_COST_STRICT_CHECK(4, 1, 0, 2, 3);
    /* 4->1->0->3->2 */
    FIVE_COST_STRICT_CHECK(4, 1, 0, 3, 2);
    /* 4->1->2->0->3 */
    FIVE_COST_STRICT_CHECK(4, 1, 2, 0, 3);
    /* 4->1->2->3->0 */
    FIVE_COST_STRICT_CHECK(4, 1, 2, 3, 0);
    /* 4->1->3->0->2 */
    FIVE_COST_STRICT_CHECK(4, 1, 3, 0, 2);
    /* 4->1->3->2->0 */
    FIVE_COST_STRICT_CHECK(4, 1, 3, 2, 0);
    /* 4->2->0->1->3 */
    FIVE_COST_STRICT_CHECK(4, 2, 0, 1, 3);
    /* 4->2->0->3->1 */
    FIVE_COST_STRICT_CHECK(4, 2, 0, 3, 1);
    /* 4->2->1->0->3 */
    FIVE_COST_STRICT_CHECK(4, 2, 1, 0, 3);
    /* 4->2->1->3->0 */
    FIVE_COST_STRICT_CHECK(4, 2, 1, 3, 0);
    /* 4->2->3->0->1 */
    FIVE_COST_STRICT_CHECK(4, 2, 3, 0, 1);
    /* 4->2->3->1->0 */
    FIVE_COST_STRICT_CHECK(4, 2, 3, 1, 0);
    /* 4->3->0->1->2 */
    FIVE_COST_STRICT_CHECK(4, 3, 0, 1, 2);
    /* 4->3->0->2->1 */
    FIVE_COST_STRICT_CHECK(4, 3, 0, 2, 1);
    /* 4->3->1->0->2 */
    FIVE_COST_STRICT_CHECK(4, 3, 1, 0, 2);
    /* 4->3->1->2->0 */
    FIVE_COST_STRICT_CHECK(4, 3, 1, 2, 0);
    /* 4->3->2->0->1 */
    FIVE_COST_STRICT_CHECK(4, 3, 2, 0, 1);
    /* 4->3->2->1->0 */
    FIVE_COST_STRICT_CHECK(4, 3, 2, 1, 0);
  }

  return(0);
}

/*
 * check_five_cost_backward(prob, j, s, c)
 *   checks whether there exists a five-job sequence which dominates
 *   the specified sequence (in the backward manner).
 *        j: job list
 *        s: starting time
 *        c: completion time
 *
 */
char check_five_cost_backward(sips *prob, int *j, int s, int c)
{
#if defined(SIPSI) && !defined(COST_REAL) && !defined(COST_LONGLONG)
  unsigned int f, g;
#else /* SIPSI && !COST_REAL && !COST_LONGLONG */
  cost_t f, g;
#endif /* SIPSI && !COST_REAL && !COST_LONGLONG */

  /* 0->1->2->3->4 */
  f = FIVE_COST(0, 1, 2, 3, 4);

  /* 0->1->3->4->2 */
  FIVE_COST_CHECK2(0, 1, 3, 4, 2);
  /* 0->1->4->2->3 */
  FIVE_COST_CHECK2(0, 1, 4, 2, 3);
  /* 0->1->4->3->2 */
  FIVE_COST_CHECK2(0, 1, 4, 3, 2);

  if(prob->sjob[j[2]]->tno < prob->sjob[j[1]]->tno) {
    /* 0->2->3->4->1 */
    FIVE_COST_CHECK(0, 2, 3, 4, 1);
    /* 0->2->4->1->3 */
    FIVE_COST_CHECK(0, 2, 4, 1, 3);
    /* 0->2->4->3->1 */
    FIVE_COST_CHECK(0, 2, 4, 3, 1);
  } else {
    /* 0->2->3->4->1 */
    FIVE_COST_STRICT_CHECK(0, 2, 3, 4, 1);
    /* 0->2->4->1->3 */
    FIVE_COST_STRICT_CHECK(0, 2, 4, 1, 3);
    /* 0->2->4->3->1 */
    FIVE_COST_STRICT_CHECK(0, 2, 4, 3, 1);
  }

  if(prob->sjob[j[3]]->tno < prob->sjob[j[1]]->tno) {
    /* 0->3->1->4->2 */
    FIVE_COST_CHECK(0, 3, 1, 4, 2);
    /* 0->3->2->4->1 */
    FIVE_COST_CHECK(0, 3, 2, 4, 1);
    /* 0->3->4->1->2 */
    FIVE_COST_CHECK(0, 3, 4, 1, 2);
    /* 0->3->4->2->1 */
    FIVE_COST_CHECK(0, 3, 4, 2, 1);
  } else {
    /* 0->3->1->4->2 */
    FIVE_COST_STRICT_CHECK(0, 3, 1, 4, 2);
    /* 0->3->2->4->1 */
    FIVE_COST_STRICT_CHECK(0, 3, 2, 4, 1);
    /* 0->3->4->1->2 */
    FIVE_COST_STRICT_CHECK(0, 3, 4, 1, 2);
    /* 0->3->4->2->1 */
    FIVE_COST_STRICT_CHECK(0, 3, 4, 2, 1);
  }

  if(prob->sjob[j[4]]->tno < prob->sjob[j[1]]->tno) {
    /* 0->4->1->2->3 */
    FIVE_COST_CHECK(0, 4, 1, 2, 3);
    /* 0->4->1->3->2 */
    FIVE_COST_CHECK(0, 4, 1, 3, 2);
    /* 0->4->2->1->3 */
    FIVE_COST_CHECK(0, 4, 2, 1, 3);
    /* 0->4->2->3->1 */
    FIVE_COST_CHECK(0, 4, 2, 3, 1);
    /* 0->4->3->1->2 */
    FIVE_COST_CHECK(0, 4, 3, 1, 2);
    /* 0->4->3->2->1 */
    FIVE_COST_CHECK(0, 4, 3, 2, 1);
  } else {
    /* 0->4->1->2->3 */
    FIVE_COST_STRICT_CHECK(0, 4, 1, 2, 3);
    /* 0->4->1->3->2 */
    FIVE_COST_STRICT_CHECK(0, 4, 1, 3, 2);
    /* 0->4->2->1->3 */
    FIVE_COST_STRICT_CHECK(0, 4, 2, 1, 3);
    /* 0->4->2->3->1 */
    FIVE_COST_STRICT_CHECK(0, 4, 2, 3, 1);
    /* 0->4->3->1->2 */
    FIVE_COST_STRICT_CHECK(0, 4, 3, 1, 2);
    /* 0->4->3->2->1 */
    FIVE_COST_STRICT_CHECK(0, 4, 3, 2, 1);
  }

  if(prob->sjob[j[1]]->tno < prob->sjob[j[0]]->tno) {
    /* 1->0->3->4->2 */
    FIVE_COST_CHECK(1, 0, 3, 4, 2);
    /* 1->0->4->2->3 */
    FIVE_COST_CHECK(1, 0, 4, 2, 3);
    /* 1->0->4->3->2 */
    FIVE_COST_CHECK(1, 0, 4, 3, 2);
    /* 1->2->3->4->0 */
    FIVE_COST_CHECK(1, 2, 3, 4, 0);
    /* 1->2->4->0->3 */
    FIVE_COST_CHECK(1, 2, 4, 0, 3);
    /* 1->2->4->3->0 */
    FIVE_COST_CHECK(1, 2, 4, 3, 0);
    /* 1->3->0->4->2 */
    FIVE_COST_CHECK(1, 3, 0, 4, 2);
    /* 1->3->2->4->0 */
    FIVE_COST_CHECK(1, 3, 2, 4, 0);
    /* 1->3->4->0->2 */
    FIVE_COST_CHECK(1, 3, 4, 0, 2);
    /* 1->3->4->2->0 */
    FIVE_COST_CHECK(1, 3, 4, 2, 0);
    /* 1->4->0->2->3 */
    FIVE_COST_CHECK(1, 4, 0, 2, 3);
    /* 1->4->0->3->2 */
    FIVE_COST_CHECK(1, 4, 0, 3, 2);
    /* 1->4->2->0->3 */
    FIVE_COST_CHECK(1, 4, 2, 0, 3);
    /* 1->4->2->3->0 */
    FIVE_COST_CHECK(1, 4, 2, 3, 0);
    /* 1->4->3->0->2 */
    FIVE_COST_CHECK(1, 4, 3, 0, 2);
    /* 1->4->3->2->0 */
    FIVE_COST_CHECK(1, 4, 3, 2, 0);
  } else {
    /* 1->0->3->4->2 */
    FIVE_COST_STRICT_CHECK(1, 0, 3, 4, 2);
    /* 1->0->4->2->3 */
    FIVE_COST_STRICT_CHECK(1, 0, 4, 2, 3);
    /* 1->0->4->3->2 */
    FIVE_COST_STRICT_CHECK(1, 0, 4, 3, 2);
    /* 1->2->3->4->0 */
    FIVE_COST_STRICT_CHECK(1, 2, 3, 4, 0);
    /* 1->2->4->0->3 */
    FIVE_COST_STRICT_CHECK(1, 2, 4, 0, 3);
    /* 1->2->4->3->0 */
    FIVE_COST_STRICT_CHECK(1, 2, 4, 3, 0);
    /* 1->3->0->4->2 */
    FIVE_COST_STRICT_CHECK(1, 3, 0, 4, 2);
    /* 1->3->2->4->0 */
    FIVE_COST_STRICT_CHECK(1, 3, 2, 4, 0);
    /* 1->3->4->0->2 */
    FIVE_COST_STRICT_CHECK(1, 3, 4, 0, 2);
    /* 1->3->4->2->0 */
    FIVE_COST_STRICT_CHECK(1, 3, 4, 2, 0);
    /* 1->4->0->2->3 */
    FIVE_COST_STRICT_CHECK(1, 4, 0, 2, 3);
    /* 1->4->0->3->2 */
    FIVE_COST_STRICT_CHECK(1, 4, 0, 3, 2);
    /* 1->4->2->0->3 */
    FIVE_COST_STRICT_CHECK(1, 4, 2, 0, 3);
    /* 1->4->2->3->0 */
    FIVE_COST_STRICT_CHECK(1, 4, 2, 3, 0);
    /* 1->4->3->0->2 */
    FIVE_COST_STRICT_CHECK(1, 4, 3, 0, 2);
    /* 1->4->3->2->0 */
    FIVE_COST_STRICT_CHECK(1, 4, 3, 2, 0);
  }

  if(prob->sjob[j[2]]->tno < prob->sjob[j[0]]->tno) {
    /* 2->0->3->4->1 */
    FIVE_COST_CHECK(2, 0, 3, 4, 1);
    /* 2->0->4->1->3 */
    FIVE_COST_CHECK(2, 0, 4, 1, 3);
    /* 2->0->4->3->1 */
    FIVE_COST_CHECK(2, 0, 4, 3, 1);
    /* 2->1->3->4->0 */
    FIVE_COST_CHECK(2, 1, 3, 4, 0);
    /* 2->1->4->0->3 */
    FIVE_COST_CHECK(2, 1, 4, 0, 3);
    /* 2->1->4->3->0 */
    FIVE_COST_CHECK(2, 1, 4, 3, 0);
    /* 2->3->0->4->1 */
    FIVE_COST_CHECK(2, 3, 0, 4, 1);
    /* 2->3->1->4->0 */
    FIVE_COST_CHECK(2, 3, 1, 4, 0);
    /* 2->3->4->0->1 */
    FIVE_COST_CHECK(2, 3, 4, 0, 1);
    /* 2->3->4->1->0 */
    FIVE_COST_CHECK(2, 3, 4, 1, 0);
    /* 2->4->0->1->3 */
    FIVE_COST_CHECK(2, 4, 0, 1, 3);
    /* 2->4->0->3->1 */
    FIVE_COST_CHECK(2, 4, 0, 3, 1);
    /* 2->4->1->0->3 */
    FIVE_COST_CHECK(2, 4, 1, 0, 3);
    /* 2->4->1->3->0 */
    FIVE_COST_CHECK(2, 4, 1, 3, 0);
    /* 2->4->3->0->1 */
    FIVE_COST_CHECK(2, 4, 3, 0, 1);
    /* 2->4->3->1->0 */
    FIVE_COST_CHECK(2, 4, 3, 1, 0);
  } else {
    /* 2->0->3->4->1 */
    FIVE_COST_STRICT_CHECK(2, 0, 3, 4, 1);
    /* 2->0->4->1->3 */
    FIVE_COST_STRICT_CHECK(2, 0, 4, 1, 3);
    /* 2->0->4->3->1 */
    FIVE_COST_STRICT_CHECK(2, 0, 4, 3, 1);
    /* 2->1->3->4->0 */
    FIVE_COST_STRICT_CHECK(2, 1, 3, 4, 0);
    /* 2->1->4->0->3 */
    FIVE_COST_STRICT_CHECK(2, 1, 4, 0, 3);
    /* 2->1->4->3->0 */
    FIVE_COST_STRICT_CHECK(2, 1, 4, 3, 0);
    /* 2->3->0->4->1 */
    FIVE_COST_STRICT_CHECK(2, 3, 0, 4, 1);
    /* 2->3->1->4->0 */
    FIVE_COST_STRICT_CHECK(2, 3, 1, 4, 0);
    /* 2->3->4->0->1 */
    FIVE_COST_STRICT_CHECK(2, 3, 4, 0, 1);
    /* 2->3->4->1->0 */
    FIVE_COST_STRICT_CHECK(2, 3, 4, 1, 0);
    /* 2->4->0->1->3 */
    FIVE_COST_STRICT_CHECK(2, 4, 0, 1, 3);
    /* 2->4->0->3->1 */
    FIVE_COST_STRICT_CHECK(2, 4, 0, 3, 1);
    /* 2->4->1->0->3 */
    FIVE_COST_STRICT_CHECK(2, 4, 1, 0, 3);
    /* 2->4->1->3->0 */
    FIVE_COST_STRICT_CHECK(2, 4, 1, 3, 0);
    /* 2->4->3->0->1 */
    FIVE_COST_STRICT_CHECK(2, 4, 3, 0, 1);
    /* 2->4->3->1->0 */
    FIVE_COST_STRICT_CHECK(2, 4, 3, 1, 0);
  }

  if(prob->sjob[j[3]]->tno < prob->sjob[j[0]]->tno) {
    /* 3->0->1->4->2 */
    FIVE_COST_CHECK(3, 0, 1, 4, 2);
    /* 3->0->2->4->1 */
    FIVE_COST_CHECK(3, 0, 2, 4, 1);
    /* 3->0->4->1->2 */
    FIVE_COST_CHECK(3, 0, 4, 1, 2);
    /* 3->0->4->2->1 */
    FIVE_COST_CHECK(3, 0, 4, 2, 1);
    /* 3->1->0->4->2 */
    FIVE_COST_CHECK(3, 1, 0, 4, 2);
    /* 3->1->2->4->0 */
    FIVE_COST_CHECK(3, 1, 2, 4, 0);
    /* 3->1->4->0->2 */
    FIVE_COST_CHECK(3, 1, 4, 0, 2);
    /* 3->1->4->2->0 */
    FIVE_COST_CHECK(3, 1, 4, 2, 0);
    /* 3->2->0->4->1 */
    FIVE_COST_CHECK(3, 2, 0, 4, 1);
    /* 3->2->1->4->0 */
    FIVE_COST_CHECK(3, 2, 1, 4, 0);
    /* 3->2->4->0->1 */
    FIVE_COST_CHECK(3, 2, 4, 0, 1);
    /* 3->2->4->1->0 */
    FIVE_COST_CHECK(3, 2, 4, 1, 0);
    /* 3->4->0->1->2 */
    FIVE_COST_CHECK(3, 4, 0, 1, 2);
    /* 3->4->0->2->1 */
    FIVE_COST_CHECK(3, 4, 0, 2, 1);
    /* 3->4->1->0->2 */
    FIVE_COST_CHECK(3, 4, 1, 0, 2);
    /* 3->4->1->2->0 */
    FIVE_COST_CHECK(3, 4, 1, 2, 0);
    /* 3->4->2->0->1 */
    FIVE_COST_CHECK(3, 4, 2, 0, 1);
    /* 3->4->2->1->0 */
    FIVE_COST_CHECK(3, 4, 2, 1, 0);
  } else {
    /* 3->0->1->4->2 */
    FIVE_COST_STRICT_CHECK(3, 0, 1, 4, 2);
    /* 3->0->2->4->1 */
    FIVE_COST_STRICT_CHECK(3, 0, 2, 4, 1);
    /* 3->0->4->1->2 */
    FIVE_COST_STRICT_CHECK(3, 0, 4, 1, 2);
    /* 3->0->4->2->1 */
    FIVE_COST_STRICT_CHECK(3, 0, 4, 2, 1);
    /* 3->1->0->4->2 */
    FIVE_COST_STRICT_CHECK(3, 1, 0, 4, 2);
    /* 3->1->2->4->0 */
    FIVE_COST_STRICT_CHECK(3, 1, 2, 4, 0);
    /* 3->1->4->0->2 */
    FIVE_COST_STRICT_CHECK(3, 1, 4, 0, 2);
    /* 3->1->4->2->0 */
    FIVE_COST_STRICT_CHECK(3, 1, 4, 2, 0);
    /* 3->2->0->4->1 */
    FIVE_COST_STRICT_CHECK(3, 2, 0, 4, 1);
    /* 3->2->1->4->0 */
    FIVE_COST_STRICT_CHECK(3, 2, 1, 4, 0);
    /* 3->2->4->0->1 */
    FIVE_COST_STRICT_CHECK(3, 2, 4, 0, 1);
    /* 3->2->4->1->0 */
    FIVE_COST_STRICT_CHECK(3, 2, 4, 1, 0);
    /* 3->4->0->1->2 */
    FIVE_COST_STRICT_CHECK(3, 4, 0, 1, 2);
    /* 3->4->0->2->1 */
    FIVE_COST_STRICT_CHECK(3, 4, 0, 2, 1);
    /* 3->4->1->0->2 */
    FIVE_COST_STRICT_CHECK(3, 4, 1, 0, 2);
    /* 3->4->1->2->0 */
    FIVE_COST_STRICT_CHECK(3, 4, 1, 2, 0);
    /* 3->4->2->0->1 */
    FIVE_COST_STRICT_CHECK(3, 4, 2, 0, 1);
    /* 3->4->2->1->0 */
    FIVE_COST_STRICT_CHECK(3, 4, 2, 1, 0);
  }

  if(prob->sjob[j[4]]->tno < prob->sjob[j[0]]->tno) {
    /* 4->0->1->2->3 */
    FIVE_COST_CHECK(4, 0, 1, 2, 3);
    /* 4->0->1->3->2 */
    FIVE_COST_CHECK(4, 0, 1, 3, 2);
    /* 4->0->2->1->3 */
    FIVE_COST_CHECK(4, 0, 2, 1, 3);
    /* 4->0->2->3->1 */
    FIVE_COST_CHECK(4, 0, 2, 3, 1);
    /* 4->0->3->1->2 */
    FIVE_COST_CHECK(4, 0, 3, 1, 2);
    /* 4->0->3->2->1 */
    FIVE_COST_CHECK(4, 0, 3, 2, 1);
    /* 4->1->0->2->3 */
    FIVE_COST_CHECK(4, 1, 0, 2, 3);
    /* 4->1->0->3->2 */
    FIVE_COST_CHECK(4, 1, 0, 3, 2);
    /* 4->1->2->0->3 */
    FIVE_COST_CHECK(4, 1, 2, 0, 3);
    /* 4->1->2->3->0 */
    FIVE_COST_CHECK(4, 1, 2, 3, 0);
    /* 4->1->3->0->2 */
    FIVE_COST_CHECK(4, 1, 3, 0, 2);
    /* 4->1->3->2->0 */
    FIVE_COST_CHECK(4, 1, 3, 2, 0);
    /* 4->2->0->1->3 */
    FIVE_COST_CHECK(4, 2, 0, 1, 3);
    /* 4->2->0->3->1 */
    FIVE_COST_CHECK(4, 2, 0, 3, 1);
    /* 4->2->1->0->3 */
    FIVE_COST_CHECK(4, 2, 1, 0, 3);
    /* 4->2->1->3->0 */
    FIVE_COST_CHECK(4, 2, 1, 3, 0);
    /* 4->2->3->0->1 */
    FIVE_COST_CHECK(4, 2, 3, 0, 1);
    /* 4->2->3->1->0 */
    FIVE_COST_CHECK(4, 2, 3, 1, 0);
    /* 4->3->0->1->2 */
    FIVE_COST_CHECK(4, 3, 0, 1, 2);
    /* 4->3->0->2->1 */
    FIVE_COST_CHECK(4, 3, 0, 2, 1);
    /* 4->3->1->0->2 */
    FIVE_COST_CHECK(4, 3, 1, 0, 2);
    /* 4->3->1->2->0 */
    FIVE_COST_CHECK(4, 3, 1, 2, 0);
    /* 4->3->2->0->1 */
    FIVE_COST_CHECK(4, 3, 2, 0, 1);
    /* 4->3->2->1->0 */
    FIVE_COST_CHECK(4, 3, 2, 1, 0);
  } else {
    /* 4->0->1->2->3 */
    FIVE_COST_STRICT_CHECK(4, 0, 1, 2, 3);
    /* 4->0->1->3->2 */
    FIVE_COST_STRICT_CHECK(4, 0, 1, 3, 2);
    /* 4->0->2->1->3 */
    FIVE_COST_STRICT_CHECK(4, 0, 2, 1, 3);
    /* 4->0->2->3->1 */
    FIVE_COST_STRICT_CHECK(4, 0, 2, 3, 1);
    /* 4->0->3->1->2 */
    FIVE_COST_STRICT_CHECK(4, 0, 3, 1, 2);
    /* 4->0->3->2->1 */
    FIVE_COST_STRICT_CHECK(4, 0, 3, 2, 1);
    /* 4->1->0->2->3 */
    FIVE_COST_STRICT_CHECK(4, 1, 0, 2, 3);
    /* 4->1->0->3->2 */
    FIVE_COST_STRICT_CHECK(4, 1, 0, 3, 2);
    /* 4->1->2->0->3 */
    FIVE_COST_STRICT_CHECK(4, 1, 2, 0, 3);
    /* 4->1->2->3->0 */
    FIVE_COST_STRICT_CHECK(4, 1, 2, 3, 0);
    /* 4->1->3->0->2 */
    FIVE_COST_STRICT_CHECK(4, 1, 3, 0, 2);
    /* 4->1->3->2->0 */
    FIVE_COST_STRICT_CHECK(4, 1, 3, 2, 0);
    /* 4->2->0->1->3 */
    FIVE_COST_STRICT_CHECK(4, 2, 0, 1, 3);
    /* 4->2->0->3->1 */
    FIVE_COST_STRICT_CHECK(4, 2, 0, 3, 1);
    /* 4->2->1->0->3 */
    FIVE_COST_STRICT_CHECK(4, 2, 1, 0, 3);
    /* 4->2->1->3->0 */
    FIVE_COST_STRICT_CHECK(4, 2, 1, 3, 0);
    /* 4->2->3->0->1 */
    FIVE_COST_STRICT_CHECK(4, 2, 3, 0, 1);
    /* 4->2->3->1->0 */
    FIVE_COST_STRICT_CHECK(4, 2, 3, 1, 0);
    /* 4->3->0->1->2 */
    FIVE_COST_STRICT_CHECK(4, 3, 0, 1, 2);
    /* 4->3->0->2->1 */
    FIVE_COST_STRICT_CHECK(4, 3, 0, 2, 1);
    /* 4->3->1->0->2 */
    FIVE_COST_STRICT_CHECK(4, 3, 1, 0, 2);
    /* 4->3->1->2->0 */
    FIVE_COST_STRICT_CHECK(4, 3, 1, 2, 0);
    /* 4->3->2->0->1 */
    FIVE_COST_STRICT_CHECK(4, 3, 2, 0, 1);
    /* 4->3->2->1->0 */
    FIVE_COST_STRICT_CHECK(4, 3, 2, 1, 0);
  }

  return(0);
}

#if defined(SIPSI) && !defined(COST_REAL) && !defined(COST_LONGLONG)
#define SIX_COST(j0, j1, j2, j3, j4, j5)				\
  (long long int)							\
  (prob->sjob[j[j0]]->f[s + prob->sjob[j[j0]]->p]			\
   + prob->sjob[j[j1]]->f[s + prob->sjob[j[j0]]->p + prob->sjob[j[j1]]->p] \
   + prob->sjob[j[j2]]->f[s + prob->sjob[j[j0]]->p + prob->sjob[j[j1]]->p \
			  + prob->sjob[j[j2]]->p])			\
  + (long long int)							\
  (prob->sjob[j[j3]]->f[c - prob->sjob[j[j4]]->p  - prob->sjob[j[j5]]->p] \
   + prob->sjob[j[j4]]->f[c - prob->sjob[j[j5]]->p]			\
   + prob->sjob[j[j5]]->f[c])
#else /* SIPSI && !COST_REAL && !COST_LONGLONG */
#ifdef COST_LONGLONG
#define SIX_COST(j0, j1, j2, j3, j4, j5)				\
  (unsigned long long)							\
  (prob->sjob[j[j0]]->f[s + prob->sjob[j[j0]]->p]			\
   + prob->sjob[j[j1]]->f[s + prob->sjob[j[j0]]->p + prob->sjob[j[j1]]->p] \
   + prob->sjob[j[j2]]->f[s + prob->sjob[j[j0]]->p + prob->sjob[j[j1]]->p \
			  + prob->sjob[j[j2]]->p])			\
  + (unsigned long long)						\
  (prob->sjob[j[j3]]->f[c - prob->sjob[j[j4]]->p  - prob->sjob[j[j5]]->p] \
   + prob->sjob[j[j4]]->f[c - prob->sjob[j[j5]]->p]			\
   + prob->sjob[j[j5]]->f[c])
#else /* COST_LONGLONG */
#define SIX_COST(j0, j1, j2, j3, j4, j5)				\
  prob->sjob[j[j0]]->f[s + prob->sjob[j[j0]]->p]			 \
  + prob->sjob[j[j1]]->f[s + prob->sjob[j[j0]]->p + prob->sjob[j[j1]]->p] \
  + prob->sjob[j[j2]]->f[s + prob->sjob[j[j0]]->p + prob->sjob[j[j1]]->p \
			 + prob->sjob[j[j2]]->p]			\
  + prob->sjob[j[j3]]->f[c - prob->sjob[j[j4]]->p - prob->sjob[j[j5]]->p] \
  + prob->sjob[j[j4]]->f[c - prob->sjob[j[j5]]->p]			 \
  + prob->sjob[j[j5]]->f[c]
#endif /* COST_LONGLONG */
#endif /* SIPSI && !COST_REAL && !COST_LONGLONG */

#define SIX_COST_CHECK3(j0, j1, j2, j3, j4, j5) {			\
    g = SIX_COST(j0, j1, j2, j3, j4, j5);				\
    if(obj_lesser(g, f)							\
       || (obj_equal(g, f)						\
	   && prob->sjob[j[j3]]->tno < prob->sjob[j[3]]->tno)) {	\
      return(1);							\
    }									\
  }
#define SIX_COST_CHECK(j0, j1, j2, j3, j4, j5) {			\
    g = SIX_COST(j0, j1, j2, j3, j4, j5);				\
    if(obj_lesser_equal(g, f)) {					\
      return(1);							\
    }									\
  }
#define SIX_COST_STRICT_CHECK(j0, j1, j2, j3, j4, j5) {		\
    g = SIX_COST(j0, j1, j2, j3, j4, j5);			\
    if(obj_lesser(g, f)) {					\
      return(1);						\
    }								\
  }

/*
 * check_six_cost_forward(prob, j, s, c)
 *   checks whether there exists a six-job sequence which dominates
 *   the specified sequence (in the forward manner).
 *        j: job list
 *        s: starting time
 *        c: completion time
 *
 */
char check_six_cost_forward(sips *prob, int *j, int s, int c)
{
#if defined(SIPSI) && !defined(COST_REAL) && !defined(COST_LONGLONG)
  long long int f, g;
#else /* SIPSI && !COST_REAL && !COST_LONGLONG */
#ifdef COST_LONGLONG
  unsigned long long f, g;
#else /* COST_LONGLONG */
  cost_t f, g;
#endif /* COST_LONGLONG */
#endif /* SIPSI && !COST_REAL && !COST_LONGLONG */

  /* 0->1->2->3->4->5 */
  f = SIX_COST(0, 1, 2, 3, 4, 5);

  if(prob->sjob[j[1]]->tno < prob->sjob[j[0]]->tno) {
    /* 1->2->0->3->4->5 */
    SIX_COST_CHECK(1, 2, 0, 3, 4, 5);
    /* 1->2->0->3->5->4 */
    SIX_COST_CHECK(1, 2, 0, 3, 5, 4);
    /* 1->2->0->4->3->5 */
    SIX_COST_CHECK(1, 2, 0, 4, 3, 5);
    /* 1->2->0->4->5->3 */
    SIX_COST_CHECK(1, 2, 0, 4, 5, 3);
    /* 1->2->0->5->3->4 */
    SIX_COST_CHECK(1, 2, 0, 5, 3, 4);
    /* 1->2->0->5->4->3 */
    SIX_COST_CHECK(1, 2, 0, 5, 4, 3);
    /* 1->2->3->0->4->5 */
    SIX_COST_CHECK(1, 2, 3, 0, 4, 5);
    /* 1->2->3->0->5->4 */
    SIX_COST_CHECK(1, 2, 3, 0, 5, 4);
    /* 1->2->3->4->0->5 */
    SIX_COST_CHECK(1, 2, 3, 4, 0, 5);
    /* 1->2->3->4->5->0 */
    SIX_COST_CHECK(1, 2, 3, 4, 5, 0);
    /* 1->2->3->5->0->4 */
    SIX_COST_CHECK(1, 2, 3, 5, 0, 4);
    /* 1->2->3->5->4->0 */
    SIX_COST_CHECK(1, 2, 3, 5, 4, 0);
    /* 1->2->4->0->3->5 */
    SIX_COST_CHECK(1, 2, 4, 0, 3, 5);
    /* 1->2->4->0->5->3 */
    SIX_COST_CHECK(1, 2, 4, 0, 5, 3);
    /* 1->2->4->3->0->5 */
    SIX_COST_CHECK(1, 2, 4, 3, 0, 5);
    /* 1->2->4->3->5->0 */
    SIX_COST_CHECK(1, 2, 4, 3, 5, 0);
    /* 1->2->4->5->0->3 */
    SIX_COST_CHECK(1, 2, 4, 5, 0, 3);
    /* 1->2->4->5->3->0 */
    SIX_COST_CHECK(1, 2, 4, 5, 3, 0);
    /* 1->2->5->0->3->4 */
    SIX_COST_CHECK(1, 2, 5, 0, 3, 4);
    /* 1->2->5->0->4->3 */
    SIX_COST_CHECK(1, 2, 5, 0, 4, 3);
    /* 1->2->5->3->0->4 */
    SIX_COST_CHECK(1, 2, 5, 3, 0, 4);
    /* 1->2->5->3->4->0 */
    SIX_COST_CHECK(1, 2, 5, 3, 4, 0);
    /* 1->2->5->4->0->3 */
    SIX_COST_CHECK(1, 2, 5, 4, 0, 3);
    /* 1->2->5->4->3->0 */
    SIX_COST_CHECK(1, 2, 5, 4, 3, 0);
    /* 1->3->0->2->4->5 */
    SIX_COST_CHECK(1, 3, 0, 2, 4, 5);
    /* 1->3->0->2->5->4 */
    SIX_COST_CHECK(1, 3, 0, 2, 5, 4);
    /* 1->3->0->4->2->5 */
    SIX_COST_CHECK(1, 3, 0, 4, 2, 5);
    /* 1->3->0->4->5->2 */
    SIX_COST_CHECK(1, 3, 0, 4, 5, 2);
    /* 1->3->0->5->2->4 */
    SIX_COST_CHECK(1, 3, 0, 5, 2, 4);
    /* 1->3->0->5->4->2 */
    SIX_COST_CHECK(1, 3, 0, 5, 4, 2);
    /* 1->3->2->0->4->5 */
    SIX_COST_CHECK(1, 3, 2, 0, 4, 5);
    /* 1->3->2->0->5->4 */
    SIX_COST_CHECK(1, 3, 2, 0, 5, 4);
    /* 1->3->2->4->0->5 */
    SIX_COST_CHECK(1, 3, 2, 4, 0, 5);
    /* 1->3->2->4->5->0 */
    SIX_COST_CHECK(1, 3, 2, 4, 5, 0);
    /* 1->3->2->5->0->4 */
    SIX_COST_CHECK(1, 3, 2, 5, 0, 4);
    /* 1->3->2->5->4->0 */
    SIX_COST_CHECK(1, 3, 2, 5, 4, 0);
    /* 1->3->4->0->2->5 */
    SIX_COST_CHECK(1, 3, 4, 0, 2, 5);
    /* 1->3->4->0->5->2 */
    SIX_COST_CHECK(1, 3, 4, 0, 5, 2);
    /* 1->3->4->2->0->5 */
    SIX_COST_CHECK(1, 3, 4, 2, 0, 5);
    /* 1->3->4->2->5->0 */
    SIX_COST_CHECK(1, 3, 4, 2, 5, 0);
    /* 1->3->4->5->0->2 */
    SIX_COST_CHECK(1, 3, 4, 5, 0, 2);
    /* 1->3->4->5->2->0 */
    SIX_COST_CHECK(1, 3, 4, 5, 2, 0);
    /* 1->3->5->0->2->4 */
    SIX_COST_CHECK(1, 3, 5, 0, 2, 4);
    /* 1->3->5->0->4->2 */
    SIX_COST_CHECK(1, 3, 5, 0, 4, 2);
    /* 1->3->5->2->0->4 */
    SIX_COST_CHECK(1, 3, 5, 2, 0, 4);
    /* 1->3->5->2->4->0 */
    SIX_COST_CHECK(1, 3, 5, 2, 4, 0);
    /* 1->3->5->4->0->2 */
    SIX_COST_CHECK(1, 3, 5, 4, 0, 2);
    /* 1->3->5->4->2->0 */
    SIX_COST_CHECK(1, 3, 5, 4, 2, 0);
    /* 1->4->0->2->3->5 */
    SIX_COST_CHECK(1, 4, 0, 2, 3, 5);
    /* 1->4->0->2->5->3 */
    SIX_COST_CHECK(1, 4, 0, 2, 5, 3);
    /* 1->4->0->3->2->5 */
    SIX_COST_CHECK(1, 4, 0, 3, 2, 5);
    /* 1->4->0->3->5->2 */
    SIX_COST_CHECK(1, 4, 0, 3, 5, 2);
    /* 1->4->0->5->2->3 */
    SIX_COST_CHECK(1, 4, 0, 5, 2, 3);
    /* 1->4->0->5->3->2 */
    SIX_COST_CHECK(1, 4, 0, 5, 3, 2);
    /* 1->4->2->0->3->5 */
    SIX_COST_CHECK(1, 4, 2, 0, 3, 5);
    /* 1->4->2->0->5->3 */
    SIX_COST_CHECK(1, 4, 2, 0, 5, 3);
    /* 1->4->2->3->0->5 */
    SIX_COST_CHECK(1, 4, 2, 3, 0, 5);
    /* 1->4->2->3->5->0 */
    SIX_COST_CHECK(1, 4, 2, 3, 5, 0);
    /* 1->4->2->5->0->3 */
    SIX_COST_CHECK(1, 4, 2, 5, 0, 3);
    /* 1->4->2->5->3->0 */
    SIX_COST_CHECK(1, 4, 2, 5, 3, 0);
    /* 1->4->3->0->2->5 */
    SIX_COST_CHECK(1, 4, 3, 0, 2, 5);
    /* 1->4->3->0->5->2 */
    SIX_COST_CHECK(1, 4, 3, 0, 5, 2);
    /* 1->4->3->2->0->5 */
    SIX_COST_CHECK(1, 4, 3, 2, 0, 5);
    /* 1->4->3->2->5->0 */
    SIX_COST_CHECK(1, 4, 3, 2, 5, 0);
    /* 1->4->3->5->0->2 */
    SIX_COST_CHECK(1, 4, 3, 5, 0, 2);
    /* 1->4->3->5->2->0 */
    SIX_COST_CHECK(1, 4, 3, 5, 2, 0);
    /* 1->4->5->0->2->3 */
    SIX_COST_CHECK(1, 4, 5, 0, 2, 3);
    /* 1->4->5->0->3->2 */
    SIX_COST_CHECK(1, 4, 5, 0, 3, 2);
    /* 1->4->5->2->0->3 */
    SIX_COST_CHECK(1, 4, 5, 2, 0, 3);
    /* 1->4->5->2->3->0 */
    SIX_COST_CHECK(1, 4, 5, 2, 3, 0);
    /* 1->4->5->3->0->2 */
    SIX_COST_CHECK(1, 4, 5, 3, 0, 2);
    /* 1->4->5->3->2->0 */
    SIX_COST_CHECK(1, 4, 5, 3, 2, 0);
    /* 1->5->0->2->3->4 */
    SIX_COST_CHECK(1, 5, 0, 2, 3, 4);
    /* 1->5->0->2->4->3 */
    SIX_COST_CHECK(1, 5, 0, 2, 4, 3);
    /* 1->5->0->3->2->4 */
    SIX_COST_CHECK(1, 5, 0, 3, 2, 4);
    /* 1->5->0->3->4->2 */
    SIX_COST_CHECK(1, 5, 0, 3, 4, 2);
    /* 1->5->0->4->2->3 */
    SIX_COST_CHECK(1, 5, 0, 4, 2, 3);
    /* 1->5->0->4->3->2 */
    SIX_COST_CHECK(1, 5, 0, 4, 3, 2);
    /* 1->5->2->0->3->4 */
    SIX_COST_CHECK(1, 5, 2, 0, 3, 4);
    /* 1->5->2->0->4->3 */
    SIX_COST_CHECK(1, 5, 2, 0, 4, 3);
    /* 1->5->2->3->0->4 */
    SIX_COST_CHECK(1, 5, 2, 3, 0, 4);
    /* 1->5->2->3->4->0 */
    SIX_COST_CHECK(1, 5, 2, 3, 4, 0);
    /* 1->5->2->4->0->3 */
    SIX_COST_CHECK(1, 5, 2, 4, 0, 3);
    /* 1->5->2->4->3->0 */
    SIX_COST_CHECK(1, 5, 2, 4, 3, 0);
    /* 1->5->3->0->2->4 */
    SIX_COST_CHECK(1, 5, 3, 0, 2, 4);
    /* 1->5->3->0->4->2 */
    SIX_COST_CHECK(1, 5, 3, 0, 4, 2);
    /* 1->5->3->2->0->4 */
    SIX_COST_CHECK(1, 5, 3, 2, 0, 4);
    /* 1->5->3->2->4->0 */
    SIX_COST_CHECK(1, 5, 3, 2, 4, 0);
    /* 1->5->3->4->0->2 */
    SIX_COST_CHECK(1, 5, 3, 4, 0, 2);
    /* 1->5->3->4->2->0 */
    SIX_COST_CHECK(1, 5, 3, 4, 2, 0);
    /* 1->5->4->0->2->3 */
    SIX_COST_CHECK(1, 5, 4, 0, 2, 3);
    /* 1->5->4->0->3->2 */
    SIX_COST_CHECK(1, 5, 4, 0, 3, 2);
    /* 1->5->4->2->0->3 */
    SIX_COST_CHECK(1, 5, 4, 2, 0, 3);
    /* 1->5->4->2->3->0 */
    SIX_COST_CHECK(1, 5, 4, 2, 3, 0);
    /* 1->5->4->3->0->2 */
    SIX_COST_CHECK(1, 5, 4, 3, 0, 2);
    /* 1->5->4->3->2->0 */
    SIX_COST_CHECK(1, 5, 4, 3, 2, 0);
  } else {
    /* 1->2->0->3->4->5 */
    SIX_COST_STRICT_CHECK(1, 2, 0, 3, 4, 5);
    /* 1->2->0->3->5->4 */
    SIX_COST_STRICT_CHECK(1, 2, 0, 3, 5, 4);
    /* 1->2->0->4->3->5 */
    SIX_COST_STRICT_CHECK(1, 2, 0, 4, 3, 5);
    /* 1->2->0->4->5->3 */
    SIX_COST_STRICT_CHECK(1, 2, 0, 4, 5, 3);
    /* 1->2->0->5->3->4 */
    SIX_COST_STRICT_CHECK(1, 2, 0, 5, 3, 4);
    /* 1->2->0->5->4->3 */
    SIX_COST_STRICT_CHECK(1, 2, 0, 5, 4, 3);
    /* 1->2->3->0->4->5 */
    SIX_COST_STRICT_CHECK(1, 2, 3, 0, 4, 5);
    /* 1->2->3->0->5->4 */
    SIX_COST_STRICT_CHECK(1, 2, 3, 0, 5, 4);
    /* 1->2->3->4->0->5 */
    SIX_COST_STRICT_CHECK(1, 2, 3, 4, 0, 5);
    /* 1->2->3->4->5->0 */
    SIX_COST_STRICT_CHECK(1, 2, 3, 4, 5, 0);
    /* 1->2->3->5->0->4 */
    SIX_COST_STRICT_CHECK(1, 2, 3, 5, 0, 4);
    /* 1->2->3->5->4->0 */
    SIX_COST_STRICT_CHECK(1, 2, 3, 5, 4, 0);
    /* 1->2->4->0->3->5 */
    SIX_COST_STRICT_CHECK(1, 2, 4, 0, 3, 5);
    /* 1->2->4->0->5->3 */
    SIX_COST_STRICT_CHECK(1, 2, 4, 0, 5, 3);
    /* 1->2->4->3->0->5 */
    SIX_COST_STRICT_CHECK(1, 2, 4, 3, 0, 5);
    /* 1->2->4->3->5->0 */
    SIX_COST_STRICT_CHECK(1, 2, 4, 3, 5, 0);
    /* 1->2->4->5->0->3 */
    SIX_COST_STRICT_CHECK(1, 2, 4, 5, 0, 3);
    /* 1->2->4->5->3->0 */
    SIX_COST_STRICT_CHECK(1, 2, 4, 5, 3, 0);
    /* 1->2->5->0->3->4 */
    SIX_COST_STRICT_CHECK(1, 2, 5, 0, 3, 4);
    /* 1->2->5->0->4->3 */
    SIX_COST_STRICT_CHECK(1, 2, 5, 0, 4, 3);
    /* 1->2->5->3->0->4 */
    SIX_COST_STRICT_CHECK(1, 2, 5, 3, 0, 4);
    /* 1->2->5->3->4->0 */
    SIX_COST_STRICT_CHECK(1, 2, 5, 3, 4, 0);
    /* 1->2->5->4->0->3 */
    SIX_COST_STRICT_CHECK(1, 2, 5, 4, 0, 3);
    /* 1->2->5->4->3->0 */
    SIX_COST_STRICT_CHECK(1, 2, 5, 4, 3, 0);
    /* 1->3->0->2->4->5 */
    SIX_COST_STRICT_CHECK(1, 3, 0, 2, 4, 5);
    /* 1->3->0->2->5->4 */
    SIX_COST_STRICT_CHECK(1, 3, 0, 2, 5, 4);
    /* 1->3->0->4->2->5 */
    SIX_COST_STRICT_CHECK(1, 3, 0, 4, 2, 5);
    /* 1->3->0->4->5->2 */
    SIX_COST_STRICT_CHECK(1, 3, 0, 4, 5, 2);
    /* 1->3->0->5->2->4 */
    SIX_COST_STRICT_CHECK(1, 3, 0, 5, 2, 4);
    /* 1->3->0->5->4->2 */
    SIX_COST_STRICT_CHECK(1, 3, 0, 5, 4, 2);
    /* 1->3->2->0->4->5 */
    SIX_COST_STRICT_CHECK(1, 3, 2, 0, 4, 5);
    /* 1->3->2->0->5->4 */
    SIX_COST_STRICT_CHECK(1, 3, 2, 0, 5, 4);
    /* 1->3->2->4->0->5 */
    SIX_COST_STRICT_CHECK(1, 3, 2, 4, 0, 5);
    /* 1->3->2->4->5->0 */
    SIX_COST_STRICT_CHECK(1, 3, 2, 4, 5, 0);
    /* 1->3->2->5->0->4 */
    SIX_COST_STRICT_CHECK(1, 3, 2, 5, 0, 4);
    /* 1->3->2->5->4->0 */
    SIX_COST_STRICT_CHECK(1, 3, 2, 5, 4, 0);
    /* 1->3->4->0->2->5 */
    SIX_COST_STRICT_CHECK(1, 3, 4, 0, 2, 5);
    /* 1->3->4->0->5->2 */
    SIX_COST_STRICT_CHECK(1, 3, 4, 0, 5, 2);
    /* 1->3->4->2->0->5 */
    SIX_COST_STRICT_CHECK(1, 3, 4, 2, 0, 5);
    /* 1->3->4->2->5->0 */
    SIX_COST_STRICT_CHECK(1, 3, 4, 2, 5, 0);
    /* 1->3->4->5->0->2 */
    SIX_COST_STRICT_CHECK(1, 3, 4, 5, 0, 2);
    /* 1->3->4->5->2->0 */
    SIX_COST_STRICT_CHECK(1, 3, 4, 5, 2, 0);
    /* 1->3->5->0->2->4 */
    SIX_COST_STRICT_CHECK(1, 3, 5, 0, 2, 4);
    /* 1->3->5->0->4->2 */
    SIX_COST_STRICT_CHECK(1, 3, 5, 0, 4, 2);
    /* 1->3->5->2->0->4 */
    SIX_COST_STRICT_CHECK(1, 3, 5, 2, 0, 4);
    /* 1->3->5->2->4->0 */
    SIX_COST_STRICT_CHECK(1, 3, 5, 2, 4, 0);
    /* 1->3->5->4->0->2 */
    SIX_COST_STRICT_CHECK(1, 3, 5, 4, 0, 2);
    /* 1->3->5->4->2->0 */
    SIX_COST_STRICT_CHECK(1, 3, 5, 4, 2, 0);
    /* 1->4->0->2->3->5 */
    SIX_COST_STRICT_CHECK(1, 4, 0, 2, 3, 5);
    /* 1->4->0->2->5->3 */
    SIX_COST_STRICT_CHECK(1, 4, 0, 2, 5, 3);
    /* 1->4->0->3->2->5 */
    SIX_COST_STRICT_CHECK(1, 4, 0, 3, 2, 5);
    /* 1->4->0->3->5->2 */
    SIX_COST_STRICT_CHECK(1, 4, 0, 3, 5, 2);
    /* 1->4->0->5->2->3 */
    SIX_COST_STRICT_CHECK(1, 4, 0, 5, 2, 3);
    /* 1->4->0->5->3->2 */
    SIX_COST_STRICT_CHECK(1, 4, 0, 5, 3, 2);
    /* 1->4->2->0->3->5 */
    SIX_COST_STRICT_CHECK(1, 4, 2, 0, 3, 5);
    /* 1->4->2->0->5->3 */
    SIX_COST_STRICT_CHECK(1, 4, 2, 0, 5, 3);
    /* 1->4->2->3->0->5 */
    SIX_COST_STRICT_CHECK(1, 4, 2, 3, 0, 5);
    /* 1->4->2->3->5->0 */
    SIX_COST_STRICT_CHECK(1, 4, 2, 3, 5, 0);
    /* 1->4->2->5->0->3 */
    SIX_COST_STRICT_CHECK(1, 4, 2, 5, 0, 3);
    /* 1->4->2->5->3->0 */
    SIX_COST_STRICT_CHECK(1, 4, 2, 5, 3, 0);
    /* 1->4->3->0->2->5 */
    SIX_COST_STRICT_CHECK(1, 4, 3, 0, 2, 5);
    /* 1->4->3->0->5->2 */
    SIX_COST_STRICT_CHECK(1, 4, 3, 0, 5, 2);
    /* 1->4->3->2->0->5 */
    SIX_COST_STRICT_CHECK(1, 4, 3, 2, 0, 5);
    /* 1->4->3->2->5->0 */
    SIX_COST_STRICT_CHECK(1, 4, 3, 2, 5, 0);
    /* 1->4->3->5->0->2 */
    SIX_COST_STRICT_CHECK(1, 4, 3, 5, 0, 2);
    /* 1->4->3->5->2->0 */
    SIX_COST_STRICT_CHECK(1, 4, 3, 5, 2, 0);
    /* 1->4->5->0->2->3 */
    SIX_COST_STRICT_CHECK(1, 4, 5, 0, 2, 3);
    /* 1->4->5->0->3->2 */
    SIX_COST_STRICT_CHECK(1, 4, 5, 0, 3, 2);
    /* 1->4->5->2->0->3 */
    SIX_COST_STRICT_CHECK(1, 4, 5, 2, 0, 3);
    /* 1->4->5->2->3->0 */
    SIX_COST_STRICT_CHECK(1, 4, 5, 2, 3, 0);
    /* 1->4->5->3->0->2 */
    SIX_COST_STRICT_CHECK(1, 4, 5, 3, 0, 2);
    /* 1->4->5->3->2->0 */
    SIX_COST_STRICT_CHECK(1, 4, 5, 3, 2, 0);
    /* 1->5->0->2->3->4 */
    SIX_COST_STRICT_CHECK(1, 5, 0, 2, 3, 4);
    /* 1->5->0->2->4->3 */
    SIX_COST_STRICT_CHECK(1, 5, 0, 2, 4, 3);
    /* 1->5->0->3->2->4 */
    SIX_COST_STRICT_CHECK(1, 5, 0, 3, 2, 4);
    /* 1->5->0->3->4->2 */
    SIX_COST_STRICT_CHECK(1, 5, 0, 3, 4, 2);
    /* 1->5->0->4->2->3 */
    SIX_COST_STRICT_CHECK(1, 5, 0, 4, 2, 3);
    /* 1->5->0->4->3->2 */
    SIX_COST_STRICT_CHECK(1, 5, 0, 4, 3, 2);
    /* 1->5->2->0->3->4 */
    SIX_COST_STRICT_CHECK(1, 5, 2, 0, 3, 4);
    /* 1->5->2->0->4->3 */
    SIX_COST_STRICT_CHECK(1, 5, 2, 0, 4, 3);
    /* 1->5->2->3->0->4 */
    SIX_COST_STRICT_CHECK(1, 5, 2, 3, 0, 4);
    /* 1->5->2->3->4->0 */
    SIX_COST_STRICT_CHECK(1, 5, 2, 3, 4, 0);
    /* 1->5->2->4->0->3 */
    SIX_COST_STRICT_CHECK(1, 5, 2, 4, 0, 3);
    /* 1->5->2->4->3->0 */
    SIX_COST_STRICT_CHECK(1, 5, 2, 4, 3, 0);
    /* 1->5->3->0->2->4 */
    SIX_COST_STRICT_CHECK(1, 5, 3, 0, 2, 4);
    /* 1->5->3->0->4->2 */
    SIX_COST_STRICT_CHECK(1, 5, 3, 0, 4, 2);
    /* 1->5->3->2->0->4 */
    SIX_COST_STRICT_CHECK(1, 5, 3, 2, 0, 4);
    /* 1->5->3->2->4->0 */
    SIX_COST_STRICT_CHECK(1, 5, 3, 2, 4, 0);
    /* 1->5->3->4->0->2 */
    SIX_COST_STRICT_CHECK(1, 5, 3, 4, 0, 2);
    /* 1->5->3->4->2->0 */
    SIX_COST_STRICT_CHECK(1, 5, 3, 4, 2, 0);
    /* 1->5->4->0->2->3 */
    SIX_COST_STRICT_CHECK(1, 5, 4, 0, 2, 3);
    /* 1->5->4->0->3->2 */
    SIX_COST_STRICT_CHECK(1, 5, 4, 0, 3, 2);
    /* 1->5->4->2->0->3 */
    SIX_COST_STRICT_CHECK(1, 5, 4, 2, 0, 3);
    /* 1->5->4->2->3->0 */
    SIX_COST_STRICT_CHECK(1, 5, 4, 2, 3, 0);
    /* 1->5->4->3->0->2 */
    SIX_COST_STRICT_CHECK(1, 5, 4, 3, 0, 2);
    /* 1->5->4->3->2->0 */
    SIX_COST_STRICT_CHECK(1, 5, 4, 3, 2, 0);
  }

  if(prob->sjob[j[2]]->tno < prob->sjob[j[0]]->tno) {
    /* 2->0->1->3->4->5 */
    SIX_COST_CHECK(2, 0, 1, 3, 4, 5);
    /* 2->0->1->3->5->4 */
    SIX_COST_CHECK(2, 0, 1, 3, 5, 4);
    /* 2->0->1->4->3->5 */
    SIX_COST_CHECK(2, 0, 1, 4, 3, 5);
    /* 2->0->1->4->5->3 */
    SIX_COST_CHECK(2, 0, 1, 4, 5, 3);
    /* 2->0->1->5->3->4 */
    SIX_COST_CHECK(2, 0, 1, 5, 3, 4);
    /* 2->0->1->5->4->3 */
    SIX_COST_CHECK(2, 0, 1, 5, 4, 3);
    /* 2->0->3->1->4->5 */
    SIX_COST_CHECK(2, 0, 3, 1, 4, 5);
    /* 2->0->3->1->5->4 */
    SIX_COST_CHECK(2, 0, 3, 1, 5, 4);
    /* 2->0->3->4->1->5 */
    SIX_COST_CHECK(2, 0, 3, 4, 1, 5);
    /* 2->0->3->4->5->1 */
    SIX_COST_CHECK(2, 0, 3, 4, 5, 1);
    /* 2->0->3->5->1->4 */
    SIX_COST_CHECK(2, 0, 3, 5, 1, 4);
    /* 2->0->3->5->4->1 */
    SIX_COST_CHECK(2, 0, 3, 5, 4, 1);
    /* 2->0->4->1->3->5 */
    SIX_COST_CHECK(2, 0, 4, 1, 3, 5);
    /* 2->0->4->1->5->3 */
    SIX_COST_CHECK(2, 0, 4, 1, 5, 3);
    /* 2->0->4->3->1->5 */
    SIX_COST_CHECK(2, 0, 4, 3, 1, 5);
    /* 2->0->4->3->5->1 */
    SIX_COST_CHECK(2, 0, 4, 3, 5, 1);
    /* 2->0->4->5->1->3 */
    SIX_COST_CHECK(2, 0, 4, 5, 1, 3);
    /* 2->0->4->5->3->1 */
    SIX_COST_CHECK(2, 0, 4, 5, 3, 1);
    /* 2->0->5->1->3->4 */
    SIX_COST_CHECK(2, 0, 5, 1, 3, 4);
    /* 2->0->5->1->4->3 */
    SIX_COST_CHECK(2, 0, 5, 1, 4, 3);
    /* 2->0->5->3->1->4 */
    SIX_COST_CHECK(2, 0, 5, 3, 1, 4);
    /* 2->0->5->3->4->1 */
    SIX_COST_CHECK(2, 0, 5, 3, 4, 1);
    /* 2->0->5->4->1->3 */
    SIX_COST_CHECK(2, 0, 5, 4, 1, 3);
    /* 2->0->5->4->3->1 */
    SIX_COST_CHECK(2, 0, 5, 4, 3, 1);
    /* 2->1->0->3->4->5 */
    SIX_COST_CHECK(2, 1, 0, 3, 4, 5);
    /* 2->1->0->3->5->4 */
    SIX_COST_CHECK(2, 1, 0, 3, 5, 4);
    /* 2->1->0->4->3->5 */
    SIX_COST_CHECK(2, 1, 0, 4, 3, 5);
    /* 2->1->0->4->5->3 */
    SIX_COST_CHECK(2, 1, 0, 4, 5, 3);
    /* 2->1->0->5->3->4 */
    SIX_COST_CHECK(2, 1, 0, 5, 3, 4);
    /* 2->1->0->5->4->3 */
    SIX_COST_CHECK(2, 1, 0, 5, 4, 3);
    /* 2->1->3->0->4->5 */
    SIX_COST_CHECK(2, 1, 3, 0, 4, 5);
    /* 2->1->3->0->5->4 */
    SIX_COST_CHECK(2, 1, 3, 0, 5, 4);
    /* 2->1->3->4->0->5 */
    SIX_COST_CHECK(2, 1, 3, 4, 0, 5);
    /* 2->1->3->4->5->0 */
    SIX_COST_CHECK(2, 1, 3, 4, 5, 0);
    /* 2->1->3->5->0->4 */
    SIX_COST_CHECK(2, 1, 3, 5, 0, 4);
    /* 2->1->3->5->4->0 */
    SIX_COST_CHECK(2, 1, 3, 5, 4, 0);
    /* 2->1->4->0->3->5 */
    SIX_COST_CHECK(2, 1, 4, 0, 3, 5);
    /* 2->1->4->0->5->3 */
    SIX_COST_CHECK(2, 1, 4, 0, 5, 3);
    /* 2->1->4->3->0->5 */
    SIX_COST_CHECK(2, 1, 4, 3, 0, 5);
    /* 2->1->4->3->5->0 */
    SIX_COST_CHECK(2, 1, 4, 3, 5, 0);
    /* 2->1->4->5->0->3 */
    SIX_COST_CHECK(2, 1, 4, 5, 0, 3);
    /* 2->1->4->5->3->0 */
    SIX_COST_CHECK(2, 1, 4, 5, 3, 0);
    /* 2->1->5->0->3->4 */
    SIX_COST_CHECK(2, 1, 5, 0, 3, 4);
    /* 2->1->5->0->4->3 */
    SIX_COST_CHECK(2, 1, 5, 0, 4, 3);
    /* 2->1->5->3->0->4 */
    SIX_COST_CHECK(2, 1, 5, 3, 0, 4);
    /* 2->1->5->3->4->0 */
    SIX_COST_CHECK(2, 1, 5, 3, 4, 0);
    /* 2->1->5->4->0->3 */
    SIX_COST_CHECK(2, 1, 5, 4, 0, 3);
    /* 2->1->5->4->3->0 */
    SIX_COST_CHECK(2, 1, 5, 4, 3, 0);
    /* 2->3->0->1->4->5 */
    SIX_COST_CHECK(2, 3, 0, 1, 4, 5);
    /* 2->3->0->1->5->4 */
    SIX_COST_CHECK(2, 3, 0, 1, 5, 4);
    /* 2->3->0->4->1->5 */
    SIX_COST_CHECK(2, 3, 0, 4, 1, 5);
    /* 2->3->0->4->5->1 */
    SIX_COST_CHECK(2, 3, 0, 4, 5, 1);
    /* 2->3->0->5->1->4 */
    SIX_COST_CHECK(2, 3, 0, 5, 1, 4);
    /* 2->3->0->5->4->1 */
    SIX_COST_CHECK(2, 3, 0, 5, 4, 1);
    /* 2->3->1->0->4->5 */
    SIX_COST_CHECK(2, 3, 1, 0, 4, 5);
    /* 2->3->1->0->5->4 */
    SIX_COST_CHECK(2, 3, 1, 0, 5, 4);
    /* 2->3->1->4->0->5 */
    SIX_COST_CHECK(2, 3, 1, 4, 0, 5);
    /* 2->3->1->4->5->0 */
    SIX_COST_CHECK(2, 3, 1, 4, 5, 0);
    /* 2->3->1->5->0->4 */
    SIX_COST_CHECK(2, 3, 1, 5, 0, 4);
    /* 2->3->1->5->4->0 */
    SIX_COST_CHECK(2, 3, 1, 5, 4, 0);
    /* 2->3->4->0->1->5 */
    SIX_COST_CHECK(2, 3, 4, 0, 1, 5);
    /* 2->3->4->0->5->1 */
    SIX_COST_CHECK(2, 3, 4, 0, 5, 1);
    /* 2->3->4->1->0->5 */
    SIX_COST_CHECK(2, 3, 4, 1, 0, 5);
    /* 2->3->4->1->5->0 */
    SIX_COST_CHECK(2, 3, 4, 1, 5, 0);
    /* 2->3->4->5->0->1 */
    SIX_COST_CHECK(2, 3, 4, 5, 0, 1);
    /* 2->3->4->5->1->0 */
    SIX_COST_CHECK(2, 3, 4, 5, 1, 0);
    /* 2->3->5->0->1->4 */
    SIX_COST_CHECK(2, 3, 5, 0, 1, 4);
    /* 2->3->5->0->4->1 */
    SIX_COST_CHECK(2, 3, 5, 0, 4, 1);
    /* 2->3->5->1->0->4 */
    SIX_COST_CHECK(2, 3, 5, 1, 0, 4);
    /* 2->3->5->1->4->0 */
    SIX_COST_CHECK(2, 3, 5, 1, 4, 0);
    /* 2->3->5->4->0->1 */
    SIX_COST_CHECK(2, 3, 5, 4, 0, 1);
    /* 2->3->5->4->1->0 */
    SIX_COST_CHECK(2, 3, 5, 4, 1, 0);
    /* 2->4->0->1->3->5 */
    SIX_COST_CHECK(2, 4, 0, 1, 3, 5);
    /* 2->4->0->1->5->3 */
    SIX_COST_CHECK(2, 4, 0, 1, 5, 3);
    /* 2->4->0->3->1->5 */
    SIX_COST_CHECK(2, 4, 0, 3, 1, 5);
    /* 2->4->0->3->5->1 */
    SIX_COST_CHECK(2, 4, 0, 3, 5, 1);
    /* 2->4->0->5->1->3 */
    SIX_COST_CHECK(2, 4, 0, 5, 1, 3);
    /* 2->4->0->5->3->1 */
    SIX_COST_CHECK(2, 4, 0, 5, 3, 1);
    /* 2->4->1->0->3->5 */
    SIX_COST_CHECK(2, 4, 1, 0, 3, 5);
    /* 2->4->1->0->5->3 */
    SIX_COST_CHECK(2, 4, 1, 0, 5, 3);
    /* 2->4->1->3->0->5 */
    SIX_COST_CHECK(2, 4, 1, 3, 0, 5);
    /* 2->4->1->3->5->0 */
    SIX_COST_CHECK(2, 4, 1, 3, 5, 0);
    /* 2->4->1->5->0->3 */
    SIX_COST_CHECK(2, 4, 1, 5, 0, 3);
    /* 2->4->1->5->3->0 */
    SIX_COST_CHECK(2, 4, 1, 5, 3, 0);
    /* 2->4->3->0->1->5 */
    SIX_COST_CHECK(2, 4, 3, 0, 1, 5);
    /* 2->4->3->0->5->1 */
    SIX_COST_CHECK(2, 4, 3, 0, 5, 1);
    /* 2->4->3->1->0->5 */
    SIX_COST_CHECK(2, 4, 3, 1, 0, 5);
    /* 2->4->3->1->5->0 */
    SIX_COST_CHECK(2, 4, 3, 1, 5, 0);
    /* 2->4->3->5->0->1 */
    SIX_COST_CHECK(2, 4, 3, 5, 0, 1);
    /* 2->4->3->5->1->0 */
    SIX_COST_CHECK(2, 4, 3, 5, 1, 0);
    /* 2->4->5->0->1->3 */
    SIX_COST_CHECK(2, 4, 5, 0, 1, 3);
    /* 2->4->5->0->3->1 */
    SIX_COST_CHECK(2, 4, 5, 0, 3, 1);
    /* 2->4->5->1->0->3 */
    SIX_COST_CHECK(2, 4, 5, 1, 0, 3);
    /* 2->4->5->1->3->0 */
    SIX_COST_CHECK(2, 4, 5, 1, 3, 0);
    /* 2->4->5->3->0->1 */
    SIX_COST_CHECK(2, 4, 5, 3, 0, 1);
    /* 2->4->5->3->1->0 */
    SIX_COST_CHECK(2, 4, 5, 3, 1, 0);
    /* 2->5->0->1->3->4 */
    SIX_COST_CHECK(2, 5, 0, 1, 3, 4);
    /* 2->5->0->1->4->3 */
    SIX_COST_CHECK(2, 5, 0, 1, 4, 3);
    /* 2->5->0->3->1->4 */
    SIX_COST_CHECK(2, 5, 0, 3, 1, 4);
    /* 2->5->0->3->4->1 */
    SIX_COST_CHECK(2, 5, 0, 3, 4, 1);
    /* 2->5->0->4->1->3 */
    SIX_COST_CHECK(2, 5, 0, 4, 1, 3);
    /* 2->5->0->4->3->1 */
    SIX_COST_CHECK(2, 5, 0, 4, 3, 1);
    /* 2->5->1->0->3->4 */
    SIX_COST_CHECK(2, 5, 1, 0, 3, 4);
    /* 2->5->1->0->4->3 */
    SIX_COST_CHECK(2, 5, 1, 0, 4, 3);
    /* 2->5->1->3->0->4 */
    SIX_COST_CHECK(2, 5, 1, 3, 0, 4);
    /* 2->5->1->3->4->0 */
    SIX_COST_CHECK(2, 5, 1, 3, 4, 0);
    /* 2->5->1->4->0->3 */
    SIX_COST_CHECK(2, 5, 1, 4, 0, 3);
    /* 2->5->1->4->3->0 */
    SIX_COST_CHECK(2, 5, 1, 4, 3, 0);
    /* 2->5->3->0->1->4 */
    SIX_COST_CHECK(2, 5, 3, 0, 1, 4);
    /* 2->5->3->0->4->1 */
    SIX_COST_CHECK(2, 5, 3, 0, 4, 1);
    /* 2->5->3->1->0->4 */
    SIX_COST_CHECK(2, 5, 3, 1, 0, 4);
    /* 2->5->3->1->4->0 */
    SIX_COST_CHECK(2, 5, 3, 1, 4, 0);
    /* 2->5->3->4->0->1 */
    SIX_COST_CHECK(2, 5, 3, 4, 0, 1);
    /* 2->5->3->4->1->0 */
    SIX_COST_CHECK(2, 5, 3, 4, 1, 0);
    /* 2->5->4->0->1->3 */
    SIX_COST_CHECK(2, 5, 4, 0, 1, 3);
    /* 2->5->4->0->3->1 */
    SIX_COST_CHECK(2, 5, 4, 0, 3, 1);
    /* 2->5->4->1->0->3 */
    SIX_COST_CHECK(2, 5, 4, 1, 0, 3);
    /* 2->5->4->1->3->0 */
    SIX_COST_CHECK(2, 5, 4, 1, 3, 0);
    /* 2->5->4->3->0->1 */
    SIX_COST_CHECK(2, 5, 4, 3, 0, 1);
    /* 2->5->4->3->1->0 */
    SIX_COST_CHECK(2, 5, 4, 3, 1, 0);
  } else {
    /* 2->0->1->3->4->5 */
    SIX_COST_STRICT_CHECK(2, 0, 1, 3, 4, 5);
    /* 2->0->1->3->5->4 */
    SIX_COST_STRICT_CHECK(2, 0, 1, 3, 5, 4);
    /* 2->0->1->4->3->5 */
    SIX_COST_STRICT_CHECK(2, 0, 1, 4, 3, 5);
    /* 2->0->1->4->5->3 */
    SIX_COST_STRICT_CHECK(2, 0, 1, 4, 5, 3);
    /* 2->0->1->5->3->4 */
    SIX_COST_STRICT_CHECK(2, 0, 1, 5, 3, 4);
    /* 2->0->1->5->4->3 */
    SIX_COST_STRICT_CHECK(2, 0, 1, 5, 4, 3);
    /* 2->0->3->1->4->5 */
    SIX_COST_STRICT_CHECK(2, 0, 3, 1, 4, 5);
    /* 2->0->3->1->5->4 */
    SIX_COST_STRICT_CHECK(2, 0, 3, 1, 5, 4);
    /* 2->0->3->4->1->5 */
    SIX_COST_STRICT_CHECK(2, 0, 3, 4, 1, 5);
    /* 2->0->3->4->5->1 */
    SIX_COST_STRICT_CHECK(2, 0, 3, 4, 5, 1);
    /* 2->0->3->5->1->4 */
    SIX_COST_STRICT_CHECK(2, 0, 3, 5, 1, 4);
    /* 2->0->3->5->4->1 */
    SIX_COST_STRICT_CHECK(2, 0, 3, 5, 4, 1);
    /* 2->0->4->1->3->5 */
    SIX_COST_STRICT_CHECK(2, 0, 4, 1, 3, 5);
    /* 2->0->4->1->5->3 */
    SIX_COST_STRICT_CHECK(2, 0, 4, 1, 5, 3);
    /* 2->0->4->3->1->5 */
    SIX_COST_STRICT_CHECK(2, 0, 4, 3, 1, 5);
    /* 2->0->4->3->5->1 */
    SIX_COST_STRICT_CHECK(2, 0, 4, 3, 5, 1);
    /* 2->0->4->5->1->3 */
    SIX_COST_STRICT_CHECK(2, 0, 4, 5, 1, 3);
    /* 2->0->4->5->3->1 */
    SIX_COST_STRICT_CHECK(2, 0, 4, 5, 3, 1);
    /* 2->0->5->1->3->4 */
    SIX_COST_STRICT_CHECK(2, 0, 5, 1, 3, 4);
    /* 2->0->5->1->4->3 */
    SIX_COST_STRICT_CHECK(2, 0, 5, 1, 4, 3);
    /* 2->0->5->3->1->4 */
    SIX_COST_STRICT_CHECK(2, 0, 5, 3, 1, 4);
    /* 2->0->5->3->4->1 */
    SIX_COST_STRICT_CHECK(2, 0, 5, 3, 4, 1);
    /* 2->0->5->4->1->3 */
    SIX_COST_STRICT_CHECK(2, 0, 5, 4, 1, 3);
    /* 2->0->5->4->3->1 */
    SIX_COST_STRICT_CHECK(2, 0, 5, 4, 3, 1);
    /* 2->1->0->3->4->5 */
    SIX_COST_STRICT_CHECK(2, 1, 0, 3, 4, 5);
    /* 2->1->0->3->5->4 */
    SIX_COST_STRICT_CHECK(2, 1, 0, 3, 5, 4);
    /* 2->1->0->4->3->5 */
    SIX_COST_STRICT_CHECK(2, 1, 0, 4, 3, 5);
    /* 2->1->0->4->5->3 */
    SIX_COST_STRICT_CHECK(2, 1, 0, 4, 5, 3);
    /* 2->1->0->5->3->4 */
    SIX_COST_STRICT_CHECK(2, 1, 0, 5, 3, 4);
    /* 2->1->0->5->4->3 */
    SIX_COST_STRICT_CHECK(2, 1, 0, 5, 4, 3);
    /* 2->1->3->0->4->5 */
    SIX_COST_STRICT_CHECK(2, 1, 3, 0, 4, 5);
    /* 2->1->3->0->5->4 */
    SIX_COST_STRICT_CHECK(2, 1, 3, 0, 5, 4);
    /* 2->1->3->4->0->5 */
    SIX_COST_STRICT_CHECK(2, 1, 3, 4, 0, 5);
    /* 2->1->3->4->5->0 */
    SIX_COST_STRICT_CHECK(2, 1, 3, 4, 5, 0);
    /* 2->1->3->5->0->4 */
    SIX_COST_STRICT_CHECK(2, 1, 3, 5, 0, 4);
    /* 2->1->3->5->4->0 */
    SIX_COST_STRICT_CHECK(2, 1, 3, 5, 4, 0);
    /* 2->1->4->0->3->5 */
    SIX_COST_STRICT_CHECK(2, 1, 4, 0, 3, 5);
    /* 2->1->4->0->5->3 */
    SIX_COST_STRICT_CHECK(2, 1, 4, 0, 5, 3);
    /* 2->1->4->3->0->5 */
    SIX_COST_STRICT_CHECK(2, 1, 4, 3, 0, 5);
    /* 2->1->4->3->5->0 */
    SIX_COST_STRICT_CHECK(2, 1, 4, 3, 5, 0);
    /* 2->1->4->5->0->3 */
    SIX_COST_STRICT_CHECK(2, 1, 4, 5, 0, 3);
    /* 2->1->4->5->3->0 */
    SIX_COST_STRICT_CHECK(2, 1, 4, 5, 3, 0);
    /* 2->1->5->0->3->4 */
    SIX_COST_STRICT_CHECK(2, 1, 5, 0, 3, 4);
    /* 2->1->5->0->4->3 */
    SIX_COST_STRICT_CHECK(2, 1, 5, 0, 4, 3);
    /* 2->1->5->3->0->4 */
    SIX_COST_STRICT_CHECK(2, 1, 5, 3, 0, 4);
    /* 2->1->5->3->4->0 */
    SIX_COST_STRICT_CHECK(2, 1, 5, 3, 4, 0);
    /* 2->1->5->4->0->3 */
    SIX_COST_STRICT_CHECK(2, 1, 5, 4, 0, 3);
    /* 2->1->5->4->3->0 */
    SIX_COST_STRICT_CHECK(2, 1, 5, 4, 3, 0);
    /* 2->3->0->1->4->5 */
    SIX_COST_STRICT_CHECK(2, 3, 0, 1, 4, 5);
    /* 2->3->0->1->5->4 */
    SIX_COST_STRICT_CHECK(2, 3, 0, 1, 5, 4);
    /* 2->3->0->4->1->5 */
    SIX_COST_STRICT_CHECK(2, 3, 0, 4, 1, 5);
    /* 2->3->0->4->5->1 */
    SIX_COST_STRICT_CHECK(2, 3, 0, 4, 5, 1);
    /* 2->3->0->5->1->4 */
    SIX_COST_STRICT_CHECK(2, 3, 0, 5, 1, 4);
    /* 2->3->0->5->4->1 */
    SIX_COST_STRICT_CHECK(2, 3, 0, 5, 4, 1);
    /* 2->3->1->0->4->5 */
    SIX_COST_STRICT_CHECK(2, 3, 1, 0, 4, 5);
    /* 2->3->1->0->5->4 */
    SIX_COST_STRICT_CHECK(2, 3, 1, 0, 5, 4);
    /* 2->3->1->4->0->5 */
    SIX_COST_STRICT_CHECK(2, 3, 1, 4, 0, 5);
    /* 2->3->1->4->5->0 */
    SIX_COST_STRICT_CHECK(2, 3, 1, 4, 5, 0);
    /* 2->3->1->5->0->4 */
    SIX_COST_STRICT_CHECK(2, 3, 1, 5, 0, 4);
    /* 2->3->1->5->4->0 */
    SIX_COST_STRICT_CHECK(2, 3, 1, 5, 4, 0);
    /* 2->3->4->0->1->5 */
    SIX_COST_STRICT_CHECK(2, 3, 4, 0, 1, 5);
    /* 2->3->4->0->5->1 */
    SIX_COST_STRICT_CHECK(2, 3, 4, 0, 5, 1);
    /* 2->3->4->1->0->5 */
    SIX_COST_STRICT_CHECK(2, 3, 4, 1, 0, 5);
    /* 2->3->4->1->5->0 */
    SIX_COST_STRICT_CHECK(2, 3, 4, 1, 5, 0);
    /* 2->3->4->5->0->1 */
    SIX_COST_STRICT_CHECK(2, 3, 4, 5, 0, 1);
    /* 2->3->4->5->1->0 */
    SIX_COST_STRICT_CHECK(2, 3, 4, 5, 1, 0);
    /* 2->3->5->0->1->4 */
    SIX_COST_STRICT_CHECK(2, 3, 5, 0, 1, 4);
    /* 2->3->5->0->4->1 */
    SIX_COST_STRICT_CHECK(2, 3, 5, 0, 4, 1);
    /* 2->3->5->1->0->4 */
    SIX_COST_STRICT_CHECK(2, 3, 5, 1, 0, 4);
    /* 2->3->5->1->4->0 */
    SIX_COST_STRICT_CHECK(2, 3, 5, 1, 4, 0);
    /* 2->3->5->4->0->1 */
    SIX_COST_STRICT_CHECK(2, 3, 5, 4, 0, 1);
    /* 2->3->5->4->1->0 */
    SIX_COST_STRICT_CHECK(2, 3, 5, 4, 1, 0);
    /* 2->4->0->1->3->5 */
    SIX_COST_STRICT_CHECK(2, 4, 0, 1, 3, 5);
    /* 2->4->0->1->5->3 */
    SIX_COST_STRICT_CHECK(2, 4, 0, 1, 5, 3);
    /* 2->4->0->3->1->5 */
    SIX_COST_STRICT_CHECK(2, 4, 0, 3, 1, 5);
    /* 2->4->0->3->5->1 */
    SIX_COST_STRICT_CHECK(2, 4, 0, 3, 5, 1);
    /* 2->4->0->5->1->3 */
    SIX_COST_STRICT_CHECK(2, 4, 0, 5, 1, 3);
    /* 2->4->0->5->3->1 */
    SIX_COST_STRICT_CHECK(2, 4, 0, 5, 3, 1);
    /* 2->4->1->0->3->5 */
    SIX_COST_STRICT_CHECK(2, 4, 1, 0, 3, 5);
    /* 2->4->1->0->5->3 */
    SIX_COST_STRICT_CHECK(2, 4, 1, 0, 5, 3);
    /* 2->4->1->3->0->5 */
    SIX_COST_STRICT_CHECK(2, 4, 1, 3, 0, 5);
    /* 2->4->1->3->5->0 */
    SIX_COST_STRICT_CHECK(2, 4, 1, 3, 5, 0);
    /* 2->4->1->5->0->3 */
    SIX_COST_STRICT_CHECK(2, 4, 1, 5, 0, 3);
    /* 2->4->1->5->3->0 */
    SIX_COST_STRICT_CHECK(2, 4, 1, 5, 3, 0);
    /* 2->4->3->0->1->5 */
    SIX_COST_STRICT_CHECK(2, 4, 3, 0, 1, 5);
    /* 2->4->3->0->5->1 */
    SIX_COST_STRICT_CHECK(2, 4, 3, 0, 5, 1);
    /* 2->4->3->1->0->5 */
    SIX_COST_STRICT_CHECK(2, 4, 3, 1, 0, 5);
    /* 2->4->3->1->5->0 */
    SIX_COST_STRICT_CHECK(2, 4, 3, 1, 5, 0);
    /* 2->4->3->5->0->1 */
    SIX_COST_STRICT_CHECK(2, 4, 3, 5, 0, 1);
    /* 2->4->3->5->1->0 */
    SIX_COST_STRICT_CHECK(2, 4, 3, 5, 1, 0);
    /* 2->4->5->0->1->3 */
    SIX_COST_STRICT_CHECK(2, 4, 5, 0, 1, 3);
    /* 2->4->5->0->3->1 */
    SIX_COST_STRICT_CHECK(2, 4, 5, 0, 3, 1);
    /* 2->4->5->1->0->3 */
    SIX_COST_STRICT_CHECK(2, 4, 5, 1, 0, 3);
    /* 2->4->5->1->3->0 */
    SIX_COST_STRICT_CHECK(2, 4, 5, 1, 3, 0);
    /* 2->4->5->3->0->1 */
    SIX_COST_STRICT_CHECK(2, 4, 5, 3, 0, 1);
    /* 2->4->5->3->1->0 */
    SIX_COST_STRICT_CHECK(2, 4, 5, 3, 1, 0);
    /* 2->5->0->1->3->4 */
    SIX_COST_STRICT_CHECK(2, 5, 0, 1, 3, 4);
    /* 2->5->0->1->4->3 */
    SIX_COST_STRICT_CHECK(2, 5, 0, 1, 4, 3);
    /* 2->5->0->3->1->4 */
    SIX_COST_STRICT_CHECK(2, 5, 0, 3, 1, 4);
    /* 2->5->0->3->4->1 */
    SIX_COST_STRICT_CHECK(2, 5, 0, 3, 4, 1);
    /* 2->5->0->4->1->3 */
    SIX_COST_STRICT_CHECK(2, 5, 0, 4, 1, 3);
    /* 2->5->0->4->3->1 */
    SIX_COST_STRICT_CHECK(2, 5, 0, 4, 3, 1);
    /* 2->5->1->0->3->4 */
    SIX_COST_STRICT_CHECK(2, 5, 1, 0, 3, 4);
    /* 2->5->1->0->4->3 */
    SIX_COST_STRICT_CHECK(2, 5, 1, 0, 4, 3);
    /* 2->5->1->3->0->4 */
    SIX_COST_STRICT_CHECK(2, 5, 1, 3, 0, 4);
    /* 2->5->1->3->4->0 */
    SIX_COST_STRICT_CHECK(2, 5, 1, 3, 4, 0);
    /* 2->5->1->4->0->3 */
    SIX_COST_STRICT_CHECK(2, 5, 1, 4, 0, 3);
    /* 2->5->1->4->3->0 */
    SIX_COST_STRICT_CHECK(2, 5, 1, 4, 3, 0);
    /* 2->5->3->0->1->4 */
    SIX_COST_STRICT_CHECK(2, 5, 3, 0, 1, 4);
    /* 2->5->3->0->4->1 */
    SIX_COST_STRICT_CHECK(2, 5, 3, 0, 4, 1);
    /* 2->5->3->1->0->4 */
    SIX_COST_STRICT_CHECK(2, 5, 3, 1, 0, 4);
    /* 2->5->3->1->4->0 */
    SIX_COST_STRICT_CHECK(2, 5, 3, 1, 4, 0);
    /* 2->5->3->4->0->1 */
    SIX_COST_STRICT_CHECK(2, 5, 3, 4, 0, 1);
    /* 2->5->3->4->1->0 */
    SIX_COST_STRICT_CHECK(2, 5, 3, 4, 1, 0);
    /* 2->5->4->0->1->3 */
    SIX_COST_STRICT_CHECK(2, 5, 4, 0, 1, 3);
    /* 2->5->4->0->3->1 */
    SIX_COST_STRICT_CHECK(2, 5, 4, 0, 3, 1);
    /* 2->5->4->1->0->3 */
    SIX_COST_STRICT_CHECK(2, 5, 4, 1, 0, 3);
    /* 2->5->4->1->3->0 */
    SIX_COST_STRICT_CHECK(2, 5, 4, 1, 3, 0);
    /* 2->5->4->3->0->1 */
    SIX_COST_STRICT_CHECK(2, 5, 4, 3, 0, 1);
    /* 2->5->4->3->1->0 */
    SIX_COST_STRICT_CHECK(2, 5, 4, 3, 1, 0);
  }

  if(prob->sjob[j[3]]->tno < prob->sjob[j[0]]->tno) {
    /* 3->0->1->2->4->5 */
    SIX_COST_CHECK(3, 0, 1, 2, 4, 5);
    /* 3->0->1->2->5->4 */
    SIX_COST_CHECK(3, 0, 1, 2, 5, 4);
    /* 3->0->1->4->2->5 */
    SIX_COST_CHECK(3, 0, 1, 4, 2, 5);
    /* 3->0->1->4->5->2 */
    SIX_COST_CHECK(3, 0, 1, 4, 5, 2);
    /* 3->0->1->5->2->4 */
    SIX_COST_CHECK(3, 0, 1, 5, 2, 4);
    /* 3->0->1->5->4->2 */
    SIX_COST_CHECK(3, 0, 1, 5, 4, 2);
    /* 3->0->2->1->4->5 */
    SIX_COST_CHECK(3, 0, 2, 1, 4, 5);
    /* 3->0->2->1->5->4 */
    SIX_COST_CHECK(3, 0, 2, 1, 5, 4);
    /* 3->0->2->4->1->5 */
    SIX_COST_CHECK(3, 0, 2, 4, 1, 5);
    /* 3->0->2->4->5->1 */
    SIX_COST_CHECK(3, 0, 2, 4, 5, 1);
    /* 3->0->2->5->1->4 */
    SIX_COST_CHECK(3, 0, 2, 5, 1, 4);
    /* 3->0->2->5->4->1 */
    SIX_COST_CHECK(3, 0, 2, 5, 4, 1);
    /* 3->0->4->1->2->5 */
    SIX_COST_CHECK(3, 0, 4, 1, 2, 5);
    /* 3->0->4->1->5->2 */
    SIX_COST_CHECK(3, 0, 4, 1, 5, 2);
    /* 3->0->4->2->1->5 */
    SIX_COST_CHECK(3, 0, 4, 2, 1, 5);
    /* 3->0->4->2->5->1 */
    SIX_COST_CHECK(3, 0, 4, 2, 5, 1);
    /* 3->0->4->5->1->2 */
    SIX_COST_CHECK(3, 0, 4, 5, 1, 2);
    /* 3->0->4->5->2->1 */
    SIX_COST_CHECK(3, 0, 4, 5, 2, 1);
    /* 3->0->5->1->2->4 */
    SIX_COST_CHECK(3, 0, 5, 1, 2, 4);
    /* 3->0->5->1->4->2 */
    SIX_COST_CHECK(3, 0, 5, 1, 4, 2);
    /* 3->0->5->2->1->4 */
    SIX_COST_CHECK(3, 0, 5, 2, 1, 4);
    /* 3->0->5->2->4->1 */
    SIX_COST_CHECK(3, 0, 5, 2, 4, 1);
    /* 3->0->5->4->1->2 */
    SIX_COST_CHECK(3, 0, 5, 4, 1, 2);
    /* 3->0->5->4->2->1 */
    SIX_COST_CHECK(3, 0, 5, 4, 2, 1);
    /* 3->1->0->2->4->5 */
    SIX_COST_CHECK(3, 1, 0, 2, 4, 5);
    /* 3->1->0->2->5->4 */
    SIX_COST_CHECK(3, 1, 0, 2, 5, 4);
    /* 3->1->0->4->2->5 */
    SIX_COST_CHECK(3, 1, 0, 4, 2, 5);
    /* 3->1->0->4->5->2 */
    SIX_COST_CHECK(3, 1, 0, 4, 5, 2);
    /* 3->1->0->5->2->4 */
    SIX_COST_CHECK(3, 1, 0, 5, 2, 4);
    /* 3->1->0->5->4->2 */
    SIX_COST_CHECK(3, 1, 0, 5, 4, 2);
    /* 3->1->2->0->4->5 */
    SIX_COST_CHECK(3, 1, 2, 0, 4, 5);
    /* 3->1->2->0->5->4 */
    SIX_COST_CHECK(3, 1, 2, 0, 5, 4);
    /* 3->1->2->4->0->5 */
    SIX_COST_CHECK(3, 1, 2, 4, 0, 5);
    /* 3->1->2->4->5->0 */
    SIX_COST_CHECK(3, 1, 2, 4, 5, 0);
    /* 3->1->2->5->0->4 */
    SIX_COST_CHECK(3, 1, 2, 5, 0, 4);
    /* 3->1->2->5->4->0 */
    SIX_COST_CHECK(3, 1, 2, 5, 4, 0);
    /* 3->1->4->0->2->5 */
    SIX_COST_CHECK(3, 1, 4, 0, 2, 5);
    /* 3->1->4->0->5->2 */
    SIX_COST_CHECK(3, 1, 4, 0, 5, 2);
    /* 3->1->4->2->0->5 */
    SIX_COST_CHECK(3, 1, 4, 2, 0, 5);
    /* 3->1->4->2->5->0 */
    SIX_COST_CHECK(3, 1, 4, 2, 5, 0);
    /* 3->1->4->5->0->2 */
    SIX_COST_CHECK(3, 1, 4, 5, 0, 2);
    /* 3->1->4->5->2->0 */
    SIX_COST_CHECK(3, 1, 4, 5, 2, 0);
    /* 3->1->5->0->2->4 */
    SIX_COST_CHECK(3, 1, 5, 0, 2, 4);
    /* 3->1->5->0->4->2 */
    SIX_COST_CHECK(3, 1, 5, 0, 4, 2);
    /* 3->1->5->2->0->4 */
    SIX_COST_CHECK(3, 1, 5, 2, 0, 4);
    /* 3->1->5->2->4->0 */
    SIX_COST_CHECK(3, 1, 5, 2, 4, 0);
    /* 3->1->5->4->0->2 */
    SIX_COST_CHECK(3, 1, 5, 4, 0, 2);
    /* 3->1->5->4->2->0 */
    SIX_COST_CHECK(3, 1, 5, 4, 2, 0);
    /* 3->2->0->1->4->5 */
    SIX_COST_CHECK(3, 2, 0, 1, 4, 5);
    /* 3->2->0->1->5->4 */
    SIX_COST_CHECK(3, 2, 0, 1, 5, 4);
    /* 3->2->0->4->1->5 */
    SIX_COST_CHECK(3, 2, 0, 4, 1, 5);
    /* 3->2->0->4->5->1 */
    SIX_COST_CHECK(3, 2, 0, 4, 5, 1);
    /* 3->2->0->5->1->4 */
    SIX_COST_CHECK(3, 2, 0, 5, 1, 4);
    /* 3->2->0->5->4->1 */
    SIX_COST_CHECK(3, 2, 0, 5, 4, 1);
    /* 3->2->1->0->4->5 */
    SIX_COST_CHECK(3, 2, 1, 0, 4, 5);
    /* 3->2->1->0->5->4 */
    SIX_COST_CHECK(3, 2, 1, 0, 5, 4);
    /* 3->2->1->4->0->5 */
    SIX_COST_CHECK(3, 2, 1, 4, 0, 5);
    /* 3->2->1->4->5->0 */
    SIX_COST_CHECK(3, 2, 1, 4, 5, 0);
    /* 3->2->1->5->0->4 */
    SIX_COST_CHECK(3, 2, 1, 5, 0, 4);
    /* 3->2->1->5->4->0 */
    SIX_COST_CHECK(3, 2, 1, 5, 4, 0);
    /* 3->2->4->0->1->5 */
    SIX_COST_CHECK(3, 2, 4, 0, 1, 5);
    /* 3->2->4->0->5->1 */
    SIX_COST_CHECK(3, 2, 4, 0, 5, 1);
    /* 3->2->4->1->0->5 */
    SIX_COST_CHECK(3, 2, 4, 1, 0, 5);
    /* 3->2->4->1->5->0 */
    SIX_COST_CHECK(3, 2, 4, 1, 5, 0);
    /* 3->2->4->5->0->1 */
    SIX_COST_CHECK(3, 2, 4, 5, 0, 1);
    /* 3->2->4->5->1->0 */
    SIX_COST_CHECK(3, 2, 4, 5, 1, 0);
    /* 3->2->5->0->1->4 */
    SIX_COST_CHECK(3, 2, 5, 0, 1, 4);
    /* 3->2->5->0->4->1 */
    SIX_COST_CHECK(3, 2, 5, 0, 4, 1);
    /* 3->2->5->1->0->4 */
    SIX_COST_CHECK(3, 2, 5, 1, 0, 4);
    /* 3->2->5->1->4->0 */
    SIX_COST_CHECK(3, 2, 5, 1, 4, 0);
    /* 3->2->5->4->0->1 */
    SIX_COST_CHECK(3, 2, 5, 4, 0, 1);
    /* 3->2->5->4->1->0 */
    SIX_COST_CHECK(3, 2, 5, 4, 1, 0);
    /* 3->4->0->1->2->5 */
    SIX_COST_CHECK(3, 4, 0, 1, 2, 5);
    /* 3->4->0->1->5->2 */
    SIX_COST_CHECK(3, 4, 0, 1, 5, 2);
    /* 3->4->0->2->1->5 */
    SIX_COST_CHECK(3, 4, 0, 2, 1, 5);
    /* 3->4->0->2->5->1 */
    SIX_COST_CHECK(3, 4, 0, 2, 5, 1);
    /* 3->4->0->5->1->2 */
    SIX_COST_CHECK(3, 4, 0, 5, 1, 2);
    /* 3->4->0->5->2->1 */
    SIX_COST_CHECK(3, 4, 0, 5, 2, 1);
    /* 3->4->1->0->2->5 */
    SIX_COST_CHECK(3, 4, 1, 0, 2, 5);
    /* 3->4->1->0->5->2 */
    SIX_COST_CHECK(3, 4, 1, 0, 5, 2);
    /* 3->4->1->2->0->5 */
    SIX_COST_CHECK(3, 4, 1, 2, 0, 5);
    /* 3->4->1->2->5->0 */
    SIX_COST_CHECK(3, 4, 1, 2, 5, 0);
    /* 3->4->1->5->0->2 */
    SIX_COST_CHECK(3, 4, 1, 5, 0, 2);
    /* 3->4->1->5->2->0 */
    SIX_COST_CHECK(3, 4, 1, 5, 2, 0);
    /* 3->4->2->0->1->5 */
    SIX_COST_CHECK(3, 4, 2, 0, 1, 5);
    /* 3->4->2->0->5->1 */
    SIX_COST_CHECK(3, 4, 2, 0, 5, 1);
    /* 3->4->2->1->0->5 */
    SIX_COST_CHECK(3, 4, 2, 1, 0, 5);
    /* 3->4->2->1->5->0 */
    SIX_COST_CHECK(3, 4, 2, 1, 5, 0);
    /* 3->4->2->5->0->1 */
    SIX_COST_CHECK(3, 4, 2, 5, 0, 1);
    /* 3->4->2->5->1->0 */
    SIX_COST_CHECK(3, 4, 2, 5, 1, 0);
    /* 3->4->5->0->1->2 */
    SIX_COST_CHECK(3, 4, 5, 0, 1, 2);
    /* 3->4->5->0->2->1 */
    SIX_COST_CHECK(3, 4, 5, 0, 2, 1);
    /* 3->4->5->1->0->2 */
    SIX_COST_CHECK(3, 4, 5, 1, 0, 2);
    /* 3->4->5->1->2->0 */
    SIX_COST_CHECK(3, 4, 5, 1, 2, 0);
    /* 3->4->5->2->0->1 */
    SIX_COST_CHECK(3, 4, 5, 2, 0, 1);
    /* 3->4->5->2->1->0 */
    SIX_COST_CHECK(3, 4, 5, 2, 1, 0);
    /* 3->5->0->1->2->4 */
    SIX_COST_CHECK(3, 5, 0, 1, 2, 4);
    /* 3->5->0->1->4->2 */
    SIX_COST_CHECK(3, 5, 0, 1, 4, 2);
    /* 3->5->0->2->1->4 */
    SIX_COST_CHECK(3, 5, 0, 2, 1, 4);
    /* 3->5->0->2->4->1 */
    SIX_COST_CHECK(3, 5, 0, 2, 4, 1);
    /* 3->5->0->4->1->2 */
    SIX_COST_CHECK(3, 5, 0, 4, 1, 2);
    /* 3->5->0->4->2->1 */
    SIX_COST_CHECK(3, 5, 0, 4, 2, 1);
    /* 3->5->1->0->2->4 */
    SIX_COST_CHECK(3, 5, 1, 0, 2, 4);
    /* 3->5->1->0->4->2 */
    SIX_COST_CHECK(3, 5, 1, 0, 4, 2);
    /* 3->5->1->2->0->4 */
    SIX_COST_CHECK(3, 5, 1, 2, 0, 4);
    /* 3->5->1->2->4->0 */
    SIX_COST_CHECK(3, 5, 1, 2, 4, 0);
    /* 3->5->1->4->0->2 */
    SIX_COST_CHECK(3, 5, 1, 4, 0, 2);
    /* 3->5->1->4->2->0 */
    SIX_COST_CHECK(3, 5, 1, 4, 2, 0);
    /* 3->5->2->0->1->4 */
    SIX_COST_CHECK(3, 5, 2, 0, 1, 4);
    /* 3->5->2->0->4->1 */
    SIX_COST_CHECK(3, 5, 2, 0, 4, 1);
    /* 3->5->2->1->0->4 */
    SIX_COST_CHECK(3, 5, 2, 1, 0, 4);
    /* 3->5->2->1->4->0 */
    SIX_COST_CHECK(3, 5, 2, 1, 4, 0);
    /* 3->5->2->4->0->1 */
    SIX_COST_CHECK(3, 5, 2, 4, 0, 1);
    /* 3->5->2->4->1->0 */
    SIX_COST_CHECK(3, 5, 2, 4, 1, 0);
    /* 3->5->4->0->1->2 */
    SIX_COST_CHECK(3, 5, 4, 0, 1, 2);
    /* 3->5->4->0->2->1 */
    SIX_COST_CHECK(3, 5, 4, 0, 2, 1);
    /* 3->5->4->1->0->2 */
    SIX_COST_CHECK(3, 5, 4, 1, 0, 2);
    /* 3->5->4->1->2->0 */
    SIX_COST_CHECK(3, 5, 4, 1, 2, 0);
    /* 3->5->4->2->0->1 */
    SIX_COST_CHECK(3, 5, 4, 2, 0, 1);
    /* 3->5->4->2->1->0 */
    SIX_COST_CHECK(3, 5, 4, 2, 1, 0);
  } else {
    /* 3->0->1->2->4->5 */
    SIX_COST_STRICT_CHECK(3, 0, 1, 2, 4, 5);
    /* 3->0->1->2->5->4 */
    SIX_COST_STRICT_CHECK(3, 0, 1, 2, 5, 4);
    /* 3->0->1->4->2->5 */
    SIX_COST_STRICT_CHECK(3, 0, 1, 4, 2, 5);
    /* 3->0->1->4->5->2 */
    SIX_COST_STRICT_CHECK(3, 0, 1, 4, 5, 2);
    /* 3->0->1->5->2->4 */
    SIX_COST_STRICT_CHECK(3, 0, 1, 5, 2, 4);
    /* 3->0->1->5->4->2 */
    SIX_COST_STRICT_CHECK(3, 0, 1, 5, 4, 2);
    /* 3->0->2->1->4->5 */
    SIX_COST_STRICT_CHECK(3, 0, 2, 1, 4, 5);
    /* 3->0->2->1->5->4 */
    SIX_COST_STRICT_CHECK(3, 0, 2, 1, 5, 4);
    /* 3->0->2->4->1->5 */
    SIX_COST_STRICT_CHECK(3, 0, 2, 4, 1, 5);
    /* 3->0->2->4->5->1 */
    SIX_COST_STRICT_CHECK(3, 0, 2, 4, 5, 1);
    /* 3->0->2->5->1->4 */
    SIX_COST_STRICT_CHECK(3, 0, 2, 5, 1, 4);
    /* 3->0->2->5->4->1 */
    SIX_COST_STRICT_CHECK(3, 0, 2, 5, 4, 1);
    /* 3->0->4->1->2->5 */
    SIX_COST_STRICT_CHECK(3, 0, 4, 1, 2, 5);
    /* 3->0->4->1->5->2 */
    SIX_COST_STRICT_CHECK(3, 0, 4, 1, 5, 2);
    /* 3->0->4->2->1->5 */
    SIX_COST_STRICT_CHECK(3, 0, 4, 2, 1, 5);
    /* 3->0->4->2->5->1 */
    SIX_COST_STRICT_CHECK(3, 0, 4, 2, 5, 1);
    /* 3->0->4->5->1->2 */
    SIX_COST_STRICT_CHECK(3, 0, 4, 5, 1, 2);
    /* 3->0->4->5->2->1 */
    SIX_COST_STRICT_CHECK(3, 0, 4, 5, 2, 1);
    /* 3->0->5->1->2->4 */
    SIX_COST_STRICT_CHECK(3, 0, 5, 1, 2, 4);
    /* 3->0->5->1->4->2 */
    SIX_COST_STRICT_CHECK(3, 0, 5, 1, 4, 2);
    /* 3->0->5->2->1->4 */
    SIX_COST_STRICT_CHECK(3, 0, 5, 2, 1, 4);
    /* 3->0->5->2->4->1 */
    SIX_COST_STRICT_CHECK(3, 0, 5, 2, 4, 1);
    /* 3->0->5->4->1->2 */
    SIX_COST_STRICT_CHECK(3, 0, 5, 4, 1, 2);
    /* 3->0->5->4->2->1 */
    SIX_COST_STRICT_CHECK(3, 0, 5, 4, 2, 1);
    /* 3->1->0->2->4->5 */
    SIX_COST_STRICT_CHECK(3, 1, 0, 2, 4, 5);
    /* 3->1->0->2->5->4 */
    SIX_COST_STRICT_CHECK(3, 1, 0, 2, 5, 4);
    /* 3->1->0->4->2->5 */
    SIX_COST_STRICT_CHECK(3, 1, 0, 4, 2, 5);
    /* 3->1->0->4->5->2 */
    SIX_COST_STRICT_CHECK(3, 1, 0, 4, 5, 2);
    /* 3->1->0->5->2->4 */
    SIX_COST_STRICT_CHECK(3, 1, 0, 5, 2, 4);
    /* 3->1->0->5->4->2 */
    SIX_COST_STRICT_CHECK(3, 1, 0, 5, 4, 2);
    /* 3->1->2->0->4->5 */
    SIX_COST_STRICT_CHECK(3, 1, 2, 0, 4, 5);
    /* 3->1->2->0->5->4 */
    SIX_COST_STRICT_CHECK(3, 1, 2, 0, 5, 4);
    /* 3->1->2->4->0->5 */
    SIX_COST_STRICT_CHECK(3, 1, 2, 4, 0, 5);
    /* 3->1->2->4->5->0 */
    SIX_COST_STRICT_CHECK(3, 1, 2, 4, 5, 0);
    /* 3->1->2->5->0->4 */
    SIX_COST_STRICT_CHECK(3, 1, 2, 5, 0, 4);
    /* 3->1->2->5->4->0 */
    SIX_COST_STRICT_CHECK(3, 1, 2, 5, 4, 0);
    /* 3->1->4->0->2->5 */
    SIX_COST_STRICT_CHECK(3, 1, 4, 0, 2, 5);
    /* 3->1->4->0->5->2 */
    SIX_COST_STRICT_CHECK(3, 1, 4, 0, 5, 2);
    /* 3->1->4->2->0->5 */
    SIX_COST_STRICT_CHECK(3, 1, 4, 2, 0, 5);
    /* 3->1->4->2->5->0 */
    SIX_COST_STRICT_CHECK(3, 1, 4, 2, 5, 0);
    /* 3->1->4->5->0->2 */
    SIX_COST_STRICT_CHECK(3, 1, 4, 5, 0, 2);
    /* 3->1->4->5->2->0 */
    SIX_COST_STRICT_CHECK(3, 1, 4, 5, 2, 0);
    /* 3->1->5->0->2->4 */
    SIX_COST_STRICT_CHECK(3, 1, 5, 0, 2, 4);
    /* 3->1->5->0->4->2 */
    SIX_COST_STRICT_CHECK(3, 1, 5, 0, 4, 2);
    /* 3->1->5->2->0->4 */
    SIX_COST_STRICT_CHECK(3, 1, 5, 2, 0, 4);
    /* 3->1->5->2->4->0 */
    SIX_COST_STRICT_CHECK(3, 1, 5, 2, 4, 0);
    /* 3->1->5->4->0->2 */
    SIX_COST_STRICT_CHECK(3, 1, 5, 4, 0, 2);
    /* 3->1->5->4->2->0 */
    SIX_COST_STRICT_CHECK(3, 1, 5, 4, 2, 0);
    /* 3->2->0->1->4->5 */
    SIX_COST_STRICT_CHECK(3, 2, 0, 1, 4, 5);
    /* 3->2->0->1->5->4 */
    SIX_COST_STRICT_CHECK(3, 2, 0, 1, 5, 4);
    /* 3->2->0->4->1->5 */
    SIX_COST_STRICT_CHECK(3, 2, 0, 4, 1, 5);
    /* 3->2->0->4->5->1 */
    SIX_COST_STRICT_CHECK(3, 2, 0, 4, 5, 1);
    /* 3->2->0->5->1->4 */
    SIX_COST_STRICT_CHECK(3, 2, 0, 5, 1, 4);
    /* 3->2->0->5->4->1 */
    SIX_COST_STRICT_CHECK(3, 2, 0, 5, 4, 1);
    /* 3->2->1->0->4->5 */
    SIX_COST_STRICT_CHECK(3, 2, 1, 0, 4, 5);
    /* 3->2->1->0->5->4 */
    SIX_COST_STRICT_CHECK(3, 2, 1, 0, 5, 4);
    /* 3->2->1->4->0->5 */
    SIX_COST_STRICT_CHECK(3, 2, 1, 4, 0, 5);
    /* 3->2->1->4->5->0 */
    SIX_COST_STRICT_CHECK(3, 2, 1, 4, 5, 0);
    /* 3->2->1->5->0->4 */
    SIX_COST_STRICT_CHECK(3, 2, 1, 5, 0, 4);
    /* 3->2->1->5->4->0 */
    SIX_COST_STRICT_CHECK(3, 2, 1, 5, 4, 0);
    /* 3->2->4->0->1->5 */
    SIX_COST_STRICT_CHECK(3, 2, 4, 0, 1, 5);
    /* 3->2->4->0->5->1 */
    SIX_COST_STRICT_CHECK(3, 2, 4, 0, 5, 1);
    /* 3->2->4->1->0->5 */
    SIX_COST_STRICT_CHECK(3, 2, 4, 1, 0, 5);
    /* 3->2->4->1->5->0 */
    SIX_COST_STRICT_CHECK(3, 2, 4, 1, 5, 0);
    /* 3->2->4->5->0->1 */
    SIX_COST_STRICT_CHECK(3, 2, 4, 5, 0, 1);
    /* 3->2->4->5->1->0 */
    SIX_COST_STRICT_CHECK(3, 2, 4, 5, 1, 0);
    /* 3->2->5->0->1->4 */
    SIX_COST_STRICT_CHECK(3, 2, 5, 0, 1, 4);
    /* 3->2->5->0->4->1 */
    SIX_COST_STRICT_CHECK(3, 2, 5, 0, 4, 1);
    /* 3->2->5->1->0->4 */
    SIX_COST_STRICT_CHECK(3, 2, 5, 1, 0, 4);
    /* 3->2->5->1->4->0 */
    SIX_COST_STRICT_CHECK(3, 2, 5, 1, 4, 0);
    /* 3->2->5->4->0->1 */
    SIX_COST_STRICT_CHECK(3, 2, 5, 4, 0, 1);
    /* 3->2->5->4->1->0 */
    SIX_COST_STRICT_CHECK(3, 2, 5, 4, 1, 0);
    /* 3->4->0->1->2->5 */
    SIX_COST_STRICT_CHECK(3, 4, 0, 1, 2, 5);
    /* 3->4->0->1->5->2 */
    SIX_COST_STRICT_CHECK(3, 4, 0, 1, 5, 2);
    /* 3->4->0->2->1->5 */
    SIX_COST_STRICT_CHECK(3, 4, 0, 2, 1, 5);
    /* 3->4->0->2->5->1 */
    SIX_COST_STRICT_CHECK(3, 4, 0, 2, 5, 1);
    /* 3->4->0->5->1->2 */
    SIX_COST_STRICT_CHECK(3, 4, 0, 5, 1, 2);
    /* 3->4->0->5->2->1 */
    SIX_COST_STRICT_CHECK(3, 4, 0, 5, 2, 1);
    /* 3->4->1->0->2->5 */
    SIX_COST_STRICT_CHECK(3, 4, 1, 0, 2, 5);
    /* 3->4->1->0->5->2 */
    SIX_COST_STRICT_CHECK(3, 4, 1, 0, 5, 2);
    /* 3->4->1->2->0->5 */
    SIX_COST_STRICT_CHECK(3, 4, 1, 2, 0, 5);
    /* 3->4->1->2->5->0 */
    SIX_COST_STRICT_CHECK(3, 4, 1, 2, 5, 0);
    /* 3->4->1->5->0->2 */
    SIX_COST_STRICT_CHECK(3, 4, 1, 5, 0, 2);
    /* 3->4->1->5->2->0 */
    SIX_COST_STRICT_CHECK(3, 4, 1, 5, 2, 0);
    /* 3->4->2->0->1->5 */
    SIX_COST_STRICT_CHECK(3, 4, 2, 0, 1, 5);
    /* 3->4->2->0->5->1 */
    SIX_COST_STRICT_CHECK(3, 4, 2, 0, 5, 1);
    /* 3->4->2->1->0->5 */
    SIX_COST_STRICT_CHECK(3, 4, 2, 1, 0, 5);
    /* 3->4->2->1->5->0 */
    SIX_COST_STRICT_CHECK(3, 4, 2, 1, 5, 0);
    /* 3->4->2->5->0->1 */
    SIX_COST_STRICT_CHECK(3, 4, 2, 5, 0, 1);
    /* 3->4->2->5->1->0 */
    SIX_COST_STRICT_CHECK(3, 4, 2, 5, 1, 0);
    /* 3->4->5->0->1->2 */
    SIX_COST_STRICT_CHECK(3, 4, 5, 0, 1, 2);
    /* 3->4->5->0->2->1 */
    SIX_COST_STRICT_CHECK(3, 4, 5, 0, 2, 1);
    /* 3->4->5->1->0->2 */
    SIX_COST_STRICT_CHECK(3, 4, 5, 1, 0, 2);
    /* 3->4->5->1->2->0 */
    SIX_COST_STRICT_CHECK(3, 4, 5, 1, 2, 0);
    /* 3->4->5->2->0->1 */
    SIX_COST_STRICT_CHECK(3, 4, 5, 2, 0, 1);
    /* 3->4->5->2->1->0 */
    SIX_COST_STRICT_CHECK(3, 4, 5, 2, 1, 0);
    /* 3->5->0->1->2->4 */
    SIX_COST_STRICT_CHECK(3, 5, 0, 1, 2, 4);
    /* 3->5->0->1->4->2 */
    SIX_COST_STRICT_CHECK(3, 5, 0, 1, 4, 2);
    /* 3->5->0->2->1->4 */
    SIX_COST_STRICT_CHECK(3, 5, 0, 2, 1, 4);
    /* 3->5->0->2->4->1 */
    SIX_COST_STRICT_CHECK(3, 5, 0, 2, 4, 1);
    /* 3->5->0->4->1->2 */
    SIX_COST_STRICT_CHECK(3, 5, 0, 4, 1, 2);
    /* 3->5->0->4->2->1 */
    SIX_COST_STRICT_CHECK(3, 5, 0, 4, 2, 1);
    /* 3->5->1->0->2->4 */
    SIX_COST_STRICT_CHECK(3, 5, 1, 0, 2, 4);
    /* 3->5->1->0->4->2 */
    SIX_COST_STRICT_CHECK(3, 5, 1, 0, 4, 2);
    /* 3->5->1->2->0->4 */
    SIX_COST_STRICT_CHECK(3, 5, 1, 2, 0, 4);
    /* 3->5->1->2->4->0 */
    SIX_COST_STRICT_CHECK(3, 5, 1, 2, 4, 0);
    /* 3->5->1->4->0->2 */
    SIX_COST_STRICT_CHECK(3, 5, 1, 4, 0, 2);
    /* 3->5->1->4->2->0 */
    SIX_COST_STRICT_CHECK(3, 5, 1, 4, 2, 0);
    /* 3->5->2->0->1->4 */
    SIX_COST_STRICT_CHECK(3, 5, 2, 0, 1, 4);
    /* 3->5->2->0->4->1 */
    SIX_COST_STRICT_CHECK(3, 5, 2, 0, 4, 1);
    /* 3->5->2->1->0->4 */
    SIX_COST_STRICT_CHECK(3, 5, 2, 1, 0, 4);
    /* 3->5->2->1->4->0 */
    SIX_COST_STRICT_CHECK(3, 5, 2, 1, 4, 0);
    /* 3->5->2->4->0->1 */
    SIX_COST_STRICT_CHECK(3, 5, 2, 4, 0, 1);
    /* 3->5->2->4->1->0 */
    SIX_COST_STRICT_CHECK(3, 5, 2, 4, 1, 0);
    /* 3->5->4->0->1->2 */
    SIX_COST_STRICT_CHECK(3, 5, 4, 0, 1, 2);
    /* 3->5->4->0->2->1 */
    SIX_COST_STRICT_CHECK(3, 5, 4, 0, 2, 1);
    /* 3->5->4->1->0->2 */
    SIX_COST_STRICT_CHECK(3, 5, 4, 1, 0, 2);
    /* 3->5->4->1->2->0 */
    SIX_COST_STRICT_CHECK(3, 5, 4, 1, 2, 0);
    /* 3->5->4->2->0->1 */
    SIX_COST_STRICT_CHECK(3, 5, 4, 2, 0, 1);
    /* 3->5->4->2->1->0 */
    SIX_COST_STRICT_CHECK(3, 5, 4, 2, 1, 0);
  }

  if(prob->sjob[j[4]]->tno < prob->sjob[j[0]]->tno) {
    /* 4->0->1->2->3->5 */
    SIX_COST_CHECK(4, 0, 1, 2, 3, 5);
    /* 4->0->1->2->5->3 */
    SIX_COST_CHECK(4, 0, 1, 2, 5, 3);
    /* 4->0->1->3->2->5 */
    SIX_COST_CHECK(4, 0, 1, 3, 2, 5);
    /* 4->0->1->3->5->2 */
    SIX_COST_CHECK(4, 0, 1, 3, 5, 2);
    /* 4->0->1->5->2->3 */
    SIX_COST_CHECK(4, 0, 1, 5, 2, 3);
    /* 4->0->1->5->3->2 */
    SIX_COST_CHECK(4, 0, 1, 5, 3, 2);
    /* 4->0->2->1->3->5 */
    SIX_COST_CHECK(4, 0, 2, 1, 3, 5);
    /* 4->0->2->1->5->3 */
    SIX_COST_CHECK(4, 0, 2, 1, 5, 3);
    /* 4->0->2->3->1->5 */
    SIX_COST_CHECK(4, 0, 2, 3, 1, 5);
    /* 4->0->2->3->5->1 */
    SIX_COST_CHECK(4, 0, 2, 3, 5, 1);
    /* 4->0->2->5->1->3 */
    SIX_COST_CHECK(4, 0, 2, 5, 1, 3);
    /* 4->0->2->5->3->1 */
    SIX_COST_CHECK(4, 0, 2, 5, 3, 1);
    /* 4->0->3->1->2->5 */
    SIX_COST_CHECK(4, 0, 3, 1, 2, 5);
    /* 4->0->3->1->5->2 */
    SIX_COST_CHECK(4, 0, 3, 1, 5, 2);
    /* 4->0->3->2->1->5 */
    SIX_COST_CHECK(4, 0, 3, 2, 1, 5);
    /* 4->0->3->2->5->1 */
    SIX_COST_CHECK(4, 0, 3, 2, 5, 1);
    /* 4->0->3->5->1->2 */
    SIX_COST_CHECK(4, 0, 3, 5, 1, 2);
    /* 4->0->3->5->2->1 */
    SIX_COST_CHECK(4, 0, 3, 5, 2, 1);
    /* 4->0->5->1->2->3 */
    SIX_COST_CHECK(4, 0, 5, 1, 2, 3);
    /* 4->0->5->1->3->2 */
    SIX_COST_CHECK(4, 0, 5, 1, 3, 2);
    /* 4->0->5->2->1->3 */
    SIX_COST_CHECK(4, 0, 5, 2, 1, 3);
    /* 4->0->5->2->3->1 */
    SIX_COST_CHECK(4, 0, 5, 2, 3, 1);
    /* 4->0->5->3->1->2 */
    SIX_COST_CHECK(4, 0, 5, 3, 1, 2);
    /* 4->0->5->3->2->1 */
    SIX_COST_CHECK(4, 0, 5, 3, 2, 1);
    /* 4->1->0->2->3->5 */
    SIX_COST_CHECK(4, 1, 0, 2, 3, 5);
    /* 4->1->0->2->5->3 */
    SIX_COST_CHECK(4, 1, 0, 2, 5, 3);
    /* 4->1->0->3->2->5 */
    SIX_COST_CHECK(4, 1, 0, 3, 2, 5);
    /* 4->1->0->3->5->2 */
    SIX_COST_CHECK(4, 1, 0, 3, 5, 2);
    /* 4->1->0->5->2->3 */
    SIX_COST_CHECK(4, 1, 0, 5, 2, 3);
    /* 4->1->0->5->3->2 */
    SIX_COST_CHECK(4, 1, 0, 5, 3, 2);
    /* 4->1->2->0->3->5 */
    SIX_COST_CHECK(4, 1, 2, 0, 3, 5);
    /* 4->1->2->0->5->3 */
    SIX_COST_CHECK(4, 1, 2, 0, 5, 3);
    /* 4->1->2->3->0->5 */
    SIX_COST_CHECK(4, 1, 2, 3, 0, 5);
    /* 4->1->2->3->5->0 */
    SIX_COST_CHECK(4, 1, 2, 3, 5, 0);
    /* 4->1->2->5->0->3 */
    SIX_COST_CHECK(4, 1, 2, 5, 0, 3);
    /* 4->1->2->5->3->0 */
    SIX_COST_CHECK(4, 1, 2, 5, 3, 0);
    /* 4->1->3->0->2->5 */
    SIX_COST_CHECK(4, 1, 3, 0, 2, 5);
    /* 4->1->3->0->5->2 */
    SIX_COST_CHECK(4, 1, 3, 0, 5, 2);
    /* 4->1->3->2->0->5 */
    SIX_COST_CHECK(4, 1, 3, 2, 0, 5);
    /* 4->1->3->2->5->0 */
    SIX_COST_CHECK(4, 1, 3, 2, 5, 0);
    /* 4->1->3->5->0->2 */
    SIX_COST_CHECK(4, 1, 3, 5, 0, 2);
    /* 4->1->3->5->2->0 */
    SIX_COST_CHECK(4, 1, 3, 5, 2, 0);
    /* 4->1->5->0->2->3 */
    SIX_COST_CHECK(4, 1, 5, 0, 2, 3);
    /* 4->1->5->0->3->2 */
    SIX_COST_CHECK(4, 1, 5, 0, 3, 2);
    /* 4->1->5->2->0->3 */
    SIX_COST_CHECK(4, 1, 5, 2, 0, 3);
    /* 4->1->5->2->3->0 */
    SIX_COST_CHECK(4, 1, 5, 2, 3, 0);
    /* 4->1->5->3->0->2 */
    SIX_COST_CHECK(4, 1, 5, 3, 0, 2);
    /* 4->1->5->3->2->0 */
    SIX_COST_CHECK(4, 1, 5, 3, 2, 0);
    /* 4->2->0->1->3->5 */
    SIX_COST_CHECK(4, 2, 0, 1, 3, 5);
    /* 4->2->0->1->5->3 */
    SIX_COST_CHECK(4, 2, 0, 1, 5, 3);
    /* 4->2->0->3->1->5 */
    SIX_COST_CHECK(4, 2, 0, 3, 1, 5);
    /* 4->2->0->3->5->1 */
    SIX_COST_CHECK(4, 2, 0, 3, 5, 1);
    /* 4->2->0->5->1->3 */
    SIX_COST_CHECK(4, 2, 0, 5, 1, 3);
    /* 4->2->0->5->3->1 */
    SIX_COST_CHECK(4, 2, 0, 5, 3, 1);
    /* 4->2->1->0->3->5 */
    SIX_COST_CHECK(4, 2, 1, 0, 3, 5);
    /* 4->2->1->0->5->3 */
    SIX_COST_CHECK(4, 2, 1, 0, 5, 3);
    /* 4->2->1->3->0->5 */
    SIX_COST_CHECK(4, 2, 1, 3, 0, 5);
    /* 4->2->1->3->5->0 */
    SIX_COST_CHECK(4, 2, 1, 3, 5, 0);
    /* 4->2->1->5->0->3 */
    SIX_COST_CHECK(4, 2, 1, 5, 0, 3);
    /* 4->2->1->5->3->0 */
    SIX_COST_CHECK(4, 2, 1, 5, 3, 0);
    /* 4->2->3->0->1->5 */
    SIX_COST_CHECK(4, 2, 3, 0, 1, 5);
    /* 4->2->3->0->5->1 */
    SIX_COST_CHECK(4, 2, 3, 0, 5, 1);
    /* 4->2->3->1->0->5 */
    SIX_COST_CHECK(4, 2, 3, 1, 0, 5);
    /* 4->2->3->1->5->0 */
    SIX_COST_CHECK(4, 2, 3, 1, 5, 0);
    /* 4->2->3->5->0->1 */
    SIX_COST_CHECK(4, 2, 3, 5, 0, 1);
    /* 4->2->3->5->1->0 */
    SIX_COST_CHECK(4, 2, 3, 5, 1, 0);
    /* 4->2->5->0->1->3 */
    SIX_COST_CHECK(4, 2, 5, 0, 1, 3);
    /* 4->2->5->0->3->1 */
    SIX_COST_CHECK(4, 2, 5, 0, 3, 1);
    /* 4->2->5->1->0->3 */
    SIX_COST_CHECK(4, 2, 5, 1, 0, 3);
    /* 4->2->5->1->3->0 */
    SIX_COST_CHECK(4, 2, 5, 1, 3, 0);
    /* 4->2->5->3->0->1 */
    SIX_COST_CHECK(4, 2, 5, 3, 0, 1);
    /* 4->2->5->3->1->0 */
    SIX_COST_CHECK(4, 2, 5, 3, 1, 0);
    /* 4->3->0->1->2->5 */
    SIX_COST_CHECK(4, 3, 0, 1, 2, 5);
    /* 4->3->0->1->5->2 */
    SIX_COST_CHECK(4, 3, 0, 1, 5, 2);
    /* 4->3->0->2->1->5 */
    SIX_COST_CHECK(4, 3, 0, 2, 1, 5);
    /* 4->3->0->2->5->1 */
    SIX_COST_CHECK(4, 3, 0, 2, 5, 1);
    /* 4->3->0->5->1->2 */
    SIX_COST_CHECK(4, 3, 0, 5, 1, 2);
    /* 4->3->0->5->2->1 */
    SIX_COST_CHECK(4, 3, 0, 5, 2, 1);
    /* 4->3->1->0->2->5 */
    SIX_COST_CHECK(4, 3, 1, 0, 2, 5);
    /* 4->3->1->0->5->2 */
    SIX_COST_CHECK(4, 3, 1, 0, 5, 2);
    /* 4->3->1->2->0->5 */
    SIX_COST_CHECK(4, 3, 1, 2, 0, 5);
    /* 4->3->1->2->5->0 */
    SIX_COST_CHECK(4, 3, 1, 2, 5, 0);
    /* 4->3->1->5->0->2 */
    SIX_COST_CHECK(4, 3, 1, 5, 0, 2);
    /* 4->3->1->5->2->0 */
    SIX_COST_CHECK(4, 3, 1, 5, 2, 0);
    /* 4->3->2->0->1->5 */
    SIX_COST_CHECK(4, 3, 2, 0, 1, 5);
    /* 4->3->2->0->5->1 */
    SIX_COST_CHECK(4, 3, 2, 0, 5, 1);
    /* 4->3->2->1->0->5 */
    SIX_COST_CHECK(4, 3, 2, 1, 0, 5);
    /* 4->3->2->1->5->0 */
    SIX_COST_CHECK(4, 3, 2, 1, 5, 0);
    /* 4->3->2->5->0->1 */
    SIX_COST_CHECK(4, 3, 2, 5, 0, 1);
    /* 4->3->2->5->1->0 */
    SIX_COST_CHECK(4, 3, 2, 5, 1, 0);
    /* 4->3->5->0->1->2 */
    SIX_COST_CHECK(4, 3, 5, 0, 1, 2);
    /* 4->3->5->0->2->1 */
    SIX_COST_CHECK(4, 3, 5, 0, 2, 1);
    /* 4->3->5->1->0->2 */
    SIX_COST_CHECK(4, 3, 5, 1, 0, 2);
    /* 4->3->5->1->2->0 */
    SIX_COST_CHECK(4, 3, 5, 1, 2, 0);
    /* 4->3->5->2->0->1 */
    SIX_COST_CHECK(4, 3, 5, 2, 0, 1);
    /* 4->3->5->2->1->0 */
    SIX_COST_CHECK(4, 3, 5, 2, 1, 0);
    /* 4->5->0->1->2->3 */
    SIX_COST_CHECK(4, 5, 0, 1, 2, 3);
    /* 4->5->0->1->3->2 */
    SIX_COST_CHECK(4, 5, 0, 1, 3, 2);
    /* 4->5->0->2->1->3 */
    SIX_COST_CHECK(4, 5, 0, 2, 1, 3);
    /* 4->5->0->2->3->1 */
    SIX_COST_CHECK(4, 5, 0, 2, 3, 1);
    /* 4->5->0->3->1->2 */
    SIX_COST_CHECK(4, 5, 0, 3, 1, 2);
    /* 4->5->0->3->2->1 */
    SIX_COST_CHECK(4, 5, 0, 3, 2, 1);
    /* 4->5->1->0->2->3 */
    SIX_COST_CHECK(4, 5, 1, 0, 2, 3);
    /* 4->5->1->0->3->2 */
    SIX_COST_CHECK(4, 5, 1, 0, 3, 2);
    /* 4->5->1->2->0->3 */
    SIX_COST_CHECK(4, 5, 1, 2, 0, 3);
    /* 4->5->1->2->3->0 */
    SIX_COST_CHECK(4, 5, 1, 2, 3, 0);
    /* 4->5->1->3->0->2 */
    SIX_COST_CHECK(4, 5, 1, 3, 0, 2);
    /* 4->5->1->3->2->0 */
    SIX_COST_CHECK(4, 5, 1, 3, 2, 0);
    /* 4->5->2->0->1->3 */
    SIX_COST_CHECK(4, 5, 2, 0, 1, 3);
    /* 4->5->2->0->3->1 */
    SIX_COST_CHECK(4, 5, 2, 0, 3, 1);
    /* 4->5->2->1->0->3 */
    SIX_COST_CHECK(4, 5, 2, 1, 0, 3);
    /* 4->5->2->1->3->0 */
    SIX_COST_CHECK(4, 5, 2, 1, 3, 0);
    /* 4->5->2->3->0->1 */
    SIX_COST_CHECK(4, 5, 2, 3, 0, 1);
    /* 4->5->2->3->1->0 */
    SIX_COST_CHECK(4, 5, 2, 3, 1, 0);
    /* 4->5->3->0->1->2 */
    SIX_COST_CHECK(4, 5, 3, 0, 1, 2);
    /* 4->5->3->0->2->1 */
    SIX_COST_CHECK(4, 5, 3, 0, 2, 1);
    /* 4->5->3->1->0->2 */
    SIX_COST_CHECK(4, 5, 3, 1, 0, 2);
    /* 4->5->3->1->2->0 */
    SIX_COST_CHECK(4, 5, 3, 1, 2, 0);
    /* 4->5->3->2->0->1 */
    SIX_COST_CHECK(4, 5, 3, 2, 0, 1);
    /* 4->5->3->2->1->0 */
    SIX_COST_CHECK(4, 5, 3, 2, 1, 0);
  } else {
    /* 4->0->1->2->3->5 */
    SIX_COST_STRICT_CHECK(4, 0, 1, 2, 3, 5);
    /* 4->0->1->2->5->3 */
    SIX_COST_STRICT_CHECK(4, 0, 1, 2, 5, 3);
    /* 4->0->1->3->2->5 */
    SIX_COST_STRICT_CHECK(4, 0, 1, 3, 2, 5);
    /* 4->0->1->3->5->2 */
    SIX_COST_STRICT_CHECK(4, 0, 1, 3, 5, 2);
    /* 4->0->1->5->2->3 */
    SIX_COST_STRICT_CHECK(4, 0, 1, 5, 2, 3);
    /* 4->0->1->5->3->2 */
    SIX_COST_STRICT_CHECK(4, 0, 1, 5, 3, 2);
    /* 4->0->2->1->3->5 */
    SIX_COST_STRICT_CHECK(4, 0, 2, 1, 3, 5);
    /* 4->0->2->1->5->3 */
    SIX_COST_STRICT_CHECK(4, 0, 2, 1, 5, 3);
    /* 4->0->2->3->1->5 */
    SIX_COST_STRICT_CHECK(4, 0, 2, 3, 1, 5);
    /* 4->0->2->3->5->1 */
    SIX_COST_STRICT_CHECK(4, 0, 2, 3, 5, 1);
    /* 4->0->2->5->1->3 */
    SIX_COST_STRICT_CHECK(4, 0, 2, 5, 1, 3);
    /* 4->0->2->5->3->1 */
    SIX_COST_STRICT_CHECK(4, 0, 2, 5, 3, 1);
    /* 4->0->3->1->2->5 */
    SIX_COST_STRICT_CHECK(4, 0, 3, 1, 2, 5);
    /* 4->0->3->1->5->2 */
    SIX_COST_STRICT_CHECK(4, 0, 3, 1, 5, 2);
    /* 4->0->3->2->1->5 */
    SIX_COST_STRICT_CHECK(4, 0, 3, 2, 1, 5);
    /* 4->0->3->2->5->1 */
    SIX_COST_STRICT_CHECK(4, 0, 3, 2, 5, 1);
    /* 4->0->3->5->1->2 */
    SIX_COST_STRICT_CHECK(4, 0, 3, 5, 1, 2);
    /* 4->0->3->5->2->1 */
    SIX_COST_STRICT_CHECK(4, 0, 3, 5, 2, 1);
    /* 4->0->5->1->2->3 */
    SIX_COST_STRICT_CHECK(4, 0, 5, 1, 2, 3);
    /* 4->0->5->1->3->2 */
    SIX_COST_STRICT_CHECK(4, 0, 5, 1, 3, 2);
    /* 4->0->5->2->1->3 */
    SIX_COST_STRICT_CHECK(4, 0, 5, 2, 1, 3);
    /* 4->0->5->2->3->1 */
    SIX_COST_STRICT_CHECK(4, 0, 5, 2, 3, 1);
    /* 4->0->5->3->1->2 */
    SIX_COST_STRICT_CHECK(4, 0, 5, 3, 1, 2);
    /* 4->0->5->3->2->1 */
    SIX_COST_STRICT_CHECK(4, 0, 5, 3, 2, 1);
    /* 4->1->0->2->3->5 */
    SIX_COST_STRICT_CHECK(4, 1, 0, 2, 3, 5);
    /* 4->1->0->2->5->3 */
    SIX_COST_STRICT_CHECK(4, 1, 0, 2, 5, 3);
    /* 4->1->0->3->2->5 */
    SIX_COST_STRICT_CHECK(4, 1, 0, 3, 2, 5);
    /* 4->1->0->3->5->2 */
    SIX_COST_STRICT_CHECK(4, 1, 0, 3, 5, 2);
    /* 4->1->0->5->2->3 */
    SIX_COST_STRICT_CHECK(4, 1, 0, 5, 2, 3);
    /* 4->1->0->5->3->2 */
    SIX_COST_STRICT_CHECK(4, 1, 0, 5, 3, 2);
    /* 4->1->2->0->3->5 */
    SIX_COST_STRICT_CHECK(4, 1, 2, 0, 3, 5);
    /* 4->1->2->0->5->3 */
    SIX_COST_STRICT_CHECK(4, 1, 2, 0, 5, 3);
    /* 4->1->2->3->0->5 */
    SIX_COST_STRICT_CHECK(4, 1, 2, 3, 0, 5);
    /* 4->1->2->3->5->0 */
    SIX_COST_STRICT_CHECK(4, 1, 2, 3, 5, 0);
    /* 4->1->2->5->0->3 */
    SIX_COST_STRICT_CHECK(4, 1, 2, 5, 0, 3);
    /* 4->1->2->5->3->0 */
    SIX_COST_STRICT_CHECK(4, 1, 2, 5, 3, 0);
    /* 4->1->3->0->2->5 */
    SIX_COST_STRICT_CHECK(4, 1, 3, 0, 2, 5);
    /* 4->1->3->0->5->2 */
    SIX_COST_STRICT_CHECK(4, 1, 3, 0, 5, 2);
    /* 4->1->3->2->0->5 */
    SIX_COST_STRICT_CHECK(4, 1, 3, 2, 0, 5);
    /* 4->1->3->2->5->0 */
    SIX_COST_STRICT_CHECK(4, 1, 3, 2, 5, 0);
    /* 4->1->3->5->0->2 */
    SIX_COST_STRICT_CHECK(4, 1, 3, 5, 0, 2);
    /* 4->1->3->5->2->0 */
    SIX_COST_STRICT_CHECK(4, 1, 3, 5, 2, 0);
    /* 4->1->5->0->2->3 */
    SIX_COST_STRICT_CHECK(4, 1, 5, 0, 2, 3);
    /* 4->1->5->0->3->2 */
    SIX_COST_STRICT_CHECK(4, 1, 5, 0, 3, 2);
    /* 4->1->5->2->0->3 */
    SIX_COST_STRICT_CHECK(4, 1, 5, 2, 0, 3);
    /* 4->1->5->2->3->0 */
    SIX_COST_STRICT_CHECK(4, 1, 5, 2, 3, 0);
    /* 4->1->5->3->0->2 */
    SIX_COST_STRICT_CHECK(4, 1, 5, 3, 0, 2);
    /* 4->1->5->3->2->0 */
    SIX_COST_STRICT_CHECK(4, 1, 5, 3, 2, 0);
    /* 4->2->0->1->3->5 */
    SIX_COST_STRICT_CHECK(4, 2, 0, 1, 3, 5);
    /* 4->2->0->1->5->3 */
    SIX_COST_STRICT_CHECK(4, 2, 0, 1, 5, 3);
    /* 4->2->0->3->1->5 */
    SIX_COST_STRICT_CHECK(4, 2, 0, 3, 1, 5);
    /* 4->2->0->3->5->1 */
    SIX_COST_STRICT_CHECK(4, 2, 0, 3, 5, 1);
    /* 4->2->0->5->1->3 */
    SIX_COST_STRICT_CHECK(4, 2, 0, 5, 1, 3);
    /* 4->2->0->5->3->1 */
    SIX_COST_STRICT_CHECK(4, 2, 0, 5, 3, 1);
    /* 4->2->1->0->3->5 */
    SIX_COST_STRICT_CHECK(4, 2, 1, 0, 3, 5);
    /* 4->2->1->0->5->3 */
    SIX_COST_STRICT_CHECK(4, 2, 1, 0, 5, 3);
    /* 4->2->1->3->0->5 */
    SIX_COST_STRICT_CHECK(4, 2, 1, 3, 0, 5);
    /* 4->2->1->3->5->0 */
    SIX_COST_STRICT_CHECK(4, 2, 1, 3, 5, 0);
    /* 4->2->1->5->0->3 */
    SIX_COST_STRICT_CHECK(4, 2, 1, 5, 0, 3);
    /* 4->2->1->5->3->0 */
    SIX_COST_STRICT_CHECK(4, 2, 1, 5, 3, 0);
    /* 4->2->3->0->1->5 */
    SIX_COST_STRICT_CHECK(4, 2, 3, 0, 1, 5);
    /* 4->2->3->0->5->1 */
    SIX_COST_STRICT_CHECK(4, 2, 3, 0, 5, 1);
    /* 4->2->3->1->0->5 */
    SIX_COST_STRICT_CHECK(4, 2, 3, 1, 0, 5);
    /* 4->2->3->1->5->0 */
    SIX_COST_STRICT_CHECK(4, 2, 3, 1, 5, 0);
    /* 4->2->3->5->0->1 */
    SIX_COST_STRICT_CHECK(4, 2, 3, 5, 0, 1);
    /* 4->2->3->5->1->0 */
    SIX_COST_STRICT_CHECK(4, 2, 3, 5, 1, 0);
    /* 4->2->5->0->1->3 */
    SIX_COST_STRICT_CHECK(4, 2, 5, 0, 1, 3);
    /* 4->2->5->0->3->1 */
    SIX_COST_STRICT_CHECK(4, 2, 5, 0, 3, 1);
    /* 4->2->5->1->0->3 */
    SIX_COST_STRICT_CHECK(4, 2, 5, 1, 0, 3);
    /* 4->2->5->1->3->0 */
    SIX_COST_STRICT_CHECK(4, 2, 5, 1, 3, 0);
    /* 4->2->5->3->0->1 */
    SIX_COST_STRICT_CHECK(4, 2, 5, 3, 0, 1);
    /* 4->2->5->3->1->0 */
    SIX_COST_STRICT_CHECK(4, 2, 5, 3, 1, 0);
    /* 4->3->0->1->2->5 */
    SIX_COST_STRICT_CHECK(4, 3, 0, 1, 2, 5);
    /* 4->3->0->1->5->2 */
    SIX_COST_STRICT_CHECK(4, 3, 0, 1, 5, 2);
    /* 4->3->0->2->1->5 */
    SIX_COST_STRICT_CHECK(4, 3, 0, 2, 1, 5);
    /* 4->3->0->2->5->1 */
    SIX_COST_STRICT_CHECK(4, 3, 0, 2, 5, 1);
    /* 4->3->0->5->1->2 */
    SIX_COST_STRICT_CHECK(4, 3, 0, 5, 1, 2);
    /* 4->3->0->5->2->1 */
    SIX_COST_STRICT_CHECK(4, 3, 0, 5, 2, 1);
    /* 4->3->1->0->2->5 */
    SIX_COST_STRICT_CHECK(4, 3, 1, 0, 2, 5);
    /* 4->3->1->0->5->2 */
    SIX_COST_STRICT_CHECK(4, 3, 1, 0, 5, 2);
    /* 4->3->1->2->0->5 */
    SIX_COST_STRICT_CHECK(4, 3, 1, 2, 0, 5);
    /* 4->3->1->2->5->0 */
    SIX_COST_STRICT_CHECK(4, 3, 1, 2, 5, 0);
    /* 4->3->1->5->0->2 */
    SIX_COST_STRICT_CHECK(4, 3, 1, 5, 0, 2);
    /* 4->3->1->5->2->0 */
    SIX_COST_STRICT_CHECK(4, 3, 1, 5, 2, 0);
    /* 4->3->2->0->1->5 */
    SIX_COST_STRICT_CHECK(4, 3, 2, 0, 1, 5);
    /* 4->3->2->0->5->1 */
    SIX_COST_STRICT_CHECK(4, 3, 2, 0, 5, 1);
    /* 4->3->2->1->0->5 */
    SIX_COST_STRICT_CHECK(4, 3, 2, 1, 0, 5);
    /* 4->3->2->1->5->0 */
    SIX_COST_STRICT_CHECK(4, 3, 2, 1, 5, 0);
    /* 4->3->2->5->0->1 */
    SIX_COST_STRICT_CHECK(4, 3, 2, 5, 0, 1);
    /* 4->3->2->5->1->0 */
    SIX_COST_STRICT_CHECK(4, 3, 2, 5, 1, 0);
    /* 4->3->5->0->1->2 */
    SIX_COST_STRICT_CHECK(4, 3, 5, 0, 1, 2);
    /* 4->3->5->0->2->1 */
    SIX_COST_STRICT_CHECK(4, 3, 5, 0, 2, 1);
    /* 4->3->5->1->0->2 */
    SIX_COST_STRICT_CHECK(4, 3, 5, 1, 0, 2);
    /* 4->3->5->1->2->0 */
    SIX_COST_STRICT_CHECK(4, 3, 5, 1, 2, 0);
    /* 4->3->5->2->0->1 */
    SIX_COST_STRICT_CHECK(4, 3, 5, 2, 0, 1);
    /* 4->3->5->2->1->0 */
    SIX_COST_STRICT_CHECK(4, 3, 5, 2, 1, 0);
    /* 4->5->0->1->2->3 */
    SIX_COST_STRICT_CHECK(4, 5, 0, 1, 2, 3);
    /* 4->5->0->1->3->2 */
    SIX_COST_STRICT_CHECK(4, 5, 0, 1, 3, 2);
    /* 4->5->0->2->1->3 */
    SIX_COST_STRICT_CHECK(4, 5, 0, 2, 1, 3);
    /* 4->5->0->2->3->1 */
    SIX_COST_STRICT_CHECK(4, 5, 0, 2, 3, 1);
    /* 4->5->0->3->1->2 */
    SIX_COST_STRICT_CHECK(4, 5, 0, 3, 1, 2);
    /* 4->5->0->3->2->1 */
    SIX_COST_STRICT_CHECK(4, 5, 0, 3, 2, 1);
    /* 4->5->1->0->2->3 */
    SIX_COST_STRICT_CHECK(4, 5, 1, 0, 2, 3);
    /* 4->5->1->0->3->2 */
    SIX_COST_STRICT_CHECK(4, 5, 1, 0, 3, 2);
    /* 4->5->1->2->0->3 */
    SIX_COST_STRICT_CHECK(4, 5, 1, 2, 0, 3);
    /* 4->5->1->2->3->0 */
    SIX_COST_STRICT_CHECK(4, 5, 1, 2, 3, 0);
    /* 4->5->1->3->0->2 */
    SIX_COST_STRICT_CHECK(4, 5, 1, 3, 0, 2);
    /* 4->5->1->3->2->0 */
    SIX_COST_STRICT_CHECK(4, 5, 1, 3, 2, 0);
    /* 4->5->2->0->1->3 */
    SIX_COST_STRICT_CHECK(4, 5, 2, 0, 1, 3);
    /* 4->5->2->0->3->1 */
    SIX_COST_STRICT_CHECK(4, 5, 2, 0, 3, 1);
    /* 4->5->2->1->0->3 */
    SIX_COST_STRICT_CHECK(4, 5, 2, 1, 0, 3);
    /* 4->5->2->1->3->0 */
    SIX_COST_STRICT_CHECK(4, 5, 2, 1, 3, 0);
    /* 4->5->2->3->0->1 */
    SIX_COST_STRICT_CHECK(4, 5, 2, 3, 0, 1);
    /* 4->5->2->3->1->0 */
    SIX_COST_STRICT_CHECK(4, 5, 2, 3, 1, 0);
    /* 4->5->3->0->1->2 */
    SIX_COST_STRICT_CHECK(4, 5, 3, 0, 1, 2);
    /* 4->5->3->0->2->1 */
    SIX_COST_STRICT_CHECK(4, 5, 3, 0, 2, 1);
    /* 4->5->3->1->0->2 */
    SIX_COST_STRICT_CHECK(4, 5, 3, 1, 0, 2);
    /* 4->5->3->1->2->0 */
    SIX_COST_STRICT_CHECK(4, 5, 3, 1, 2, 0);
    /* 4->5->3->2->0->1 */
    SIX_COST_STRICT_CHECK(4, 5, 3, 2, 0, 1);
    /* 4->5->3->2->1->0 */
    SIX_COST_STRICT_CHECK(4, 5, 3, 2, 1, 0);
  }

  if(prob->sjob[j[5]]->tno < prob->sjob[j[0]]->tno) {
    /* 5->0->1->2->3->4 */
    SIX_COST_CHECK(5, 0, 1, 2, 3, 4);
    /* 5->0->1->2->4->3 */
    SIX_COST_CHECK(5, 0, 1, 2, 4, 3);
    /* 5->0->1->3->2->4 */
    SIX_COST_CHECK(5, 0, 1, 3, 2, 4);
    /* 5->0->1->3->4->2 */
    SIX_COST_CHECK(5, 0, 1, 3, 4, 2);
    /* 5->0->1->4->2->3 */
    SIX_COST_CHECK(5, 0, 1, 4, 2, 3);
    /* 5->0->1->4->3->2 */
    SIX_COST_CHECK(5, 0, 1, 4, 3, 2);
    /* 5->0->2->1->3->4 */
    SIX_COST_CHECK(5, 0, 2, 1, 3, 4);
    /* 5->0->2->1->4->3 */
    SIX_COST_CHECK(5, 0, 2, 1, 4, 3);
    /* 5->0->2->3->1->4 */
    SIX_COST_CHECK(5, 0, 2, 3, 1, 4);
    /* 5->0->2->3->4->1 */
    SIX_COST_CHECK(5, 0, 2, 3, 4, 1);
    /* 5->0->2->4->1->3 */
    SIX_COST_CHECK(5, 0, 2, 4, 1, 3);
    /* 5->0->2->4->3->1 */
    SIX_COST_CHECK(5, 0, 2, 4, 3, 1);
    /* 5->0->3->1->2->4 */
    SIX_COST_CHECK(5, 0, 3, 1, 2, 4);
    /* 5->0->3->1->4->2 */
    SIX_COST_CHECK(5, 0, 3, 1, 4, 2);
    /* 5->0->3->2->1->4 */
    SIX_COST_CHECK(5, 0, 3, 2, 1, 4);
    /* 5->0->3->2->4->1 */
    SIX_COST_CHECK(5, 0, 3, 2, 4, 1);
    /* 5->0->3->4->1->2 */
    SIX_COST_CHECK(5, 0, 3, 4, 1, 2);
    /* 5->0->3->4->2->1 */
    SIX_COST_CHECK(5, 0, 3, 4, 2, 1);
    /* 5->0->4->1->2->3 */
    SIX_COST_CHECK(5, 0, 4, 1, 2, 3);
    /* 5->0->4->1->3->2 */
    SIX_COST_CHECK(5, 0, 4, 1, 3, 2);
    /* 5->0->4->2->1->3 */
    SIX_COST_CHECK(5, 0, 4, 2, 1, 3);
    /* 5->0->4->2->3->1 */
    SIX_COST_CHECK(5, 0, 4, 2, 3, 1);
    /* 5->0->4->3->1->2 */
    SIX_COST_CHECK(5, 0, 4, 3, 1, 2);
    /* 5->0->4->3->2->1 */
    SIX_COST_CHECK(5, 0, 4, 3, 2, 1);
    /* 5->1->0->2->3->4 */
    SIX_COST_CHECK(5, 1, 0, 2, 3, 4);
    /* 5->1->0->2->4->3 */
    SIX_COST_CHECK(5, 1, 0, 2, 4, 3);
    /* 5->1->0->3->2->4 */
    SIX_COST_CHECK(5, 1, 0, 3, 2, 4);
    /* 5->1->0->3->4->2 */
    SIX_COST_CHECK(5, 1, 0, 3, 4, 2);
    /* 5->1->0->4->2->3 */
    SIX_COST_CHECK(5, 1, 0, 4, 2, 3);
    /* 5->1->0->4->3->2 */
    SIX_COST_CHECK(5, 1, 0, 4, 3, 2);
    /* 5->1->2->0->3->4 */
    SIX_COST_CHECK(5, 1, 2, 0, 3, 4);
    /* 5->1->2->0->4->3 */
    SIX_COST_CHECK(5, 1, 2, 0, 4, 3);
    /* 5->1->2->3->0->4 */
    SIX_COST_CHECK(5, 1, 2, 3, 0, 4);
    /* 5->1->2->3->4->0 */
    SIX_COST_CHECK(5, 1, 2, 3, 4, 0);
    /* 5->1->2->4->0->3 */
    SIX_COST_CHECK(5, 1, 2, 4, 0, 3);
    /* 5->1->2->4->3->0 */
    SIX_COST_CHECK(5, 1, 2, 4, 3, 0);
    /* 5->1->3->0->2->4 */
    SIX_COST_CHECK(5, 1, 3, 0, 2, 4);
    /* 5->1->3->0->4->2 */
    SIX_COST_CHECK(5, 1, 3, 0, 4, 2);
    /* 5->1->3->2->0->4 */
    SIX_COST_CHECK(5, 1, 3, 2, 0, 4);
    /* 5->1->3->2->4->0 */
    SIX_COST_CHECK(5, 1, 3, 2, 4, 0);
    /* 5->1->3->4->0->2 */
    SIX_COST_CHECK(5, 1, 3, 4, 0, 2);
    /* 5->1->3->4->2->0 */
    SIX_COST_CHECK(5, 1, 3, 4, 2, 0);
    /* 5->1->4->0->2->3 */
    SIX_COST_CHECK(5, 1, 4, 0, 2, 3);
    /* 5->1->4->0->3->2 */
    SIX_COST_CHECK(5, 1, 4, 0, 3, 2);
    /* 5->1->4->2->0->3 */
    SIX_COST_CHECK(5, 1, 4, 2, 0, 3);
    /* 5->1->4->2->3->0 */
    SIX_COST_CHECK(5, 1, 4, 2, 3, 0);
    /* 5->1->4->3->0->2 */
    SIX_COST_CHECK(5, 1, 4, 3, 0, 2);
    /* 5->1->4->3->2->0 */
    SIX_COST_CHECK(5, 1, 4, 3, 2, 0);
    /* 5->2->0->1->3->4 */
    SIX_COST_CHECK(5, 2, 0, 1, 3, 4);
    /* 5->2->0->1->4->3 */
    SIX_COST_CHECK(5, 2, 0, 1, 4, 3);
    /* 5->2->0->3->1->4 */
    SIX_COST_CHECK(5, 2, 0, 3, 1, 4);
    /* 5->2->0->3->4->1 */
    SIX_COST_CHECK(5, 2, 0, 3, 4, 1);
    /* 5->2->0->4->1->3 */
    SIX_COST_CHECK(5, 2, 0, 4, 1, 3);
    /* 5->2->0->4->3->1 */
    SIX_COST_CHECK(5, 2, 0, 4, 3, 1);
    /* 5->2->1->0->3->4 */
    SIX_COST_CHECK(5, 2, 1, 0, 3, 4);
    /* 5->2->1->0->4->3 */
    SIX_COST_CHECK(5, 2, 1, 0, 4, 3);
    /* 5->2->1->3->0->4 */
    SIX_COST_CHECK(5, 2, 1, 3, 0, 4);
    /* 5->2->1->3->4->0 */
    SIX_COST_CHECK(5, 2, 1, 3, 4, 0);
    /* 5->2->1->4->0->3 */
    SIX_COST_CHECK(5, 2, 1, 4, 0, 3);
    /* 5->2->1->4->3->0 */
    SIX_COST_CHECK(5, 2, 1, 4, 3, 0);
    /* 5->2->3->0->1->4 */
    SIX_COST_CHECK(5, 2, 3, 0, 1, 4);
    /* 5->2->3->0->4->1 */
    SIX_COST_CHECK(5, 2, 3, 0, 4, 1);
    /* 5->2->3->1->0->4 */
    SIX_COST_CHECK(5, 2, 3, 1, 0, 4);
    /* 5->2->3->1->4->0 */
    SIX_COST_CHECK(5, 2, 3, 1, 4, 0);
    /* 5->2->3->4->0->1 */
    SIX_COST_CHECK(5, 2, 3, 4, 0, 1);
    /* 5->2->3->4->1->0 */
    SIX_COST_CHECK(5, 2, 3, 4, 1, 0);
    /* 5->2->4->0->1->3 */
    SIX_COST_CHECK(5, 2, 4, 0, 1, 3);
    /* 5->2->4->0->3->1 */
    SIX_COST_CHECK(5, 2, 4, 0, 3, 1);
    /* 5->2->4->1->0->3 */
    SIX_COST_CHECK(5, 2, 4, 1, 0, 3);
    /* 5->2->4->1->3->0 */
    SIX_COST_CHECK(5, 2, 4, 1, 3, 0);
    /* 5->2->4->3->0->1 */
    SIX_COST_CHECK(5, 2, 4, 3, 0, 1);
    /* 5->2->4->3->1->0 */
    SIX_COST_CHECK(5, 2, 4, 3, 1, 0);
    /* 5->3->0->1->2->4 */
    SIX_COST_CHECK(5, 3, 0, 1, 2, 4);
    /* 5->3->0->1->4->2 */
    SIX_COST_CHECK(5, 3, 0, 1, 4, 2);
    /* 5->3->0->2->1->4 */
    SIX_COST_CHECK(5, 3, 0, 2, 1, 4);
    /* 5->3->0->2->4->1 */
    SIX_COST_CHECK(5, 3, 0, 2, 4, 1);
    /* 5->3->0->4->1->2 */
    SIX_COST_CHECK(5, 3, 0, 4, 1, 2);
    /* 5->3->0->4->2->1 */
    SIX_COST_CHECK(5, 3, 0, 4, 2, 1);
    /* 5->3->1->0->2->4 */
    SIX_COST_CHECK(5, 3, 1, 0, 2, 4);
    /* 5->3->1->0->4->2 */
    SIX_COST_CHECK(5, 3, 1, 0, 4, 2);
    /* 5->3->1->2->0->4 */
    SIX_COST_CHECK(5, 3, 1, 2, 0, 4);
    /* 5->3->1->2->4->0 */
    SIX_COST_CHECK(5, 3, 1, 2, 4, 0);
    /* 5->3->1->4->0->2 */
    SIX_COST_CHECK(5, 3, 1, 4, 0, 2);
    /* 5->3->1->4->2->0 */
    SIX_COST_CHECK(5, 3, 1, 4, 2, 0);
    /* 5->3->2->0->1->4 */
    SIX_COST_CHECK(5, 3, 2, 0, 1, 4);
    /* 5->3->2->0->4->1 */
    SIX_COST_CHECK(5, 3, 2, 0, 4, 1);
    /* 5->3->2->1->0->4 */
    SIX_COST_CHECK(5, 3, 2, 1, 0, 4);
    /* 5->3->2->1->4->0 */
    SIX_COST_CHECK(5, 3, 2, 1, 4, 0);
    /* 5->3->2->4->0->1 */
    SIX_COST_CHECK(5, 3, 2, 4, 0, 1);
    /* 5->3->2->4->1->0 */
    SIX_COST_CHECK(5, 3, 2, 4, 1, 0);
    /* 5->3->4->0->1->2 */
    SIX_COST_CHECK(5, 3, 4, 0, 1, 2);
    /* 5->3->4->0->2->1 */
    SIX_COST_CHECK(5, 3, 4, 0, 2, 1);
    /* 5->3->4->1->0->2 */
    SIX_COST_CHECK(5, 3, 4, 1, 0, 2);
    /* 5->3->4->1->2->0 */
    SIX_COST_CHECK(5, 3, 4, 1, 2, 0);
    /* 5->3->4->2->0->1 */
    SIX_COST_CHECK(5, 3, 4, 2, 0, 1);
    /* 5->3->4->2->1->0 */
    SIX_COST_CHECK(5, 3, 4, 2, 1, 0);
    /* 5->4->0->1->2->3 */
    SIX_COST_CHECK(5, 4, 0, 1, 2, 3);
    /* 5->4->0->1->3->2 */
    SIX_COST_CHECK(5, 4, 0, 1, 3, 2);
    /* 5->4->0->2->1->3 */
    SIX_COST_CHECK(5, 4, 0, 2, 1, 3);
    /* 5->4->0->2->3->1 */
    SIX_COST_CHECK(5, 4, 0, 2, 3, 1);
    /* 5->4->0->3->1->2 */
    SIX_COST_CHECK(5, 4, 0, 3, 1, 2);
    /* 5->4->0->3->2->1 */
    SIX_COST_CHECK(5, 4, 0, 3, 2, 1);
    /* 5->4->1->0->2->3 */
    SIX_COST_CHECK(5, 4, 1, 0, 2, 3);
    /* 5->4->1->0->3->2 */
    SIX_COST_CHECK(5, 4, 1, 0, 3, 2);
    /* 5->4->1->2->0->3 */
    SIX_COST_CHECK(5, 4, 1, 2, 0, 3);
    /* 5->4->1->2->3->0 */
    SIX_COST_CHECK(5, 4, 1, 2, 3, 0);
    /* 5->4->1->3->0->2 */
    SIX_COST_CHECK(5, 4, 1, 3, 0, 2);
    /* 5->4->1->3->2->0 */
    SIX_COST_CHECK(5, 4, 1, 3, 2, 0);
    /* 5->4->2->0->1->3 */
    SIX_COST_CHECK(5, 4, 2, 0, 1, 3);
    /* 5->4->2->0->3->1 */
    SIX_COST_CHECK(5, 4, 2, 0, 3, 1);
    /* 5->4->2->1->0->3 */
    SIX_COST_CHECK(5, 4, 2, 1, 0, 3);
    /* 5->4->2->1->3->0 */
    SIX_COST_CHECK(5, 4, 2, 1, 3, 0);
    /* 5->4->2->3->0->1 */
    SIX_COST_CHECK(5, 4, 2, 3, 0, 1);
    /* 5->4->2->3->1->0 */
    SIX_COST_CHECK(5, 4, 2, 3, 1, 0);
    /* 5->4->3->0->1->2 */
    SIX_COST_CHECK(5, 4, 3, 0, 1, 2);
    /* 5->4->3->0->2->1 */
    SIX_COST_CHECK(5, 4, 3, 0, 2, 1);
    /* 5->4->3->1->0->2 */
    SIX_COST_CHECK(5, 4, 3, 1, 0, 2);
    /* 5->4->3->1->2->0 */
    SIX_COST_CHECK(5, 4, 3, 1, 2, 0);
    /* 5->4->3->2->0->1 */
    SIX_COST_CHECK(5, 4, 3, 2, 0, 1);
    /* 5->4->3->2->1->0 */
    SIX_COST_CHECK(5, 4, 3, 2, 1, 0);
  } else {
    /* 5->0->1->2->3->4 */
    SIX_COST_STRICT_CHECK(5, 0, 1, 2, 3, 4);
    /* 5->0->1->2->4->3 */
    SIX_COST_STRICT_CHECK(5, 0, 1, 2, 4, 3);
    /* 5->0->1->3->2->4 */
    SIX_COST_STRICT_CHECK(5, 0, 1, 3, 2, 4);
    /* 5->0->1->3->4->2 */
    SIX_COST_STRICT_CHECK(5, 0, 1, 3, 4, 2);
    /* 5->0->1->4->2->3 */
    SIX_COST_STRICT_CHECK(5, 0, 1, 4, 2, 3);
    /* 5->0->1->4->3->2 */
    SIX_COST_STRICT_CHECK(5, 0, 1, 4, 3, 2);
    /* 5->0->2->1->3->4 */
    SIX_COST_STRICT_CHECK(5, 0, 2, 1, 3, 4);
    /* 5->0->2->1->4->3 */
    SIX_COST_STRICT_CHECK(5, 0, 2, 1, 4, 3);
    /* 5->0->2->3->1->4 */
    SIX_COST_STRICT_CHECK(5, 0, 2, 3, 1, 4);
    /* 5->0->2->3->4->1 */
    SIX_COST_STRICT_CHECK(5, 0, 2, 3, 4, 1);
    /* 5->0->2->4->1->3 */
    SIX_COST_STRICT_CHECK(5, 0, 2, 4, 1, 3);
    /* 5->0->2->4->3->1 */
    SIX_COST_STRICT_CHECK(5, 0, 2, 4, 3, 1);
    /* 5->0->3->1->2->4 */
    SIX_COST_STRICT_CHECK(5, 0, 3, 1, 2, 4);
    /* 5->0->3->1->4->2 */
    SIX_COST_STRICT_CHECK(5, 0, 3, 1, 4, 2);
    /* 5->0->3->2->1->4 */
    SIX_COST_STRICT_CHECK(5, 0, 3, 2, 1, 4);
    /* 5->0->3->2->4->1 */
    SIX_COST_STRICT_CHECK(5, 0, 3, 2, 4, 1);
    /* 5->0->3->4->1->2 */
    SIX_COST_STRICT_CHECK(5, 0, 3, 4, 1, 2);
    /* 5->0->3->4->2->1 */
    SIX_COST_STRICT_CHECK(5, 0, 3, 4, 2, 1);
    /* 5->0->4->1->2->3 */
    SIX_COST_STRICT_CHECK(5, 0, 4, 1, 2, 3);
    /* 5->0->4->1->3->2 */
    SIX_COST_STRICT_CHECK(5, 0, 4, 1, 3, 2);
    /* 5->0->4->2->1->3 */
    SIX_COST_STRICT_CHECK(5, 0, 4, 2, 1, 3);
    /* 5->0->4->2->3->1 */
    SIX_COST_STRICT_CHECK(5, 0, 4, 2, 3, 1);
    /* 5->0->4->3->1->2 */
    SIX_COST_STRICT_CHECK(5, 0, 4, 3, 1, 2);
    /* 5->0->4->3->2->1 */
    SIX_COST_STRICT_CHECK(5, 0, 4, 3, 2, 1);
    /* 5->1->0->2->3->4 */
    SIX_COST_STRICT_CHECK(5, 1, 0, 2, 3, 4);
    /* 5->1->0->2->4->3 */
    SIX_COST_STRICT_CHECK(5, 1, 0, 2, 4, 3);
    /* 5->1->0->3->2->4 */
    SIX_COST_STRICT_CHECK(5, 1, 0, 3, 2, 4);
    /* 5->1->0->3->4->2 */
    SIX_COST_STRICT_CHECK(5, 1, 0, 3, 4, 2);
    /* 5->1->0->4->2->3 */
    SIX_COST_STRICT_CHECK(5, 1, 0, 4, 2, 3);
    /* 5->1->0->4->3->2 */
    SIX_COST_STRICT_CHECK(5, 1, 0, 4, 3, 2);
    /* 5->1->2->0->3->4 */
    SIX_COST_STRICT_CHECK(5, 1, 2, 0, 3, 4);
    /* 5->1->2->0->4->3 */
    SIX_COST_STRICT_CHECK(5, 1, 2, 0, 4, 3);
    /* 5->1->2->3->0->4 */
    SIX_COST_STRICT_CHECK(5, 1, 2, 3, 0, 4);
    /* 5->1->2->3->4->0 */
    SIX_COST_STRICT_CHECK(5, 1, 2, 3, 4, 0);
    /* 5->1->2->4->0->3 */
    SIX_COST_STRICT_CHECK(5, 1, 2, 4, 0, 3);
    /* 5->1->2->4->3->0 */
    SIX_COST_STRICT_CHECK(5, 1, 2, 4, 3, 0);
    /* 5->1->3->0->2->4 */
    SIX_COST_STRICT_CHECK(5, 1, 3, 0, 2, 4);
    /* 5->1->3->0->4->2 */
    SIX_COST_STRICT_CHECK(5, 1, 3, 0, 4, 2);
    /* 5->1->3->2->0->4 */
    SIX_COST_STRICT_CHECK(5, 1, 3, 2, 0, 4);
    /* 5->1->3->2->4->0 */
    SIX_COST_STRICT_CHECK(5, 1, 3, 2, 4, 0);
    /* 5->1->3->4->0->2 */
    SIX_COST_STRICT_CHECK(5, 1, 3, 4, 0, 2);
    /* 5->1->3->4->2->0 */
    SIX_COST_STRICT_CHECK(5, 1, 3, 4, 2, 0);
    /* 5->1->4->0->2->3 */
    SIX_COST_STRICT_CHECK(5, 1, 4, 0, 2, 3);
    /* 5->1->4->0->3->2 */
    SIX_COST_STRICT_CHECK(5, 1, 4, 0, 3, 2);
    /* 5->1->4->2->0->3 */
    SIX_COST_STRICT_CHECK(5, 1, 4, 2, 0, 3);
    /* 5->1->4->2->3->0 */
    SIX_COST_STRICT_CHECK(5, 1, 4, 2, 3, 0);
    /* 5->1->4->3->0->2 */
    SIX_COST_STRICT_CHECK(5, 1, 4, 3, 0, 2);
    /* 5->1->4->3->2->0 */
    SIX_COST_STRICT_CHECK(5, 1, 4, 3, 2, 0);
    /* 5->2->0->1->3->4 */
    SIX_COST_STRICT_CHECK(5, 2, 0, 1, 3, 4);
    /* 5->2->0->1->4->3 */
    SIX_COST_STRICT_CHECK(5, 2, 0, 1, 4, 3);
    /* 5->2->0->3->1->4 */
    SIX_COST_STRICT_CHECK(5, 2, 0, 3, 1, 4);
    /* 5->2->0->3->4->1 */
    SIX_COST_STRICT_CHECK(5, 2, 0, 3, 4, 1);
    /* 5->2->0->4->1->3 */
    SIX_COST_STRICT_CHECK(5, 2, 0, 4, 1, 3);
    /* 5->2->0->4->3->1 */
    SIX_COST_STRICT_CHECK(5, 2, 0, 4, 3, 1);
    /* 5->2->1->0->3->4 */
    SIX_COST_STRICT_CHECK(5, 2, 1, 0, 3, 4);
    /* 5->2->1->0->4->3 */
    SIX_COST_STRICT_CHECK(5, 2, 1, 0, 4, 3);
    /* 5->2->1->3->0->4 */
    SIX_COST_STRICT_CHECK(5, 2, 1, 3, 0, 4);
    /* 5->2->1->3->4->0 */
    SIX_COST_STRICT_CHECK(5, 2, 1, 3, 4, 0);
    /* 5->2->1->4->0->3 */
    SIX_COST_STRICT_CHECK(5, 2, 1, 4, 0, 3);
    /* 5->2->1->4->3->0 */
    SIX_COST_STRICT_CHECK(5, 2, 1, 4, 3, 0);
    /* 5->2->3->0->1->4 */
    SIX_COST_STRICT_CHECK(5, 2, 3, 0, 1, 4);
    /* 5->2->3->0->4->1 */
    SIX_COST_STRICT_CHECK(5, 2, 3, 0, 4, 1);
    /* 5->2->3->1->0->4 */
    SIX_COST_STRICT_CHECK(5, 2, 3, 1, 0, 4);
    /* 5->2->3->1->4->0 */
    SIX_COST_STRICT_CHECK(5, 2, 3, 1, 4, 0);
    /* 5->2->3->4->0->1 */
    SIX_COST_STRICT_CHECK(5, 2, 3, 4, 0, 1);
    /* 5->2->3->4->1->0 */
    SIX_COST_STRICT_CHECK(5, 2, 3, 4, 1, 0);
    /* 5->2->4->0->1->3 */
    SIX_COST_STRICT_CHECK(5, 2, 4, 0, 1, 3);
    /* 5->2->4->0->3->1 */
    SIX_COST_STRICT_CHECK(5, 2, 4, 0, 3, 1);
    /* 5->2->4->1->0->3 */
    SIX_COST_STRICT_CHECK(5, 2, 4, 1, 0, 3);
    /* 5->2->4->1->3->0 */
    SIX_COST_STRICT_CHECK(5, 2, 4, 1, 3, 0);
    /* 5->2->4->3->0->1 */
    SIX_COST_STRICT_CHECK(5, 2, 4, 3, 0, 1);
    /* 5->2->4->3->1->0 */
    SIX_COST_STRICT_CHECK(5, 2, 4, 3, 1, 0);
    /* 5->3->0->1->2->4 */
    SIX_COST_STRICT_CHECK(5, 3, 0, 1, 2, 4);
    /* 5->3->0->1->4->2 */
    SIX_COST_STRICT_CHECK(5, 3, 0, 1, 4, 2);
    /* 5->3->0->2->1->4 */
    SIX_COST_STRICT_CHECK(5, 3, 0, 2, 1, 4);
    /* 5->3->0->2->4->1 */
    SIX_COST_STRICT_CHECK(5, 3, 0, 2, 4, 1);
    /* 5->3->0->4->1->2 */
    SIX_COST_STRICT_CHECK(5, 3, 0, 4, 1, 2);
    /* 5->3->0->4->2->1 */
    SIX_COST_STRICT_CHECK(5, 3, 0, 4, 2, 1);
    /* 5->3->1->0->2->4 */
    SIX_COST_STRICT_CHECK(5, 3, 1, 0, 2, 4);
    /* 5->3->1->0->4->2 */
    SIX_COST_STRICT_CHECK(5, 3, 1, 0, 4, 2);
    /* 5->3->1->2->0->4 */
    SIX_COST_STRICT_CHECK(5, 3, 1, 2, 0, 4);
    /* 5->3->1->2->4->0 */
    SIX_COST_STRICT_CHECK(5, 3, 1, 2, 4, 0);
    /* 5->3->1->4->0->2 */
    SIX_COST_STRICT_CHECK(5, 3, 1, 4, 0, 2);
    /* 5->3->1->4->2->0 */
    SIX_COST_STRICT_CHECK(5, 3, 1, 4, 2, 0);
    /* 5->3->2->0->1->4 */
    SIX_COST_STRICT_CHECK(5, 3, 2, 0, 1, 4);
    /* 5->3->2->0->4->1 */
    SIX_COST_STRICT_CHECK(5, 3, 2, 0, 4, 1);
    /* 5->3->2->1->0->4 */
    SIX_COST_STRICT_CHECK(5, 3, 2, 1, 0, 4);
    /* 5->3->2->1->4->0 */
    SIX_COST_STRICT_CHECK(5, 3, 2, 1, 4, 0);
    /* 5->3->2->4->0->1 */
    SIX_COST_STRICT_CHECK(5, 3, 2, 4, 0, 1);
    /* 5->3->2->4->1->0 */
    SIX_COST_STRICT_CHECK(5, 3, 2, 4, 1, 0);
    /* 5->3->4->0->1->2 */
    SIX_COST_STRICT_CHECK(5, 3, 4, 0, 1, 2);
    /* 5->3->4->0->2->1 */
    SIX_COST_STRICT_CHECK(5, 3, 4, 0, 2, 1);
    /* 5->3->4->1->0->2 */
    SIX_COST_STRICT_CHECK(5, 3, 4, 1, 0, 2);
    /* 5->3->4->1->2->0 */
    SIX_COST_STRICT_CHECK(5, 3, 4, 1, 2, 0);
    /* 5->3->4->2->0->1 */
    SIX_COST_STRICT_CHECK(5, 3, 4, 2, 0, 1);
    /* 5->3->4->2->1->0 */
    SIX_COST_STRICT_CHECK(5, 3, 4, 2, 1, 0);
    /* 5->4->0->1->2->3 */
    SIX_COST_STRICT_CHECK(5, 4, 0, 1, 2, 3);
    /* 5->4->0->1->3->2 */
    SIX_COST_STRICT_CHECK(5, 4, 0, 1, 3, 2);
    /* 5->4->0->2->1->3 */
    SIX_COST_STRICT_CHECK(5, 4, 0, 2, 1, 3);
    /* 5->4->0->2->3->1 */
    SIX_COST_STRICT_CHECK(5, 4, 0, 2, 3, 1);
    /* 5->4->0->3->1->2 */
    SIX_COST_STRICT_CHECK(5, 4, 0, 3, 1, 2);
    /* 5->4->0->3->2->1 */
    SIX_COST_STRICT_CHECK(5, 4, 0, 3, 2, 1);
    /* 5->4->1->0->2->3 */
    SIX_COST_STRICT_CHECK(5, 4, 1, 0, 2, 3);
    /* 5->4->1->0->3->2 */
    SIX_COST_STRICT_CHECK(5, 4, 1, 0, 3, 2);
    /* 5->4->1->2->0->3 */
    SIX_COST_STRICT_CHECK(5, 4, 1, 2, 0, 3);
    /* 5->4->1->2->3->0 */
    SIX_COST_STRICT_CHECK(5, 4, 1, 2, 3, 0);
    /* 5->4->1->3->0->2 */
    SIX_COST_STRICT_CHECK(5, 4, 1, 3, 0, 2);
    /* 5->4->1->3->2->0 */
    SIX_COST_STRICT_CHECK(5, 4, 1, 3, 2, 0);
    /* 5->4->2->0->1->3 */
    SIX_COST_STRICT_CHECK(5, 4, 2, 0, 1, 3);
    /* 5->4->2->0->3->1 */
    SIX_COST_STRICT_CHECK(5, 4, 2, 0, 3, 1);
    /* 5->4->2->1->0->3 */
    SIX_COST_STRICT_CHECK(5, 4, 2, 1, 0, 3);
    /* 5->4->2->1->3->0 */
    SIX_COST_STRICT_CHECK(5, 4, 2, 1, 3, 0);
    /* 5->4->2->3->0->1 */
    SIX_COST_STRICT_CHECK(5, 4, 2, 3, 0, 1);
    /* 5->4->2->3->1->0 */
    SIX_COST_STRICT_CHECK(5, 4, 2, 3, 1, 0);
    /* 5->4->3->0->1->2 */
    SIX_COST_STRICT_CHECK(5, 4, 3, 0, 1, 2);
    /* 5->4->3->0->2->1 */
    SIX_COST_STRICT_CHECK(5, 4, 3, 0, 2, 1);
    /* 5->4->3->1->0->2 */
    SIX_COST_STRICT_CHECK(5, 4, 3, 1, 0, 2);
    /* 5->4->3->1->2->0 */
    SIX_COST_STRICT_CHECK(5, 4, 3, 1, 2, 0);
    /* 5->4->3->2->0->1 */
    SIX_COST_STRICT_CHECK(5, 4, 3, 2, 0, 1);
    /* 5->4->3->2->1->0 */
    SIX_COST_STRICT_CHECK(5, 4, 3, 2, 1, 0);
  }

  return(0);
}

/*
 * check_six_cost_backward(prob, j, s, c)
 *   checks whether there exists a six-job sequence which dominates
 *   the specified sequence (in the forward manner).
 *        j: job list
 *        s: starting time
 *        c: completion time
 *
 */
char check_six_cost_backward(sips *prob, int *j, int s, int c)
{
#if defined(SIPSI) && !defined(COST_REAL) && !defined(COST_LONGLONG)
  long long int f, g;
#else /* SIPSI && !COST_REAL && !COST_LONGLONG */
#ifdef COST_LONGLONG
  unsigned long long f, g;
#else /* COST_LONGLONG */
  cost_t f, g;
#endif /* COST_LONGLONG */
#endif /* SIPSI && !COST_REAL && !COST_LONGLONG */

  /* 0->1->2->3->4->5 */
  f = SIX_COST(0, 1, 2, 3, 4, 5);

  /* 0->1->2->4->5->3 */
  SIX_COST_CHECK3(0, 1, 2, 4, 5, 3);
  /* 0->1->2->5->3->4 */
  SIX_COST_CHECK3(0, 1, 2, 5, 3, 4);
  /* 0->1->2->5->4->3 */
  SIX_COST_CHECK3(0, 1, 2, 5, 4, 3);

  if(prob->sjob[j[3]]->tno < prob->sjob[j[2]]->tno) {
    /* 0->1->3->4->5->2 */
    SIX_COST_CHECK(0, 1, 3, 4, 5, 2);
    /* 0->1->3->5->2->4 */
    SIX_COST_CHECK(0, 1, 3, 5, 2, 4);
    /* 0->1->3->5->4->2 */
    SIX_COST_CHECK(0, 1, 3, 5, 4, 2);
  } else {
    /* 0->1->3->4->5->2 */
    SIX_COST_STRICT_CHECK(0, 1, 3, 4, 5, 2);
    /* 0->1->3->5->2->4 */
    SIX_COST_STRICT_CHECK(0, 1, 3, 5, 2, 4);
    /* 0->1->3->5->4->2 */
    SIX_COST_STRICT_CHECK(0, 1, 3, 5, 4, 2);
  }

  if(prob->sjob[j[4]]->tno < prob->sjob[j[2]]->tno) {
    /* 0->1->4->2->5->3 */
    SIX_COST_CHECK(0, 1, 4, 2, 5, 3);
    /* 0->1->4->3->5->2 */
    SIX_COST_CHECK(0, 1, 4, 3, 5, 2);
    /* 0->1->4->5->2->3 */
    SIX_COST_CHECK(0, 1, 4, 5, 2, 3);
    /* 0->1->4->5->3->2 */
    SIX_COST_CHECK(0, 1, 4, 5, 3, 2);
  } else {
    /* 0->1->4->2->5->3 */
    SIX_COST_STRICT_CHECK(0, 1, 4, 2, 5, 3);
    /* 0->1->4->3->5->2 */
    SIX_COST_STRICT_CHECK(0, 1, 4, 3, 5, 2);
    /* 0->1->4->5->2->3 */
    SIX_COST_STRICT_CHECK(0, 1, 4, 5, 2, 3);
    /* 0->1->4->5->3->2 */
    SIX_COST_STRICT_CHECK(0, 1, 4, 5, 3, 2);
  }

  if(prob->sjob[j[5]]->tno < prob->sjob[j[2]]->tno) {
    /* 0->1->5->2->3->4 */
    SIX_COST_CHECK(0, 1, 5, 2, 3, 4);
    /* 0->1->5->2->4->3 */
    SIX_COST_CHECK(0, 1, 5, 2, 4, 3);
    /* 0->1->5->3->2->4 */
    SIX_COST_CHECK(0, 1, 5, 3, 2, 4);
    /* 0->1->5->3->4->2 */
    SIX_COST_CHECK(0, 1, 5, 3, 4, 2);
    /* 0->1->5->4->2->3 */
    SIX_COST_CHECK(0, 1, 5, 4, 2, 3);
    /* 0->1->5->4->3->2 */
    SIX_COST_CHECK(0, 1, 5, 4, 3, 2);
  } else {
    /* 0->1->5->2->3->4 */
    SIX_COST_STRICT_CHECK(0, 1, 5, 2, 3, 4);
    /* 0->1->5->2->4->3 */
    SIX_COST_STRICT_CHECK(0, 1, 5, 2, 4, 3);
    /* 0->1->5->3->2->4 */
    SIX_COST_STRICT_CHECK(0, 1, 5, 3, 2, 4);
    /* 0->1->5->3->4->2 */
    SIX_COST_STRICT_CHECK(0, 1, 5, 3, 4, 2);
    /* 0->1->5->4->2->3 */
    SIX_COST_STRICT_CHECK(0, 1, 5, 4, 2, 3);
    /* 0->1->5->4->3->2 */
    SIX_COST_STRICT_CHECK(0, 1, 5, 4, 3, 2);
  }

  if(prob->sjob[j[2]]->tno < prob->sjob[j[1]]->tno) {
    /* 0->2->1->4->5->3 */
    SIX_COST_CHECK(0, 2, 1, 4, 5, 3);
    /* 0->2->1->5->3->4 */
    SIX_COST_CHECK(0, 2, 1, 5, 3, 4);
    /* 0->2->1->5->4->3 */
    SIX_COST_CHECK(0, 2, 1, 5, 4, 3);
    /* 0->2->3->4->5->1 */
    SIX_COST_CHECK(0, 2, 3, 4, 5, 1);
    /* 0->2->3->5->1->4 */
    SIX_COST_CHECK(0, 2, 3, 5, 1, 4);
    /* 0->2->3->5->4->1 */
    SIX_COST_CHECK(0, 2, 3, 5, 4, 1);
    /* 0->2->4->1->5->3 */
    SIX_COST_CHECK(0, 2, 4, 1, 5, 3);
    /* 0->2->4->3->5->1 */
    SIX_COST_CHECK(0, 2, 4, 3, 5, 1);
    /* 0->2->4->5->1->3 */
    SIX_COST_CHECK(0, 2, 4, 5, 1, 3);
    /* 0->2->4->5->3->1 */
    SIX_COST_CHECK(0, 2, 4, 5, 3, 1);
    /* 0->2->5->1->3->4 */
    SIX_COST_CHECK(0, 2, 5, 1, 3, 4);
    /* 0->2->5->1->4->3 */
    SIX_COST_CHECK(0, 2, 5, 1, 4, 3);
    /* 0->2->5->3->1->4 */
    SIX_COST_CHECK(0, 2, 5, 3, 1, 4);
    /* 0->2->5->3->4->1 */
    SIX_COST_CHECK(0, 2, 5, 3, 4, 1);
    /* 0->2->5->4->1->3 */
    SIX_COST_CHECK(0, 2, 5, 4, 1, 3);
    /* 0->2->5->4->3->1 */
    SIX_COST_CHECK(0, 2, 5, 4, 3, 1);
  } else {
    /* 0->2->1->4->5->3 */
    SIX_COST_STRICT_CHECK(0, 2, 1, 4, 5, 3);
    /* 0->2->1->5->3->4 */
    SIX_COST_STRICT_CHECK(0, 2, 1, 5, 3, 4);
    /* 0->2->1->5->4->3 */
    SIX_COST_STRICT_CHECK(0, 2, 1, 5, 4, 3);
    /* 0->2->3->4->5->1 */
    SIX_COST_STRICT_CHECK(0, 2, 3, 4, 5, 1);
    /* 0->2->3->5->1->4 */
    SIX_COST_STRICT_CHECK(0, 2, 3, 5, 1, 4);
    /* 0->2->3->5->4->1 */
    SIX_COST_STRICT_CHECK(0, 2, 3, 5, 4, 1);
    /* 0->2->4->1->5->3 */
    SIX_COST_STRICT_CHECK(0, 2, 4, 1, 5, 3);
    /* 0->2->4->3->5->1 */
    SIX_COST_STRICT_CHECK(0, 2, 4, 3, 5, 1);
    /* 0->2->4->5->1->3 */
    SIX_COST_STRICT_CHECK(0, 2, 4, 5, 1, 3);
    /* 0->2->4->5->3->1 */
    SIX_COST_STRICT_CHECK(0, 2, 4, 5, 3, 1);
    /* 0->2->5->1->3->4 */
    SIX_COST_STRICT_CHECK(0, 2, 5, 1, 3, 4);
    /* 0->2->5->1->4->3 */
    SIX_COST_STRICT_CHECK(0, 2, 5, 1, 4, 3);
    /* 0->2->5->3->1->4 */
    SIX_COST_STRICT_CHECK(0, 2, 5, 3, 1, 4);
    /* 0->2->5->3->4->1 */
    SIX_COST_STRICT_CHECK(0, 2, 5, 3, 4, 1);
    /* 0->2->5->4->1->3 */
    SIX_COST_STRICT_CHECK(0, 2, 5, 4, 1, 3);
    /* 0->2->5->4->3->1 */
    SIX_COST_STRICT_CHECK(0, 2, 5, 4, 3, 1);
  }

  if(prob->sjob[j[3]]->tno < prob->sjob[j[1]]->tno) {
    /* 0->3->1->4->5->2 */
    SIX_COST_CHECK(0, 3, 1, 4, 5, 2);
    /* 0->3->1->5->2->4 */
    SIX_COST_CHECK(0, 3, 1, 5, 2, 4);
    /* 0->3->1->5->4->2 */
    SIX_COST_CHECK(0, 3, 1, 5, 4, 2);
    /* 0->3->2->4->5->1 */
    SIX_COST_CHECK(0, 3, 2, 4, 5, 1);
    /* 0->3->2->5->1->4 */
    SIX_COST_CHECK(0, 3, 2, 5, 1, 4);
    /* 0->3->2->5->4->1 */
    SIX_COST_CHECK(0, 3, 2, 5, 4, 1);
    /* 0->3->4->1->5->2 */
    SIX_COST_CHECK(0, 3, 4, 1, 5, 2);
    /* 0->3->4->2->5->1 */
    SIX_COST_CHECK(0, 3, 4, 2, 5, 1);
    /* 0->3->4->5->1->2 */
    SIX_COST_CHECK(0, 3, 4, 5, 1, 2);
    /* 0->3->4->5->2->1 */
    SIX_COST_CHECK(0, 3, 4, 5, 2, 1);
    /* 0->3->5->1->2->4 */
    SIX_COST_CHECK(0, 3, 5, 1, 2, 4);
    /* 0->3->5->1->4->2 */
    SIX_COST_CHECK(0, 3, 5, 1, 4, 2);
    /* 0->3->5->2->1->4 */
    SIX_COST_CHECK(0, 3, 5, 2, 1, 4);
    /* 0->3->5->2->4->1 */
    SIX_COST_CHECK(0, 3, 5, 2, 4, 1);
    /* 0->3->5->4->1->2 */
    SIX_COST_CHECK(0, 3, 5, 4, 1, 2);
    /* 0->3->5->4->2->1 */
    SIX_COST_CHECK(0, 3, 5, 4, 2, 1);
  } else {
    /* 0->3->1->4->5->2 */
    SIX_COST_STRICT_CHECK(0, 3, 1, 4, 5, 2);
    /* 0->3->1->5->2->4 */
    SIX_COST_STRICT_CHECK(0, 3, 1, 5, 2, 4);
    /* 0->3->1->5->4->2 */
    SIX_COST_STRICT_CHECK(0, 3, 1, 5, 4, 2);
    /* 0->3->2->4->5->1 */
    SIX_COST_STRICT_CHECK(0, 3, 2, 4, 5, 1);
    /* 0->3->2->5->1->4 */
    SIX_COST_STRICT_CHECK(0, 3, 2, 5, 1, 4);
    /* 0->3->2->5->4->1 */
    SIX_COST_STRICT_CHECK(0, 3, 2, 5, 4, 1);
    /* 0->3->4->1->5->2 */
    SIX_COST_STRICT_CHECK(0, 3, 4, 1, 5, 2);
    /* 0->3->4->2->5->1 */
    SIX_COST_STRICT_CHECK(0, 3, 4, 2, 5, 1);
    /* 0->3->4->5->1->2 */
    SIX_COST_STRICT_CHECK(0, 3, 4, 5, 1, 2);
    /* 0->3->4->5->2->1 */
    SIX_COST_STRICT_CHECK(0, 3, 4, 5, 2, 1);
    /* 0->3->5->1->2->4 */
    SIX_COST_STRICT_CHECK(0, 3, 5, 1, 2, 4);
    /* 0->3->5->1->4->2 */
    SIX_COST_STRICT_CHECK(0, 3, 5, 1, 4, 2);
    /* 0->3->5->2->1->4 */
    SIX_COST_STRICT_CHECK(0, 3, 5, 2, 1, 4);
    /* 0->3->5->2->4->1 */
    SIX_COST_STRICT_CHECK(0, 3, 5, 2, 4, 1);
    /* 0->3->5->4->1->2 */
    SIX_COST_STRICT_CHECK(0, 3, 5, 4, 1, 2);
    /* 0->3->5->4->2->1 */
    SIX_COST_STRICT_CHECK(0, 3, 5, 4, 2, 1);
  }

  if(prob->sjob[j[4]]->tno < prob->sjob[j[1]]->tno) {
    /* 0->4->1->2->5->3 */
    SIX_COST_CHECK(0, 4, 1, 2, 5, 3);
    /* 0->4->1->3->5->2 */
    SIX_COST_CHECK(0, 4, 1, 3, 5, 2);
    /* 0->4->1->5->2->3 */
    SIX_COST_CHECK(0, 4, 1, 5, 2, 3);
    /* 0->4->1->5->3->2 */
    SIX_COST_CHECK(0, 4, 1, 5, 3, 2);
    /* 0->4->2->1->5->3 */
    SIX_COST_CHECK(0, 4, 2, 1, 5, 3);
    /* 0->4->2->3->5->1 */
    SIX_COST_CHECK(0, 4, 2, 3, 5, 1);
    /* 0->4->2->5->1->3 */
    SIX_COST_CHECK(0, 4, 2, 5, 1, 3);
    /* 0->4->2->5->3->1 */
    SIX_COST_CHECK(0, 4, 2, 5, 3, 1);
    /* 0->4->3->1->5->2 */
    SIX_COST_CHECK(0, 4, 3, 1, 5, 2);
    /* 0->4->3->2->5->1 */
    SIX_COST_CHECK(0, 4, 3, 2, 5, 1);
    /* 0->4->3->5->1->2 */
    SIX_COST_CHECK(0, 4, 3, 5, 1, 2);
    /* 0->4->3->5->2->1 */
    SIX_COST_CHECK(0, 4, 3, 5, 2, 1);
    /* 0->4->5->1->2->3 */
    SIX_COST_CHECK(0, 4, 5, 1, 2, 3);
    /* 0->4->5->1->3->2 */
    SIX_COST_CHECK(0, 4, 5, 1, 3, 2);
    /* 0->4->5->2->1->3 */
    SIX_COST_CHECK(0, 4, 5, 2, 1, 3);
    /* 0->4->5->2->3->1 */
    SIX_COST_CHECK(0, 4, 5, 2, 3, 1);
    /* 0->4->5->3->1->2 */
    SIX_COST_CHECK(0, 4, 5, 3, 1, 2);
    /* 0->4->5->3->2->1 */
    SIX_COST_CHECK(0, 4, 5, 3, 2, 1);
  } else {
    /* 0->4->1->2->5->3 */
    SIX_COST_STRICT_CHECK(0, 4, 1, 2, 5, 3);
    /* 0->4->1->3->5->2 */
    SIX_COST_STRICT_CHECK(0, 4, 1, 3, 5, 2);
    /* 0->4->1->5->2->3 */
    SIX_COST_STRICT_CHECK(0, 4, 1, 5, 2, 3);
    /* 0->4->1->5->3->2 */
    SIX_COST_STRICT_CHECK(0, 4, 1, 5, 3, 2);
    /* 0->4->2->1->5->3 */
    SIX_COST_STRICT_CHECK(0, 4, 2, 1, 5, 3);
    /* 0->4->2->3->5->1 */
    SIX_COST_STRICT_CHECK(0, 4, 2, 3, 5, 1);
    /* 0->4->2->5->1->3 */
    SIX_COST_STRICT_CHECK(0, 4, 2, 5, 1, 3);
    /* 0->4->2->5->3->1 */
    SIX_COST_STRICT_CHECK(0, 4, 2, 5, 3, 1);
    /* 0->4->3->1->5->2 */
    SIX_COST_STRICT_CHECK(0, 4, 3, 1, 5, 2);
    /* 0->4->3->2->5->1 */
    SIX_COST_STRICT_CHECK(0, 4, 3, 2, 5, 1);
    /* 0->4->3->5->1->2 */
    SIX_COST_STRICT_CHECK(0, 4, 3, 5, 1, 2);
    /* 0->4->3->5->2->1 */
    SIX_COST_STRICT_CHECK(0, 4, 3, 5, 2, 1);
    /* 0->4->5->1->2->3 */
    SIX_COST_STRICT_CHECK(0, 4, 5, 1, 2, 3);
    /* 0->4->5->1->3->2 */
    SIX_COST_STRICT_CHECK(0, 4, 5, 1, 3, 2);
    /* 0->4->5->2->1->3 */
    SIX_COST_STRICT_CHECK(0, 4, 5, 2, 1, 3);
    /* 0->4->5->2->3->1 */
    SIX_COST_STRICT_CHECK(0, 4, 5, 2, 3, 1);
    /* 0->4->5->3->1->2 */
    SIX_COST_STRICT_CHECK(0, 4, 5, 3, 1, 2);
    /* 0->4->5->3->2->1 */
    SIX_COST_STRICT_CHECK(0, 4, 5, 3, 2, 1);
  }

  if(prob->sjob[j[5]]->tno < prob->sjob[j[1]]->tno) {
    /* 0->5->1->2->3->4 */
    SIX_COST_CHECK(0, 5, 1, 2, 3, 4);
    /* 0->5->1->2->4->3 */
    SIX_COST_CHECK(0, 5, 1, 2, 4, 3);
    /* 0->5->1->3->2->4 */
    SIX_COST_CHECK(0, 5, 1, 3, 2, 4);
    /* 0->5->1->3->4->2 */
    SIX_COST_CHECK(0, 5, 1, 3, 4, 2);
    /* 0->5->1->4->2->3 */
    SIX_COST_CHECK(0, 5, 1, 4, 2, 3);
    /* 0->5->1->4->3->2 */
    SIX_COST_CHECK(0, 5, 1, 4, 3, 2);
    /* 0->5->2->1->3->4 */
    SIX_COST_CHECK(0, 5, 2, 1, 3, 4);
    /* 0->5->2->1->4->3 */
    SIX_COST_CHECK(0, 5, 2, 1, 4, 3);
    /* 0->5->2->3->1->4 */
    SIX_COST_CHECK(0, 5, 2, 3, 1, 4);
    /* 0->5->2->3->4->1 */
    SIX_COST_CHECK(0, 5, 2, 3, 4, 1);
    /* 0->5->2->4->1->3 */
    SIX_COST_CHECK(0, 5, 2, 4, 1, 3);
    /* 0->5->2->4->3->1 */
    SIX_COST_CHECK(0, 5, 2, 4, 3, 1);
    /* 0->5->3->1->2->4 */
    SIX_COST_CHECK(0, 5, 3, 1, 2, 4);
    /* 0->5->3->1->4->2 */
    SIX_COST_CHECK(0, 5, 3, 1, 4, 2);
    /* 0->5->3->2->1->4 */
    SIX_COST_CHECK(0, 5, 3, 2, 1, 4);
    /* 0->5->3->2->4->1 */
    SIX_COST_CHECK(0, 5, 3, 2, 4, 1);
    /* 0->5->3->4->1->2 */
    SIX_COST_CHECK(0, 5, 3, 4, 1, 2);
    /* 0->5->3->4->2->1 */
    SIX_COST_CHECK(0, 5, 3, 4, 2, 1);
    /* 0->5->4->1->2->3 */
    SIX_COST_CHECK(0, 5, 4, 1, 2, 3);
    /* 0->5->4->1->3->2 */
    SIX_COST_CHECK(0, 5, 4, 1, 3, 2);
    /* 0->5->4->2->1->3 */
    SIX_COST_CHECK(0, 5, 4, 2, 1, 3);
    /* 0->5->4->2->3->1 */
    SIX_COST_CHECK(0, 5, 4, 2, 3, 1);
    /* 0->5->4->3->1->2 */
    SIX_COST_CHECK(0, 5, 4, 3, 1, 2);
    /* 0->5->4->3->2->1 */
    SIX_COST_CHECK(0, 5, 4, 3, 2, 1);
  } else {
    /* 0->5->1->2->3->4 */
    SIX_COST_STRICT_CHECK(0, 5, 1, 2, 3, 4);
    /* 0->5->1->2->4->3 */
    SIX_COST_STRICT_CHECK(0, 5, 1, 2, 4, 3);
    /* 0->5->1->3->2->4 */
    SIX_COST_STRICT_CHECK(0, 5, 1, 3, 2, 4);
    /* 0->5->1->3->4->2 */
    SIX_COST_STRICT_CHECK(0, 5, 1, 3, 4, 2);
    /* 0->5->1->4->2->3 */
    SIX_COST_STRICT_CHECK(0, 5, 1, 4, 2, 3);
    /* 0->5->1->4->3->2 */
    SIX_COST_STRICT_CHECK(0, 5, 1, 4, 3, 2);
    /* 0->5->2->1->3->4 */
    SIX_COST_STRICT_CHECK(0, 5, 2, 1, 3, 4);
    /* 0->5->2->1->4->3 */
    SIX_COST_STRICT_CHECK(0, 5, 2, 1, 4, 3);
    /* 0->5->2->3->1->4 */
    SIX_COST_STRICT_CHECK(0, 5, 2, 3, 1, 4);
    /* 0->5->2->3->4->1 */
    SIX_COST_STRICT_CHECK(0, 5, 2, 3, 4, 1);
    /* 0->5->2->4->1->3 */
    SIX_COST_STRICT_CHECK(0, 5, 2, 4, 1, 3);
    /* 0->5->2->4->3->1 */
    SIX_COST_STRICT_CHECK(0, 5, 2, 4, 3, 1);
    /* 0->5->3->1->2->4 */
    SIX_COST_STRICT_CHECK(0, 5, 3, 1, 2, 4);
    /* 0->5->3->1->4->2 */
    SIX_COST_STRICT_CHECK(0, 5, 3, 1, 4, 2);
    /* 0->5->3->2->1->4 */
    SIX_COST_STRICT_CHECK(0, 5, 3, 2, 1, 4);
    /* 0->5->3->2->4->1 */
    SIX_COST_STRICT_CHECK(0, 5, 3, 2, 4, 1);
    /* 0->5->3->4->1->2 */
    SIX_COST_STRICT_CHECK(0, 5, 3, 4, 1, 2);
    /* 0->5->3->4->2->1 */
    SIX_COST_STRICT_CHECK(0, 5, 3, 4, 2, 1);
    /* 0->5->4->1->2->3 */
    SIX_COST_STRICT_CHECK(0, 5, 4, 1, 2, 3);
    /* 0->5->4->1->3->2 */
    SIX_COST_STRICT_CHECK(0, 5, 4, 1, 3, 2);
    /* 0->5->4->2->1->3 */
    SIX_COST_STRICT_CHECK(0, 5, 4, 2, 1, 3);
    /* 0->5->4->2->3->1 */
    SIX_COST_STRICT_CHECK(0, 5, 4, 2, 3, 1);
    /* 0->5->4->3->1->2 */
    SIX_COST_STRICT_CHECK(0, 5, 4, 3, 1, 2);
    /* 0->5->4->3->2->1 */
    SIX_COST_STRICT_CHECK(0, 5, 4, 3, 2, 1);
  }

  if(prob->sjob[j[1]]->tno < prob->sjob[j[0]]->tno) {
    /* 1->0->2->4->5->3 */
    SIX_COST_CHECK(1, 0, 2, 4, 5, 3);
    /* 1->0->2->5->3->4 */
    SIX_COST_CHECK(1, 0, 2, 5, 3, 4);
    /* 1->0->2->5->4->3 */
    SIX_COST_CHECK(1, 0, 2, 5, 4, 3);
    /* 1->0->3->4->5->2 */
    SIX_COST_CHECK(1, 0, 3, 4, 5, 2);
    /* 1->0->3->5->2->4 */
    SIX_COST_CHECK(1, 0, 3, 5, 2, 4);
    /* 1->0->3->5->4->2 */
    SIX_COST_CHECK(1, 0, 3, 5, 4, 2);
    /* 1->0->4->2->5->3 */
    SIX_COST_CHECK(1, 0, 4, 2, 5, 3);
    /* 1->0->4->3->5->2 */
    SIX_COST_CHECK(1, 0, 4, 3, 5, 2);
    /* 1->0->4->5->2->3 */
    SIX_COST_CHECK(1, 0, 4, 5, 2, 3);
    /* 1->0->4->5->3->2 */
    SIX_COST_CHECK(1, 0, 4, 5, 3, 2);
    /* 1->0->5->2->3->4 */
    SIX_COST_CHECK(1, 0, 5, 2, 3, 4);
    /* 1->0->5->2->4->3 */
    SIX_COST_CHECK(1, 0, 5, 2, 4, 3);
    /* 1->0->5->3->2->4 */
    SIX_COST_CHECK(1, 0, 5, 3, 2, 4);
    /* 1->0->5->3->4->2 */
    SIX_COST_CHECK(1, 0, 5, 3, 4, 2);
    /* 1->0->5->4->2->3 */
    SIX_COST_CHECK(1, 0, 5, 4, 2, 3);
    /* 1->0->5->4->3->2 */
    SIX_COST_CHECK(1, 0, 5, 4, 3, 2);
    /* 1->2->0->4->5->3 */
    SIX_COST_CHECK(1, 2, 0, 4, 5, 3);
    /* 1->2->0->5->3->4 */
    SIX_COST_CHECK(1, 2, 0, 5, 3, 4);
    /* 1->2->0->5->4->3 */
    SIX_COST_CHECK(1, 2, 0, 5, 4, 3);
    /* 1->2->3->4->5->0 */
    SIX_COST_CHECK(1, 2, 3, 4, 5, 0);
    /* 1->2->3->5->0->4 */
    SIX_COST_CHECK(1, 2, 3, 5, 0, 4);
    /* 1->2->3->5->4->0 */
    SIX_COST_CHECK(1, 2, 3, 5, 4, 0);
    /* 1->2->4->0->5->3 */
    SIX_COST_CHECK(1, 2, 4, 0, 5, 3);
    /* 1->2->4->3->5->0 */
    SIX_COST_CHECK(1, 2, 4, 3, 5, 0);
    /* 1->2->4->5->0->3 */
    SIX_COST_CHECK(1, 2, 4, 5, 0, 3);
    /* 1->2->4->5->3->0 */
    SIX_COST_CHECK(1, 2, 4, 5, 3, 0);
    /* 1->2->5->0->3->4 */
    SIX_COST_CHECK(1, 2, 5, 0, 3, 4);
    /* 1->2->5->0->4->3 */
    SIX_COST_CHECK(1, 2, 5, 0, 4, 3);
    /* 1->2->5->3->0->4 */
    SIX_COST_CHECK(1, 2, 5, 3, 0, 4);
    /* 1->2->5->3->4->0 */
    SIX_COST_CHECK(1, 2, 5, 3, 4, 0);
    /* 1->2->5->4->0->3 */
    SIX_COST_CHECK(1, 2, 5, 4, 0, 3);
    /* 1->2->5->4->3->0 */
    SIX_COST_CHECK(1, 2, 5, 4, 3, 0);
    /* 1->3->0->4->5->2 */
    SIX_COST_CHECK(1, 3, 0, 4, 5, 2);
    /* 1->3->0->5->2->4 */
    SIX_COST_CHECK(1, 3, 0, 5, 2, 4);
    /* 1->3->0->5->4->2 */
    SIX_COST_CHECK(1, 3, 0, 5, 4, 2);
    /* 1->3->2->4->5->0 */
    SIX_COST_CHECK(1, 3, 2, 4, 5, 0);
    /* 1->3->2->5->0->4 */
    SIX_COST_CHECK(1, 3, 2, 5, 0, 4);
    /* 1->3->2->5->4->0 */
    SIX_COST_CHECK(1, 3, 2, 5, 4, 0);
    /* 1->3->4->0->5->2 */
    SIX_COST_CHECK(1, 3, 4, 0, 5, 2);
    /* 1->3->4->2->5->0 */
    SIX_COST_CHECK(1, 3, 4, 2, 5, 0);
    /* 1->3->4->5->0->2 */
    SIX_COST_CHECK(1, 3, 4, 5, 0, 2);
    /* 1->3->4->5->2->0 */
    SIX_COST_CHECK(1, 3, 4, 5, 2, 0);
    /* 1->3->5->0->2->4 */
    SIX_COST_CHECK(1, 3, 5, 0, 2, 4);
    /* 1->3->5->0->4->2 */
    SIX_COST_CHECK(1, 3, 5, 0, 4, 2);
    /* 1->3->5->2->0->4 */
    SIX_COST_CHECK(1, 3, 5, 2, 0, 4);
    /* 1->3->5->2->4->0 */
    SIX_COST_CHECK(1, 3, 5, 2, 4, 0);
    /* 1->3->5->4->0->2 */
    SIX_COST_CHECK(1, 3, 5, 4, 0, 2);
    /* 1->3->5->4->2->0 */
    SIX_COST_CHECK(1, 3, 5, 4, 2, 0);
    /* 1->4->0->2->5->3 */
    SIX_COST_CHECK(1, 4, 0, 2, 5, 3);
    /* 1->4->0->3->5->2 */
    SIX_COST_CHECK(1, 4, 0, 3, 5, 2);
    /* 1->4->0->5->2->3 */
    SIX_COST_CHECK(1, 4, 0, 5, 2, 3);
    /* 1->4->0->5->3->2 */
    SIX_COST_CHECK(1, 4, 0, 5, 3, 2);
    /* 1->4->2->0->5->3 */
    SIX_COST_CHECK(1, 4, 2, 0, 5, 3);
    /* 1->4->2->3->5->0 */
    SIX_COST_CHECK(1, 4, 2, 3, 5, 0);
    /* 1->4->2->5->0->3 */
    SIX_COST_CHECK(1, 4, 2, 5, 0, 3);
    /* 1->4->2->5->3->0 */
    SIX_COST_CHECK(1, 4, 2, 5, 3, 0);
    /* 1->4->3->0->5->2 */
    SIX_COST_CHECK(1, 4, 3, 0, 5, 2);
    /* 1->4->3->2->5->0 */
    SIX_COST_CHECK(1, 4, 3, 2, 5, 0);
    /* 1->4->3->5->0->2 */
    SIX_COST_CHECK(1, 4, 3, 5, 0, 2);
    /* 1->4->3->5->2->0 */
    SIX_COST_CHECK(1, 4, 3, 5, 2, 0);
    /* 1->4->5->0->2->3 */
    SIX_COST_CHECK(1, 4, 5, 0, 2, 3);
    /* 1->4->5->0->3->2 */
    SIX_COST_CHECK(1, 4, 5, 0, 3, 2);
    /* 1->4->5->2->0->3 */
    SIX_COST_CHECK(1, 4, 5, 2, 0, 3);
    /* 1->4->5->2->3->0 */
    SIX_COST_CHECK(1, 4, 5, 2, 3, 0);
    /* 1->4->5->3->0->2 */
    SIX_COST_CHECK(1, 4, 5, 3, 0, 2);
    /* 1->4->5->3->2->0 */
    SIX_COST_CHECK(1, 4, 5, 3, 2, 0);
    /* 1->5->0->2->3->4 */
    SIX_COST_CHECK(1, 5, 0, 2, 3, 4);
    /* 1->5->0->2->4->3 */
    SIX_COST_CHECK(1, 5, 0, 2, 4, 3);
    /* 1->5->0->3->2->4 */
    SIX_COST_CHECK(1, 5, 0, 3, 2, 4);
    /* 1->5->0->3->4->2 */
    SIX_COST_CHECK(1, 5, 0, 3, 4, 2);
    /* 1->5->0->4->2->3 */
    SIX_COST_CHECK(1, 5, 0, 4, 2, 3);
    /* 1->5->0->4->3->2 */
    SIX_COST_CHECK(1, 5, 0, 4, 3, 2);
    /* 1->5->2->0->3->4 */
    SIX_COST_CHECK(1, 5, 2, 0, 3, 4);
    /* 1->5->2->0->4->3 */
    SIX_COST_CHECK(1, 5, 2, 0, 4, 3);
    /* 1->5->2->3->0->4 */
    SIX_COST_CHECK(1, 5, 2, 3, 0, 4);
    /* 1->5->2->3->4->0 */
    SIX_COST_CHECK(1, 5, 2, 3, 4, 0);
    /* 1->5->2->4->0->3 */
    SIX_COST_CHECK(1, 5, 2, 4, 0, 3);
    /* 1->5->2->4->3->0 */
    SIX_COST_CHECK(1, 5, 2, 4, 3, 0);
    /* 1->5->3->0->2->4 */
    SIX_COST_CHECK(1, 5, 3, 0, 2, 4);
    /* 1->5->3->0->4->2 */
    SIX_COST_CHECK(1, 5, 3, 0, 4, 2);
    /* 1->5->3->2->0->4 */
    SIX_COST_CHECK(1, 5, 3, 2, 0, 4);
    /* 1->5->3->2->4->0 */
    SIX_COST_CHECK(1, 5, 3, 2, 4, 0);
    /* 1->5->3->4->0->2 */
    SIX_COST_CHECK(1, 5, 3, 4, 0, 2);
    /* 1->5->3->4->2->0 */
    SIX_COST_CHECK(1, 5, 3, 4, 2, 0);
    /* 1->5->4->0->2->3 */
    SIX_COST_CHECK(1, 5, 4, 0, 2, 3);
    /* 1->5->4->0->3->2 */
    SIX_COST_CHECK(1, 5, 4, 0, 3, 2);
    /* 1->5->4->2->0->3 */
    SIX_COST_CHECK(1, 5, 4, 2, 0, 3);
    /* 1->5->4->2->3->0 */
    SIX_COST_CHECK(1, 5, 4, 2, 3, 0);
    /* 1->5->4->3->0->2 */
    SIX_COST_CHECK(1, 5, 4, 3, 0, 2);
    /* 1->5->4->3->2->0 */
    SIX_COST_CHECK(1, 5, 4, 3, 2, 0);
  } else {
    /* 1->0->2->4->5->3 */
    SIX_COST_STRICT_CHECK(1, 0, 2, 4, 5, 3);
    /* 1->0->2->5->3->4 */
    SIX_COST_STRICT_CHECK(1, 0, 2, 5, 3, 4);
    /* 1->0->2->5->4->3 */
    SIX_COST_STRICT_CHECK(1, 0, 2, 5, 4, 3);
    /* 1->0->3->4->5->2 */
    SIX_COST_STRICT_CHECK(1, 0, 3, 4, 5, 2);
    /* 1->0->3->5->2->4 */
    SIX_COST_STRICT_CHECK(1, 0, 3, 5, 2, 4);
    /* 1->0->3->5->4->2 */
    SIX_COST_STRICT_CHECK(1, 0, 3, 5, 4, 2);
    /* 1->0->4->2->5->3 */
    SIX_COST_STRICT_CHECK(1, 0, 4, 2, 5, 3);
    /* 1->0->4->3->5->2 */
    SIX_COST_STRICT_CHECK(1, 0, 4, 3, 5, 2);
    /* 1->0->4->5->2->3 */
    SIX_COST_STRICT_CHECK(1, 0, 4, 5, 2, 3);
    /* 1->0->4->5->3->2 */
    SIX_COST_STRICT_CHECK(1, 0, 4, 5, 3, 2);
    /* 1->0->5->2->3->4 */
    SIX_COST_STRICT_CHECK(1, 0, 5, 2, 3, 4);
    /* 1->0->5->2->4->3 */
    SIX_COST_STRICT_CHECK(1, 0, 5, 2, 4, 3);
    /* 1->0->5->3->2->4 */
    SIX_COST_STRICT_CHECK(1, 0, 5, 3, 2, 4);
    /* 1->0->5->3->4->2 */
    SIX_COST_STRICT_CHECK(1, 0, 5, 3, 4, 2);
    /* 1->0->5->4->2->3 */
    SIX_COST_STRICT_CHECK(1, 0, 5, 4, 2, 3);
    /* 1->0->5->4->3->2 */
    SIX_COST_STRICT_CHECK(1, 0, 5, 4, 3, 2);
    /* 1->2->0->4->5->3 */
    SIX_COST_STRICT_CHECK(1, 2, 0, 4, 5, 3);
    /* 1->2->0->5->3->4 */
    SIX_COST_STRICT_CHECK(1, 2, 0, 5, 3, 4);
    /* 1->2->0->5->4->3 */
    SIX_COST_STRICT_CHECK(1, 2, 0, 5, 4, 3);
    /* 1->2->3->4->5->0 */
    SIX_COST_STRICT_CHECK(1, 2, 3, 4, 5, 0);
    /* 1->2->3->5->0->4 */
    SIX_COST_STRICT_CHECK(1, 2, 3, 5, 0, 4);
    /* 1->2->3->5->4->0 */
    SIX_COST_STRICT_CHECK(1, 2, 3, 5, 4, 0);
    /* 1->2->4->0->5->3 */
    SIX_COST_STRICT_CHECK(1, 2, 4, 0, 5, 3);
    /* 1->2->4->3->5->0 */
    SIX_COST_STRICT_CHECK(1, 2, 4, 3, 5, 0);
    /* 1->2->4->5->0->3 */
    SIX_COST_STRICT_CHECK(1, 2, 4, 5, 0, 3);
    /* 1->2->4->5->3->0 */
    SIX_COST_STRICT_CHECK(1, 2, 4, 5, 3, 0);
    /* 1->2->5->0->3->4 */
    SIX_COST_STRICT_CHECK(1, 2, 5, 0, 3, 4);
    /* 1->2->5->0->4->3 */
    SIX_COST_STRICT_CHECK(1, 2, 5, 0, 4, 3);
    /* 1->2->5->3->0->4 */
    SIX_COST_STRICT_CHECK(1, 2, 5, 3, 0, 4);
    /* 1->2->5->3->4->0 */
    SIX_COST_STRICT_CHECK(1, 2, 5, 3, 4, 0);
    /* 1->2->5->4->0->3 */
    SIX_COST_STRICT_CHECK(1, 2, 5, 4, 0, 3);
    /* 1->2->5->4->3->0 */
    SIX_COST_STRICT_CHECK(1, 2, 5, 4, 3, 0);
    /* 1->3->0->4->5->2 */
    SIX_COST_STRICT_CHECK(1, 3, 0, 4, 5, 2);
    /* 1->3->0->5->2->4 */
    SIX_COST_STRICT_CHECK(1, 3, 0, 5, 2, 4);
    /* 1->3->0->5->4->2 */
    SIX_COST_STRICT_CHECK(1, 3, 0, 5, 4, 2);
    /* 1->3->2->4->5->0 */
    SIX_COST_STRICT_CHECK(1, 3, 2, 4, 5, 0);
    /* 1->3->2->5->0->4 */
    SIX_COST_STRICT_CHECK(1, 3, 2, 5, 0, 4);
    /* 1->3->2->5->4->0 */
    SIX_COST_STRICT_CHECK(1, 3, 2, 5, 4, 0);
    /* 1->3->4->0->5->2 */
    SIX_COST_STRICT_CHECK(1, 3, 4, 0, 5, 2);
    /* 1->3->4->2->5->0 */
    SIX_COST_STRICT_CHECK(1, 3, 4, 2, 5, 0);
    /* 1->3->4->5->0->2 */
    SIX_COST_STRICT_CHECK(1, 3, 4, 5, 0, 2);
    /* 1->3->4->5->2->0 */
    SIX_COST_STRICT_CHECK(1, 3, 4, 5, 2, 0);
    /* 1->3->5->0->2->4 */
    SIX_COST_STRICT_CHECK(1, 3, 5, 0, 2, 4);
    /* 1->3->5->0->4->2 */
    SIX_COST_STRICT_CHECK(1, 3, 5, 0, 4, 2);
    /* 1->3->5->2->0->4 */
    SIX_COST_STRICT_CHECK(1, 3, 5, 2, 0, 4);
    /* 1->3->5->2->4->0 */
    SIX_COST_STRICT_CHECK(1, 3, 5, 2, 4, 0);
    /* 1->3->5->4->0->2 */
    SIX_COST_STRICT_CHECK(1, 3, 5, 4, 0, 2);
    /* 1->3->5->4->2->0 */
    SIX_COST_STRICT_CHECK(1, 3, 5, 4, 2, 0);
    /* 1->4->0->2->5->3 */
    SIX_COST_STRICT_CHECK(1, 4, 0, 2, 5, 3);
    /* 1->4->0->3->5->2 */
    SIX_COST_STRICT_CHECK(1, 4, 0, 3, 5, 2);
    /* 1->4->0->5->2->3 */
    SIX_COST_STRICT_CHECK(1, 4, 0, 5, 2, 3);
    /* 1->4->0->5->3->2 */
    SIX_COST_STRICT_CHECK(1, 4, 0, 5, 3, 2);
    /* 1->4->2->0->5->3 */
    SIX_COST_STRICT_CHECK(1, 4, 2, 0, 5, 3);
    /* 1->4->2->3->5->0 */
    SIX_COST_STRICT_CHECK(1, 4, 2, 3, 5, 0);
    /* 1->4->2->5->0->3 */
    SIX_COST_STRICT_CHECK(1, 4, 2, 5, 0, 3);
    /* 1->4->2->5->3->0 */
    SIX_COST_STRICT_CHECK(1, 4, 2, 5, 3, 0);
    /* 1->4->3->0->5->2 */
    SIX_COST_STRICT_CHECK(1, 4, 3, 0, 5, 2);
    /* 1->4->3->2->5->0 */
    SIX_COST_STRICT_CHECK(1, 4, 3, 2, 5, 0);
    /* 1->4->3->5->0->2 */
    SIX_COST_STRICT_CHECK(1, 4, 3, 5, 0, 2);
    /* 1->4->3->5->2->0 */
    SIX_COST_STRICT_CHECK(1, 4, 3, 5, 2, 0);
    /* 1->4->5->0->2->3 */
    SIX_COST_STRICT_CHECK(1, 4, 5, 0, 2, 3);
    /* 1->4->5->0->3->2 */
    SIX_COST_STRICT_CHECK(1, 4, 5, 0, 3, 2);
    /* 1->4->5->2->0->3 */
    SIX_COST_STRICT_CHECK(1, 4, 5, 2, 0, 3);
    /* 1->4->5->2->3->0 */
    SIX_COST_STRICT_CHECK(1, 4, 5, 2, 3, 0);
    /* 1->4->5->3->0->2 */
    SIX_COST_STRICT_CHECK(1, 4, 5, 3, 0, 2);
    /* 1->4->5->3->2->0 */
    SIX_COST_STRICT_CHECK(1, 4, 5, 3, 2, 0);
    /* 1->5->0->2->3->4 */
    SIX_COST_STRICT_CHECK(1, 5, 0, 2, 3, 4);
    /* 1->5->0->2->4->3 */
    SIX_COST_STRICT_CHECK(1, 5, 0, 2, 4, 3);
    /* 1->5->0->3->2->4 */
    SIX_COST_STRICT_CHECK(1, 5, 0, 3, 2, 4);
    /* 1->5->0->3->4->2 */
    SIX_COST_STRICT_CHECK(1, 5, 0, 3, 4, 2);
    /* 1->5->0->4->2->3 */
    SIX_COST_STRICT_CHECK(1, 5, 0, 4, 2, 3);
    /* 1->5->0->4->3->2 */
    SIX_COST_STRICT_CHECK(1, 5, 0, 4, 3, 2);
    /* 1->5->2->0->3->4 */
    SIX_COST_STRICT_CHECK(1, 5, 2, 0, 3, 4);
    /* 1->5->2->0->4->3 */
    SIX_COST_STRICT_CHECK(1, 5, 2, 0, 4, 3);
    /* 1->5->2->3->0->4 */
    SIX_COST_STRICT_CHECK(1, 5, 2, 3, 0, 4);
    /* 1->5->2->3->4->0 */
    SIX_COST_STRICT_CHECK(1, 5, 2, 3, 4, 0);
    /* 1->5->2->4->0->3 */
    SIX_COST_STRICT_CHECK(1, 5, 2, 4, 0, 3);
    /* 1->5->2->4->3->0 */
    SIX_COST_STRICT_CHECK(1, 5, 2, 4, 3, 0);
    /* 1->5->3->0->2->4 */
    SIX_COST_STRICT_CHECK(1, 5, 3, 0, 2, 4);
    /* 1->5->3->0->4->2 */
    SIX_COST_STRICT_CHECK(1, 5, 3, 0, 4, 2);
    /* 1->5->3->2->0->4 */
    SIX_COST_STRICT_CHECK(1, 5, 3, 2, 0, 4);
    /* 1->5->3->2->4->0 */
    SIX_COST_STRICT_CHECK(1, 5, 3, 2, 4, 0);
    /* 1->5->3->4->0->2 */
    SIX_COST_STRICT_CHECK(1, 5, 3, 4, 0, 2);
    /* 1->5->3->4->2->0 */
    SIX_COST_STRICT_CHECK(1, 5, 3, 4, 2, 0);
    /* 1->5->4->0->2->3 */
    SIX_COST_STRICT_CHECK(1, 5, 4, 0, 2, 3);
    /* 1->5->4->0->3->2 */
    SIX_COST_STRICT_CHECK(1, 5, 4, 0, 3, 2);
    /* 1->5->4->2->0->3 */
    SIX_COST_STRICT_CHECK(1, 5, 4, 2, 0, 3);
    /* 1->5->4->2->3->0 */
    SIX_COST_STRICT_CHECK(1, 5, 4, 2, 3, 0);
    /* 1->5->4->3->0->2 */
    SIX_COST_STRICT_CHECK(1, 5, 4, 3, 0, 2);
    /* 1->5->4->3->2->0 */
    SIX_COST_STRICT_CHECK(1, 5, 4, 3, 2, 0);
  }

  if(prob->sjob[j[2]]->tno < prob->sjob[j[0]]->tno) {
    /* 2->0->1->4->5->3 */
    SIX_COST_CHECK(2, 0, 1, 4, 5, 3);
    /* 2->0->1->5->3->4 */
    SIX_COST_CHECK(2, 0, 1, 5, 3, 4);
    /* 2->0->1->5->4->3 */
    SIX_COST_CHECK(2, 0, 1, 5, 4, 3);
    /* 2->0->3->4->5->1 */
    SIX_COST_CHECK(2, 0, 3, 4, 5, 1);
    /* 2->0->3->5->1->4 */
    SIX_COST_CHECK(2, 0, 3, 5, 1, 4);
    /* 2->0->3->5->4->1 */
    SIX_COST_CHECK(2, 0, 3, 5, 4, 1);
    /* 2->0->4->1->5->3 */
    SIX_COST_CHECK(2, 0, 4, 1, 5, 3);
    /* 2->0->4->3->5->1 */
    SIX_COST_CHECK(2, 0, 4, 3, 5, 1);
    /* 2->0->4->5->1->3 */
    SIX_COST_CHECK(2, 0, 4, 5, 1, 3);
    /* 2->0->4->5->3->1 */
    SIX_COST_CHECK(2, 0, 4, 5, 3, 1);
    /* 2->0->5->1->3->4 */
    SIX_COST_CHECK(2, 0, 5, 1, 3, 4);
    /* 2->0->5->1->4->3 */
    SIX_COST_CHECK(2, 0, 5, 1, 4, 3);
    /* 2->0->5->3->1->4 */
    SIX_COST_CHECK(2, 0, 5, 3, 1, 4);
    /* 2->0->5->3->4->1 */
    SIX_COST_CHECK(2, 0, 5, 3, 4, 1);
    /* 2->0->5->4->1->3 */
    SIX_COST_CHECK(2, 0, 5, 4, 1, 3);
    /* 2->0->5->4->3->1 */
    SIX_COST_CHECK(2, 0, 5, 4, 3, 1);
    /* 2->1->0->4->5->3 */
    SIX_COST_CHECK(2, 1, 0, 4, 5, 3);
    /* 2->1->0->5->3->4 */
    SIX_COST_CHECK(2, 1, 0, 5, 3, 4);
    /* 2->1->0->5->4->3 */
    SIX_COST_CHECK(2, 1, 0, 5, 4, 3);
    /* 2->1->3->4->5->0 */
    SIX_COST_CHECK(2, 1, 3, 4, 5, 0);
    /* 2->1->3->5->0->4 */
    SIX_COST_CHECK(2, 1, 3, 5, 0, 4);
    /* 2->1->3->5->4->0 */
    SIX_COST_CHECK(2, 1, 3, 5, 4, 0);
    /* 2->1->4->0->5->3 */
    SIX_COST_CHECK(2, 1, 4, 0, 5, 3);
    /* 2->1->4->3->5->0 */
    SIX_COST_CHECK(2, 1, 4, 3, 5, 0);
    /* 2->1->4->5->0->3 */
    SIX_COST_CHECK(2, 1, 4, 5, 0, 3);
    /* 2->1->4->5->3->0 */
    SIX_COST_CHECK(2, 1, 4, 5, 3, 0);
    /* 2->1->5->0->3->4 */
    SIX_COST_CHECK(2, 1, 5, 0, 3, 4);
    /* 2->1->5->0->4->3 */
    SIX_COST_CHECK(2, 1, 5, 0, 4, 3);
    /* 2->1->5->3->0->4 */
    SIX_COST_CHECK(2, 1, 5, 3, 0, 4);
    /* 2->1->5->3->4->0 */
    SIX_COST_CHECK(2, 1, 5, 3, 4, 0);
    /* 2->1->5->4->0->3 */
    SIX_COST_CHECK(2, 1, 5, 4, 0, 3);
    /* 2->1->5->4->3->0 */
    SIX_COST_CHECK(2, 1, 5, 4, 3, 0);
    /* 2->3->0->4->5->1 */
    SIX_COST_CHECK(2, 3, 0, 4, 5, 1);
    /* 2->3->0->5->1->4 */
    SIX_COST_CHECK(2, 3, 0, 5, 1, 4);
    /* 2->3->0->5->4->1 */
    SIX_COST_CHECK(2, 3, 0, 5, 4, 1);
    /* 2->3->1->4->5->0 */
    SIX_COST_CHECK(2, 3, 1, 4, 5, 0);
    /* 2->3->1->5->0->4 */
    SIX_COST_CHECK(2, 3, 1, 5, 0, 4);
    /* 2->3->1->5->4->0 */
    SIX_COST_CHECK(2, 3, 1, 5, 4, 0);
    /* 2->3->4->0->5->1 */
    SIX_COST_CHECK(2, 3, 4, 0, 5, 1);
    /* 2->3->4->1->5->0 */
    SIX_COST_CHECK(2, 3, 4, 1, 5, 0);
    /* 2->3->4->5->0->1 */
    SIX_COST_CHECK(2, 3, 4, 5, 0, 1);
    /* 2->3->4->5->1->0 */
    SIX_COST_CHECK(2, 3, 4, 5, 1, 0);
    /* 2->3->5->0->1->4 */
    SIX_COST_CHECK(2, 3, 5, 0, 1, 4);
    /* 2->3->5->0->4->1 */
    SIX_COST_CHECK(2, 3, 5, 0, 4, 1);
    /* 2->3->5->1->0->4 */
    SIX_COST_CHECK(2, 3, 5, 1, 0, 4);
    /* 2->3->5->1->4->0 */
    SIX_COST_CHECK(2, 3, 5, 1, 4, 0);
    /* 2->3->5->4->0->1 */
    SIX_COST_CHECK(2, 3, 5, 4, 0, 1);
    /* 2->3->5->4->1->0 */
    SIX_COST_CHECK(2, 3, 5, 4, 1, 0);
    /* 2->4->0->1->5->3 */
    SIX_COST_CHECK(2, 4, 0, 1, 5, 3);
    /* 2->4->0->3->5->1 */
    SIX_COST_CHECK(2, 4, 0, 3, 5, 1);
    /* 2->4->0->5->1->3 */
    SIX_COST_CHECK(2, 4, 0, 5, 1, 3);
    /* 2->4->0->5->3->1 */
    SIX_COST_CHECK(2, 4, 0, 5, 3, 1);
    /* 2->4->1->0->5->3 */
    SIX_COST_CHECK(2, 4, 1, 0, 5, 3);
    /* 2->4->1->3->5->0 */
    SIX_COST_CHECK(2, 4, 1, 3, 5, 0);
    /* 2->4->1->5->0->3 */
    SIX_COST_CHECK(2, 4, 1, 5, 0, 3);
    /* 2->4->1->5->3->0 */
    SIX_COST_CHECK(2, 4, 1, 5, 3, 0);
    /* 2->4->3->0->5->1 */
    SIX_COST_CHECK(2, 4, 3, 0, 5, 1);
    /* 2->4->3->1->5->0 */
    SIX_COST_CHECK(2, 4, 3, 1, 5, 0);
    /* 2->4->3->5->0->1 */
    SIX_COST_CHECK(2, 4, 3, 5, 0, 1);
    /* 2->4->3->5->1->0 */
    SIX_COST_CHECK(2, 4, 3, 5, 1, 0);
    /* 2->4->5->0->1->3 */
    SIX_COST_CHECK(2, 4, 5, 0, 1, 3);
    /* 2->4->5->0->3->1 */
    SIX_COST_CHECK(2, 4, 5, 0, 3, 1);
    /* 2->4->5->1->0->3 */
    SIX_COST_CHECK(2, 4, 5, 1, 0, 3);
    /* 2->4->5->1->3->0 */
    SIX_COST_CHECK(2, 4, 5, 1, 3, 0);
    /* 2->4->5->3->0->1 */
    SIX_COST_CHECK(2, 4, 5, 3, 0, 1);
    /* 2->4->5->3->1->0 */
    SIX_COST_CHECK(2, 4, 5, 3, 1, 0);
    /* 2->5->0->1->3->4 */
    SIX_COST_CHECK(2, 5, 0, 1, 3, 4);
    /* 2->5->0->1->4->3 */
    SIX_COST_CHECK(2, 5, 0, 1, 4, 3);
    /* 2->5->0->3->1->4 */
    SIX_COST_CHECK(2, 5, 0, 3, 1, 4);
    /* 2->5->0->3->4->1 */
    SIX_COST_CHECK(2, 5, 0, 3, 4, 1);
    /* 2->5->0->4->1->3 */
    SIX_COST_CHECK(2, 5, 0, 4, 1, 3);
    /* 2->5->0->4->3->1 */
    SIX_COST_CHECK(2, 5, 0, 4, 3, 1);
    /* 2->5->1->0->3->4 */
    SIX_COST_CHECK(2, 5, 1, 0, 3, 4);
    /* 2->5->1->0->4->3 */
    SIX_COST_CHECK(2, 5, 1, 0, 4, 3);
    /* 2->5->1->3->0->4 */
    SIX_COST_CHECK(2, 5, 1, 3, 0, 4);
    /* 2->5->1->3->4->0 */
    SIX_COST_CHECK(2, 5, 1, 3, 4, 0);
    /* 2->5->1->4->0->3 */
    SIX_COST_CHECK(2, 5, 1, 4, 0, 3);
    /* 2->5->1->4->3->0 */
    SIX_COST_CHECK(2, 5, 1, 4, 3, 0);
    /* 2->5->3->0->1->4 */
    SIX_COST_CHECK(2, 5, 3, 0, 1, 4);
    /* 2->5->3->0->4->1 */
    SIX_COST_CHECK(2, 5, 3, 0, 4, 1);
    /* 2->5->3->1->0->4 */
    SIX_COST_CHECK(2, 5, 3, 1, 0, 4);
    /* 2->5->3->1->4->0 */
    SIX_COST_CHECK(2, 5, 3, 1, 4, 0);
    /* 2->5->3->4->0->1 */
    SIX_COST_CHECK(2, 5, 3, 4, 0, 1);
    /* 2->5->3->4->1->0 */
    SIX_COST_CHECK(2, 5, 3, 4, 1, 0);
    /* 2->5->4->0->1->3 */
    SIX_COST_CHECK(2, 5, 4, 0, 1, 3);
    /* 2->5->4->0->3->1 */
    SIX_COST_CHECK(2, 5, 4, 0, 3, 1);
    /* 2->5->4->1->0->3 */
    SIX_COST_CHECK(2, 5, 4, 1, 0, 3);
    /* 2->5->4->1->3->0 */
    SIX_COST_CHECK(2, 5, 4, 1, 3, 0);
    /* 2->5->4->3->0->1 */
    SIX_COST_CHECK(2, 5, 4, 3, 0, 1);
    /* 2->5->4->3->1->0 */
    SIX_COST_CHECK(2, 5, 4, 3, 1, 0);
  } else {
    /* 2->0->1->4->5->3 */
    SIX_COST_STRICT_CHECK(2, 0, 1, 4, 5, 3);
    /* 2->0->1->5->3->4 */
    SIX_COST_STRICT_CHECK(2, 0, 1, 5, 3, 4);
    /* 2->0->1->5->4->3 */
    SIX_COST_STRICT_CHECK(2, 0, 1, 5, 4, 3);
    /* 2->0->3->4->5->1 */
    SIX_COST_STRICT_CHECK(2, 0, 3, 4, 5, 1);
    /* 2->0->3->5->1->4 */
    SIX_COST_STRICT_CHECK(2, 0, 3, 5, 1, 4);
    /* 2->0->3->5->4->1 */
    SIX_COST_STRICT_CHECK(2, 0, 3, 5, 4, 1);
    /* 2->0->4->1->5->3 */
    SIX_COST_STRICT_CHECK(2, 0, 4, 1, 5, 3);
    /* 2->0->4->3->5->1 */
    SIX_COST_STRICT_CHECK(2, 0, 4, 3, 5, 1);
    /* 2->0->4->5->1->3 */
    SIX_COST_STRICT_CHECK(2, 0, 4, 5, 1, 3);
    /* 2->0->4->5->3->1 */
    SIX_COST_STRICT_CHECK(2, 0, 4, 5, 3, 1);
    /* 2->0->5->1->3->4 */
    SIX_COST_STRICT_CHECK(2, 0, 5, 1, 3, 4);
    /* 2->0->5->1->4->3 */
    SIX_COST_STRICT_CHECK(2, 0, 5, 1, 4, 3);
    /* 2->0->5->3->1->4 */
    SIX_COST_STRICT_CHECK(2, 0, 5, 3, 1, 4);
    /* 2->0->5->3->4->1 */
    SIX_COST_STRICT_CHECK(2, 0, 5, 3, 4, 1);
    /* 2->0->5->4->1->3 */
    SIX_COST_STRICT_CHECK(2, 0, 5, 4, 1, 3);
    /* 2->0->5->4->3->1 */
    SIX_COST_STRICT_CHECK(2, 0, 5, 4, 3, 1);
    /* 2->1->0->4->5->3 */
    SIX_COST_STRICT_CHECK(2, 1, 0, 4, 5, 3);
    /* 2->1->0->5->3->4 */
    SIX_COST_STRICT_CHECK(2, 1, 0, 5, 3, 4);
    /* 2->1->0->5->4->3 */
    SIX_COST_STRICT_CHECK(2, 1, 0, 5, 4, 3);
    /* 2->1->3->4->5->0 */
    SIX_COST_STRICT_CHECK(2, 1, 3, 4, 5, 0);
    /* 2->1->3->5->0->4 */
    SIX_COST_STRICT_CHECK(2, 1, 3, 5, 0, 4);
    /* 2->1->3->5->4->0 */
    SIX_COST_STRICT_CHECK(2, 1, 3, 5, 4, 0);
    /* 2->1->4->0->5->3 */
    SIX_COST_STRICT_CHECK(2, 1, 4, 0, 5, 3);
    /* 2->1->4->3->5->0 */
    SIX_COST_STRICT_CHECK(2, 1, 4, 3, 5, 0);
    /* 2->1->4->5->0->3 */
    SIX_COST_STRICT_CHECK(2, 1, 4, 5, 0, 3);
    /* 2->1->4->5->3->0 */
    SIX_COST_STRICT_CHECK(2, 1, 4, 5, 3, 0);
    /* 2->1->5->0->3->4 */
    SIX_COST_STRICT_CHECK(2, 1, 5, 0, 3, 4);
    /* 2->1->5->0->4->3 */
    SIX_COST_STRICT_CHECK(2, 1, 5, 0, 4, 3);
    /* 2->1->5->3->0->4 */
    SIX_COST_STRICT_CHECK(2, 1, 5, 3, 0, 4);
    /* 2->1->5->3->4->0 */
    SIX_COST_STRICT_CHECK(2, 1, 5, 3, 4, 0);
    /* 2->1->5->4->0->3 */
    SIX_COST_STRICT_CHECK(2, 1, 5, 4, 0, 3);
    /* 2->1->5->4->3->0 */
    SIX_COST_STRICT_CHECK(2, 1, 5, 4, 3, 0);
    /* 2->3->0->4->5->1 */
    SIX_COST_STRICT_CHECK(2, 3, 0, 4, 5, 1);
    /* 2->3->0->5->1->4 */
    SIX_COST_STRICT_CHECK(2, 3, 0, 5, 1, 4);
    /* 2->3->0->5->4->1 */
    SIX_COST_STRICT_CHECK(2, 3, 0, 5, 4, 1);
    /* 2->3->1->4->5->0 */
    SIX_COST_STRICT_CHECK(2, 3, 1, 4, 5, 0);
    /* 2->3->1->5->0->4 */
    SIX_COST_STRICT_CHECK(2, 3, 1, 5, 0, 4);
    /* 2->3->1->5->4->0 */
    SIX_COST_STRICT_CHECK(2, 3, 1, 5, 4, 0);
    /* 2->3->4->0->5->1 */
    SIX_COST_STRICT_CHECK(2, 3, 4, 0, 5, 1);
    /* 2->3->4->1->5->0 */
    SIX_COST_STRICT_CHECK(2, 3, 4, 1, 5, 0);
    /* 2->3->4->5->0->1 */
    SIX_COST_STRICT_CHECK(2, 3, 4, 5, 0, 1);
    /* 2->3->4->5->1->0 */
    SIX_COST_STRICT_CHECK(2, 3, 4, 5, 1, 0);
    /* 2->3->5->0->1->4 */
    SIX_COST_STRICT_CHECK(2, 3, 5, 0, 1, 4);
    /* 2->3->5->0->4->1 */
    SIX_COST_STRICT_CHECK(2, 3, 5, 0, 4, 1);
    /* 2->3->5->1->0->4 */
    SIX_COST_STRICT_CHECK(2, 3, 5, 1, 0, 4);
    /* 2->3->5->1->4->0 */
    SIX_COST_STRICT_CHECK(2, 3, 5, 1, 4, 0);
    /* 2->3->5->4->0->1 */
    SIX_COST_STRICT_CHECK(2, 3, 5, 4, 0, 1);
    /* 2->3->5->4->1->0 */
    SIX_COST_STRICT_CHECK(2, 3, 5, 4, 1, 0);
    /* 2->4->0->1->5->3 */
    SIX_COST_STRICT_CHECK(2, 4, 0, 1, 5, 3);
    /* 2->4->0->3->5->1 */
    SIX_COST_STRICT_CHECK(2, 4, 0, 3, 5, 1);
    /* 2->4->0->5->1->3 */
    SIX_COST_STRICT_CHECK(2, 4, 0, 5, 1, 3);
    /* 2->4->0->5->3->1 */
    SIX_COST_STRICT_CHECK(2, 4, 0, 5, 3, 1);
    /* 2->4->1->0->5->3 */
    SIX_COST_STRICT_CHECK(2, 4, 1, 0, 5, 3);
    /* 2->4->1->3->5->0 */
    SIX_COST_STRICT_CHECK(2, 4, 1, 3, 5, 0);
    /* 2->4->1->5->0->3 */
    SIX_COST_STRICT_CHECK(2, 4, 1, 5, 0, 3);
    /* 2->4->1->5->3->0 */
    SIX_COST_STRICT_CHECK(2, 4, 1, 5, 3, 0);
    /* 2->4->3->0->5->1 */
    SIX_COST_STRICT_CHECK(2, 4, 3, 0, 5, 1);
    /* 2->4->3->1->5->0 */
    SIX_COST_STRICT_CHECK(2, 4, 3, 1, 5, 0);
    /* 2->4->3->5->0->1 */
    SIX_COST_STRICT_CHECK(2, 4, 3, 5, 0, 1);
    /* 2->4->3->5->1->0 */
    SIX_COST_STRICT_CHECK(2, 4, 3, 5, 1, 0);
    /* 2->4->5->0->1->3 */
    SIX_COST_STRICT_CHECK(2, 4, 5, 0, 1, 3);
    /* 2->4->5->0->3->1 */
    SIX_COST_STRICT_CHECK(2, 4, 5, 0, 3, 1);
    /* 2->4->5->1->0->3 */
    SIX_COST_STRICT_CHECK(2, 4, 5, 1, 0, 3);
    /* 2->4->5->1->3->0 */
    SIX_COST_STRICT_CHECK(2, 4, 5, 1, 3, 0);
    /* 2->4->5->3->0->1 */
    SIX_COST_STRICT_CHECK(2, 4, 5, 3, 0, 1);
    /* 2->4->5->3->1->0 */
    SIX_COST_STRICT_CHECK(2, 4, 5, 3, 1, 0);
    /* 2->5->0->1->3->4 */
    SIX_COST_STRICT_CHECK(2, 5, 0, 1, 3, 4);
    /* 2->5->0->1->4->3 */
    SIX_COST_STRICT_CHECK(2, 5, 0, 1, 4, 3);
    /* 2->5->0->3->1->4 */
    SIX_COST_STRICT_CHECK(2, 5, 0, 3, 1, 4);
    /* 2->5->0->3->4->1 */
    SIX_COST_STRICT_CHECK(2, 5, 0, 3, 4, 1);
    /* 2->5->0->4->1->3 */
    SIX_COST_STRICT_CHECK(2, 5, 0, 4, 1, 3);
    /* 2->5->0->4->3->1 */
    SIX_COST_STRICT_CHECK(2, 5, 0, 4, 3, 1);
    /* 2->5->1->0->3->4 */
    SIX_COST_STRICT_CHECK(2, 5, 1, 0, 3, 4);
    /* 2->5->1->0->4->3 */
    SIX_COST_STRICT_CHECK(2, 5, 1, 0, 4, 3);
    /* 2->5->1->3->0->4 */
    SIX_COST_STRICT_CHECK(2, 5, 1, 3, 0, 4);
    /* 2->5->1->3->4->0 */
    SIX_COST_STRICT_CHECK(2, 5, 1, 3, 4, 0);
    /* 2->5->1->4->0->3 */
    SIX_COST_STRICT_CHECK(2, 5, 1, 4, 0, 3);
    /* 2->5->1->4->3->0 */
    SIX_COST_STRICT_CHECK(2, 5, 1, 4, 3, 0);
    /* 2->5->3->0->1->4 */
    SIX_COST_STRICT_CHECK(2, 5, 3, 0, 1, 4);
    /* 2->5->3->0->4->1 */
    SIX_COST_STRICT_CHECK(2, 5, 3, 0, 4, 1);
    /* 2->5->3->1->0->4 */
    SIX_COST_STRICT_CHECK(2, 5, 3, 1, 0, 4);
    /* 2->5->3->1->4->0 */
    SIX_COST_STRICT_CHECK(2, 5, 3, 1, 4, 0);
    /* 2->5->3->4->0->1 */
    SIX_COST_STRICT_CHECK(2, 5, 3, 4, 0, 1);
    /* 2->5->3->4->1->0 */
    SIX_COST_STRICT_CHECK(2, 5, 3, 4, 1, 0);
    /* 2->5->4->0->1->3 */
    SIX_COST_STRICT_CHECK(2, 5, 4, 0, 1, 3);
    /* 2->5->4->0->3->1 */
    SIX_COST_STRICT_CHECK(2, 5, 4, 0, 3, 1);
    /* 2->5->4->1->0->3 */
    SIX_COST_STRICT_CHECK(2, 5, 4, 1, 0, 3);
    /* 2->5->4->1->3->0 */
    SIX_COST_STRICT_CHECK(2, 5, 4, 1, 3, 0);
    /* 2->5->4->3->0->1 */
    SIX_COST_STRICT_CHECK(2, 5, 4, 3, 0, 1);
    /* 2->5->4->3->1->0 */
    SIX_COST_STRICT_CHECK(2, 5, 4, 3, 1, 0);
  }

  if(prob->sjob[j[3]]->tno < prob->sjob[j[0]]->tno) {
    /* 3->0->1->4->5->2 */
    SIX_COST_CHECK(3, 0, 1, 4, 5, 2);
    /* 3->0->1->5->2->4 */
    SIX_COST_CHECK(3, 0, 1, 5, 2, 4);
    /* 3->0->1->5->4->2 */
    SIX_COST_CHECK(3, 0, 1, 5, 4, 2);
    /* 3->0->2->4->5->1 */
    SIX_COST_CHECK(3, 0, 2, 4, 5, 1);
    /* 3->0->2->5->1->4 */
    SIX_COST_CHECK(3, 0, 2, 5, 1, 4);
    /* 3->0->2->5->4->1 */
    SIX_COST_CHECK(3, 0, 2, 5, 4, 1);
    /* 3->0->4->1->5->2 */
    SIX_COST_CHECK(3, 0, 4, 1, 5, 2);
    /* 3->0->4->2->5->1 */
    SIX_COST_CHECK(3, 0, 4, 2, 5, 1);
    /* 3->0->4->5->1->2 */
    SIX_COST_CHECK(3, 0, 4, 5, 1, 2);
    /* 3->0->4->5->2->1 */
    SIX_COST_CHECK(3, 0, 4, 5, 2, 1);
    /* 3->0->5->1->2->4 */
    SIX_COST_CHECK(3, 0, 5, 1, 2, 4);
    /* 3->0->5->1->4->2 */
    SIX_COST_CHECK(3, 0, 5, 1, 4, 2);
    /* 3->0->5->2->1->4 */
    SIX_COST_CHECK(3, 0, 5, 2, 1, 4);
    /* 3->0->5->2->4->1 */
    SIX_COST_CHECK(3, 0, 5, 2, 4, 1);
    /* 3->0->5->4->1->2 */
    SIX_COST_CHECK(3, 0, 5, 4, 1, 2);
    /* 3->0->5->4->2->1 */
    SIX_COST_CHECK(3, 0, 5, 4, 2, 1);
    /* 3->1->0->4->5->2 */
    SIX_COST_CHECK(3, 1, 0, 4, 5, 2);
    /* 3->1->0->5->2->4 */
    SIX_COST_CHECK(3, 1, 0, 5, 2, 4);
    /* 3->1->0->5->4->2 */
    SIX_COST_CHECK(3, 1, 0, 5, 4, 2);
    /* 3->1->2->4->5->0 */
    SIX_COST_CHECK(3, 1, 2, 4, 5, 0);
    /* 3->1->2->5->0->4 */
    SIX_COST_CHECK(3, 1, 2, 5, 0, 4);
    /* 3->1->2->5->4->0 */
    SIX_COST_CHECK(3, 1, 2, 5, 4, 0);
    /* 3->1->4->0->5->2 */
    SIX_COST_CHECK(3, 1, 4, 0, 5, 2);
    /* 3->1->4->2->5->0 */
    SIX_COST_CHECK(3, 1, 4, 2, 5, 0);
    /* 3->1->4->5->0->2 */
    SIX_COST_CHECK(3, 1, 4, 5, 0, 2);
    /* 3->1->4->5->2->0 */
    SIX_COST_CHECK(3, 1, 4, 5, 2, 0);
    /* 3->1->5->0->2->4 */
    SIX_COST_CHECK(3, 1, 5, 0, 2, 4);
    /* 3->1->5->0->4->2 */
    SIX_COST_CHECK(3, 1, 5, 0, 4, 2);
    /* 3->1->5->2->0->4 */
    SIX_COST_CHECK(3, 1, 5, 2, 0, 4);
    /* 3->1->5->2->4->0 */
    SIX_COST_CHECK(3, 1, 5, 2, 4, 0);
    /* 3->1->5->4->0->2 */
    SIX_COST_CHECK(3, 1, 5, 4, 0, 2);
    /* 3->1->5->4->2->0 */
    SIX_COST_CHECK(3, 1, 5, 4, 2, 0);
    /* 3->2->0->4->5->1 */
    SIX_COST_CHECK(3, 2, 0, 4, 5, 1);
    /* 3->2->0->5->1->4 */
    SIX_COST_CHECK(3, 2, 0, 5, 1, 4);
    /* 3->2->0->5->4->1 */
    SIX_COST_CHECK(3, 2, 0, 5, 4, 1);
    /* 3->2->1->4->5->0 */
    SIX_COST_CHECK(3, 2, 1, 4, 5, 0);
    /* 3->2->1->5->0->4 */
    SIX_COST_CHECK(3, 2, 1, 5, 0, 4);
    /* 3->2->1->5->4->0 */
    SIX_COST_CHECK(3, 2, 1, 5, 4, 0);
    /* 3->2->4->0->5->1 */
    SIX_COST_CHECK(3, 2, 4, 0, 5, 1);
    /* 3->2->4->1->5->0 */
    SIX_COST_CHECK(3, 2, 4, 1, 5, 0);
    /* 3->2->4->5->0->1 */
    SIX_COST_CHECK(3, 2, 4, 5, 0, 1);
    /* 3->2->4->5->1->0 */
    SIX_COST_CHECK(3, 2, 4, 5, 1, 0);
    /* 3->2->5->0->1->4 */
    SIX_COST_CHECK(3, 2, 5, 0, 1, 4);
    /* 3->2->5->0->4->1 */
    SIX_COST_CHECK(3, 2, 5, 0, 4, 1);
    /* 3->2->5->1->0->4 */
    SIX_COST_CHECK(3, 2, 5, 1, 0, 4);
    /* 3->2->5->1->4->0 */
    SIX_COST_CHECK(3, 2, 5, 1, 4, 0);
    /* 3->2->5->4->0->1 */
    SIX_COST_CHECK(3, 2, 5, 4, 0, 1);
    /* 3->2->5->4->1->0 */
    SIX_COST_CHECK(3, 2, 5, 4, 1, 0);
    /* 3->4->0->1->5->2 */
    SIX_COST_CHECK(3, 4, 0, 1, 5, 2);
    /* 3->4->0->2->5->1 */
    SIX_COST_CHECK(3, 4, 0, 2, 5, 1);
    /* 3->4->0->5->1->2 */
    SIX_COST_CHECK(3, 4, 0, 5, 1, 2);
    /* 3->4->0->5->2->1 */
    SIX_COST_CHECK(3, 4, 0, 5, 2, 1);
    /* 3->4->1->0->5->2 */
    SIX_COST_CHECK(3, 4, 1, 0, 5, 2);
    /* 3->4->1->2->5->0 */
    SIX_COST_CHECK(3, 4, 1, 2, 5, 0);
    /* 3->4->1->5->0->2 */
    SIX_COST_CHECK(3, 4, 1, 5, 0, 2);
    /* 3->4->1->5->2->0 */
    SIX_COST_CHECK(3, 4, 1, 5, 2, 0);
    /* 3->4->2->0->5->1 */
    SIX_COST_CHECK(3, 4, 2, 0, 5, 1);
    /* 3->4->2->1->5->0 */
    SIX_COST_CHECK(3, 4, 2, 1, 5, 0);
    /* 3->4->2->5->0->1 */
    SIX_COST_CHECK(3, 4, 2, 5, 0, 1);
    /* 3->4->2->5->1->0 */
    SIX_COST_CHECK(3, 4, 2, 5, 1, 0);
    /* 3->4->5->0->1->2 */
    SIX_COST_CHECK(3, 4, 5, 0, 1, 2);
    /* 3->4->5->0->2->1 */
    SIX_COST_CHECK(3, 4, 5, 0, 2, 1);
    /* 3->4->5->1->0->2 */
    SIX_COST_CHECK(3, 4, 5, 1, 0, 2);
    /* 3->4->5->1->2->0 */
    SIX_COST_CHECK(3, 4, 5, 1, 2, 0);
    /* 3->4->5->2->0->1 */
    SIX_COST_CHECK(3, 4, 5, 2, 0, 1);
    /* 3->4->5->2->1->0 */
    SIX_COST_CHECK(3, 4, 5, 2, 1, 0);
    /* 3->5->0->1->2->4 */
    SIX_COST_CHECK(3, 5, 0, 1, 2, 4);
    /* 3->5->0->1->4->2 */
    SIX_COST_CHECK(3, 5, 0, 1, 4, 2);
    /* 3->5->0->2->1->4 */
    SIX_COST_CHECK(3, 5, 0, 2, 1, 4);
    /* 3->5->0->2->4->1 */
    SIX_COST_CHECK(3, 5, 0, 2, 4, 1);
    /* 3->5->0->4->1->2 */
    SIX_COST_CHECK(3, 5, 0, 4, 1, 2);
    /* 3->5->0->4->2->1 */
    SIX_COST_CHECK(3, 5, 0, 4, 2, 1);
    /* 3->5->1->0->2->4 */
    SIX_COST_CHECK(3, 5, 1, 0, 2, 4);
    /* 3->5->1->0->4->2 */
    SIX_COST_CHECK(3, 5, 1, 0, 4, 2);
    /* 3->5->1->2->0->4 */
    SIX_COST_CHECK(3, 5, 1, 2, 0, 4);
    /* 3->5->1->2->4->0 */
    SIX_COST_CHECK(3, 5, 1, 2, 4, 0);
    /* 3->5->1->4->0->2 */
    SIX_COST_CHECK(3, 5, 1, 4, 0, 2);
    /* 3->5->1->4->2->0 */
    SIX_COST_CHECK(3, 5, 1, 4, 2, 0);
    /* 3->5->2->0->1->4 */
    SIX_COST_CHECK(3, 5, 2, 0, 1, 4);
    /* 3->5->2->0->4->1 */
    SIX_COST_CHECK(3, 5, 2, 0, 4, 1);
    /* 3->5->2->1->0->4 */
    SIX_COST_CHECK(3, 5, 2, 1, 0, 4);
    /* 3->5->2->1->4->0 */
    SIX_COST_CHECK(3, 5, 2, 1, 4, 0);
    /* 3->5->2->4->0->1 */
    SIX_COST_CHECK(3, 5, 2, 4, 0, 1);
    /* 3->5->2->4->1->0 */
    SIX_COST_CHECK(3, 5, 2, 4, 1, 0);
    /* 3->5->4->0->1->2 */
    SIX_COST_CHECK(3, 5, 4, 0, 1, 2);
    /* 3->5->4->0->2->1 */
    SIX_COST_CHECK(3, 5, 4, 0, 2, 1);
    /* 3->5->4->1->0->2 */
    SIX_COST_CHECK(3, 5, 4, 1, 0, 2);
    /* 3->5->4->1->2->0 */
    SIX_COST_CHECK(3, 5, 4, 1, 2, 0);
    /* 3->5->4->2->0->1 */
    SIX_COST_CHECK(3, 5, 4, 2, 0, 1);
    /* 3->5->4->2->1->0 */
    SIX_COST_CHECK(3, 5, 4, 2, 1, 0);
  } else {
    /* 3->0->1->4->5->2 */
    SIX_COST_STRICT_CHECK(3, 0, 1, 4, 5, 2);
    /* 3->0->1->5->2->4 */
    SIX_COST_STRICT_CHECK(3, 0, 1, 5, 2, 4);
    /* 3->0->1->5->4->2 */
    SIX_COST_STRICT_CHECK(3, 0, 1, 5, 4, 2);
    /* 3->0->2->4->5->1 */
    SIX_COST_STRICT_CHECK(3, 0, 2, 4, 5, 1);
    /* 3->0->2->5->1->4 */
    SIX_COST_STRICT_CHECK(3, 0, 2, 5, 1, 4);
    /* 3->0->2->5->4->1 */
    SIX_COST_STRICT_CHECK(3, 0, 2, 5, 4, 1);
    /* 3->0->4->1->5->2 */
    SIX_COST_STRICT_CHECK(3, 0, 4, 1, 5, 2);
    /* 3->0->4->2->5->1 */
    SIX_COST_STRICT_CHECK(3, 0, 4, 2, 5, 1);
    /* 3->0->4->5->1->2 */
    SIX_COST_STRICT_CHECK(3, 0, 4, 5, 1, 2);
    /* 3->0->4->5->2->1 */
    SIX_COST_STRICT_CHECK(3, 0, 4, 5, 2, 1);
    /* 3->0->5->1->2->4 */
    SIX_COST_STRICT_CHECK(3, 0, 5, 1, 2, 4);
    /* 3->0->5->1->4->2 */
    SIX_COST_STRICT_CHECK(3, 0, 5, 1, 4, 2);
    /* 3->0->5->2->1->4 */
    SIX_COST_STRICT_CHECK(3, 0, 5, 2, 1, 4);
    /* 3->0->5->2->4->1 */
    SIX_COST_STRICT_CHECK(3, 0, 5, 2, 4, 1);
    /* 3->0->5->4->1->2 */
    SIX_COST_STRICT_CHECK(3, 0, 5, 4, 1, 2);
    /* 3->0->5->4->2->1 */
    SIX_COST_STRICT_CHECK(3, 0, 5, 4, 2, 1);
    /* 3->1->0->4->5->2 */
    SIX_COST_STRICT_CHECK(3, 1, 0, 4, 5, 2);
    /* 3->1->0->5->2->4 */
    SIX_COST_STRICT_CHECK(3, 1, 0, 5, 2, 4);
    /* 3->1->0->5->4->2 */
    SIX_COST_STRICT_CHECK(3, 1, 0, 5, 4, 2);
    /* 3->1->2->4->5->0 */
    SIX_COST_STRICT_CHECK(3, 1, 2, 4, 5, 0);
    /* 3->1->2->5->0->4 */
    SIX_COST_STRICT_CHECK(3, 1, 2, 5, 0, 4);
    /* 3->1->2->5->4->0 */
    SIX_COST_STRICT_CHECK(3, 1, 2, 5, 4, 0);
    /* 3->1->4->0->5->2 */
    SIX_COST_STRICT_CHECK(3, 1, 4, 0, 5, 2);
    /* 3->1->4->2->5->0 */
    SIX_COST_STRICT_CHECK(3, 1, 4, 2, 5, 0);
    /* 3->1->4->5->0->2 */
    SIX_COST_STRICT_CHECK(3, 1, 4, 5, 0, 2);
    /* 3->1->4->5->2->0 */
    SIX_COST_STRICT_CHECK(3, 1, 4, 5, 2, 0);
    /* 3->1->5->0->2->4 */
    SIX_COST_STRICT_CHECK(3, 1, 5, 0, 2, 4);
    /* 3->1->5->0->4->2 */
    SIX_COST_STRICT_CHECK(3, 1, 5, 0, 4, 2);
    /* 3->1->5->2->0->4 */
    SIX_COST_STRICT_CHECK(3, 1, 5, 2, 0, 4);
    /* 3->1->5->2->4->0 */
    SIX_COST_STRICT_CHECK(3, 1, 5, 2, 4, 0);
    /* 3->1->5->4->0->2 */
    SIX_COST_STRICT_CHECK(3, 1, 5, 4, 0, 2);
    /* 3->1->5->4->2->0 */
    SIX_COST_STRICT_CHECK(3, 1, 5, 4, 2, 0);
    /* 3->2->0->4->5->1 */
    SIX_COST_STRICT_CHECK(3, 2, 0, 4, 5, 1);
    /* 3->2->0->5->1->4 */
    SIX_COST_STRICT_CHECK(3, 2, 0, 5, 1, 4);
    /* 3->2->0->5->4->1 */
    SIX_COST_STRICT_CHECK(3, 2, 0, 5, 4, 1);
    /* 3->2->1->4->5->0 */
    SIX_COST_STRICT_CHECK(3, 2, 1, 4, 5, 0);
    /* 3->2->1->5->0->4 */
    SIX_COST_STRICT_CHECK(3, 2, 1, 5, 0, 4);
    /* 3->2->1->5->4->0 */
    SIX_COST_STRICT_CHECK(3, 2, 1, 5, 4, 0);
    /* 3->2->4->0->5->1 */
    SIX_COST_STRICT_CHECK(3, 2, 4, 0, 5, 1);
    /* 3->2->4->1->5->0 */
    SIX_COST_STRICT_CHECK(3, 2, 4, 1, 5, 0);
    /* 3->2->4->5->0->1 */
    SIX_COST_STRICT_CHECK(3, 2, 4, 5, 0, 1);
    /* 3->2->4->5->1->0 */
    SIX_COST_STRICT_CHECK(3, 2, 4, 5, 1, 0);
    /* 3->2->5->0->1->4 */
    SIX_COST_STRICT_CHECK(3, 2, 5, 0, 1, 4);
    /* 3->2->5->0->4->1 */
    SIX_COST_STRICT_CHECK(3, 2, 5, 0, 4, 1);
    /* 3->2->5->1->0->4 */
    SIX_COST_STRICT_CHECK(3, 2, 5, 1, 0, 4);
    /* 3->2->5->1->4->0 */
    SIX_COST_STRICT_CHECK(3, 2, 5, 1, 4, 0);
    /* 3->2->5->4->0->1 */
    SIX_COST_STRICT_CHECK(3, 2, 5, 4, 0, 1);
    /* 3->2->5->4->1->0 */
    SIX_COST_STRICT_CHECK(3, 2, 5, 4, 1, 0);
    /* 3->4->0->1->5->2 */
    SIX_COST_STRICT_CHECK(3, 4, 0, 1, 5, 2);
    /* 3->4->0->2->5->1 */
    SIX_COST_STRICT_CHECK(3, 4, 0, 2, 5, 1);
    /* 3->4->0->5->1->2 */
    SIX_COST_STRICT_CHECK(3, 4, 0, 5, 1, 2);
    /* 3->4->0->5->2->1 */
    SIX_COST_STRICT_CHECK(3, 4, 0, 5, 2, 1);
    /* 3->4->1->0->5->2 */
    SIX_COST_STRICT_CHECK(3, 4, 1, 0, 5, 2);
    /* 3->4->1->2->5->0 */
    SIX_COST_STRICT_CHECK(3, 4, 1, 2, 5, 0);
    /* 3->4->1->5->0->2 */
    SIX_COST_STRICT_CHECK(3, 4, 1, 5, 0, 2);
    /* 3->4->1->5->2->0 */
    SIX_COST_STRICT_CHECK(3, 4, 1, 5, 2, 0);
    /* 3->4->2->0->5->1 */
    SIX_COST_STRICT_CHECK(3, 4, 2, 0, 5, 1);
    /* 3->4->2->1->5->0 */
    SIX_COST_STRICT_CHECK(3, 4, 2, 1, 5, 0);
    /* 3->4->2->5->0->1 */
    SIX_COST_STRICT_CHECK(3, 4, 2, 5, 0, 1);
    /* 3->4->2->5->1->0 */
    SIX_COST_STRICT_CHECK(3, 4, 2, 5, 1, 0);
    /* 3->4->5->0->1->2 */
    SIX_COST_STRICT_CHECK(3, 4, 5, 0, 1, 2);
    /* 3->4->5->0->2->1 */
    SIX_COST_STRICT_CHECK(3, 4, 5, 0, 2, 1);
    /* 3->4->5->1->0->2 */
    SIX_COST_STRICT_CHECK(3, 4, 5, 1, 0, 2);
    /* 3->4->5->1->2->0 */
    SIX_COST_STRICT_CHECK(3, 4, 5, 1, 2, 0);
    /* 3->4->5->2->0->1 */
    SIX_COST_STRICT_CHECK(3, 4, 5, 2, 0, 1);
    /* 3->4->5->2->1->0 */
    SIX_COST_STRICT_CHECK(3, 4, 5, 2, 1, 0);
    /* 3->5->0->1->2->4 */
    SIX_COST_STRICT_CHECK(3, 5, 0, 1, 2, 4);
    /* 3->5->0->1->4->2 */
    SIX_COST_STRICT_CHECK(3, 5, 0, 1, 4, 2);
    /* 3->5->0->2->1->4 */
    SIX_COST_STRICT_CHECK(3, 5, 0, 2, 1, 4);
    /* 3->5->0->2->4->1 */
    SIX_COST_STRICT_CHECK(3, 5, 0, 2, 4, 1);
    /* 3->5->0->4->1->2 */
    SIX_COST_STRICT_CHECK(3, 5, 0, 4, 1, 2);
    /* 3->5->0->4->2->1 */
    SIX_COST_STRICT_CHECK(3, 5, 0, 4, 2, 1);
    /* 3->5->1->0->2->4 */
    SIX_COST_STRICT_CHECK(3, 5, 1, 0, 2, 4);
    /* 3->5->1->0->4->2 */
    SIX_COST_STRICT_CHECK(3, 5, 1, 0, 4, 2);
    /* 3->5->1->2->0->4 */
    SIX_COST_STRICT_CHECK(3, 5, 1, 2, 0, 4);
    /* 3->5->1->2->4->0 */
    SIX_COST_STRICT_CHECK(3, 5, 1, 2, 4, 0);
    /* 3->5->1->4->0->2 */
    SIX_COST_STRICT_CHECK(3, 5, 1, 4, 0, 2);
    /* 3->5->1->4->2->0 */
    SIX_COST_STRICT_CHECK(3, 5, 1, 4, 2, 0);
    /* 3->5->2->0->1->4 */
    SIX_COST_STRICT_CHECK(3, 5, 2, 0, 1, 4);
    /* 3->5->2->0->4->1 */
    SIX_COST_STRICT_CHECK(3, 5, 2, 0, 4, 1);
    /* 3->5->2->1->0->4 */
    SIX_COST_STRICT_CHECK(3, 5, 2, 1, 0, 4);
    /* 3->5->2->1->4->0 */
    SIX_COST_STRICT_CHECK(3, 5, 2, 1, 4, 0);
    /* 3->5->2->4->0->1 */
    SIX_COST_STRICT_CHECK(3, 5, 2, 4, 0, 1);
    /* 3->5->2->4->1->0 */
    SIX_COST_STRICT_CHECK(3, 5, 2, 4, 1, 0);
    /* 3->5->4->0->1->2 */
    SIX_COST_STRICT_CHECK(3, 5, 4, 0, 1, 2);
    /* 3->5->4->0->2->1 */
    SIX_COST_STRICT_CHECK(3, 5, 4, 0, 2, 1);
    /* 3->5->4->1->0->2 */
    SIX_COST_STRICT_CHECK(3, 5, 4, 1, 0, 2);
    /* 3->5->4->1->2->0 */
    SIX_COST_STRICT_CHECK(3, 5, 4, 1, 2, 0);
    /* 3->5->4->2->0->1 */
    SIX_COST_STRICT_CHECK(3, 5, 4, 2, 0, 1);
    /* 3->5->4->2->1->0 */
    SIX_COST_STRICT_CHECK(3, 5, 4, 2, 1, 0);
  }

  if(prob->sjob[j[4]]->tno < prob->sjob[j[0]]->tno) {
    /* 4->0->1->2->5->3 */
    SIX_COST_CHECK(4, 0, 1, 2, 5, 3);
    /* 4->0->1->3->5->2 */
    SIX_COST_CHECK(4, 0, 1, 3, 5, 2);
    /* 4->0->1->5->2->3 */
    SIX_COST_CHECK(4, 0, 1, 5, 2, 3);
    /* 4->0->1->5->3->2 */
    SIX_COST_CHECK(4, 0, 1, 5, 3, 2);
    /* 4->0->2->1->5->3 */
    SIX_COST_CHECK(4, 0, 2, 1, 5, 3);
    /* 4->0->2->3->5->1 */
    SIX_COST_CHECK(4, 0, 2, 3, 5, 1);
    /* 4->0->2->5->1->3 */
    SIX_COST_CHECK(4, 0, 2, 5, 1, 3);
    /* 4->0->2->5->3->1 */
    SIX_COST_CHECK(4, 0, 2, 5, 3, 1);
    /* 4->0->3->1->5->2 */
    SIX_COST_CHECK(4, 0, 3, 1, 5, 2);
    /* 4->0->3->2->5->1 */
    SIX_COST_CHECK(4, 0, 3, 2, 5, 1);
    /* 4->0->3->5->1->2 */
    SIX_COST_CHECK(4, 0, 3, 5, 1, 2);
    /* 4->0->3->5->2->1 */
    SIX_COST_CHECK(4, 0, 3, 5, 2, 1);
    /* 4->0->5->1->2->3 */
    SIX_COST_CHECK(4, 0, 5, 1, 2, 3);
    /* 4->0->5->1->3->2 */
    SIX_COST_CHECK(4, 0, 5, 1, 3, 2);
    /* 4->0->5->2->1->3 */
    SIX_COST_CHECK(4, 0, 5, 2, 1, 3);
    /* 4->0->5->2->3->1 */
    SIX_COST_CHECK(4, 0, 5, 2, 3, 1);
    /* 4->0->5->3->1->2 */
    SIX_COST_CHECK(4, 0, 5, 3, 1, 2);
    /* 4->0->5->3->2->1 */
    SIX_COST_CHECK(4, 0, 5, 3, 2, 1);
    /* 4->1->0->2->5->3 */
    SIX_COST_CHECK(4, 1, 0, 2, 5, 3);
    /* 4->1->0->3->5->2 */
    SIX_COST_CHECK(4, 1, 0, 3, 5, 2);
    /* 4->1->0->5->2->3 */
    SIX_COST_CHECK(4, 1, 0, 5, 2, 3);
    /* 4->1->0->5->3->2 */
    SIX_COST_CHECK(4, 1, 0, 5, 3, 2);
    /* 4->1->2->0->5->3 */
    SIX_COST_CHECK(4, 1, 2, 0, 5, 3);
    /* 4->1->2->3->5->0 */
    SIX_COST_CHECK(4, 1, 2, 3, 5, 0);
    /* 4->1->2->5->0->3 */
    SIX_COST_CHECK(4, 1, 2, 5, 0, 3);
    /* 4->1->2->5->3->0 */
    SIX_COST_CHECK(4, 1, 2, 5, 3, 0);
    /* 4->1->3->0->5->2 */
    SIX_COST_CHECK(4, 1, 3, 0, 5, 2);
    /* 4->1->3->2->5->0 */
    SIX_COST_CHECK(4, 1, 3, 2, 5, 0);
    /* 4->1->3->5->0->2 */
    SIX_COST_CHECK(4, 1, 3, 5, 0, 2);
    /* 4->1->3->5->2->0 */
    SIX_COST_CHECK(4, 1, 3, 5, 2, 0);
    /* 4->1->5->0->2->3 */
    SIX_COST_CHECK(4, 1, 5, 0, 2, 3);
    /* 4->1->5->0->3->2 */
    SIX_COST_CHECK(4, 1, 5, 0, 3, 2);
    /* 4->1->5->2->0->3 */
    SIX_COST_CHECK(4, 1, 5, 2, 0, 3);
    /* 4->1->5->2->3->0 */
    SIX_COST_CHECK(4, 1, 5, 2, 3, 0);
    /* 4->1->5->3->0->2 */
    SIX_COST_CHECK(4, 1, 5, 3, 0, 2);
    /* 4->1->5->3->2->0 */
    SIX_COST_CHECK(4, 1, 5, 3, 2, 0);
    /* 4->2->0->1->5->3 */
    SIX_COST_CHECK(4, 2, 0, 1, 5, 3);
    /* 4->2->0->3->5->1 */
    SIX_COST_CHECK(4, 2, 0, 3, 5, 1);
    /* 4->2->0->5->1->3 */
    SIX_COST_CHECK(4, 2, 0, 5, 1, 3);
    /* 4->2->0->5->3->1 */
    SIX_COST_CHECK(4, 2, 0, 5, 3, 1);
    /* 4->2->1->0->5->3 */
    SIX_COST_CHECK(4, 2, 1, 0, 5, 3);
    /* 4->2->1->3->5->0 */
    SIX_COST_CHECK(4, 2, 1, 3, 5, 0);
    /* 4->2->1->5->0->3 */
    SIX_COST_CHECK(4, 2, 1, 5, 0, 3);
    /* 4->2->1->5->3->0 */
    SIX_COST_CHECK(4, 2, 1, 5, 3, 0);
    /* 4->2->3->0->5->1 */
    SIX_COST_CHECK(4, 2, 3, 0, 5, 1);
    /* 4->2->3->1->5->0 */
    SIX_COST_CHECK(4, 2, 3, 1, 5, 0);
    /* 4->2->3->5->0->1 */
    SIX_COST_CHECK(4, 2, 3, 5, 0, 1);
    /* 4->2->3->5->1->0 */
    SIX_COST_CHECK(4, 2, 3, 5, 1, 0);
    /* 4->2->5->0->1->3 */
    SIX_COST_CHECK(4, 2, 5, 0, 1, 3);
    /* 4->2->5->0->3->1 */
    SIX_COST_CHECK(4, 2, 5, 0, 3, 1);
    /* 4->2->5->1->0->3 */
    SIX_COST_CHECK(4, 2, 5, 1, 0, 3);
    /* 4->2->5->1->3->0 */
    SIX_COST_CHECK(4, 2, 5, 1, 3, 0);
    /* 4->2->5->3->0->1 */
    SIX_COST_CHECK(4, 2, 5, 3, 0, 1);
    /* 4->2->5->3->1->0 */
    SIX_COST_CHECK(4, 2, 5, 3, 1, 0);
    /* 4->3->0->1->5->2 */
    SIX_COST_CHECK(4, 3, 0, 1, 5, 2);
    /* 4->3->0->2->5->1 */
    SIX_COST_CHECK(4, 3, 0, 2, 5, 1);
    /* 4->3->0->5->1->2 */
    SIX_COST_CHECK(4, 3, 0, 5, 1, 2);
    /* 4->3->0->5->2->1 */
    SIX_COST_CHECK(4, 3, 0, 5, 2, 1);
    /* 4->3->1->0->5->2 */
    SIX_COST_CHECK(4, 3, 1, 0, 5, 2);
    /* 4->3->1->2->5->0 */
    SIX_COST_CHECK(4, 3, 1, 2, 5, 0);
    /* 4->3->1->5->0->2 */
    SIX_COST_CHECK(4, 3, 1, 5, 0, 2);
    /* 4->3->1->5->2->0 */
    SIX_COST_CHECK(4, 3, 1, 5, 2, 0);
    /* 4->3->2->0->5->1 */
    SIX_COST_CHECK(4, 3, 2, 0, 5, 1);
    /* 4->3->2->1->5->0 */
    SIX_COST_CHECK(4, 3, 2, 1, 5, 0);
    /* 4->3->2->5->0->1 */
    SIX_COST_CHECK(4, 3, 2, 5, 0, 1);
    /* 4->3->2->5->1->0 */
    SIX_COST_CHECK(4, 3, 2, 5, 1, 0);
    /* 4->3->5->0->1->2 */
    SIX_COST_CHECK(4, 3, 5, 0, 1, 2);
    /* 4->3->5->0->2->1 */
    SIX_COST_CHECK(4, 3, 5, 0, 2, 1);
    /* 4->3->5->1->0->2 */
    SIX_COST_CHECK(4, 3, 5, 1, 0, 2);
    /* 4->3->5->1->2->0 */
    SIX_COST_CHECK(4, 3, 5, 1, 2, 0);
    /* 4->3->5->2->0->1 */
    SIX_COST_CHECK(4, 3, 5, 2, 0, 1);
    /* 4->3->5->2->1->0 */
    SIX_COST_CHECK(4, 3, 5, 2, 1, 0);
    /* 4->5->0->1->2->3 */
    SIX_COST_CHECK(4, 5, 0, 1, 2, 3);
    /* 4->5->0->1->3->2 */
    SIX_COST_CHECK(4, 5, 0, 1, 3, 2);
    /* 4->5->0->2->1->3 */
    SIX_COST_CHECK(4, 5, 0, 2, 1, 3);
    /* 4->5->0->2->3->1 */
    SIX_COST_CHECK(4, 5, 0, 2, 3, 1);
    /* 4->5->0->3->1->2 */
    SIX_COST_CHECK(4, 5, 0, 3, 1, 2);
    /* 4->5->0->3->2->1 */
    SIX_COST_CHECK(4, 5, 0, 3, 2, 1);
    /* 4->5->1->0->2->3 */
    SIX_COST_CHECK(4, 5, 1, 0, 2, 3);
    /* 4->5->1->0->3->2 */
    SIX_COST_CHECK(4, 5, 1, 0, 3, 2);
    /* 4->5->1->2->0->3 */
    SIX_COST_CHECK(4, 5, 1, 2, 0, 3);
    /* 4->5->1->2->3->0 */
    SIX_COST_CHECK(4, 5, 1, 2, 3, 0);
    /* 4->5->1->3->0->2 */
    SIX_COST_CHECK(4, 5, 1, 3, 0, 2);
    /* 4->5->1->3->2->0 */
    SIX_COST_CHECK(4, 5, 1, 3, 2, 0);
    /* 4->5->2->0->1->3 */
    SIX_COST_CHECK(4, 5, 2, 0, 1, 3);
    /* 4->5->2->0->3->1 */
    SIX_COST_CHECK(4, 5, 2, 0, 3, 1);
    /* 4->5->2->1->0->3 */
    SIX_COST_CHECK(4, 5, 2, 1, 0, 3);
    /* 4->5->2->1->3->0 */
    SIX_COST_CHECK(4, 5, 2, 1, 3, 0);
    /* 4->5->2->3->0->1 */
    SIX_COST_CHECK(4, 5, 2, 3, 0, 1);
    /* 4->5->2->3->1->0 */
    SIX_COST_CHECK(4, 5, 2, 3, 1, 0);
    /* 4->5->3->0->1->2 */
    SIX_COST_CHECK(4, 5, 3, 0, 1, 2);
    /* 4->5->3->0->2->1 */
    SIX_COST_CHECK(4, 5, 3, 0, 2, 1);
    /* 4->5->3->1->0->2 */
    SIX_COST_CHECK(4, 5, 3, 1, 0, 2);
    /* 4->5->3->1->2->0 */
    SIX_COST_CHECK(4, 5, 3, 1, 2, 0);
    /* 4->5->3->2->0->1 */
    SIX_COST_CHECK(4, 5, 3, 2, 0, 1);
    /* 4->5->3->2->1->0 */
    SIX_COST_CHECK(4, 5, 3, 2, 1, 0);
  } else {
    /* 4->0->1->2->5->3 */
    SIX_COST_STRICT_CHECK(4, 0, 1, 2, 5, 3);
    /* 4->0->1->3->5->2 */
    SIX_COST_STRICT_CHECK(4, 0, 1, 3, 5, 2);
    /* 4->0->1->5->2->3 */
    SIX_COST_STRICT_CHECK(4, 0, 1, 5, 2, 3);
    /* 4->0->1->5->3->2 */
    SIX_COST_STRICT_CHECK(4, 0, 1, 5, 3, 2);
    /* 4->0->2->1->5->3 */
    SIX_COST_STRICT_CHECK(4, 0, 2, 1, 5, 3);
    /* 4->0->2->3->5->1 */
    SIX_COST_STRICT_CHECK(4, 0, 2, 3, 5, 1);
    /* 4->0->2->5->1->3 */
    SIX_COST_STRICT_CHECK(4, 0, 2, 5, 1, 3);
    /* 4->0->2->5->3->1 */
    SIX_COST_STRICT_CHECK(4, 0, 2, 5, 3, 1);
    /* 4->0->3->1->5->2 */
    SIX_COST_STRICT_CHECK(4, 0, 3, 1, 5, 2);
    /* 4->0->3->2->5->1 */
    SIX_COST_STRICT_CHECK(4, 0, 3, 2, 5, 1);
    /* 4->0->3->5->1->2 */
    SIX_COST_STRICT_CHECK(4, 0, 3, 5, 1, 2);
    /* 4->0->3->5->2->1 */
    SIX_COST_STRICT_CHECK(4, 0, 3, 5, 2, 1);
    /* 4->0->5->1->2->3 */
    SIX_COST_STRICT_CHECK(4, 0, 5, 1, 2, 3);
    /* 4->0->5->1->3->2 */
    SIX_COST_STRICT_CHECK(4, 0, 5, 1, 3, 2);
    /* 4->0->5->2->1->3 */
    SIX_COST_STRICT_CHECK(4, 0, 5, 2, 1, 3);
    /* 4->0->5->2->3->1 */
    SIX_COST_STRICT_CHECK(4, 0, 5, 2, 3, 1);
    /* 4->0->5->3->1->2 */
    SIX_COST_STRICT_CHECK(4, 0, 5, 3, 1, 2);
    /* 4->0->5->3->2->1 */
    SIX_COST_STRICT_CHECK(4, 0, 5, 3, 2, 1);
    /* 4->1->0->2->5->3 */
    SIX_COST_STRICT_CHECK(4, 1, 0, 2, 5, 3);
    /* 4->1->0->3->5->2 */
    SIX_COST_STRICT_CHECK(4, 1, 0, 3, 5, 2);
    /* 4->1->0->5->2->3 */
    SIX_COST_STRICT_CHECK(4, 1, 0, 5, 2, 3);
    /* 4->1->0->5->3->2 */
    SIX_COST_STRICT_CHECK(4, 1, 0, 5, 3, 2);
    /* 4->1->2->0->5->3 */
    SIX_COST_STRICT_CHECK(4, 1, 2, 0, 5, 3);
    /* 4->1->2->3->5->0 */
    SIX_COST_STRICT_CHECK(4, 1, 2, 3, 5, 0);
    /* 4->1->2->5->0->3 */
    SIX_COST_STRICT_CHECK(4, 1, 2, 5, 0, 3);
    /* 4->1->2->5->3->0 */
    SIX_COST_STRICT_CHECK(4, 1, 2, 5, 3, 0);
    /* 4->1->3->0->5->2 */
    SIX_COST_STRICT_CHECK(4, 1, 3, 0, 5, 2);
    /* 4->1->3->2->5->0 */
    SIX_COST_STRICT_CHECK(4, 1, 3, 2, 5, 0);
    /* 4->1->3->5->0->2 */
    SIX_COST_STRICT_CHECK(4, 1, 3, 5, 0, 2);
    /* 4->1->3->5->2->0 */
    SIX_COST_STRICT_CHECK(4, 1, 3, 5, 2, 0);
    /* 4->1->5->0->2->3 */
    SIX_COST_STRICT_CHECK(4, 1, 5, 0, 2, 3);
    /* 4->1->5->0->3->2 */
    SIX_COST_STRICT_CHECK(4, 1, 5, 0, 3, 2);
    /* 4->1->5->2->0->3 */
    SIX_COST_STRICT_CHECK(4, 1, 5, 2, 0, 3);
    /* 4->1->5->2->3->0 */
    SIX_COST_STRICT_CHECK(4, 1, 5, 2, 3, 0);
    /* 4->1->5->3->0->2 */
    SIX_COST_STRICT_CHECK(4, 1, 5, 3, 0, 2);
    /* 4->1->5->3->2->0 */
    SIX_COST_STRICT_CHECK(4, 1, 5, 3, 2, 0);
    /* 4->2->0->1->5->3 */
    SIX_COST_STRICT_CHECK(4, 2, 0, 1, 5, 3);
    /* 4->2->0->3->5->1 */
    SIX_COST_STRICT_CHECK(4, 2, 0, 3, 5, 1);
    /* 4->2->0->5->1->3 */
    SIX_COST_STRICT_CHECK(4, 2, 0, 5, 1, 3);
    /* 4->2->0->5->3->1 */
    SIX_COST_STRICT_CHECK(4, 2, 0, 5, 3, 1);
    /* 4->2->1->0->5->3 */
    SIX_COST_STRICT_CHECK(4, 2, 1, 0, 5, 3);
    /* 4->2->1->3->5->0 */
    SIX_COST_STRICT_CHECK(4, 2, 1, 3, 5, 0);
    /* 4->2->1->5->0->3 */
    SIX_COST_STRICT_CHECK(4, 2, 1, 5, 0, 3);
    /* 4->2->1->5->3->0 */
    SIX_COST_STRICT_CHECK(4, 2, 1, 5, 3, 0);
    /* 4->2->3->0->5->1 */
    SIX_COST_STRICT_CHECK(4, 2, 3, 0, 5, 1);
    /* 4->2->3->1->5->0 */
    SIX_COST_STRICT_CHECK(4, 2, 3, 1, 5, 0);
    /* 4->2->3->5->0->1 */
    SIX_COST_STRICT_CHECK(4, 2, 3, 5, 0, 1);
    /* 4->2->3->5->1->0 */
    SIX_COST_STRICT_CHECK(4, 2, 3, 5, 1, 0);
    /* 4->2->5->0->1->3 */
    SIX_COST_STRICT_CHECK(4, 2, 5, 0, 1, 3);
    /* 4->2->5->0->3->1 */
    SIX_COST_STRICT_CHECK(4, 2, 5, 0, 3, 1);
    /* 4->2->5->1->0->3 */
    SIX_COST_STRICT_CHECK(4, 2, 5, 1, 0, 3);
    /* 4->2->5->1->3->0 */
    SIX_COST_STRICT_CHECK(4, 2, 5, 1, 3, 0);
    /* 4->2->5->3->0->1 */
    SIX_COST_STRICT_CHECK(4, 2, 5, 3, 0, 1);
    /* 4->2->5->3->1->0 */
    SIX_COST_STRICT_CHECK(4, 2, 5, 3, 1, 0);
    /* 4->3->0->1->5->2 */
    SIX_COST_STRICT_CHECK(4, 3, 0, 1, 5, 2);
    /* 4->3->0->2->5->1 */
    SIX_COST_STRICT_CHECK(4, 3, 0, 2, 5, 1);
    /* 4->3->0->5->1->2 */
    SIX_COST_STRICT_CHECK(4, 3, 0, 5, 1, 2);
    /* 4->3->0->5->2->1 */
    SIX_COST_STRICT_CHECK(4, 3, 0, 5, 2, 1);
    /* 4->3->1->0->5->2 */
    SIX_COST_STRICT_CHECK(4, 3, 1, 0, 5, 2);
    /* 4->3->1->2->5->0 */
    SIX_COST_STRICT_CHECK(4, 3, 1, 2, 5, 0);
    /* 4->3->1->5->0->2 */
    SIX_COST_STRICT_CHECK(4, 3, 1, 5, 0, 2);
    /* 4->3->1->5->2->0 */
    SIX_COST_STRICT_CHECK(4, 3, 1, 5, 2, 0);
    /* 4->3->2->0->5->1 */
    SIX_COST_STRICT_CHECK(4, 3, 2, 0, 5, 1);
    /* 4->3->2->1->5->0 */
    SIX_COST_STRICT_CHECK(4, 3, 2, 1, 5, 0);
    /* 4->3->2->5->0->1 */
    SIX_COST_STRICT_CHECK(4, 3, 2, 5, 0, 1);
    /* 4->3->2->5->1->0 */
    SIX_COST_STRICT_CHECK(4, 3, 2, 5, 1, 0);
    /* 4->3->5->0->1->2 */
    SIX_COST_STRICT_CHECK(4, 3, 5, 0, 1, 2);
    /* 4->3->5->0->2->1 */
    SIX_COST_STRICT_CHECK(4, 3, 5, 0, 2, 1);
    /* 4->3->5->1->0->2 */
    SIX_COST_STRICT_CHECK(4, 3, 5, 1, 0, 2);
    /* 4->3->5->1->2->0 */
    SIX_COST_STRICT_CHECK(4, 3, 5, 1, 2, 0);
    /* 4->3->5->2->0->1 */
    SIX_COST_STRICT_CHECK(4, 3, 5, 2, 0, 1);
    /* 4->3->5->2->1->0 */
    SIX_COST_STRICT_CHECK(4, 3, 5, 2, 1, 0);
    /* 4->5->0->1->2->3 */
    SIX_COST_STRICT_CHECK(4, 5, 0, 1, 2, 3);
    /* 4->5->0->1->3->2 */
    SIX_COST_STRICT_CHECK(4, 5, 0, 1, 3, 2);
    /* 4->5->0->2->1->3 */
    SIX_COST_STRICT_CHECK(4, 5, 0, 2, 1, 3);
    /* 4->5->0->2->3->1 */
    SIX_COST_STRICT_CHECK(4, 5, 0, 2, 3, 1);
    /* 4->5->0->3->1->2 */
    SIX_COST_STRICT_CHECK(4, 5, 0, 3, 1, 2);
    /* 4->5->0->3->2->1 */
    SIX_COST_STRICT_CHECK(4, 5, 0, 3, 2, 1);
    /* 4->5->1->0->2->3 */
    SIX_COST_STRICT_CHECK(4, 5, 1, 0, 2, 3);
    /* 4->5->1->0->3->2 */
    SIX_COST_STRICT_CHECK(4, 5, 1, 0, 3, 2);
    /* 4->5->1->2->0->3 */
    SIX_COST_STRICT_CHECK(4, 5, 1, 2, 0, 3);
    /* 4->5->1->2->3->0 */
    SIX_COST_STRICT_CHECK(4, 5, 1, 2, 3, 0);
    /* 4->5->1->3->0->2 */
    SIX_COST_STRICT_CHECK(4, 5, 1, 3, 0, 2);
    /* 4->5->1->3->2->0 */
    SIX_COST_STRICT_CHECK(4, 5, 1, 3, 2, 0);
    /* 4->5->2->0->1->3 */
    SIX_COST_STRICT_CHECK(4, 5, 2, 0, 1, 3);
    /* 4->5->2->0->3->1 */
    SIX_COST_STRICT_CHECK(4, 5, 2, 0, 3, 1);
    /* 4->5->2->1->0->3 */
    SIX_COST_STRICT_CHECK(4, 5, 2, 1, 0, 3);
    /* 4->5->2->1->3->0 */
    SIX_COST_STRICT_CHECK(4, 5, 2, 1, 3, 0);
    /* 4->5->2->3->0->1 */
    SIX_COST_STRICT_CHECK(4, 5, 2, 3, 0, 1);
    /* 4->5->2->3->1->0 */
    SIX_COST_STRICT_CHECK(4, 5, 2, 3, 1, 0);
    /* 4->5->3->0->1->2 */
    SIX_COST_STRICT_CHECK(4, 5, 3, 0, 1, 2);
    /* 4->5->3->0->2->1 */
    SIX_COST_STRICT_CHECK(4, 5, 3, 0, 2, 1);
    /* 4->5->3->1->0->2 */
    SIX_COST_STRICT_CHECK(4, 5, 3, 1, 0, 2);
    /* 4->5->3->1->2->0 */
    SIX_COST_STRICT_CHECK(4, 5, 3, 1, 2, 0);
    /* 4->5->3->2->0->1 */
    SIX_COST_STRICT_CHECK(4, 5, 3, 2, 0, 1);
    /* 4->5->3->2->1->0 */
    SIX_COST_STRICT_CHECK(4, 5, 3, 2, 1, 0);
  }

  if(prob->sjob[j[5]]->tno < prob->sjob[j[0]]->tno) {
    /* 5->0->1->2->3->4 */
    SIX_COST_CHECK(5, 0, 1, 2, 3, 4);
    /* 5->0->1->2->4->3 */
    SIX_COST_CHECK(5, 0, 1, 2, 4, 3);
    /* 5->0->1->3->2->4 */
    SIX_COST_CHECK(5, 0, 1, 3, 2, 4);
    /* 5->0->1->3->4->2 */
    SIX_COST_CHECK(5, 0, 1, 3, 4, 2);
    /* 5->0->1->4->2->3 */
    SIX_COST_CHECK(5, 0, 1, 4, 2, 3);
    /* 5->0->1->4->3->2 */
    SIX_COST_CHECK(5, 0, 1, 4, 3, 2);
    /* 5->0->2->1->3->4 */
    SIX_COST_CHECK(5, 0, 2, 1, 3, 4);
    /* 5->0->2->1->4->3 */
    SIX_COST_CHECK(5, 0, 2, 1, 4, 3);
    /* 5->0->2->3->1->4 */
    SIX_COST_CHECK(5, 0, 2, 3, 1, 4);
    /* 5->0->2->3->4->1 */
    SIX_COST_CHECK(5, 0, 2, 3, 4, 1);
    /* 5->0->2->4->1->3 */
    SIX_COST_CHECK(5, 0, 2, 4, 1, 3);
    /* 5->0->2->4->3->1 */
    SIX_COST_CHECK(5, 0, 2, 4, 3, 1);
    /* 5->0->3->1->2->4 */
    SIX_COST_CHECK(5, 0, 3, 1, 2, 4);
    /* 5->0->3->1->4->2 */
    SIX_COST_CHECK(5, 0, 3, 1, 4, 2);
    /* 5->0->3->2->1->4 */
    SIX_COST_CHECK(5, 0, 3, 2, 1, 4);
    /* 5->0->3->2->4->1 */
    SIX_COST_CHECK(5, 0, 3, 2, 4, 1);
    /* 5->0->3->4->1->2 */
    SIX_COST_CHECK(5, 0, 3, 4, 1, 2);
    /* 5->0->3->4->2->1 */
    SIX_COST_CHECK(5, 0, 3, 4, 2, 1);
    /* 5->0->4->1->2->3 */
    SIX_COST_CHECK(5, 0, 4, 1, 2, 3);
    /* 5->0->4->1->3->2 */
    SIX_COST_CHECK(5, 0, 4, 1, 3, 2);
    /* 5->0->4->2->1->3 */
    SIX_COST_CHECK(5, 0, 4, 2, 1, 3);
    /* 5->0->4->2->3->1 */
    SIX_COST_CHECK(5, 0, 4, 2, 3, 1);
    /* 5->0->4->3->1->2 */
    SIX_COST_CHECK(5, 0, 4, 3, 1, 2);
    /* 5->0->4->3->2->1 */
    SIX_COST_CHECK(5, 0, 4, 3, 2, 1);
    /* 5->1->0->2->3->4 */
    SIX_COST_CHECK(5, 1, 0, 2, 3, 4);
    /* 5->1->0->2->4->3 */
    SIX_COST_CHECK(5, 1, 0, 2, 4, 3);
    /* 5->1->0->3->2->4 */
    SIX_COST_CHECK(5, 1, 0, 3, 2, 4);
    /* 5->1->0->3->4->2 */
    SIX_COST_CHECK(5, 1, 0, 3, 4, 2);
    /* 5->1->0->4->2->3 */
    SIX_COST_CHECK(5, 1, 0, 4, 2, 3);
    /* 5->1->0->4->3->2 */
    SIX_COST_CHECK(5, 1, 0, 4, 3, 2);
    /* 5->1->2->0->3->4 */
    SIX_COST_CHECK(5, 1, 2, 0, 3, 4);
    /* 5->1->2->0->4->3 */
    SIX_COST_CHECK(5, 1, 2, 0, 4, 3);
    /* 5->1->2->3->0->4 */
    SIX_COST_CHECK(5, 1, 2, 3, 0, 4);
    /* 5->1->2->3->4->0 */
    SIX_COST_CHECK(5, 1, 2, 3, 4, 0);
    /* 5->1->2->4->0->3 */
    SIX_COST_CHECK(5, 1, 2, 4, 0, 3);
    /* 5->1->2->4->3->0 */
    SIX_COST_CHECK(5, 1, 2, 4, 3, 0);
    /* 5->1->3->0->2->4 */
    SIX_COST_CHECK(5, 1, 3, 0, 2, 4);
    /* 5->1->3->0->4->2 */
    SIX_COST_CHECK(5, 1, 3, 0, 4, 2);
    /* 5->1->3->2->0->4 */
    SIX_COST_CHECK(5, 1, 3, 2, 0, 4);
    /* 5->1->3->2->4->0 */
    SIX_COST_CHECK(5, 1, 3, 2, 4, 0);
    /* 5->1->3->4->0->2 */
    SIX_COST_CHECK(5, 1, 3, 4, 0, 2);
    /* 5->1->3->4->2->0 */
    SIX_COST_CHECK(5, 1, 3, 4, 2, 0);
    /* 5->1->4->0->2->3 */
    SIX_COST_CHECK(5, 1, 4, 0, 2, 3);
    /* 5->1->4->0->3->2 */
    SIX_COST_CHECK(5, 1, 4, 0, 3, 2);
    /* 5->1->4->2->0->3 */
    SIX_COST_CHECK(5, 1, 4, 2, 0, 3);
    /* 5->1->4->2->3->0 */
    SIX_COST_CHECK(5, 1, 4, 2, 3, 0);
    /* 5->1->4->3->0->2 */
    SIX_COST_CHECK(5, 1, 4, 3, 0, 2);
    /* 5->1->4->3->2->0 */
    SIX_COST_CHECK(5, 1, 4, 3, 2, 0);
    /* 5->2->0->1->3->4 */
    SIX_COST_CHECK(5, 2, 0, 1, 3, 4);
    /* 5->2->0->1->4->3 */
    SIX_COST_CHECK(5, 2, 0, 1, 4, 3);
    /* 5->2->0->3->1->4 */
    SIX_COST_CHECK(5, 2, 0, 3, 1, 4);
    /* 5->2->0->3->4->1 */
    SIX_COST_CHECK(5, 2, 0, 3, 4, 1);
    /* 5->2->0->4->1->3 */
    SIX_COST_CHECK(5, 2, 0, 4, 1, 3);
    /* 5->2->0->4->3->1 */
    SIX_COST_CHECK(5, 2, 0, 4, 3, 1);
    /* 5->2->1->0->3->4 */
    SIX_COST_CHECK(5, 2, 1, 0, 3, 4);
    /* 5->2->1->0->4->3 */
    SIX_COST_CHECK(5, 2, 1, 0, 4, 3);
    /* 5->2->1->3->0->4 */
    SIX_COST_CHECK(5, 2, 1, 3, 0, 4);
    /* 5->2->1->3->4->0 */
    SIX_COST_CHECK(5, 2, 1, 3, 4, 0);
    /* 5->2->1->4->0->3 */
    SIX_COST_CHECK(5, 2, 1, 4, 0, 3);
    /* 5->2->1->4->3->0 */
    SIX_COST_CHECK(5, 2, 1, 4, 3, 0);
    /* 5->2->3->0->1->4 */
    SIX_COST_CHECK(5, 2, 3, 0, 1, 4);
    /* 5->2->3->0->4->1 */
    SIX_COST_CHECK(5, 2, 3, 0, 4, 1);
    /* 5->2->3->1->0->4 */
    SIX_COST_CHECK(5, 2, 3, 1, 0, 4);
    /* 5->2->3->1->4->0 */
    SIX_COST_CHECK(5, 2, 3, 1, 4, 0);
    /* 5->2->3->4->0->1 */
    SIX_COST_CHECK(5, 2, 3, 4, 0, 1);
    /* 5->2->3->4->1->0 */
    SIX_COST_CHECK(5, 2, 3, 4, 1, 0);
    /* 5->2->4->0->1->3 */
    SIX_COST_CHECK(5, 2, 4, 0, 1, 3);
    /* 5->2->4->0->3->1 */
    SIX_COST_CHECK(5, 2, 4, 0, 3, 1);
    /* 5->2->4->1->0->3 */
    SIX_COST_CHECK(5, 2, 4, 1, 0, 3);
    /* 5->2->4->1->3->0 */
    SIX_COST_CHECK(5, 2, 4, 1, 3, 0);
    /* 5->2->4->3->0->1 */
    SIX_COST_CHECK(5, 2, 4, 3, 0, 1);
    /* 5->2->4->3->1->0 */
    SIX_COST_CHECK(5, 2, 4, 3, 1, 0);
    /* 5->3->0->1->2->4 */
    SIX_COST_CHECK(5, 3, 0, 1, 2, 4);
    /* 5->3->0->1->4->2 */
    SIX_COST_CHECK(5, 3, 0, 1, 4, 2);
    /* 5->3->0->2->1->4 */
    SIX_COST_CHECK(5, 3, 0, 2, 1, 4);
    /* 5->3->0->2->4->1 */
    SIX_COST_CHECK(5, 3, 0, 2, 4, 1);
    /* 5->3->0->4->1->2 */
    SIX_COST_CHECK(5, 3, 0, 4, 1, 2);
    /* 5->3->0->4->2->1 */
    SIX_COST_CHECK(5, 3, 0, 4, 2, 1);
    /* 5->3->1->0->2->4 */
    SIX_COST_CHECK(5, 3, 1, 0, 2, 4);
    /* 5->3->1->0->4->2 */
    SIX_COST_CHECK(5, 3, 1, 0, 4, 2);
    /* 5->3->1->2->0->4 */
    SIX_COST_CHECK(5, 3, 1, 2, 0, 4);
    /* 5->3->1->2->4->0 */
    SIX_COST_CHECK(5, 3, 1, 2, 4, 0);
    /* 5->3->1->4->0->2 */
    SIX_COST_CHECK(5, 3, 1, 4, 0, 2);
    /* 5->3->1->4->2->0 */
    SIX_COST_CHECK(5, 3, 1, 4, 2, 0);
    /* 5->3->2->0->1->4 */
    SIX_COST_CHECK(5, 3, 2, 0, 1, 4);
    /* 5->3->2->0->4->1 */
    SIX_COST_CHECK(5, 3, 2, 0, 4, 1);
    /* 5->3->2->1->0->4 */
    SIX_COST_CHECK(5, 3, 2, 1, 0, 4);
    /* 5->3->2->1->4->0 */
    SIX_COST_CHECK(5, 3, 2, 1, 4, 0);
    /* 5->3->2->4->0->1 */
    SIX_COST_CHECK(5, 3, 2, 4, 0, 1);
    /* 5->3->2->4->1->0 */
    SIX_COST_CHECK(5, 3, 2, 4, 1, 0);
    /* 5->3->4->0->1->2 */
    SIX_COST_CHECK(5, 3, 4, 0, 1, 2);
    /* 5->3->4->0->2->1 */
    SIX_COST_CHECK(5, 3, 4, 0, 2, 1);
    /* 5->3->4->1->0->2 */
    SIX_COST_CHECK(5, 3, 4, 1, 0, 2);
    /* 5->3->4->1->2->0 */
    SIX_COST_CHECK(5, 3, 4, 1, 2, 0);
    /* 5->3->4->2->0->1 */
    SIX_COST_CHECK(5, 3, 4, 2, 0, 1);
    /* 5->3->4->2->1->0 */
    SIX_COST_CHECK(5, 3, 4, 2, 1, 0);
    /* 5->4->0->1->2->3 */
    SIX_COST_CHECK(5, 4, 0, 1, 2, 3);
    /* 5->4->0->1->3->2 */
    SIX_COST_CHECK(5, 4, 0, 1, 3, 2);
    /* 5->4->0->2->1->3 */
    SIX_COST_CHECK(5, 4, 0, 2, 1, 3);
    /* 5->4->0->2->3->1 */
    SIX_COST_CHECK(5, 4, 0, 2, 3, 1);
    /* 5->4->0->3->1->2 */
    SIX_COST_CHECK(5, 4, 0, 3, 1, 2);
    /* 5->4->0->3->2->1 */
    SIX_COST_CHECK(5, 4, 0, 3, 2, 1);
    /* 5->4->1->0->2->3 */
    SIX_COST_CHECK(5, 4, 1, 0, 2, 3);
    /* 5->4->1->0->3->2 */
    SIX_COST_CHECK(5, 4, 1, 0, 3, 2);
    /* 5->4->1->2->0->3 */
    SIX_COST_CHECK(5, 4, 1, 2, 0, 3);
    /* 5->4->1->2->3->0 */
    SIX_COST_CHECK(5, 4, 1, 2, 3, 0);
    /* 5->4->1->3->0->2 */
    SIX_COST_CHECK(5, 4, 1, 3, 0, 2);
    /* 5->4->1->3->2->0 */
    SIX_COST_CHECK(5, 4, 1, 3, 2, 0);
    /* 5->4->2->0->1->3 */
    SIX_COST_CHECK(5, 4, 2, 0, 1, 3);
    /* 5->4->2->0->3->1 */
    SIX_COST_CHECK(5, 4, 2, 0, 3, 1);
    /* 5->4->2->1->0->3 */
    SIX_COST_CHECK(5, 4, 2, 1, 0, 3);
    /* 5->4->2->1->3->0 */
    SIX_COST_CHECK(5, 4, 2, 1, 3, 0);
    /* 5->4->2->3->0->1 */
    SIX_COST_CHECK(5, 4, 2, 3, 0, 1);
    /* 5->4->2->3->1->0 */
    SIX_COST_CHECK(5, 4, 2, 3, 1, 0);
    /* 5->4->3->0->1->2 */
    SIX_COST_CHECK(5, 4, 3, 0, 1, 2);
    /* 5->4->3->0->2->1 */
    SIX_COST_CHECK(5, 4, 3, 0, 2, 1);
    /* 5->4->3->1->0->2 */
    SIX_COST_CHECK(5, 4, 3, 1, 0, 2);
    /* 5->4->3->1->2->0 */
    SIX_COST_CHECK(5, 4, 3, 1, 2, 0);
    /* 5->4->3->2->0->1 */
    SIX_COST_CHECK(5, 4, 3, 2, 0, 1);
    /* 5->4->3->2->1->0 */
    SIX_COST_CHECK(5, 4, 3, 2, 1, 0);
  } else {
    /* 5->0->1->2->3->4 */
    SIX_COST_STRICT_CHECK(5, 0, 1, 2, 3, 4);
    /* 5->0->1->2->4->3 */
    SIX_COST_STRICT_CHECK(5, 0, 1, 2, 4, 3);
    /* 5->0->1->3->2->4 */
    SIX_COST_STRICT_CHECK(5, 0, 1, 3, 2, 4);
    /* 5->0->1->3->4->2 */
    SIX_COST_STRICT_CHECK(5, 0, 1, 3, 4, 2);
    /* 5->0->1->4->2->3 */
    SIX_COST_STRICT_CHECK(5, 0, 1, 4, 2, 3);
    /* 5->0->1->4->3->2 */
    SIX_COST_STRICT_CHECK(5, 0, 1, 4, 3, 2);
    /* 5->0->2->1->3->4 */
    SIX_COST_STRICT_CHECK(5, 0, 2, 1, 3, 4);
    /* 5->0->2->1->4->3 */
    SIX_COST_STRICT_CHECK(5, 0, 2, 1, 4, 3);
    /* 5->0->2->3->1->4 */
    SIX_COST_STRICT_CHECK(5, 0, 2, 3, 1, 4);
    /* 5->0->2->3->4->1 */
    SIX_COST_STRICT_CHECK(5, 0, 2, 3, 4, 1);
    /* 5->0->2->4->1->3 */
    SIX_COST_STRICT_CHECK(5, 0, 2, 4, 1, 3);
    /* 5->0->2->4->3->1 */
    SIX_COST_STRICT_CHECK(5, 0, 2, 4, 3, 1);
    /* 5->0->3->1->2->4 */
    SIX_COST_STRICT_CHECK(5, 0, 3, 1, 2, 4);
    /* 5->0->3->1->4->2 */
    SIX_COST_STRICT_CHECK(5, 0, 3, 1, 4, 2);
    /* 5->0->3->2->1->4 */
    SIX_COST_STRICT_CHECK(5, 0, 3, 2, 1, 4);
    /* 5->0->3->2->4->1 */
    SIX_COST_STRICT_CHECK(5, 0, 3, 2, 4, 1);
    /* 5->0->3->4->1->2 */
    SIX_COST_STRICT_CHECK(5, 0, 3, 4, 1, 2);
    /* 5->0->3->4->2->1 */
    SIX_COST_STRICT_CHECK(5, 0, 3, 4, 2, 1);
    /* 5->0->4->1->2->3 */
    SIX_COST_STRICT_CHECK(5, 0, 4, 1, 2, 3);
    /* 5->0->4->1->3->2 */
    SIX_COST_STRICT_CHECK(5, 0, 4, 1, 3, 2);
    /* 5->0->4->2->1->3 */
    SIX_COST_STRICT_CHECK(5, 0, 4, 2, 1, 3);
    /* 5->0->4->2->3->1 */
    SIX_COST_STRICT_CHECK(5, 0, 4, 2, 3, 1);
    /* 5->0->4->3->1->2 */
    SIX_COST_STRICT_CHECK(5, 0, 4, 3, 1, 2);
    /* 5->0->4->3->2->1 */
    SIX_COST_STRICT_CHECK(5, 0, 4, 3, 2, 1);
    /* 5->1->0->2->3->4 */
    SIX_COST_STRICT_CHECK(5, 1, 0, 2, 3, 4);
    /* 5->1->0->2->4->3 */
    SIX_COST_STRICT_CHECK(5, 1, 0, 2, 4, 3);
    /* 5->1->0->3->2->4 */
    SIX_COST_STRICT_CHECK(5, 1, 0, 3, 2, 4);
    /* 5->1->0->3->4->2 */
    SIX_COST_STRICT_CHECK(5, 1, 0, 3, 4, 2);
    /* 5->1->0->4->2->3 */
    SIX_COST_STRICT_CHECK(5, 1, 0, 4, 2, 3);
    /* 5->1->0->4->3->2 */
    SIX_COST_STRICT_CHECK(5, 1, 0, 4, 3, 2);
    /* 5->1->2->0->3->4 */
    SIX_COST_STRICT_CHECK(5, 1, 2, 0, 3, 4);
    /* 5->1->2->0->4->3 */
    SIX_COST_STRICT_CHECK(5, 1, 2, 0, 4, 3);
    /* 5->1->2->3->0->4 */
    SIX_COST_STRICT_CHECK(5, 1, 2, 3, 0, 4);
    /* 5->1->2->3->4->0 */
    SIX_COST_STRICT_CHECK(5, 1, 2, 3, 4, 0);
    /* 5->1->2->4->0->3 */
    SIX_COST_STRICT_CHECK(5, 1, 2, 4, 0, 3);
    /* 5->1->2->4->3->0 */
    SIX_COST_STRICT_CHECK(5, 1, 2, 4, 3, 0);
    /* 5->1->3->0->2->4 */
    SIX_COST_STRICT_CHECK(5, 1, 3, 0, 2, 4);
    /* 5->1->3->0->4->2 */
    SIX_COST_STRICT_CHECK(5, 1, 3, 0, 4, 2);
    /* 5->1->3->2->0->4 */
    SIX_COST_STRICT_CHECK(5, 1, 3, 2, 0, 4);
    /* 5->1->3->2->4->0 */
    SIX_COST_STRICT_CHECK(5, 1, 3, 2, 4, 0);
    /* 5->1->3->4->0->2 */
    SIX_COST_STRICT_CHECK(5, 1, 3, 4, 0, 2);
    /* 5->1->3->4->2->0 */
    SIX_COST_STRICT_CHECK(5, 1, 3, 4, 2, 0);
    /* 5->1->4->0->2->3 */
    SIX_COST_STRICT_CHECK(5, 1, 4, 0, 2, 3);
    /* 5->1->4->0->3->2 */
    SIX_COST_STRICT_CHECK(5, 1, 4, 0, 3, 2);
    /* 5->1->4->2->0->3 */
    SIX_COST_STRICT_CHECK(5, 1, 4, 2, 0, 3);
    /* 5->1->4->2->3->0 */
    SIX_COST_STRICT_CHECK(5, 1, 4, 2, 3, 0);
    /* 5->1->4->3->0->2 */
    SIX_COST_STRICT_CHECK(5, 1, 4, 3, 0, 2);
    /* 5->1->4->3->2->0 */
    SIX_COST_STRICT_CHECK(5, 1, 4, 3, 2, 0);
    /* 5->2->0->1->3->4 */
    SIX_COST_STRICT_CHECK(5, 2, 0, 1, 3, 4);
    /* 5->2->0->1->4->3 */
    SIX_COST_STRICT_CHECK(5, 2, 0, 1, 4, 3);
    /* 5->2->0->3->1->4 */
    SIX_COST_STRICT_CHECK(5, 2, 0, 3, 1, 4);
    /* 5->2->0->3->4->1 */
    SIX_COST_STRICT_CHECK(5, 2, 0, 3, 4, 1);
    /* 5->2->0->4->1->3 */
    SIX_COST_STRICT_CHECK(5, 2, 0, 4, 1, 3);
    /* 5->2->0->4->3->1 */
    SIX_COST_STRICT_CHECK(5, 2, 0, 4, 3, 1);
    /* 5->2->1->0->3->4 */
    SIX_COST_STRICT_CHECK(5, 2, 1, 0, 3, 4);
    /* 5->2->1->0->4->3 */
    SIX_COST_STRICT_CHECK(5, 2, 1, 0, 4, 3);
    /* 5->2->1->3->0->4 */
    SIX_COST_STRICT_CHECK(5, 2, 1, 3, 0, 4);
    /* 5->2->1->3->4->0 */
    SIX_COST_STRICT_CHECK(5, 2, 1, 3, 4, 0);
    /* 5->2->1->4->0->3 */
    SIX_COST_STRICT_CHECK(5, 2, 1, 4, 0, 3);
    /* 5->2->1->4->3->0 */
    SIX_COST_STRICT_CHECK(5, 2, 1, 4, 3, 0);
    /* 5->2->3->0->1->4 */
    SIX_COST_STRICT_CHECK(5, 2, 3, 0, 1, 4);
    /* 5->2->3->0->4->1 */
    SIX_COST_STRICT_CHECK(5, 2, 3, 0, 4, 1);
    /* 5->2->3->1->0->4 */
    SIX_COST_STRICT_CHECK(5, 2, 3, 1, 0, 4);
    /* 5->2->3->1->4->0 */
    SIX_COST_STRICT_CHECK(5, 2, 3, 1, 4, 0);
    /* 5->2->3->4->0->1 */
    SIX_COST_STRICT_CHECK(5, 2, 3, 4, 0, 1);
    /* 5->2->3->4->1->0 */
    SIX_COST_STRICT_CHECK(5, 2, 3, 4, 1, 0);
    /* 5->2->4->0->1->3 */
    SIX_COST_STRICT_CHECK(5, 2, 4, 0, 1, 3);
    /* 5->2->4->0->3->1 */
    SIX_COST_STRICT_CHECK(5, 2, 4, 0, 3, 1);
    /* 5->2->4->1->0->3 */
    SIX_COST_STRICT_CHECK(5, 2, 4, 1, 0, 3);
    /* 5->2->4->1->3->0 */
    SIX_COST_STRICT_CHECK(5, 2, 4, 1, 3, 0);
    /* 5->2->4->3->0->1 */
    SIX_COST_STRICT_CHECK(5, 2, 4, 3, 0, 1);
    /* 5->2->4->3->1->0 */
    SIX_COST_STRICT_CHECK(5, 2, 4, 3, 1, 0);
    /* 5->3->0->1->2->4 */
    SIX_COST_STRICT_CHECK(5, 3, 0, 1, 2, 4);
    /* 5->3->0->1->4->2 */
    SIX_COST_STRICT_CHECK(5, 3, 0, 1, 4, 2);
    /* 5->3->0->2->1->4 */
    SIX_COST_STRICT_CHECK(5, 3, 0, 2, 1, 4);
    /* 5->3->0->2->4->1 */
    SIX_COST_STRICT_CHECK(5, 3, 0, 2, 4, 1);
    /* 5->3->0->4->1->2 */
    SIX_COST_STRICT_CHECK(5, 3, 0, 4, 1, 2);
    /* 5->3->0->4->2->1 */
    SIX_COST_STRICT_CHECK(5, 3, 0, 4, 2, 1);
    /* 5->3->1->0->2->4 */
    SIX_COST_STRICT_CHECK(5, 3, 1, 0, 2, 4);
    /* 5->3->1->0->4->2 */
    SIX_COST_STRICT_CHECK(5, 3, 1, 0, 4, 2);
    /* 5->3->1->2->0->4 */
    SIX_COST_STRICT_CHECK(5, 3, 1, 2, 0, 4);
    /* 5->3->1->2->4->0 */
    SIX_COST_STRICT_CHECK(5, 3, 1, 2, 4, 0);
    /* 5->3->1->4->0->2 */
    SIX_COST_STRICT_CHECK(5, 3, 1, 4, 0, 2);
    /* 5->3->1->4->2->0 */
    SIX_COST_STRICT_CHECK(5, 3, 1, 4, 2, 0);
    /* 5->3->2->0->1->4 */
    SIX_COST_STRICT_CHECK(5, 3, 2, 0, 1, 4);
    /* 5->3->2->0->4->1 */
    SIX_COST_STRICT_CHECK(5, 3, 2, 0, 4, 1);
    /* 5->3->2->1->0->4 */
    SIX_COST_STRICT_CHECK(5, 3, 2, 1, 0, 4);
    /* 5->3->2->1->4->0 */
    SIX_COST_STRICT_CHECK(5, 3, 2, 1, 4, 0);
    /* 5->3->2->4->0->1 */
    SIX_COST_STRICT_CHECK(5, 3, 2, 4, 0, 1);
    /* 5->3->2->4->1->0 */
    SIX_COST_STRICT_CHECK(5, 3, 2, 4, 1, 0);
    /* 5->3->4->0->1->2 */
    SIX_COST_STRICT_CHECK(5, 3, 4, 0, 1, 2);
    /* 5->3->4->0->2->1 */
    SIX_COST_STRICT_CHECK(5, 3, 4, 0, 2, 1);
    /* 5->3->4->1->0->2 */
    SIX_COST_STRICT_CHECK(5, 3, 4, 1, 0, 2);
    /* 5->3->4->1->2->0 */
    SIX_COST_STRICT_CHECK(5, 3, 4, 1, 2, 0);
    /* 5->3->4->2->0->1 */
    SIX_COST_STRICT_CHECK(5, 3, 4, 2, 0, 1);
    /* 5->3->4->2->1->0 */
    SIX_COST_STRICT_CHECK(5, 3, 4, 2, 1, 0);
    /* 5->4->0->1->2->3 */
    SIX_COST_STRICT_CHECK(5, 4, 0, 1, 2, 3);
    /* 5->4->0->1->3->2 */
    SIX_COST_STRICT_CHECK(5, 4, 0, 1, 3, 2);
    /* 5->4->0->2->1->3 */
    SIX_COST_STRICT_CHECK(5, 4, 0, 2, 1, 3);
    /* 5->4->0->2->3->1 */
    SIX_COST_STRICT_CHECK(5, 4, 0, 2, 3, 1);
    /* 5->4->0->3->1->2 */
    SIX_COST_STRICT_CHECK(5, 4, 0, 3, 1, 2);
    /* 5->4->0->3->2->1 */
    SIX_COST_STRICT_CHECK(5, 4, 0, 3, 2, 1);
    /* 5->4->1->0->2->3 */
    SIX_COST_STRICT_CHECK(5, 4, 1, 0, 2, 3);
    /* 5->4->1->0->3->2 */
    SIX_COST_STRICT_CHECK(5, 4, 1, 0, 3, 2);
    /* 5->4->1->2->0->3 */
    SIX_COST_STRICT_CHECK(5, 4, 1, 2, 0, 3);
    /* 5->4->1->2->3->0 */
    SIX_COST_STRICT_CHECK(5, 4, 1, 2, 3, 0);
    /* 5->4->1->3->0->2 */
    SIX_COST_STRICT_CHECK(5, 4, 1, 3, 0, 2);
    /* 5->4->1->3->2->0 */
    SIX_COST_STRICT_CHECK(5, 4, 1, 3, 2, 0);
    /* 5->4->2->0->1->3 */
    SIX_COST_STRICT_CHECK(5, 4, 2, 0, 1, 3);
    /* 5->4->2->0->3->1 */
    SIX_COST_STRICT_CHECK(5, 4, 2, 0, 3, 1);
    /* 5->4->2->1->0->3 */
    SIX_COST_STRICT_CHECK(5, 4, 2, 1, 0, 3);
    /* 5->4->2->1->3->0 */
    SIX_COST_STRICT_CHECK(5, 4, 2, 1, 3, 0);
    /* 5->4->2->3->0->1 */
    SIX_COST_STRICT_CHECK(5, 4, 2, 3, 0, 1);
    /* 5->4->2->3->1->0 */
    SIX_COST_STRICT_CHECK(5, 4, 2, 3, 1, 0);
    /* 5->4->3->0->1->2 */
    SIX_COST_STRICT_CHECK(5, 4, 3, 0, 1, 2);
    /* 5->4->3->0->2->1 */
    SIX_COST_STRICT_CHECK(5, 4, 3, 0, 2, 1);
    /* 5->4->3->1->0->2 */
    SIX_COST_STRICT_CHECK(5, 4, 3, 1, 0, 2);
    /* 5->4->3->1->2->0 */
    SIX_COST_STRICT_CHECK(5, 4, 3, 1, 2, 0);
    /* 5->4->3->2->0->1 */
    SIX_COST_STRICT_CHECK(5, 4, 3, 2, 0, 1);
    /* 5->4->3->2->1->0 */
    SIX_COST_STRICT_CHECK(5, 4, 3, 2, 1, 0);
  }

  return(0);
}
