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
 *  $Id: prob_common.c,v 1.25 2015/01/12 06:50:46 tanaka Rel $
 *  $Revision: 1.25 $
 *  $Date: 2015/01/12 06:50:46 $
 *  $Author: tanaka $
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "define.h"
#include "sips_common.h"
#include "bmemory.h"
#include "fixed.h"
#include "memory.h"
#include "param.h"
#include "print.h"
#include "prob_common.h"
#include "problem.h"
#include "ptable.h"
#include "sol.h"

/*
 * SiPS_create_problem()
 *   creates a new problem and returns a pointer to it.
 *
 */
sips *SiPS_create_problem(void)
{
  sips *prob;

  prob = (sips *) xmalloc(sizeof(sips));

  prob->name = NULL;
  prob->n = 0;
#ifdef SIPSI
  N_JOBS = 0;
  prob->dn = 0;
  prob->rmin = LARGE_INTEGER;
  prob->rmax = - LARGE_INTEGER;
  prob->usrT = 0;
#endif /* SIPSI */
  prob->psum = 0;
  prob->pmin = LARGE_INTEGER;
  prob->pmax = 0;
  prob->dmin = LARGE_INTEGER;
  prob->dmax = - LARGE_INTEGER;
  prob->job = NULL;
  prob->sjob = NULL;
  prob->cfunc = NULL;
  prob->cfuncl = NULL;
  prob->T = 0;
  prob->sol = NULL;
  prob->niseq = 0;
  prob->iseq = NULL;
  prob->off = ZERO_COST;
  prob->stime = 0.0;
  prob->time = 0.0;
  prob->graph = NULL;
  prob->stage = 0;
  prob->param = (_param_t *) xmalloc(sizeof(_param_t));
  param_set_default(prob);

  return(prob);
}

/*
 * SiPS_create_problem_with_name(name)
 *   creates a new problem with name and returns a pointer to it.
 *      name: problem name
 *
 */
sips *SiPS_create_problem_with_name(char *name)
{
  sips *prob;

  if((prob = SiPS_create_problem()) == NULL) {
    return(NULL);
  }

  if(name != NULL) {
    int len = min(strlen(name), 1024);
    prob->name = (char *) xmalloc(len + 1);
    memcpy((void *) prob->name, (void *) name, len);
    prob->name[len] = '\0';
  }

  return(prob);
}

/*
 * SiPS_create_problem_from_file(name)
 *   reads an instance from the data file and returns it.
 *      name: file name (read from the standard input if NULL)
 *
 */
sips *SiPS_create_problem_from_file(char *fname)
{
  sips *prob;

  if((prob = SiPS_create_problem()) == NULL) {
    return(NULL);
  }

  if(SiPS_read_problem(prob, fname) == SIPS_FAIL) {
    SiPS_free_problem(prob);
    return(NULL);
  }

  return(prob);
}

/*
 * SiPS_free_problem(prob)
 *   frees the allocated memory space for problem data.
 * 
 */
int SiPS_free_problem(sips *prob)
{
  if(prob != NULL) {
    free_solution(prob->sol);
    problem_free_jobs(prob);
    problem_free_sjob(prob);
    xfree(prob->iseq);
    xfree(prob->param);
    xfree(prob->name);
    xfree(prob);
  }

  return(SIPS_OK);
}

#ifdef COST_REAL
/*
 * SiPS_set_cost_type(prob, type)
 *   sets the type of the cost function.
 *      type: SIPS_COST_INTEGER if cost is integer-valued,
 *            SIPS_COST_REAL    if cost is real-valued.
 * 
 */
int SiPS_set_cost_type(sips *prob, int type)
{
  null_check(prob);

  if(type != SIPS_COST_INTEGER && type != SIPS_COST_REAL) {
    return(SIPS_FAIL);
  }

  prob->param->ctype = (unsigned char) type;
#ifndef LB_DEBUG
  if(prob->param->ctype == SIPS_COST_INTEGER) {
    prob->param->lbeps = 1.0 - prob->param->eps;
  } else {
    prob->param->lbeps = prob->param->eps;
  }
#endif /* LB_DEBUG */

  return(SIPS_OK);
}
#endif /* COST_REAL */

/*
 * SiPS_get_problem_size(prob)
 *   returns the problem size (number of jobs).
 *
 */
int SiPS_get_problem_size(sips *prob)
{
  null_check(prob);

  return(prob->n);
}

/*
 * SiPS_set_job_name(prob, no, name)
 *   set the name of a job
 *     no: job number
 *   name: job name (string)
 *
 */
int SiPS_set_job_name(sips *prob, int no, char *name)
{
  size_t length;

  null_check(prob);

  if(no < 0 || no >= prob->n) {
    return(SIPS_FAIL);
  }

  xfree(prob->job[no].name);
  prob->job[no].name = NULL;

  if(name != NULL) {
    length = min(strlen(name), MAXNAMELEN) + 1;
    prob->job[no].name = (char *) xmalloc(length);
    memcpy((void *) prob->job[no].name, (void *) name, length - 1);
    *(prob->job[no].name + length - 1) = '\0';
  }

  return(SIPS_OK);
}

/*
 * SiPS_set_job_names(prob, n, nos, names)
 *   set names of jobs
 *      n: number of jobs specified
 *     no: array of job numbers
 *   name: array of job names (strings)
 *
 */
int SiPS_set_job_names(sips *prob, int n, int *nos, char **names)
{
  int i;
  size_t length;

  null_check(prob);

  for(i = 0; i < n; i++) {
    if(nos[i] < 0 || nos[i] >= prob->n) {
      return(SIPS_FAIL);
    }
  }

  for(i = 0; i < n; i++) {
    xfree(prob->job[nos[i]].name);
    prob->job[nos[i]].name = NULL;

    if(names[i] != NULL) {
      length = min(strlen(names[i]), MAXNAMELEN) + 1;
      prob->job[nos[i]].name = (char *) xmalloc(length);
      memcpy((void *) prob->job[nos[i]].name, (void *) names[i], length - 1);
      *(prob->job[nos[i]].name + length - 1) = '\0';
    }
  }

  return(SIPS_OK);
}

/*
 * SiPS_delete_job(prob, no)
 *   deletes the specified job
 *     no: job number to be deleted
 *
 */
int SiPS_delete_job(sips *prob, int no)
{
  int i;
  char *tmpname;

  null_check(prob);

  if(no < 0 || no >= prob->n) {
    return(SIPS_FAIL);
  }

  tmpname = prob->job[no].name;

  for(i = no; i < prob->n; i++) {
    prob->job[i] = prob->job[i + 1];
    prob->job[i].no = prob->job[i].rno = i;
  }

  prob->job[prob->n - 1].name = tmpname;

  problem_set_size(prob, prob->n - 1);

  return(SIPS_OK);
}

/*
 * SiPS_delete_jobs(prob, n, no)
 *   deletes the specified jobs
 *      n: array size
 *     no: array of job number to be deleted
 *
 */
int SiPS_delete_jobs(sips *prob, int n, int *no)
{
  int i, j;
  char *flag, *tmpname;

  null_check(prob);

  null_check(no);

  if(n < 0 || n > prob->n) {
    return(SIPS_FAIL);
  }

  flag = (char *) xcalloc(prob->n, 1);

  for(i = 0; i < n; i++) {
    flag[no[i]] = 1;
  }

  for(i = j = 0; i < prob->n; i++) {
    if(!flag[i]) {
      tmpname = prob->job[j].name;
      prob->job[j] = prob->job[i];
      prob->job[j].no = prob->job[j].rno = j;
      prob->job[i].name = tmpname;
      j++;
    }
  }

  problem_set_size(prob, j);

  xfree(flag);

  return(SIPS_OK);
}

/*
 * SiPS_delete_all_jobs(prob)
 *   deletes all the jobs
 *
 */
int SiPS_delete_all_jobs(sips *prob)
{
  null_check(prob);
  return(problem_set_size(prob, 0));
}

/*
 * SiPS_set_cost_function(prob, func)
 *   specifies the job cost function.
 *    func: cost function that takes two arguments: job number i and time t.
 *          Its return value is f_i(t).
 *          If func is NULL the job cost function is cleared.
 *
 */
int SiPS_set_cost_function(sips *prob, cost_t (*cfunc)(int, int))
{
  null_check(prob);

  prob->cfunc = cfunc;
  prob->cfuncl = NULL;

  return(SIPS_OK);
}

/*
 * SiPS_set_cost_function_long(prob, func)
 *   specifies the job cost function.
 *    func: cost function that takes three arguments:
 *          problem, job number i, and time t.
 *          Its return value is f_i(t).
 *          If func is NULL the job cost function is cleared.
 *
 */
int SiPS_set_cost_function_long(sips *prob, cost_t (*cfunc)(sips *, int, int))
{
  null_check(prob);

  prob->cfuncl = cfunc;
  prob->cfunc = NULL;

  return(SIPS_OK);
}


/*
 * problem_check_solvability(prob)
 *   checks whether all the jobs are specified correctly before solving
 *   the problem.
 *
 */
int problem_check_solvability(sips *prob)
{
  int i;

  null_check(prob);

  if(prob->n == 0) {
    return(SIPS_FAIL);
  }

  for(i = 0; i < prob->n; i++) {
    if(prob->job[i].p <= 0) {
      return(SIPS_FAIL);
    }
  }

#ifdef SIPSI
  if((prob->cfunc != NULL || prob->cfuncl != NULL) && prob->usrT == 0) {
    return(SIPS_FAIL);
  }
#endif /* SIPSI */

  return(SIPS_OK);
}

/*
 * problem_free_jobs(prob)
 *   frees the memory space for job data.
 *
 */
void problem_free_jobs(sips *prob)
{
  int i;

  if(prob != NULL && prob->job != NULL) {
    problem_free_objective(prob);
    for(i = 0; i <= N_JOBS + 1; i++) {
      xfree(prob->job[i].name);
#ifdef SIPSI
      xfree(prob->job[i].func);
#endif /* SIPSI */
    }
    xfree(prob->job);
    prob->job = NULL;
  }
}

/*
 * problem_free_sjob(prob)
 *   frees the memory space for job data sorted in SPT order.
 *
 */
void problem_free_sjob(sips *prob)
{
  if(prob != NULL && prob->sjob != NULL) {
    xfree(prob->sjob);
    prob->sjob = NULL;
  }
}

/*
 * problem_free_objective(prob)
 *   frees the memory space for objective function value.
 *
 */
void problem_free_objective(sips *prob)
{
#ifdef SIPSI
  int i;
#endif /* SIPSI */

  if(prob != NULL && prob->job != NULL) {
    xfree(prob->job[0].f);
    prob->job[0].f = NULL;
#ifdef SIPSI
    for(i = 0; i < N_JOBS; i++) {
      xfree(prob->job[i].func);
      prob->job[i].func = NULL;
    }
#endif /* SIPSI */
  }
}

/*
 * problem_create_graph(prob)
 *   allocates memory space for DP graph (prob->graph).
 *
 */
int problem_create_graph(sips *prob)
{
  null_check(prob);

  problem_free_graph(prob);

  prob->graph = (_graph_t *) xcalloc(sizeof(_graph_t), 1);
  prob->graph->hdir = SIPS_FORWARD;
  prob->graph->copy = (_cgraph_t *) xcalloc(sizeof(_cgraph_t), 1);

  return(SIPS_OK);
}

/*
 * problem_free_graph(prob)
 *   frees memory space for DP graph (prob->graph).
 *
 */
void problem_free_graph(sips *prob)
{
  if(prob != NULL && prob->graph != NULL) {
    free_benv(prob->graph->copy->bnode);
    free_benv(prob->graph->copy->bedge);
    ptable_free(prob->graph->copy->ptable);
    free_fixed(prob->graph->copy->fixed);
    xfree(prob->graph->copy->node2);
    xfree(prob->graph->copy);
    free_benv(prob->graph->bnode);
    free_benv(prob->graph->bedge);
    xfree(prob->graph->node1);
    xfree(prob->graph->node2);
    ptable_free(prob->graph->ptable);
    free_fixed(prob->graph->fixed);
    xfree(prob->graph);
    prob->graph = NULL;
  }
}

/*
 * ndigits(n)
 *   returns the number of digits of n.
 *
 */
int ndigits(int n)
{
  int i;

  for(i = 1; n >= 10; i++, n /= 10);

  return(i);
}

/*
 * remove_comments(c)
 *   removes commments (characters after #) in the specified string.
 *        c: string
 */
void remove_comments(char *c)
{
  char *a = c;

  for(; *c == ' ' || *c == '\t'; c++);
  for(; *c != '\0' && *c != '#' && *c != '\n'; *a++ = *c, c++);
  *a='\0';
}
