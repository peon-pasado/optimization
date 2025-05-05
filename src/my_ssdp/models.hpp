#pragma once

#include <vector>
#include <cstdlib>
#include "utils/benv.hpp"

const double eps = 1e-5;
const double mem = 1536;

typedef struct {
  int no;                 /* internal number                                */
  int rno;                /* real number                                    */
  int tno;                /* tie-breaking order                             */
  int sno;                /* order in the optimal solution                  */
  char *name;             /* name                                           */
  int p;                  /* processing time                                */
  int rd;                 /* real duedate                                   */
  int d;                  /* internal duedate                               */
  int w;                             
  int *f;              /* vector of objective function value             */
  double u;                /* Lagrangian multiplier                          */
} _job_t;

typedef struct {
  unsigned short j;       /* job no                                         */
  int s;                  /* start of a time window                         */
  int e;                  /* end of a time window                           */
} _window_t;

typedef struct {
  double mem;              /* memory size in MB                              */
  size_t adj_per_t;       /* sizeof(adj) for each t                         */
  size_t dom_per_t;       /* sizeof(dom) for each t                         */
  size_t sd_size;         /* sizeof(sd)                                     */
  int n;                  /* number of jobs                                 */
  int T;                  /* end of scheduling horizon (= prob->T)          */
  char *adj;              /* adjacent precedence data                       */
  char *dom;              /* domain data                                    */
  char *sd;               /* super domain data                              */
  char **inc;             /* incidence matrix                               */
  int *pre_psum;          /* total processing time of predecessor jobs      */
  int *suc_psum;          /* total processing time of successor jobs        */
  unsigned int *occ;      /* list of numbers of occurrences in states       */
  _window_t *window;      /* list of job time windows                       */
} _ptable_t;

typedef struct {
  int nhead;              /* number of fixed jobs at the head               */
  int ntail;              /* number of fixed jobs at the tail               */
  int fhead;           /* objective value for fixed jobs at the head     */
  int ftail;           /* objective value for fixed jobs at the tail     */
  _job_t **job;           /* list of fixed jobs                             */
  int *c;                 /* list of completion times of fixed jobs         */
  char *state;            /* boolean to check if a job is fixed or not      */
} _fixed_t;

typedef struct {
  double v[2];
  char ty[2];
  unsigned short j[2];
} _node1_t;

struct _edge2_t {
  struct _node2_t *n;    /* connected node (tail in FW and head in BW) */
  struct _edge2_t *next; /* next edge */
};

struct _node2_t {
  unsigned short j;      /* job no */
  char ty;               /* type (0: best, 1: second best) */
  double v[2];             /* objective values */
  struct _node2_t *n[2]; /* prev. nodes realizing the min and second min */
  struct _node2_t *next; /* next node with the same t */
};

struct _edge2m_t {
  struct _node2m_t *n;    /* connected node (tail in FW and head in BW) */
  struct _edge2m_t *next; /* next edge */
};

struct _node2m_t {
  unsigned short j[3];    /* job no */
  unsigned char m;        /* total modifier */
  
  char ty;                /* type (0: best, 1: second best) */
  double v[2];             /* objective values */
  struct _node2m_t *n[2]; /* prev. nodes realizing the min and second min */

  struct _edge2m_t *e;    /* connected arcs */
  struct _node2m_t *next; /* next node with the same t */
};

typedef struct {
  double mem;              /* memory usage                                   */
  double rmem;             /* real memory usage                              */
  unsigned int n_nodes;   /* number of nodes in the graph representation    */
  unsigned int n_edges;   /* number of arcs in the graph repesentation      */
  int Tmin;               /* start of current scheduling horizon            */ 
  int Tmax;               /* end of current scheduling horizon              */ 
  _ptable_t *ptable;      /* precedence table                               */
  _fixed_t *fixed;        /* fixed jobs                                     */

  _node2_t *node2;            /* array of DP states of (LR2m) indexed by time   */
  _node2m_t *node2m;
  Benv *bnode;         /* block memory environment for nodes             */
  Benv *bedge;         /* block memory environment for arcs              */
  unsigned char direction;/* current DP direction                           */
} _cgraph_t;

typedef struct {
  unsigned int n_nodes;   /* number of nodes in the graph representation    */
  unsigned int n_edges;   /* number of arcs in the graph repesentation      */
  int Tmin;               /* start of current scheduling horizon            */ 
  int Tmax;               /* end of current scheduling horizon              */ 
  _ptable_t *ptable;      /* precedence table                               */
  _fixed_t *fixed;        /* fixed jobs                                     */

  _node1_t *node1;            /* array of DP states of (LR1) indexed by time    */
  _node2_t *node2;            /* array of DP states of (LR2adj) and (LR2m)      */
  _node2m_t *node2m;

                          /* indexed by time                                */
  Benv *bnode;         /* block memory environment for nodes             */
  Benv *bedge;         /* block memory environment for arcs              */
  unsigned char direction;/* current DP direction for (LR2m)                */
  unsigned char hdir;     /* current direction for Lagrangian heuristics    */
  _cgraph_t *copy;        /* copy of DP states for (LR2m)                   */
} _graph_t;

typedef struct {
  int f;               /* objective value                                */
  int off;             /* offset for objective value                     */
  int n;                  /* number of jobs                                 */
  int *c;                 /* job completion times                           */
  _job_t **job;           /* job data                                       */
} _solution_t;


typedef struct sips {
  int n;                  /* number of jobs                                 */
  int psum;               /* total processing time                          */
  int pmin;               /* minimum processing time                        */
  int pmax;               /* maximum processing time                        */
  int dmin;               /* minimum duedate                                */
  int dmax;               /* maximum duedate                                */
  int T;                  /* end of scheduling horizon                      */
  _job_t *job;            /* job data                                       */
  _job_t **sjob;          /* SPT sequence                                   */
  _graph_t *graph;        /* graph corresponding to relaxations             */
  _solution_t *sol;       /* solution                                       */
  int niseq;              /* length of user specified initial sequence      */
  int *iseq;              /* user specified initial sequence                */
} sips;

typedef struct {
  unsigned short j;
  int s;
  int e;
} _list_t;

typedef struct {
  unsigned short j;
  int bi;
  int key;
} _list2_t;

typedef struct {
  int n;                  /* number of total modifiers */
  int an;                 /* number of modifiers to be newly added */
  unsigned char *fl;      /* 1 if job i is assigned a nonzero modifier */
  int *jobs;              /* list of jobs newly assinged a nonzero modifier */
  unsigned char **v;      /* v[j][m]: whether modifier value of job j is m */
  unsigned char *m;       /* modifier value */
} _mod_t;

typedef struct {
  unsigned short j;
  unsigned int no;
  unsigned int nd;
} _occur_t;

sips* prob;