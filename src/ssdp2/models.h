#pragma once

#include <iostream>
#include <vector>
const int inf = 1e9;
int N, N2;

struct bit_set {
    size_t n;
    std::vector<uint64_t> data;

    bit_set(size_t n) : n(n){
        data.resize((n + 63) >> 6, 0);
    }

    inline void set(size_t idx) {
        data[idx >> 6] |= (1ULL << (idx & 63));
    }

    inline void reset(size_t idx) {
        data[idx >> 6] &= ~(1ULL << (idx & 63));
    }

    void set_range(size_t l, size_t r) {
        if (l > r || r >= n) return; // Validación básica

        size_t start_block = l >> 6, end_block = r >> 6;
        uint64_t left_mask = ~0ULL << (l & 63);
        uint64_t right_mask = (1ULL << ((r & 63) + 1)) - 1;

        if (start_block == end_block) {
            data[start_block] |= (left_mask & right_mask);
        } else {
            data[start_block] |= left_mask;
            for (size_t i = start_block + 1; i < end_block; i++) {
                data[i] = ~0ULL;  // Rellena bloques completos
            }
            data[end_block] |= right_mask;
        }
    }

    inline bool test(size_t idx) const {
        return (data[idx >> 6] >> (idx & 63)) & 1;
    }
};

int total_nodes = 0;

struct Node {
    int i;
    int node[2][2];
    double val[2][2];
    bool fnode[2][2];
    Node* node_ptr[2][2];

    Node* next;
    Node(int i) : i(i), next(nullptr) {}
};
    
// Tmap funciona para L1, el mantiene
// la lista para todos los nodos, ademas
// mantiene la lista de los nodos eliminados
// en un bitset, y la informacion de los
// dos mejores caminos hasta el horizonte t
// ademas de la funcion de costo para cada i
// incorporamos el numero de nodos.
struct Tmap {
    double val[2][2];
    int node[2][2];
    Node* at;
    int* f;
    bit_set* b;
    int n_nos;
};

struct Edge2m {
  struct Node2m *n;    /* connected node (tail in FW and head in BW) */
  struct Edge2m *next; /* next edge */
};

struct Node2m {
  unsigned short j[3];    /* job no */
  unsigned char m;        /* total modifier */
  
  bool ty[2];                /* type (0: best, 1: second best) */
  double v[2];             /* objective values */
  struct Node2m *n[2]; /* prev. nodes realizing the min and second min */

  struct Edge2m *e;    /* connected arcs */
  struct Node2m *next; /* next node with the same t */
};

//contiene una lista de tmaps, para
//cada t, y una set del grafo conprimido.
struct T2map {
    Tmap* tm;
    bit_set* tb;
};

//typedef struct {
  //_real mem;              /* memory size in MB                              */
  //size_t adj_per_t;       /* sizeof(adj) for each t                         */
  //size_t dom_per_t;       /* sizeof(dom) for each t                         */
  //size_t sd_size;         /* sizeof(sd)                                     */
  //int n;                  /* number of jobs                                 */
  //int T;                  /* end of scheduling horizon (= prob->T)          */
  //char *adj;              /* adjacent precedence data                       */
  //char *dom;              /* domain data                                    */
  //char *sd;               /* super domain data                              */
  //char **inc;             /* incidence matrix                               */
  //int *pre_psum;          /* total processing time of predecessor jobs      */
  //int *suc_psum;          /* total processing time of successor jobs        */
//#ifdef NLOGN_EDGE_FINDING
  //int *btctable;          /* list -> btree conversion table                 */
//#endif /* NLOGN_EDGE_FINDING */
  //unsigned int *occ;      /* list of numbers of occurrences in states       */
  //_window_t *window;      /* list of job time windows                       */
//} _ptable_t;



struct SublGraph {
    int n_nodes;
    int n_edges;
    //_ptable_t* ptable;
    Node2m* node;
    int direction;
    int* occ;
};

//contiene informacion relevante del proceso, asi como el mu, lower bounds,
//y solucion actual.
struct info {
    int n; //number of jobs
    int* p; //processing time
    int* w; //weight
    int* d; //due date
    int* ord; //order of jobs (for tie-breaking)
    int* sol; //solution
    int* s; //start time for each job
    int* e; //end time for each job
    int fobj; //objective function value
    int T; //horizon
    int stage; //stage of the algorithm
    double lb; //lower bound
    int ub; //upper bound
    int tentative_ub; //tentative upper bound
    int verbose; //verbosity
    double* u; //Lagrangian multipliers
    double su; //sum of u
    double eps; //tolerance
    int* s_before;  //previous start times
    int* e_before;  //previous end times
};

typedef struct {
  int n;                  /* number of total modifiers */
  int an;                 /* number of modifiers to be newly added */
  unsigned char *fl;      /* 1 if job i is assigned a nonzero modifier */
  int *jobs;              /* list of jobs newly assinged a nonzero modifier */
  unsigned char **v;      /* v[j][m]: whether modifier value of job j is m */
  unsigned char *m;       /* modifier value */
} lr2m_mod_t;

typedef struct {
    int no;                 /* internal number                                */
    int rno;                /* real number                                    */
    int tno;                /* tie-breaking order                             */
    int sno;                /* order in the optimal solution                  */
    char *name;             /* name                                           */
    int p;                  /* processing time                                */
    int rd;                 /* real duedate                                   */
    int d;                  /* internal duedate                               */
    int ew;                 /* earliness weight                               */
    int tw;                 /* tardiness weight                               */
    int *f;                 /* vector of objective function value             */
    double u;               /* Lagrangian multiplier                          */
  } _job_t;

typedef struct {
    double f;               /* objective value                                */
    double off;             /* offset for objective value                     */
    int n;                  /* number of jobs                                 */
    int *c;                 /* job completion times                           */
    _job_t **job;           /* job data                                       */
} _solution_t;

typedef struct {
    int nhead;              /* number of fixed jobs at the head               */
    int ntail;              /* number of fixed jobs at the tail               */
    int fhead;           /* objective value for fixed jobs at the head     */
    int ftail;           /* objective value for fixed jobs at the tail     */
    _job_t **job;           /* list of fixed jobs                             */
    int *c;                 /* list of completion times of fixed jobs         */
    bit_set *state;            /* boolean to check if a job is fixed or not      */
} _fixed_t; 

info* prob;
Tmap* tmap;
T2map* t2m;
SublGraph* subl_graph;
lr2m_mod_t* lr2m_mod;