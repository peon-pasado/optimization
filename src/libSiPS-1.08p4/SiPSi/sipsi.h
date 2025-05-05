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
 *  $Id: sipsi.h,v 1.34 2015/01/12 06:50:32 tanaka Rel $
 *  $Revision: 1.34 $
 *  $Date: 2015/01/12 06:50:32 $
 *  $Author: tanaka $
 *
 */
#ifndef SIPSI_H
#define SIPSI_H
#include <stdlib.h>

#define SIPS_TRUE                1
#define SIPS_FALSE               0

#define SIPS_NONE               -1
#define SIPS_OK                  0
#define SIPS_SOLVED              0

#define SIPS_FAIL               -1
#define SIPS_NORMAL              1
#define SIPS_TIMELIMIT           2
#define SIPS_MEMLIMIT            3
#define SIPS_LBONLY              4

#define SIPS_OPTIMAL            10
#define SIPS_INFEASIBLE         11

#define SIPS_UNSOLVED           20

#define SIPS_MEMFAULT           98
#define SIPS_FATAL              99

#define SIPS_I_BISECTION       100
#define SIPS_I_DOMLEVEL        101
#define SIPS_I_DPSIZE          102
#define SIPS_I_DUMMYJOBS       103
#define SIPS_I_MAXMEMORY       104
#define SIPS_I_MAXMODIFIER     105
#define SIPS_I_MINUPDATE       106
#define SIPS_I_MODSWITCH       107
#define SIPS_I_NOSTAGE3        108
#define SIPS_I_UBONLY          109
#define SIPS_I_SEARCHTYPE      110
#define SIPS_I_SECSWITCH       111
#define SIPS_I_SHRINKITER1     112
#define SIPS_I_SHRINKITER2     113
#define SIPS_I_SKIPSTAGE1      114
#define SIPS_I_TIEBREAK        115
#define SIPS_I_UBITERATION     116
#define SIPS_I_UBUPDATE1       117
#define SIPS_I_UBUPDATE3       118
#define SIPS_I_VERBOSE         119
#define SIPS_I_WARMSTART       120
#define SIPS_I_ZEROMULTIPLIER  121

#define SIPS_R_EPS             130
#ifdef COST_REAL
#define SIPS_R_FUNCEPS         131
#endif /* COST_REAL */
#define SIPS_R_EXPAND1         132
#define SIPS_R_EXPAND2         133
#define SIPS_R_INITSTEP1       134
#define SIPS_R_INITSTEP2       135
#define SIPS_R_INITTERMITER1   136
#define SIPS_R_INITTERMITER2   137
#define SIPS_R_MAXSTEP1        138
#define SIPS_R_MAXSTEP2        139
#define SIPS_R_MULTIPLIER      140
#define SIPS_R_TERMITER1       141
#define SIPS_R_TERMITER2       142
#define SIPS_R_SECINIRATIO     143
#define SIPS_R_SECRATIO        144
#define SIPS_R_SHRINK1         145
#define SIPS_R_SHRINK2         146
#define SIPS_R_TERMRATIO1      147
#define SIPS_R_TERMRATIO2      148
#define SIPS_R_TIMELIMIT       149

/* problem type */
#define SIPS_PROB_NONE           0
#define SIPS_PROB_IDLETIME       1
#define SIPS_PROB_REGULAR        2
#define SIPS_PROB_VSHAPE         4

/* cost type */
#define SIPS_COST_INTEGER        0
#define SIPS_COST_REAL           1

/* search type */
#define SIPS_LS_NONE             0
#define SIPS_LS_DYNA             1
#define SIPS_LS_COMBINED_A       2
#define SIPS_LS_COMBINED_B       3
#define SIPS_LS_EDYNA            4

/* tie-breaking */
#define SIPS_TIEBREAK_HEURISTIC  0
#define SIPS_TIEBREAK_RHEURISTIC 1
#define SIPS_TIEBREAK_EDD        2
#define SIPS_TIEBREAK_SPT        3
#define SIPS_TIEBREAK_LDD        4
#define SIPS_TIEBREAK_LPT        5
#define SIPS_TIEBREAK_WSPT       6
#define SIPS_TIEBREAK_WLPT       7
#define SIPS_TIEBREAK_RELEASE    8

/* direction */
#define SIPS_BACKWARD            0
#define SIPS_FORWARD             1

#ifdef REAL_LONG
#define _real long double
#else /* REAL_LONG */
#define _real double
#endif /* REAL_LONG */

#ifdef COST_REAL
#define cost_t _real
#undef COST_LONGLONG
#else /* COST_REAL */
#ifdef COST_LONGLONG
#define cost_t long long int
#else /* COST_LONGLONG */
#define cost_t int
#endif /* COST_LONGLONG */
#endif /* COST_REAL */

typedef struct _point_t {
  int t;                  /* time                                           */
  cost_t v;               /* value                                          */
  int s;                  /* status                                         */
  struct _point_t *prev;  /* predecessor                                    */
  struct _point_t *next;  /* successor                                      */
} _point_t;

typedef struct {
  int no;                 /* internal number                                */
  int rno;                /* real number                                    */
  int tno;                /* tie-breaking order                             */
  int sno;                /* order in the optimal solution                  */
  char *name;             /* name                                           */
  int p;                  /* processing time                                */
  int rd;                 /* real duedate                                   */
  int d;                  /* internal duedate                               */
  int rr;                 /* real release date                              */
  int r;                  /* internal release date                          */
  cost_t ew;              /* earliness weight                               */
  cost_t tw;              /* tardiness weight                               */
  cost_t *f;              /* vector of objective function value             */
  _point_t *func;         /* cost function expressed by a piecewise linear  */
                          /* function                                       */
  _real u;                /* Lagrangian multiplier                          */
} _job_t;

typedef struct {
  int n;
  int nb;
  int cb;
  size_t size;
  int n_per_block;
  void *ptr;
  void *bptr;
  void *cbptr;
} _benv_t;

typedef struct {
  unsigned short j;       /* job no                                         */
  int s;                  /* start of a time window                         */
  int e;                  /* end of a time window                           */
} _window_t;

typedef struct {
  _real mem;              /* memory size in MB                              */
  size_t adj_per_t;       /* sizeof(adj) for each t                         */
  size_t dom_per_t;       /* sizeof(dom) for each t                         */
  int n;                  /* number of jobs (= prob->n + 1)                 */
  int T;                  /* end of scheduling horizon (= prob->T)          */
  char *adj;              /* adjacent precedence data                       */
  char *dom;              /* domain data                                    */
  char **inc;             /* incidence matrix                               */
  int *pre_psum;          /* total processing time of predecessor jobs      */
  int *suc_psum;          /* total processing time of successor jobs        */
#ifdef NLOGN_EDGE_FINDING
  int *btctable;          /* list -> btree conversion table                 */
#endif /* NLOGN_EDGE_FINDING */
  unsigned int *occ;      /* list of numbers of occurrences in states       */
  _window_t *window;      /* list of job time windows                       */
} _ptable_t;

typedef struct {
  int nhead;              /* number of fixed jobs at the head               */
  int ntail;              /* number of fixed jobs at the tail               */
  cost_t fhead;           /* objective value for fixed jobs at the head     */
  cost_t ftail;           /* objective value for fixed jobs at the tail     */
  _job_t **job;           /* list of fixed jobs                             */
  int *c;                 /* list of completion times of fixed jobs         */
  char *state;            /* boolean to check if a job is fixed or not      */
} _fixed_t;

typedef struct {
  _real mem;              /* memory usage                                   */
  _real rmem;             /* real memory usage                              */
  unsigned int n_nodes;   /* number of nodes in the graph representation    */
  unsigned int n_edges;   /* number of arcs in the graph repesentation      */
  int Tmin;               /* start of current scheduling horizon            */ 
  int Tmax;               /* end of current scheduling horizon              */ 
  _ptable_t *ptable;      /* precedence table                               */
  _fixed_t *fixed;        /* fixed jobs                                     */

  void *node2;            /* array of DP states of (LR2m) indexed by time   */
  _benv_t *bnode;         /* block memory environment for nodes             */
  _benv_t *bedge;         /* block memory environment for arcs              */
  unsigned char direction;/* current DP direction                           */
} _cgraph_t;

typedef struct {
  unsigned int n_nodes;   /* number of nodes in the graph representation    */
  unsigned int n_edges;   /* number of arcs in the graph repesentation      */
  int Tmin;               /* start of current scheduling horizon            */ 
  int Tmax;               /* end of current scheduling horizon              */ 
  _ptable_t *ptable;      /* precedence table                               */
  _fixed_t *fixed;        /* fixed jobs                                     */

  void *node1;            /* array of DP states of (LR1) indexed by time    */
  void *node2;            /* array of DP states of (LR2adj) and (LR2m)      */
                          /* indexed by time                                */
  _benv_t *bnode;         /* block memory environment for nodes             */
  _benv_t *bedge;         /* block memory environment for arcs              */
  unsigned char direction;/* current DP direction for (LR2m)                */
  unsigned char hdir;     /* current direction for Lagrangian heuristics    */
  _cgraph_t *copy;        /* copy of DP states for (LR2m)                   */
} _graph_t;

typedef struct {
  cost_t f;               /* objective value                                */
  cost_t off;             /* offset for objective value                     */
  int n;                  /* number of jobs                                 */
  int *c;                 /* job completion times                           */
  _job_t **job;           /* job data                                       */
} _solution_t;

typedef struct {
  unsigned char ptype;    /* problem type                                   */
			  /*  1: with idle time                             */
			  /*  2: regular cost function                      */
#ifdef COST_REAL
  unsigned char ctype;    /* objective type                                 */
			  /*  0: integer                                    */
			  /*  1: real                                       */
  _real funceps;          /* tolerance for piecewise linear function        */
#endif /* COST_REAL */
  _real eps;              /* tolerance                                      */
  _real lbeps;            /* lb/ub tolerance. eps - 1.0 if objective is     */
                          /* integral, eps otherwise.                       */

  int verbose;            /* verbose level                                  */

  int mem;                /* maximum memory size available for dynamic      */
                          /* programming                                    */
  unsigned char lb;       /* lower bound computation only                   */
  unsigned char ub;       /* upper bound computation only                   */
  unsigned char warm;     /* warm start                                     */

  _real tlimit;           /* time limit                                     */

  int dummy;              /* number of dummy jobs                           */
  unsigned char dlevel;   /* dominance check level at the start of Stage 3. */
                          /* 1-6.                                           */
  unsigned char tiebreak; /* tie breaking rule in the dominance of adjacent */
                          /* two jobs                                       */
  unsigned char ls;       /* local search for improving upper bounds        */
                          /*   0: PI heuristics                             */
                          /*   1: dynasearch                                */
                          /*   2: enhanced dynasearch                       */

  int dpsize;             /* maximum number of duplicated jobs in the       */
                          /* solution of the relaxation for applying the    */
                          /* DP-based heuristics                            */

  /* stage 1 */
  unsigned char skip1;    /* skip Stage 1                                   */
  _real u0;               /* initial multiplier                             */
  _real initstep1;        /* initial step size factor (LR1)                 */
  _real ititer1;          /* initial termination counter (LR1)              */
  _real maxstep1;         /* maximum step size factor (LR1)                 */
  _real titer1;           /* termination counter (LR1)                      */
  int siter1;             /* step size shrink counter (LR1)                 */
  _real shrink1;          /* step size shrink factor (LR1)                  */
  _real expand1;          /* step size expand factor (LR1)                  */
  _real tratio1;          /* gap update ratio for termination (LR1)         */
  unsigned char ubupd1;   /* upper bound computation at the end of stage 1  */

  /* stage 2 */
  _real initstep2;        /* initial step size factor (LR2adj)              */
  _real ititer2;          /* initial termination counter (LR2adj)           */
  _real maxstep2;         /* maximum step size factor (LR2adj)              */
  _real titer2;           /* termination counter (LR2adj)                   */
  int siter2;             /* step size shrink counter (LR2adj)              */
  _real shrink2;          /* step size shrink factor (LR2adj)               */
  _real expand2;          /* step size expand factor (LR2adj)               */
  _real tratio2;          /* gap update ratio for termination (LR2adj)      */
  int ubiter;             /* upper bound is updated once in this number of  */
                          /* iterations.  Upper bound is not updated if it  */
                          /* is zero.                                       */
  int minupdate;          /* minimum number of updates in the lower bound   */
                          /* required before termination is determined by   */
                          /* (# jobs)/(this value)                          */

  /* stage 3 */
  int secsw;              /* memory occupation ratio for sectioning         */
  unsigned char bisec;    /* The tentative upper bound is changed as in the */
                          /* bisection algorithm.  To do this, node backup  */
                          /* should always be kept, which results in the    */
                          /* increase of memory usage.                      */
  _real seciniratio;      /* The tentative upper bound is changed by        */
                          /* seciniratio at the beginning, and then by      */
  _real secratio;         /* secratio, if secratio is larger than zero.     */
                          /* If seciniratio is not larger than zero,        */
                          /* seciniratio is set to secratio.                */
  int modsw;              /* memory occupation ratio for switching modifier */
                          /* strategy                                       */
  unsigned char mod;      /* maximum number of modifiers added at one       */
                          /* iteration                                      */
  unsigned char zmplier;  /* zero multipliers are used simultaneously       */
  unsigned char ubupd3;   /* upper bound computation at every iteration of  */
                          /* stage 3  */
} _param_t;

typedef struct sips {
  char *name;             /* problem name                                   */
  int tn;                 /* number of jobs (= n + dn)                      */
  int n;                  /* number of real jobs                            */
  int dn;                 /* number of dummy jobs                           */
  int psum;               /* total processing time                          */
  int pmin;               /* minimum processing time                        */
  int pmax;               /* maximum processing time                        */
  int rmin;               /* minimum release date                           */
  int rmax;               /* maximum release date                           */
  int dmin;               /* minimum duedate                                */
  int dmax;               /* maximum duedate                                */
  int Toff;               /* offset for time                                */
  int T;                  /* end of scheduling horizon                      */
  cost_t (*cfunc)(int, int);
                          /* cost function                                  */
  cost_t (*cfuncl)(struct sips *, int, int);
                          /* cost function (long version)                   */
  cost_t off;             /* offset for objective function                  */
  int usrT;               /* user specified end of scheduling horizon       */
  _real stime;            /* initial CPU time                               */
  _real time;             /* elapsed CPU time                               */
  _job_t *job;            /* job data                                       */
  _job_t **sjob;          /* SPT sequence                                   */
  _graph_t *graph;        /* graph corresponding to relaxations             */
  _param_t *param;        /* parameters                                     */
  _solution_t *sol;       /* solution                                       */
  int niseq;              /* length of user specified initial sequence      */
  int *iseq;              /* user specified initial sequence                */
  char stage;             /* current stage                                  */
} sips;

sips *SiPS_create_problem(void);
sips *SiPS_create_problem_with_name(char *);
sips *SiPS_create_problem_from_file(char *);

int SiPS_read_problem(sips *, char *);
int SiPS_get_problem_size(sips *);
#ifdef COST_REAL
int SiPS_set_cost_type(sips *, int);
#endif /* COST_REAL */

int SiPS_free_problem(sips *);
int SiPS_TWT_set_job(sips *, int, char *, int, int, int, cost_t);
int SiPS_TWT_set_jobs(sips *, int, int *, char **, int *, int *, int *,
		      cost_t *);
int SiPS_TWET_set_job(sips *, int, char *, int, int, int, cost_t, cost_t);
int SiPS_TWET_set_jobs(sips *, int, int *, char **, int *, int *, int *,
		       cost_t *, cost_t *);
int SiPS_get_job(sips *, int, int *, int *, int *, cost_t *, cost_t *);
int SiPS_get_jobs(sips *, int, int *, int *, int *, int *, cost_t *, cost_t *);
int SiPS_set_job_name(sips *, int, char *);
int SiPS_set_job_names(sips *, int, int *, char **);
int SiPS_delete_job(sips *, int);
int SiPS_delete_jobs(sips *, int, int *);
int SiPS_delete_all_jobs(sips *);

int SiPS_set_cost_function(sips *, cost_t (*f)(int, int));
int SiPS_set_cost_function_long(sips *, cost_t (*f)(sips *, int, int));
int SiPS_set_horizon(sips *, int);

int SiPS_set_int_param(sips *, int, int);
int SiPS_set_real_param(sips *, int, double);

int SiPS_set_initial_sequence(sips *, int, int *);

int SiPS_solve(sips *);

int SiPS_get_solution(sips *, cost_t *, int *, int *);
int SiPS_get_cputime(sips *, double *);

int SiPS_print_jobs(sips *);
int SiPS_print_solution(sips *);

#endif /* SIPSI_H */
