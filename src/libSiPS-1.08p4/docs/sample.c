/* Please define the following before reading "sips.h" */
/* if they are defined when compiling the library */
/* #define COST_REAL */
/* #define COST_LONGLONG */
/* #define REAL_LONG */

/* header file */
#ifndef SIPSI /* for SiPS */
#include "sips.h"
#else /* for SiPSi */
#include "sipsi.h"
#endif

cost_t f(int, int);

int main(void)
{
  int ret;
  /* create the problem */
  sips *prob = SiPS_create_problem();

 /* specify job data */
#ifndef SIPSI /* for SiPS */
  /*                      No.  name   p   d  ew  tw */
  SiPS_TWET_set_job(prob,   0, "J1", 10, 10,  3,  1);
  SiPS_TWET_set_job(prob,   1, "J2", 20, 55,  2,  2);
  SiPS_TWET_set_job(prob,   2, "J3", 30, 40,  1,  3);
#else /* for SiPSi */
  /*                      No.  name   p   r   d  ew  tw */
  SiPS_TWET_set_job(prob,   0, "J1", 10,  0, 10,  3,  1);
  SiPS_TWET_set_job(prob,   1, "J2", 20,  5, 55,  2,  2);
  SiPS_TWET_set_job(prob,   2, "J3", 30, 10, 40,  1,  3);
#endif
  /* read from file */
  /* SiPS_read_problem(prob, "sample.dat"); */

  /* specify cost function */
  /* SiPS_set_cost_function(prob, f); */
  /* specify scheduling horizon (SiPSi) */
  /* SiPS_set_horizon(prob, 55 + 10 + 20 + 30); */

  /* solve the problem */
  ret = SiPS_solve(prob);

  /* retrieve the solution */
  /* if(ret == SIPS_SOLVED) { */
  /*   int job[3], C[3]; */
  /*   cost_t f; */
  /*   SiPS_get_solution(prob, &f, job, C); */
  /* } */

  /* just display the solution */
  if(ret == SIPS_SOLVED) {
    SiPS_print_solution(prob);
  }

  /* release the problem  */
  SiPS_free_problem(prob);

  return(ret);
}

#define max(a, b) (((a)>(b))?(a):(b))

cost_t f(int i, int t)
{
  cost_t v;

  switch (i) {
  default:
  case 0:
    v = 3*max(10-t, 0) + 1*max(t-10, 0);
    break;
  case 1:
    v = 2*max(55-t, 0) + 2*max(t-55, 0);
    break;
  case 2:
    v = 1*max(40-t, 0) + 3*max(t-40, 0);
    break;
  }

  return(v);
}
