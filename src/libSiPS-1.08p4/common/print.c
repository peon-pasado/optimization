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
 *  $Id: print.c,v 1.13 2013/05/28 13:27:21 tanaka Rel $
 *  $Revision: 1.13 $
 *  $Date: 2013/05/28 13:27:21 $
 *  $Author: tanaka $
 *
 */
#include <stdio.h>

#include "define.h"
#include "sips_common.h"
#include "timer.h"
#include "print.h"

/*
 * SiPS_print_jobs(prob)
 *   prints out the job data.
 *
 */
int SiPS_print_jobs(sips *prob)
{
  if(prob != NULL) {
    return(print_jobs(prob, stdout));
  } else {
    return(SIPS_FAIL);
  }
}

/*
 * SiPS_print_solution(prob)
 *   prints out the solution.
 *
 */
int SiPS_print_solution(sips *prob)
{
  if(prob != NULL && prob->sol != NULL) {
    return(print_sol(prob, prob->sol, stdout));
  } else {
    return(SIPS_FAIL);
  }
}

/*
 * print_jobs(fp, prob)
 *   prints out the job data.
 *       fp: file pointer
 *
 */
int print_jobs(sips *prob, FILE *fp)
{
  int i;

  if(prob->n == 0) {
    return(SIPS_OK);
  }

#ifdef SIPSI 
  fprintf(fp, "(N, P, R, D, EW, TW)=");
#else /* SIPSI */
  fprintf(fp, "(N, P, D, EW, TW)=");
#endif /* SIPSI */

  for(i = 0; i < prob->n; i++) {
    if(prob->job[i].name == NULL) {
      fprintf(fp, "(%d,", prob->job[i].rno);
    } else {
      fprintf(fp, "(%s,", prob->job[i].name);
    }
#ifdef SIPSI 
    fprintf(fp, "%d,%d,%d,", prob->job[i].p, prob->job[i].rr, prob->job[i].rd);
#else /* SIPSI */
    fprintf(fp, "%d,%d,", prob->job[i].p, prob->job[i].rd);
#endif /* SIPSI */

#ifdef COST_REAL
    print_real2(fp, prob->job[i].ew);
    fprintf(fp, ",");
    print_real2(fp, prob->job[i].tw);
#else /* COST_REAL */
    print_cost(fp, prob->job[i].ew);
    fprintf(fp, ",");
    print_cost(fp, prob->job[i].tw);
#endif /* COST_REAL */
    fprintf(fp, ")");
  }

  fprintf(fp, "\n");

  return(SIPS_OK);
}

/*
 * print_sol(prob, sol, fp)
 *   prints out the solution.
 *      sol: solution
 *       fp: file pointer
 *
 */
int print_sol(sips *prob, _solution_t *sol, FILE *fp)
{
  int i;

  if(sol->n == 0) {
    return(SIPS_FAIL);
  }
  fprintf(fp, "f=");
  print_cost(fp, sol->f + sol->off);
  fprintf(fp, "\n");
#ifdef SIPSI
  for(i = 0; i < sol->n - 1; i++) {
    if(sol->job[i]->name == NULL) {
      fprintf(fp, "(%d,%d),", sol->job[i]->rno, sol->c[i] + prob->Toff);
    } else {
      fprintf(fp, "(%s,%d),", sol->job[i]->name, sol->c[i] + prob->Toff);
    }
  }
  if(sol->job[i]->name == NULL) {
    fprintf(fp, "(%d,%d)\n", sol->job[i]->rno, sol->c[i] + prob->Toff);
  } else {
    fprintf(fp, "(%s,%d)\n", sol->job[i]->name, sol->c[i] + prob->Toff);
  }
#else /* SIPSI */
  for(i = 0; i < sol->n - 1; i++) {
    if(sol->job[i]->name == NULL) {
      fprintf(fp, "%d,", sol->job[i]->rno);
    } else {
      fprintf(fp, "%s,", sol->job[i]->name);
    }
  }
  if(sol->job[i]->name == NULL) {
    fprintf(fp, "%d\n", sol->job[i]->rno);
  } else {
    fprintf(fp, "%s\n", sol->job[i]->name);
  }
#endif /* SIPSI */

  return(SIPS_OK);
}
