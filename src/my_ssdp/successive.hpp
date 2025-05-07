#pragma once
#include "models.hpp"

#define THREE_COST(j0, j1, j2)					\
  prob->sjob[j[j0]]->f[s + prob->sjob[j[j0]]->p]		\
  + prob->sjob[j[j1]]->f[c - prob->sjob[j[j2]]->p]		\
  + prob->sjob[j[j2]]->f[c]

#define THREE_COST_CHECK(j0, j1, j2) {					\
    g = THREE_COST(j0, j1, j2);						\
    if (g < f	|| (g == f						\
	        && prob->sjob[j[j0]]->tno < prob->sjob[j[0]]->tno)) {	\
      return 1;							\
    }									\
  }

char check_three_cost(int *j, int s, int c) {
  int f, g;

  /* 0->1->2 */
  f = THREE_COST(0, 1, 2);
  /* 1->2->0 */
  THREE_COST_CHECK(1, 2, 0);
  /* 2->0->1 */
  THREE_COST_CHECK(2, 0, 1);
  /* 2->1->0 */
  THREE_COST_CHECK(2, 1, 0);

  return 0;
}

#define FOUR_COST(j0, j1, j2, j3)					\
  prob->sjob[j[j0]]->f[s + prob->sjob[j[j0]]->p]			\
  + prob->sjob[j[j1]]->f[s + prob->sjob[j[j0]]->p + prob->sjob[j[j1]]->p] \
  + prob->sjob[j[j2]]->f[c - prob->sjob[j[j3]]->p]			\
  + prob->sjob[j[j3]]->f[c]

#define FOUR_COST_CHECK1(j0, j1, j2, j3) {				\
    g = FOUR_COST(j0, j1, j2, j3);					\
    if (g < f	|| (g == f						\
	   && prob->sjob[j[j1]]->tno < prob->sjob[j[1]]->tno)) {	\
      return(1);							\
    }									\
  }
#define FOUR_COST_CHECK(j0, j1, j2, j3) {		\
    g = FOUR_COST(j0, j1, j2, j3);			\
    return g <= f;			\
  }
#define FOUR_COST_STRICT_CHECK(j0, j1, j2, j3) {	\
    g = FOUR_COST(j0, j1, j2, j3);			\
    return g < f;				\
  }

char check_four_cost_forward(int *j, int s, int c) {
  int f, g;

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

char check_four_cost_backward(int *j, int s, int c)
{
  int f, g;

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

#define FIVE_COST(j0, j1, j2, j3, j4)					\
  prob->sjob[j[j0]]->f[s + prob->sjob[j[j0]]->p]			\
  + prob->sjob[j[j1]]->f[s + prob->sjob[j[j0]]->p + prob->sjob[j[j1]]->p] \
  + prob->sjob[j[j2]]->f[c - prob->sjob[j[j3]]->p - prob->sjob[j[j4]]->p] \
  + prob->sjob[j[j3]]->f[c - prob->sjob[j[j4]]->p]			\
  + prob->sjob[j[j4]]->f[c]

#define FIVE_COST_CHECK2(j0, j1, j2, j3, j4) {				\
    g = FIVE_COST(j0, j1, j2, j3, j4);					\
    if (g < f							\
       || (g==f						\
	   && prob->sjob[j[j2]]->tno < prob->sjob[j[2]]->tno)) {	\
      return(1);							\
    }									\
  }
#define FIVE_COST_CHECK(j0, j1, j2, j3, j4) {		\
    g = FIVE_COST(j0, j1, j2, j3, j4);			\
    return g <= f;		\
  }
#define FIVE_COST_STRICT_CHECK(j0, j1, j2, j3, j4) {	\
    g = FIVE_COST(j0, j1, j2, j3, j4);			\
    return g < f;				\
  }

char check_five_cost_forward(int *j, int s, int c) {
  int f, g;
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

char check_five_cost_backward(int *j, int s, int c) {
  int f, g;

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

#define SIX_COST(j0, j1, j2, j3, j4, j5)				\
  prob->sjob[j[j0]]->f[s + prob->sjob[j[j0]]->p]			 \
  + prob->sjob[j[j1]]->f[s + prob->sjob[j[j0]]->p + prob->sjob[j[j1]]->p] \
  + prob->sjob[j[j2]]->f[s + prob->sjob[j[j0]]->p + prob->sjob[j[j1]]->p \
			 + prob->sjob[j[j2]]->p]			\
  + prob->sjob[j[j3]]->f[c - prob->sjob[j[j4]]->p - prob->sjob[j[j5]]->p] \
  + prob->sjob[j[j4]]->f[c - prob->sjob[j[j5]]->p]			 \
  + prob->sjob[j[j5]]->f[c]

#define SIX_COST_CHECK3(j0, j1, j2, j3, j4, j5) {			\
    g = SIX_COST(j0, j1, j2, j3, j4, j5);				\
    if (g < f							\
       || (g==f						\
	   && prob->sjob[j[j3]]->tno < prob->sjob[j[3]]->tno)) {	\
      return(1);							\
    }									\
  }
#define SIX_COST_CHECK(j0, j1, j2, j3, j4, j5) {			\
    g = SIX_COST(j0, j1, j2, j3, j4, j5);				\
    return g <= f;					\
  }
#define SIX_COST_STRICT_CHECK(j0, j1, j2, j3, j4, j5) {		\
    g = SIX_COST(j0, j1, j2, j3, j4, j5);			\
    return g < f;					\
  }

char check_six_cost_forward(int *j, int s, int c) {
  int f, g;
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

char check_six_cost_backward(int *j, int s, int c)
{
  int f, g;
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
  
char _check_three_forward(int c, int jn, _node2m_t *n) {
  int j[3];
  if((j[1] = n->j[0]) == prob->n) return 0;
  j[2] = jn;
  int s = c - prob->sjob[j[2]]->p - prob->sjob[j[1]]->p;
  for (auto e = n->e; e; e = e->next) {
    if (e->n->ty & 4) continue;
    if ((j[0] = e->n->j[0]) == prob->n) {
      return 0;
    } else if (j[0] == j[2]) {
      continue;
    }
    if (!check_three_cost(j, s - prob->sjob[j[0]]->p, c)) {
      return 0;
    }
  }  
  return 1;
}
  
  char _check_four_forward(int c, int jn, _node2m_t *n)
  {
    int s;
    int j[4];
    _edge2m_t *e, *e2;
  
    if((j[2] = n->j[0]) == prob->n) return 0;
  
    j[3] = jn;
    for(e = n->e; e; e = e->next) {
          if (e->n->ty & 4) continue;
      if((j[1] = e->n->j[0]) == prob->n) {
        return 0;
      } else if(j[1] == j[3]) {
        continue;
      }
  
      s = c - prob->sjob[j[3]]->p - prob->sjob[j[2]]->p - prob->sjob[j[1]]->p;
      if (!check_three_cost(j + 1, s, c)) {
        for(e2 = e->n->e; e2; e2 = e2->next) {
          if (e2->n->ty & 4) continue;
          if((j[0] = e2->n->j[0]) == prob->n) {
            return 0;
          } else if (j[0] == j[3] || j[0] == j[2]) {
            continue;
          }
          if(!check_four_cost_forward(j, s - prob->sjob[j[0]]->p, c)) {
            return 0;
          }
        }
      }
    }
  
    return(1);
  }
  
  char _check_five_forward(int c, int jn, _node2m_t *n) {
    int s1, s2;
    int j[5];
    _edge2m_t *e, *e2, *e3;
  
    if((j[3] = n->j[0]) == prob->n) return 0;
  
    j[4] = jn;
    for(e = n->e; e; e = e->next) {
      if (e->n->ty & 4) continue;
      if((j[2] = e->n->j[0]) == prob->n) return 0;
      else if(j[2] == j[4]) continue;
  
      s1 = c - prob->sjob[j[4]]->p - prob->sjob[j[3]]->p - prob->sjob[j[2]]->p;
      if (!check_three_cost(j + 2, s1, c)) {
        for(e2 = e->n->e; e2; e2 = e2->next) {
          if (e2->n->ty & 4) continue;
          if((j[1] = e2->n->j[0]) == prob->n) {
              return 0;
          } else if (j[1] == j[4] || j[1] == j[3]) {
              continue;
          }
  
      s2 = s1 - prob->sjob[j[1]]->p;
      if (!check_four_cost_forward(j + 1, s2, c)) {
          for(e3 = e2->n->e; e3; e3 = e3->next) {
              if (e3->n->ty & 4) continue;
              if((j[0] = e3->n->j[0]) == prob->n) {
                  return 0;
              } else if(j[0] == j[4] || j[0] == j[3] || j[0] == j[2]) {
                  continue;
              }
              if(!check_five_cost_forward(j, s2 - prob->sjob[j[0]]->p, c)) {
                  return 0;
              }
        }
      }
        }
      }
    }
    return 1;
  }
  
  char _check_six_forward(int c, int jn, _node2m_t *n) {
    int s1, s2, s3;
    int j[6];
    _edge2m_t *e, *e2, *e3, *e4;
  
    if((j[4] = n->j[0]) == prob->n) return 0;
    j[5] = jn;
    for(e = n->e; e; e = e->next) {
      if (e->n->ty & 4) continue;
      if((j[3] = e->n->j[0]) == prob->n) {
        return 0;
      } else if (j[3] == j[5]) {
        continue;
      }
  
      s1 = c - prob->sjob[j[5]]->p - prob->sjob[j[4]]->p - prob->sjob[j[3]]->p;
      if(!check_three_cost(j + 3, s1, c)) {
          for(e2 = e->n->e; e2; e2 = e2->next) {
              if (e2->n->ty & 4) continue;
              if((j[2] = e2->n->j[0]) == prob->n) {
                return 0;
              } else if (j[2] == j[5] || j[2] == j[4]) {
                continue;
              }
  
              s2 = s1 - prob->sjob[j[2]]->p;
              if (!check_four_cost_forward(j + 2, s2, c)) {
                for (e3 = e2->n->e; e3; e3 = e3->next) {
                  if (e3->n->ty & 4) continue;
                  if((j[1] = e3->n->j[0]) == prob->n) {
                    return 0;
                  } else if (j[1] == j[5] || j[1] == j[4] || j[1] == j[3]) {
                    continue;
                  }
  
          s3 = s2 - prob->sjob[j[1]]->p;
          if(!check_five_cost_forward(j + 1, s3, c)) {
            for(e4 = e3->n->e; e4; e4 = e4->next) {
          if (e4->n->ty & 4) continue;
          if((j[0] = e4->n->j[0]) == prob->n) {
            return 0;
          } else if(j[0] == j[5] || j[0] == j[4] || j[0] == j[3]
                    || j[0] == j[2]) {
            continue;
          }
          if(!check_six_cost_forward(j, s3 - prob->sjob[j[0]]->p, c)) {
            return 0;
          }
            }
          }
        }
      }
        }
      }
    }
    return 1;
  }
  
  