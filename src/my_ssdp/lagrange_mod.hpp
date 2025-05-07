#pragma once
#include "models.hpp"
#include "utils/constants.hpp"
#include "utils/benv.hpp"
#include "fixed.hpp"
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include "successive.hpp"

#define THREE_COST2(j0, j1, j2)						\
  prob->sjob[j[j0]]->f[c]						\
  + prob->sjob[j[j1]]->f[c - prob->sjob[j[j0]]->p]			\
  + prob->sjob[j[j2]]->f[c - prob->sjob[j[j0]]->p - prob->sjob[j[j1]]->p]

#define THREE_COST_CHECK2(j0, j1, j2) {					\
    g = THREE_COST2(j0, j1, j2);						\
    if (g < f							\
       || (g == f						\
	   && prob->sjob[j[j2]]->tno < prob->sjob[j[2]]->tno)) {	\
      return 1;							\
    }									\
  }

char _check_supernode(unsigned short *j, int c) {
  int f, g;

  // 2->1->0
  f = THREE_COST2(0, 1, 2);
  // 0->2->1
  THREE_COST_CHECK2(1, 2, 0);
  // 1->0->2
  THREE_COST_CHECK2(2, 0, 1);
  // 0->1->2
  THREE_COST_CHECK2(2, 1, 0);

  return(0);
}


char _check_adj_supernode_forward(_node2m_t *n1, _node2m_t *n2,
  int c) {
int k, l;
int s, s2;
int j[6];
_edge2m_t *e;
_node2m_t *n0;

if (n1->j[0]==prob->n || n2->j[0]==prob->n) return 0;

k = 5;
s = c - prob->sjob[n1->j[0]]->p - prob->sjob[n2->j[0]]->p;
j[k--] = n2->j[0];
if (n2->j[2] < prob->n) {
j[k--] = n2->j[1];
j[k--] = n2->j[2];
s -= prob->sjob[n2->j[2]]->p + prob->sjob[n2->j[1]]->p;
} else if (n2->j[1] < prob->n) {
j[k--] = n2->j[1];
s -= prob->sjob[n2->j[1]]->p;
}

j[k--] = n1->j[0];
if (n1->j[2] < prob->n) {
j[k--] = n1->j[1];
j[k--] = n1->j[2];
s -= prob->sjob[n1->j[2]]->p + prob->sjob[n1->j[1]]->p;
} else if (n1->j[1] < prob->n) {
j[k--] = n1->j[1];
s -= prob->sjob[n1->j[1]]->p;
}

switch(k) {
case -1:
if(check_four_cost_forward(j + 2,
     s + prob->sjob[j[0]]->p + prob->sjob[j[1]]->p,
     c)) {
return(1);
}
if(check_five_cost_forward(j + 1, s + prob->sjob[j[0]]->p, c)) {
return(1);
}
return(check_six_cost_forward(j, s, c));
case 0:
if(check_three_cost(j + 3,
s + prob->sjob[j[1]]->p + prob->sjob[j[2]]->p, c)) {
return(1);
}
if(check_four_cost_forward(j + 2, s + prob->sjob[j[1]]->p, c)) {
return(1);
}
return(check_five_cost_forward(j + 1, s, c));
case 1:
if(check_three_cost(j + 3, s + prob->sjob[j[2]]->p, c)) {
return(1);
}
return(check_four_cost_forward(j + 2, s, c));
case 2:
return(check_three_cost(j + 3, s, c));
default:
break;
}
for(e = n1->e; e; e = e->next) {
n0 = e->n;
if (n0->j[0] == prob->n) {
return(0);
}
l = k;
if (n0->j[0] == j[5] || n0->j[0] == j[4]) {
continue;
}
j[l--] = n0->j[0];
s2 = s - prob->sjob[n0->j[0]]->p;
if (n0->j[2] < prob->n) {
j[l--] = n0->j[1];
j[l--] = n0->j[2];
if (j[2] == j[5] || j[2] == j[4]) {
continue;
}
if (j[1] == j[5] || j[1] == j[4]) {
continue;
}
s2 -= prob->sjob[n0->j[2]]->p + prob->sjob[n0->j[1]]->p;
} else if(n0->j[1] < prob->n) {
j[l--] = n0->j[1];
if (j[2] == j[5] || j[2] == j[4]) {
continue;
}
s2 -= prob->sjob[n0->j[1]]->p;
}

switch(l) {
case 0:
if(check_three_cost(j + 3,
s2 + prob->sjob[j[1]]->p + prob->sjob[j[2]]->p, c)) {
continue;
}
if(check_four_cost_forward(j + 2, s2 + prob->sjob[j[1]]->p, c)) {
continue;
}
if(check_five_cost_forward(j + 1, s2, c)) {
continue;
}
return(0);
case 1:
if(check_three_cost(j + 3, s2 + prob->sjob[j[2]]->p, c)) {
continue;
}
if(check_four_cost_forward(j + 2, s2, c)) {
continue;
}
return(0);
case 2:
default:
if(check_three_cost(j + 3, s2, c)) {
continue;
}
return(0);
}
}

return(1);
}

char _check_adj_supernode_backward(_node2m_t *n1, _node2m_t *n2,
   int c)
{
int k, l;
int s, c2;
int j[6];
_edge2m_t *e;
_node2m_t *n3;

if (n1->j[0] == prob->n || n2->j[0] == prob->n) return 0;

k = 0;
s = c - prob->sjob[n1->j[0]]->p;
c += prob->sjob[n2->j[0]]->p;
if (n1->j[2] < prob->n) {
j[k++] = n1->j[2];
j[k++] = n1->j[1];
s -= prob->sjob[n1->j[2]]->p + prob->sjob[n1->j[1]]->p;
} else if (n1->j[1] < prob->n) {
j[k++] = n1->j[1];
s -= prob->sjob[n1->j[1]]->p;
}
j[k++] = n1->j[0];

if (n2->j[2] < prob->n) {
j[k++] = n2->j[2];
j[k++] = n2->j[1];
c += prob->sjob[n2->j[2]]->p + prob->sjob[n2->j[1]]->p;
} else if(n2->j[1] < prob->n) {
j[k++] = n2->j[1];
c += prob->sjob[n2->j[1]]->p;
}
j[k++] = n2->j[0];

switch(k) {
case 6:
if(check_four_cost_backward(j, s,
c - prob->sjob[j[5]]->p
- prob->sjob[j[4]]->p)) {
return(1);
}
if(check_five_cost_backward(j, s, c -  prob->sjob[j[5]]->p)) {
return(1);
}
return(check_six_cost_backward(j, s, c));
case 5:
if(check_three_cost(j, s,
c - prob->sjob[j[4]]->p - prob->sjob[j[3]]->p)) {
return(1);
}
if(check_four_cost_backward(j, s, c - prob->sjob[j[4]]->p)) {
return(1);
}
return(check_five_cost_backward(j, s, c));
case 4:
if(check_three_cost(j, s, c - prob->sjob[j[3]]->p)) {
return(1);
}
return(check_four_cost_backward(j, s, c));
case 3:
return(check_three_cost(j, s, c));
default:
break;
}

for(e = n2->e; e; e = e->next) {
n3 = e->n;
if (n3->j[0] == prob->n) return 0;
l = k;
c2 = c + prob->sjob[n3->j[0]]->p;
if (n3->j[2] < prob->n) {
j[l++] = n3->j[2];
j[l++] = n3->j[1];
if (j[2] == j[0] || j[2] == j[1]) {
continue;
}
if(j[3] == j[0] || j[3] == j[1]) {
continue;
}
c2 += prob->sjob[n3->j[2]]->p + prob->sjob[n3->j[1]]->p;
} else if(n3->j[1] < prob->n) {
j[l++] = n3->j[1];
if (j[2] == j[0] || j[2] == j[1]) {
continue;
}
c2 += prob->sjob[n3->j[1]]->p;
}
if (n3->j[0] == j[0] || n3->j[0] == j[1]) {
continue;
}
j[l++] = n3->j[0];

switch(l) {
case 5:
if(check_three_cost(j, s,
c2 - prob->sjob[j[4]]->p - prob->sjob[j[3]]->p)) {
continue;
}
if(check_four_cost_backward(j, s, c2 - prob->sjob[j[4]]->p)) {
continue;
}
if(check_five_cost_backward(j, s, c2)) {
continue;
}
return(0);
case 4:
if(check_three_cost(j, s, c2 - prob->sjob[j[3]]->p)) {
continue;
}
if(check_four_cost_backward(j, s, c2)) {
continue;
}
return(0);
case 3:
default:
if(check_three_cost(j, s, c2)) {
continue;
}
return(0);
}
}

return(1);
}

#define HAVE_JOB(xjob, xnode)					\
  (xjob == xnode->j[2] || xjob == xnode->j[1] || xjob == xnode->j[0])

#define HAVE_REAL_JOB(xjob, xnode)				\
  (xjob < prob->n && HAVE_JOB(xjob, xnode))

#define HAVE_COMMON_REAL_JOB(xnode1, xnode2)			\
  (HAVE_JOB(xnode1->j[0], xnode2)				\
   || HAVE_REAL_JOB(xnode1->j[1], xnode2)			\
   || HAVE_REAL_JOB(xnode1->j[2], xnode2))

#define REMOVE_SINGLE_EDGE {			\
    e = (*pe)->next;				\
    bedge->free_element(*pe);			\
    *pe = e;					\
    n_edges--;					\
  }

#define REMOVE_EDGES(xnode) {				\
    for (e = xnode->e; e; e = e2) {		\
      e2 = e->next;					\
      bedge->free_element(e);				\
      n_edges--;					\
    }							\
    xnode->e = nullptr;					\
  }

#define UPDATE_WINDOWS {					\
    ptable->window[cnode->j[0]].s				\
      = min(ptable->window[cnode->j[0]].s, t);			\
    ptable->window[cnode->j[0]].e				\
      = max(ptable->window[cnode->j[0]].e, t);			\
    ptable->occ[cnode->j[0]]++;					\
    if (cnode->j[1] < prob->n) {					\
      tt = t - prob->sjob[cnode->j[0]]->p;			\
      ptable->window[cnode->j[1]].s				\
	= min(ptable->window[cnode->j[1]].s, tt);		\
      ptable->window[cnode->j[1]].e				\
	= max(ptable->window[cnode->j[1]].e, tt);		\
      ptable->occ[cnode->j[1]]++;				\
      if (cnode->j[2] < prob->n) {					\
	tt -= prob->sjob[cnode->j[1]]->p;			\
	ptable->window[cnode->j[2]].s				\
	  = min(ptable->window[cnode->j[2]].s, tt);		\
	ptable->window[cnode->j[2]].e				\
	  = max(ptable->window[cnode->j[2]].e, tt);		\
	ptable->occ[cnode->j[2]]++;				\
      }								\
    }								\
  }


  void _lag2_free_eliminated_nodes() {
    int t;
    unsigned int n_nodes;
    Benv *bnode;
    _node2m_t **node;
    _node2m_t *cnode, **pnode;
  
    n_nodes = prob->graph->n_nodes;
    bnode = prob->graph->bnode;
    node = (_node2m_t **) prob->graph->node2m;
    for (t = prob->graph->Tmin; t <= prob->graph->Tmax + 1; t++) {
      pnode = node + t;
      while (*pnode) {
          if ((*pnode)->ty & 4) {
              cnode = (*pnode)->next;
              bnode->free_element(*pnode);
              *pnode = cnode;
              n_nodes--;
          } else {
              pnode = &((*pnode)->next);
          }
      }
    }
    prob->graph->n_nodes = n_nodes;
  }
  
  int _lag2_shrink_horizon_head(double *u) {
      int t, tt;
      int i, j;
      int nhead;
      double f;
      char **inc;
      unsigned int n_nodes, n_edges;
      _ptable_t *ptable;
      _fixed_t *fixed;
      Benv *bnode, *bedge;
      _node2m_t **node, *cnode, *cnode2;
      _edge2m_t *e, *e2;
      node = (_node2m_t **) prob->graph->node2m;
      cnode = node[prob->graph->Tmin];
      if (!cnode) return SIPS_INFEASIBLE;
      n_nodes = prob->graph->n_nodes;
      n_edges = prob->graph->n_edges;
      bnode = prob->graph->bnode;
      bedge = prob->graph->bedge;
      ptable = prob->graph->ptable;
      inc = ptable->inc;
      fixed = prob->graph->fixed;
      nhead = fixed->nhead;
  
      e = cnode->e;
      f = cnode->v[0];
      t = prob->graph->Tmin;
      while (!e->next&& t < prob->graph->Tmax) {
          cnode = e->n;
          if (cnode->j[2] < prob->n) {
              t += prob->sjob[cnode->j[2]]->p;
              if (is_fixed(cnode->j[2])) {
                  prob->graph->n_nodes = n_nodes;
                  prob->graph->n_edges = n_edges;
                  return SIPS_INFEASIBLE;
              }
              for(tt = prob->graph->Tmin; tt <= prob->graph->Tmax; tt++) {
                  prec_set_dom_ti(tt, cnode->j[2]);
              }
              prec_rev_dom_ti(t, cnode->j[2]);
              ptable->window[cnode->j[2]].s = t;
              ptable->window[cnode->j[2]].e = t;
              ptable->occ[cnode->j[2]] = 1;
              fixed->job[fixed->nhead] = prob->sjob[cnode->j[2]];
              fixed->c[fixed->nhead++] = t;
              fixed->fhead += prob->sjob[cnode->j[2]]->f[t];
              f -= prob->sjob[cnode->j[2]]->f[t] - u[cnode->j[2]];
              set_fixed(cnode->j[2]);
          }
  
          if (cnode->j[1] < prob->n) {
              t += prob->sjob[cnode->j[1]]->p;
              if (is_fixed(cnode->j[1])) {
                  prob->graph->n_nodes = n_nodes;
                  prob->graph->n_edges = n_edges;
                  return SIPS_INFEASIBLE;
              }
              for(tt = prob->graph->Tmin; tt <= prob->graph->Tmax; tt++) {
                  prec_set_dom_ti(tt, cnode->j[1]);
              }
              prec_rev_dom_ti(t, cnode->j[1]);
              ptable->window[cnode->j[1]].s = t;
              ptable->window[cnode->j[1]].e = t;
              ptable->occ[cnode->j[1]] = 1;
              fixed->job[fixed->nhead] = prob->sjob[cnode->j[1]];
              fixed->c[fixed->nhead++] = t;
              fixed->fhead += prob->sjob[cnode->j[1]]->f[t];
              f -= prob->sjob[cnode->j[1]]->f[t] - u[cnode->j[1]];
              set_fixed(cnode->j[1]);
          }
          t += prob->sjob[cnode->j[0]]->p;
          if(is_fixed(cnode->j[0])) {
              prob->graph->n_nodes = n_nodes;
              prob->graph->n_edges = n_edges;
              return SIPS_INFEASIBLE;
          }
          for(tt = prob->graph->Tmin; tt <= prob->graph->Tmax; tt++) {
              prec_set_dom_ti(tt, cnode->j[0]);
          }
          prec_rev_dom_ti(t, cnode->j[0]);
          ptable->window[cnode->j[0]].s = t;
          ptable->window[cnode->j[0]].e = t;
          ptable->occ[cnode->j[0]] = 1;
          fixed->job[fixed->nhead] = prob->sjob[cnode->j[0]];
          fixed->c[fixed->nhead++] = t;
          fixed->fhead += prob->sjob[cnode->j[0]]->f[t];
          f -= prob->sjob[cnode->j[0]]->f[t] - u[cnode->j[0]];
          set_fixed(cnode->j[0]);
          e = cnode->e;
      }
  
      if(t > prob->graph->Tmin) {
          bedge->free_element(node[prob->graph->Tmin]->e);
          n_edges--;
          node[prob->graph->Tmin]->e = e;
          node[prob->graph->Tmin]->v[0] = f;
          node[prob->graph->Tmin]->n[0] = cnode->n[0];
          cnode->e = nullptr;
          for (tt = t; tt > prob->graph->Tmin; tt--) {
              for(cnode = node[tt]; cnode; cnode = cnode2) {
                  REMOVE_EDGES(cnode);
                  cnode2 = cnode->next;
                  bnode->free_element(cnode);
                  n_nodes--;
              }
              node[tt] = nullptr;
          }
          node[t] = node[prob->graph->Tmin];
          node[prob->graph->Tmin] = nullptr;
          prob->graph->Tmin = t;
      }
      prob->graph->n_nodes = n_nodes;
      prob->graph->n_edges = n_edges;
  
      if (t == prob->graph->Tmax && fixed->nhead + fixed->ntail != prob->n) {
          return SIPS_INFEASIBLE;
      }
  
      if (nhead < fixed->nhead) {
          for (i = nhead; i < fixed->nhead; i++) {
              for (j = 0; j < i; j++) {
                  if(inc[fixed->job[j]->no][fixed->job[i]->no] == -1) {
                      return SIPS_INFEASIBLE;
                  } else if(inc[fixed->job[j]->no][fixed->job[i]->no] != 1) {
                      _set_inc(fixed->job[j]->no, fixed->job[i]->no);
                  }
              }
              for (j = 0; j < prob->n; j++) {
                  if(j != fixed->job[i]->no && inc[fixed->job[i]->no][j] == 0) {
                      _set_inc(fixed->job[i]->no, j);
                  }
              }
          }
      }
  
      if (fixed->nhead + fixed->ntail == prob->n
          && prob->graph->Tmin == prob->graph->Tmax) {
          return SIPS_OPTIMAL;
      }
      return SIPS_NORMAL;
  }
  
  int _lag2_shrink_horizon_tail(double *u) {
      int t, tt;
      int i, j;
      int ntail;
      double f;
      char **inc;
      unsigned int n_nodes, n_edges;
      _ptable_t *ptable;
      _fixed_t *fixed;
      Benv *bnode, *bedge;
      _node2m_t **node, *cnode, *cnode2;
      _edge2m_t *e, *e2;
  
      node = (_node2m_t **) prob->graph->node2m;
      cnode = node[prob->graph->Tmax + 1];
      if (!cnode) {
          if(prob->graph->fixed->nhead + prob->graph->fixed->ntail != prob->n) {
          return SIPS_INFEASIBLE;
          } else {
          return SIPS_OPTIMAL;
          }
      }
  
      n_nodes = prob->graph->n_nodes;
      n_edges = prob->graph->n_edges;
      bnode = prob->graph->bnode;
      bedge = prob->graph->bedge;
      ptable = prob->graph->ptable;
      inc = ptable->inc;
      fixed = prob->graph->fixed;
      ntail = fixed->ntail;
  
      e = cnode->e;
      f = cnode->v[0];
      t = prob->graph->Tmax;
    while (!e->next && t > prob->graph->Tmin) {
      cnode = e->n;
  
        if (is_fixed(cnode->j[0])) {
          prob->graph->n_nodes = n_nodes;
          prob->graph->n_edges = n_edges;
          return(SIPS_INFEASIBLE);
        }
        for(tt = prob->graph->Tmin; tt <= prob->graph->Tmax; tt++) {
          prec_set_dom_ti(tt, cnode->j[0]);
        }
        prec_rev_dom_ti(t, cnode->j[0]);
        ptable->window[cnode->j[0]].s = t;
        ptable->window[cnode->j[0]].e = t;
        ptable->occ[cnode->j[0]] = 1;
        fixed->job[prob->n - fixed->ntail - 1] = prob->sjob[cnode->j[0]];
        fixed->c[prob->n - fixed->ntail - 1] = t;
        fixed->ntail++;
        fixed->ftail += prob->sjob[cnode->j[0]]->f[t];
        f -= prob->sjob[cnode->j[0]]->f[t] - u[cnode->j[0]];
        set_fixed(cnode->j[0]);
  
      t -= prob->sjob[cnode->j[0]]->p;
  
      if (cnode->j[1] < prob->n) {
          if(is_fixed(cnode->j[1])) {
            prob->graph->n_nodes = n_nodes;
            prob->graph->n_edges = n_edges;
            return(SIPS_INFEASIBLE);
          }
          for(tt = prob->graph->Tmin; tt <= prob->graph->Tmax; tt++) {
            prec_set_dom_ti(tt, cnode->j[1]);
          }
          prec_rev_dom_ti(t, cnode->j[1]);
          ptable->window[cnode->j[1]].s = t;
          ptable->window[cnode->j[1]].e = t;
          ptable->occ[cnode->j[1]] = 1;
          fixed->job[prob->n - fixed->ntail - 1] = prob->sjob[cnode->j[1]];
          fixed->c[prob->n - fixed->ntail - 1] = t;
          fixed->ntail++;
          fixed->ftail += prob->sjob[cnode->j[1]]->f[t];
          f -= prob->sjob[cnode->j[1]]->f[t] - u[cnode->j[1]];
          set_fixed(cnode->j[1]);
        t -= prob->sjob[cnode->j[1]]->p;
      }
  
      if (cnode->j[2] < prob->n) {
          if(is_fixed(cnode->j[2])) {
            prob->graph->n_nodes = n_nodes;
            prob->graph->n_edges = n_edges;
            return(SIPS_INFEASIBLE);
          }
          for(tt = prob->graph->Tmin; tt <= prob->graph->Tmax; tt++) {
            prec_set_dom_ti(tt, cnode->j[2]);
          }
          prec_rev_dom_ti(t, cnode->j[2]);
          ptable->window[cnode->j[2]].s = t;
          ptable->window[cnode->j[2]].e = t;
          ptable->occ[cnode->j[2]] = 1;
          fixed->job[prob->n - fixed->ntail - 1] = prob->sjob[cnode->j[2]];
          fixed->c[prob->n - fixed->ntail - 1] = t;
          fixed->ntail++;
          fixed->ftail += prob->sjob[cnode->j[2]]->f[t];
          f -= prob->sjob[cnode->j[2]]->f[t] - u[cnode->j[2]];
          set_fixed(cnode->j[2]);
        t -= prob->sjob[cnode->j[2]]->p;
      }
  
      e = cnode->e;
    }
  
    if(t < prob->graph->Tmax) {
      bedge->free_element(node[prob->graph->Tmax + 1]->e);
      n_edges--;
      node[prob->graph->Tmax + 1]->e = e;
      node[prob->graph->Tmax + 1]->n[0] = cnode->n[0];
      node[prob->graph->Tmax + 1]->v[0] = f;
      cnode->e = nullptr;
      for(tt = t + 1; tt <= prob->graph->Tmax; tt++) {
        for(cnode = node[tt]; cnode; cnode = cnode2) {
          REMOVE_EDGES(cnode);
          cnode2 = cnode->next;
          bnode->free_element(cnode);
          n_nodes--;
        }
        node[tt] = nullptr;
      }
      node[t + 1] = node[prob->graph->Tmax + 1];
      node[prob->graph->Tmax + 1] = nullptr;
      prob->graph->Tmax = t;
    }
  
    prob->graph->n_nodes = n_nodes;
    prob->graph->n_edges = n_edges;
  
    if (t == prob->graph->Tmin && fixed->nhead + fixed->ntail != prob->n) {
      return SIPS_INFEASIBLE;
    }
  
    if(ntail < fixed->ntail) {
      for(i = prob->n - ntail - 1; i > prob->n - fixed->ntail - 1; i--) {
        for(j = prob->n - 1; j > i; j--) {
          if(inc[fixed->job[i]->no][fixed->job[j]->no] == -1) {
            return SIPS_INFEASIBLE;
          } else if(inc[fixed->job[i]->no][fixed->job[j]->no] != 1) {
            _set_inc(fixed->job[i]->no, fixed->job[j]->no);
          }
        }
        for(j = 0; j < prob->n; j++) {
          if(j != fixed->job[i]->no && inc[j][fixed->job[i]->no] == 0) {
            _set_inc(j, fixed->job[i]->no);
          }
        }
      }
    }
  
      if (fixed->nhead + fixed->ntail == prob->n && prob->graph->Tmin == prob->graph->Tmax) {
          return SIPS_OPTIMAL;
      }

      return SIPS_NORMAL;
  }

int _lag2_add_modifiers_LR2m_forward(double *u, double ub,
				     _mod_t *mod, unsigned char eflag) {
  int t, tt;
  int i;
  int m, mm, mmax;
  char flag, ty;
  double f, g, g2;
  unsigned int n_nodes, n_edges;
  unsigned char *mtable;
  _ptable_t *ptable;
  Benv *bnode, *bedge;
  _node2m_t **node, *cnode, *cnode2, *cnode3, *cnode4, **pnode, **nnode;
  _edge2m_t *e, *e2, **pe;

  n_nodes = prob->graph->n_nodes;
  n_edges = prob->graph->n_edges;
  node = (_node2m_t **) prob->graph->node2m;
  ptable = prob->graph->ptable;
  bnode = prob->graph->bnode;
  bedge = prob->graph->bedge;

  mtable
    = new unsigned char[prob->graph->Tmax - prob->graph->Tmin + 1]{};
  for(i = 0; i < mod->an; i++) {
    m = 1<<i;
    for(t = max(prob->graph->Tmin, ptable->window[mod->jobs[i]].e
		- prob->sjob[mod->jobs[i]]->p + 1);
	t <= prob->graph->Tmax; t++) {
      mtable[t - prob->graph->Tmin] |= m;
    }
  }

  memset(ptable->occ, 0, (prob->n + 2)*sizeof(unsigned int));
  for(i = 0; i < prob->n; i++) {
    ptable->window[i].s = prob->graph->Tmax + 1;
    ptable->window[i].e = prob->graph->Tmin;
  }
  for(i = 0; i < prob->graph->fixed->nhead; i++) {
    ptable->window[prob->graph->fixed->job[i]->no].s
      = ptable->window[prob->graph->fixed->job[i]->no].e
      = prob->graph->fixed->c[i];
  }
  for(i = 0; i < prob->graph->fixed->ntail; i++) {
    ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].s
      = ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].e
      = prob->graph->fixed->c[prob->n - i - 1];
  }

  mmax = (1<<mod->an) - 1;
  nnode = new _node2m_t*[mmax + 1];

  node[prob->graph->Tmin]->m = 0;
  node[prob->graph->Tmin]->n[0] = node[prob->graph->Tmin];
  node[prob->graph->Tmin]->ty = 0;
  node[prob->graph->Tmin]->v[0] = prob->graph->fixed->fhead;
  for(i = 0; i < prob->graph->fixed->nhead; i++) {
    node[prob->graph->Tmin]->v[0] -= u[prob->graph->fixed->job[i]->no];
  }
  node[prob->graph->Tmin]->ty |= 8;
  node[prob->graph->Tmax + 1]->m = mmax;
  for (t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
    pnode = node + t;
    while (*pnode) {
      cnode = *pnode;
      if (cnode->ty & 4) {
	      REMOVE_EDGES(cnode);
	      pnode = &(cnode->next);
	      continue;
      }
      if (!cnode->e) {
	      cnode->ty |= 4;
	      pnode = &(cnode->next);
	      continue;
      }

      if (prec_get_dom_ti(t, cnode->j[0])) {
	      REMOVE_EDGES(cnode);
	      cnode->ty |= 4;
	      pnode = &(cnode->next);
	      continue;
      }

      g = - u[cnode->j[0]] + prob->sjob[cnode->j[0]]->f[t];
      mm = mod->m[cnode->j[0]];
      if (cnode->j[1] < prob->n) {
        tt = t - prob->sjob[cnode->j[0]]->p;
        if(prec_get_dom_ti(tt, cnode->j[1])) {
          REMOVE_EDGES(cnode);
          cnode->ty |= 4;
          pnode = &(cnode->next);
          continue;
        }
        g += - u[cnode->j[1]] + prob->sjob[cnode->j[1]]->f[tt];
        mm |= mod->m[cnode->j[1]];

        if (cnode->j[2] < prob->n) {
          tt -= prob->sjob[cnode->j[1]]->p;
          if(prec_get_dom_ti(tt, cnode->j[2])) {
            REMOVE_EDGES(cnode);
            cnode->ty |= 4;
            pnode = &(cnode->next);
            continue;
          }
          g += - u[cnode->j[2]] + prob->sjob[cnode->j[2]]->f[tt];
          mm |= mod->m[cnode->j[2]];
        }
      }

      for(m = mm; m <= mmax; m++) {
        nnode[m] = cnode2 = static_cast<_node2m_t*>(bnode->alloc_element());
        n_nodes++;

        cnode2->m = m;
        cnode2->j[0] = cnode->j[0];
        cnode2->j[1] = cnode->j[1];
        cnode2->j[2] = cnode->j[2];
        cnode2->e = nullptr;
        cnode2->next = nullptr;
        cnode2->v[0] = cnode2->v[1] = dinf;
        cnode2->n[0] = cnode2->n[1] = nullptr;
        cnode2->ty = 0;
      }

      e = cnode->e;
      while(e != NULL) {
        cnode2 = e->n;
        if (cnode2->ty & 4) {
          e2 = e->next;
          bedge->free_element(e);
          n_edges--;
          e = e2;
          continue;
        }

	for(flag = 0; cnode2; cnode2 = cnode2->next) {
	  if (cnode2->ty & 12) {
	    if (++flag == 2) break;
	  }

	  if (cnode2->m & mm) {
	    continue;
	  }

	  m = cnode2->m | mod->m[cnode->j[2]];
	  if(mod->v[cnode->j[2]][m] == 1) {
	    continue;
	  }
	  m |= mod->m[cnode->j[1]];
	  if(mod->v[cnode->j[1]][m] == 1) {
	    continue;
	  }
	  m |= mod->m[cnode->j[0]];
	  if(mod->v[cnode->j[0]][m] == 1) {
	    continue;
	  }
	  if((~m)&mtable[t - prob->graph->Tmin]) {
	    continue;
	  }
	  if(HAVE_COMMON_REAL_JOB(cnode, cnode2)) {
	    continue;
	  }

	    cnode3 = cnode2->n[0];
	    if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
            if (!cnode2->n[1]) {
                continue;
            } else if (!cnode2->e->next->next) {
                cnode3 = cnode2->n[1];
                if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
                    continue;
                }
            }
            ty = 1;
            f = cnode2->v[1];
        } else {
            ty = 0;
            f = cnode2->v[0];
        }

	  if(eflag) {
	    if(ub - (f + cnode->v[0]) < 1. - eps) {
	      continue;
	    } else if(ub - (f + cnode->v[1]) < 1. - eps) {
            cnode3 = cnode->n[0];
            if(HAVE_COMMON_REAL_JOB(cnode2, cnode3)) {
                continue;
            }
	    }
	    if(_check_adj_supernode_forward(cnode2, cnode, t)) {
	      continue;
	    }
	  }

	  f += g;
	  cnode3 = nnode[m];
	  if(f < cnode3->v[0]) {
	    cnode3->v[1] = cnode3->v[0];
	    cnode3->v[0] = f;
	    cnode3->n[1] = cnode3->n[0];
	    cnode3->n[0] = cnode2;
	    cnode3->ty <<= 1;
	    cnode3->ty |= ty;
	  } else if(f < cnode3->v[1]) {
	    cnode3->v[1] = f;
	    cnode3->n[1] = cnode2;
	    cnode3->ty &= 0x1;
	    cnode3->ty |= ty << 1;
	  }

      e2 = static_cast<_edge2m_t*>(bedge->alloc_element());

	  n_edges++;
	  e2->n = cnode2;
	  e2->next = cnode3->e;
	  cnode3->e = e2;
	}

	e2 = e->next;
	bedge->free_element(e);
	n_edges--;
	e = e2;
      }

      for (m = mm; m <= mmax; m++) {
        if (!nnode[m]->e) {
            bnode->free_element(nnode[m]);
          n_nodes--;
          nnode[m] = nullptr;
          continue;
        }

        cnode2 = nnode[m];
        cnode3 = cnode2->e->n;
	if(eflag && !cnode2->e->next && cnode3->j[0] < prob->n
	   && cnode2->j[2] > prob->n && cnode3->j[2] > prob->n) {
	  if (cnode2->j[1] < prob->n) {
	    if (cnode3->j[1] > prob->n) {
	      cnode2->j[2] = cnode3->j[0];
	    } else {
	      continue;
	    }
	  } else {
	    cnode2->j[1] = cnode3->j[0];
	    cnode2->j[2] = cnode3->j[1];
	  }

	  bedge->free_element(cnode2->e);
	  n_edges--;

	  if (cnode2->j[2] < prob->n && _check_supernode(cnode2->j, t)) {
        bnode->free_element(cnode2);
	    n_nodes--;
	    nnode[m] = NULL;
	    continue;
	  }

	  g2 = - u[cnode2->j[0]] + prob->sjob[cnode2->j[0]]->f[t];
	  if (cnode2->j[1] < prob->n) {
	    tt = t - prob->sjob[cnode2->j[0]]->p;
	    g2 += - u[cnode2->j[1]] + prob->sjob[cnode2->j[1]]->f[tt];
	    if (cnode2->j[2] < prob->n) {
	      tt -= prob->sjob[cnode2->j[1]]->p;
	      g2 += - u[cnode2->j[2]] + prob->sjob[cnode2->j[2]]->f[tt];
	    }
	  }

	  cnode2->v[0] = cnode2->v[1] = dinf;
	  cnode2->n[0] = cnode2->n[1] = nullptr;
	  cnode2->ty = 0;
	  pe = &(cnode2->e);

	  for(e = cnode3->e; e; e = e->next) {
	    cnode3 = e->n;
	    if(HAVE_REAL_JOB(cnode2->j[2], cnode3)
	       || HAVE_JOB(cnode2->j[1], cnode3)
	       || HAVE_JOB(cnode2->j[0], cnode3)) {
	      continue;
	    }

	    cnode4 = cnode3->n[0];
	    if(HAVE_JOB(cnode2->j[0], cnode4)
	       || HAVE_JOB(cnode2->j[1], cnode4)
	       || HAVE_REAL_JOB(cnode2->j[2], cnode4)) {
	      if (!cnode3->n[1]) {
		continue;
	      } else if (!cnode3->e->next->next) {
		cnode4 = cnode3->n[1];
		if(HAVE_JOB(cnode2->j[0], cnode4)
		   || HAVE_JOB(cnode2->j[1], cnode4)
		   || HAVE_REAL_JOB(cnode2->j[2], cnode4)) {
		  continue;
		}
	      }
	      ty = 1;
	      f = cnode3->v[1] + g2;
	    } else {
	      ty = 0;
	      f = cnode3->v[0] + g2;
	    }

	    if(f < cnode2->v[0]) {
	      cnode2->v[1] = cnode2->v[0];
	      cnode2->v[0] = f;
	      cnode2->n[1] = cnode2->n[0];
	      cnode2->n[0] = cnode3;
	      cnode2->ty <<= 1;
	      cnode2->ty |= ty;
	    } else if(f < cnode2->v[1]) {
	      cnode2->v[1] = f;
	      cnode2->n[1] = cnode3;
	      cnode2->ty &= 0x1;
	      cnode2->ty |= ty << 1;
	    }
        e2 = static_cast<_edge2m_t*>(bedge->alloc_element());
	    n_edges++;
	    e2->n = cnode3;
	    *pe = e2;
	    pe = &(e2->next);
	  }
	  *pe = NULL;

	  if (!cnode2->e) {
	    bnode->free_element(cnode2);
	    n_nodes--;
	    nnode[m] = nullptr;
	  }
	}
      }

      for(m = mm; m <= mmax && !nnode[m]; m++);

      if(m <= mmax) {
	cnode2 = cnode->next;
	*cnode = *(nnode[m]);
	cnode->ty &= 3;
    cnode->ty |= 8;

	UPDATE_WINDOWS;

	pnode = &(cnode->next);
	bnode->free_element(nnode[m]);
	n_nodes--;

	for(m++; m <= mmax; m++) {
	  if (nnode[m]) {
	    ptable->occ[nnode[m]->j[0]]++;
	    ptable->occ[nnode[m]->j[1]]++;
	    ptable->occ[nnode[m]->j[2]]++;
	    *pnode = nnode[m];
	    (*pnode)->ty &= 3;
	    pnode = &(nnode[m]->next);
	  }
	}

	*pnode = cnode2;
      } else {
        cnode->e = nullptr;
        cnode->ty |= 4;
        pnode = &(cnode->next);
      }
    }

    if(t >= prob->graph->Tmin + 3*prob->pmax) {
      pnode = node + (t - 3*prob->pmax);
      while(*pnode) {
	if ((*pnode)->ty & 4) {
	  cnode = (*pnode)->next;
      bnode->free_element(*pnode);
	  *pnode = cnode;
	  n_nodes--;
	} else {
	  (*pnode)->ty &= 3;
	  pnode = &((*pnode)->next);
	}
      }
    }
  }
  delete[] nnode;
  delete[] mtable;

  cnode = node[prob->graph->Tmax + 1];
  cnode->v[0] = cnode->v[1] = dinf;
  cnode->n[0] = cnode->n[1] = nullptr;
  cnode->ty = 0;
  pe = &(cnode->e);
  while (*pe) {
    cnode2 = (*pe)->n;
    if (cnode2->ty & 4) {
      REMOVE_SINGLE_EDGE;
      continue;
    }
    if(cnode2->v[0] < cnode->v[0]) {
      cnode->v[0] = cnode2->v[0];
      cnode->n[0] = cnode2;
    }
    pe = &((*pe)->next);
  }

  if (!cnode->e) {
    cnode->ty |= 4;
    prob->graph->n_nodes = n_nodes;
    prob->graph->n_edges = n_edges;
    return SIPS_INFEASIBLE;
  }

  for(t = max(prob->graph->Tmin, prob->graph->Tmax - 3*prob->pmax + 1);
      t <= prob->graph->Tmax; t++) {
    pnode = node + t;
    while (*pnode) {
      if ((*pnode)->ty & 4) {
        cnode = (*pnode)->next;
        bnode->free_element(*pnode);
        *pnode = cnode;
        n_nodes--;
      } else {
	    (*pnode)->ty &= 3;
	pnode = &((*pnode)->next);
      }
    }
  }

  prob->graph->n_nodes = n_nodes;
  prob->graph->n_edges = n_edges;

  return _lag2_shrink_horizon_tail(u);
}

#undef UPDATE_WINDOWS
#define UPDATE_WINDOWS {					\
    ptable->window[cnode->j[0]].s				\
      = min(ptable->window[cnode->j[0]].s, t);			\
    ptable->window[cnode->j[0]].e				\
      = max(ptable->window[cnode->j[0]].e, t);			\
    ptable->occ[cnode->j[0]]++;					\
    if (cnode->j[1] < prob->n) {					\
      ptable->window[cnode->j[1]].s				\
	= min(ptable->window[cnode->j[1]].s, tt[0]);		\
      ptable->window[cnode->j[1]].e				\
	= max(ptable->window[cnode->j[1]].e, tt[0]);		\
      ptable->occ[cnode->j[1]]++;				\
      if(cnode->j[2] < prob->n) {					\
	ptable->window[cnode->j[2]].s				\
	  = min(ptable->window[cnode->j[2]].s, tt[1]);		\
	ptable->window[cnode->j[2]].e				\
	  = max(ptable->window[cnode->j[2]].e, tt[1]);		\
	ptable->occ[cnode->j[2]]++;				\
      }								\
    }								\
  }

int _lag2_add_modifiers_LR2m_backward(double *u, double ub,
				      _mod_t *mod, unsigned char eflag)
{
  int t, s, tt[2];
  int i;
  int psum;
  int m, m2, mm, mmax;
  char ty, flag;
  double f, g;
  unsigned int n_nodes, n_edges;
  unsigned char *mtable;
  _ptable_t *ptable;
  Benv *bnode, *bedge;
  _node2m_t **node, *cnode, *cnode2, *cnode3, **pnode, **nnode;
  _edge2m_t *e, *e2, **pe;

  n_nodes = prob->graph->n_nodes;
  n_edges = prob->graph->n_edges;
  node = (_node2m_t **) prob->graph->node2m;
  ptable = prob->graph->ptable;
  bnode = prob->graph->bnode;
  bedge = prob->graph->bedge;

  mtable = new unsigned char[prob->graph->Tmax - prob->graph->Tmin + 1]{};
  for(i = 0; i < mod->an; i++) {
    m = 1<<i;
    for(t = prob->graph->Tmin; t <= ptable->window[mod->jobs[i]].s - 1
	  && t <= prob->graph->Tmax; t++) {
      mtable[t - prob->graph->Tmin] |= m;
    }
  }
  memset(ptable->occ, 0, (prob->n + 2)*sizeof(unsigned int));
  for (i = 0; i < prob->n; i++) {
    ptable->window[i].s = prob->graph->Tmax + 1;
    ptable->window[i].e = prob->graph->Tmin;
  }
  for (i = 0; i < prob->graph->fixed->nhead; i++) {
    ptable->window[prob->graph->fixed->job[i]->no].s
      = ptable->window[prob->graph->fixed->job[i]->no].e
      = prob->graph->fixed->c[i];
  }
  for (i = 0; i < prob->graph->fixed->ntail; i++) {
    ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].s
      = ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].e
      = prob->graph->fixed->c[prob->n - i - 1];
  }

  mmax = (1<<mod->an) - 1;
  nnode = new _node2m_t *[mmax + 1];

  node[prob->graph->Tmin]->m = 0;
  node[prob->graph->Tmax + 1]->m = mmax;
  node[prob->graph->Tmax + 1]->n[0] = node[prob->graph->Tmax + 1];
  node[prob->graph->Tmax + 1]->ty = 0;
  node[prob->graph->Tmax + 1]->v[0] = prob->graph->fixed->ftail;
  for(i = 0; i < prob->graph->fixed->ntail; i++) {
    node[prob->graph->Tmax + 1]->v[0] -= u[prob->graph->fixed->job[prob->n - i - 1]->no];
  }
    node[prob->graph->Tmax + 1]->ty |= 8;

  for(t = prob->graph->Tmax; t > prob->graph->Tmin; t--) {
    pnode = node + t;
    while (*pnode) {
      cnode = *pnode;
      if (cnode->ty & 4) {
        REMOVE_EDGES(cnode);
        pnode = &(cnode->next);
        continue;
      }
      if (!cnode->e) {
        cnode->ty |= 4;
        pnode = &(cnode->next);
        continue;
      }

      if (prec_get_dom_ti(t, cnode->j[0])) {
        REMOVE_EDGES(cnode);
        cnode->ty |= 4;
        pnode = &(cnode->next);
        continue;
      }

      g = - u[cnode->j[0]] + prob->sjob[cnode->j[0]]->f[t];
      psum = prob->sjob[cnode->j[0]]->p;
      mm = mod->m[cnode->j[0]];

      tt[0] = tt[1] = t;
      s = t - prob->sjob[cnode->j[0]]->p;
      if (cnode->j[1] < prob->n) {
	if (prec_get_dom_ti(s, cnode->j[1])) {
	  REMOVE_EDGES(cnode);
	  cnode->ty |= 4;
	  pnode = &(cnode->next);
	  continue;
	}
	g += - u[cnode->j[1]] + prob->sjob[cnode->j[1]]->f[s];
	psum += prob->sjob[cnode->j[1]]->p;
	mm |= mod->m[cnode->j[1]];
	tt[0] = s;
	s -= prob->sjob[cnode->j[1]]->p;

	if (cnode->j[2] < prob->n) {
	  if (prec_get_dom_ti(s, cnode->j[2])) {
	    REMOVE_EDGES(cnode);
	    cnode->ty |= 4;
	    pnode = &(cnode->next);
	    continue;
	  }
	  g += - u[cnode->j[2]] + prob->sjob[cnode->j[2]]->f[s];
	  psum += prob->sjob[cnode->j[2]]->p;
	  mm |= mod->m[cnode->j[2]];
	  tt[1] = s;
	  s -= prob->sjob[cnode->j[2]]->p;
	}
      }

      for(m = mm; m <= mmax; m++) {
    nnode[m] = cnode2 = static_cast<_node2m_t*>(bnode->alloc_element());

	n_nodes++;

	cnode2->m = m;
	cnode2->j[0] = cnode->j[0];
	cnode2->j[1] = cnode->j[1];
	cnode2->j[2] = cnode->j[2];
	cnode2->e = nullptr;
	cnode2->next = nullptr;
	cnode2->v[0] = cnode2->v[1] = dinf;
	cnode2->n[0] = cnode2->n[1] = nullptr;
	cnode2->ty = 0;
      }

      e = cnode->e;
      while (e) {
	cnode2 = e->n;
	if (cnode2->ty & 4) {
	  e2 = e->next;
	  bedge->free_element(e);
	  n_edges--;
	  e = e2;
	  continue;
	}

	for (flag = 0; cnode2; cnode2 = cnode2->next) {
	  if (cnode2->ty & 12) {
	    if(++flag == 2) {
	      break;
	    }
	  }

	  m2 = mod->m[cnode2->j[0]] | mod->m[cnode2->j[1]]
	    | mod->m[cnode2->j[2]];
	  m = cnode2->m ^ m2;
	  if((m & mm) != mm) {
	    continue;
	  }
	  if(mod->v[cnode->j[0]][m] == 1) {
	    continue;
	  }
	  m2 = m ^ mod->m[cnode->j[0]];
	  if(mod->v[cnode->j[1]][m2] == 1) {
	    continue;
	  }
	  m2 ^= mod->m[cnode->j[1]];
	  if(mod->v[cnode->j[2]][m2] == 1) {
	    continue;
	  }
	  m2 ^= mod->m[cnode->j[2]];
	  if(m2 & mtable[s - prob->graph->Tmin]) {
	    continue;
	  }
	  cnode3 = cnode2->n[0];
	  if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
	    if(cnode2->n[1] == NULL) {
	      continue;
	    } else if (!cnode2->e->next->next) {
	      cnode3 = cnode2->n[1];
	      if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
		    continue;
	      }
	    }
	    ty = 1;
	    f = cnode2->v[1];
	  } else {
	    ty = 0;
	    f = cnode2->v[0];
	  }

	  if(eflag) {
	    if(ub - (f + cnode->v[0]) < 1. - eps) {
	      continue;
	    } else if(ub - (f + cnode->v[1]) < 1. - eps) {
            cnode3 = cnode->n[0];
            if(HAVE_COMMON_REAL_JOB(cnode2, cnode3)) {
                continue;
            }
	    }
	    if(_check_adj_supernode_backward(cnode, cnode2, t)) {
	      continue;
	    }
	  }

	  f += g;
	  cnode3 = nnode[m];
	  if(f < cnode3->v[0]) {
	    cnode3->v[1] = cnode3->v[0];
	    cnode3->v[0] = f;
	    cnode3->n[1] = cnode3->n[0];
	    cnode3->n[0] = cnode2;
	    cnode3->ty <<= 1;
	    cnode3->ty |= ty;
	  } else if(f < cnode3->v[1]) {
	    cnode3->v[1] = f;
	    cnode3->n[1] = cnode2;
	    cnode3->ty &= 1;
	    cnode3->ty |= ty << 1;
	  }
      e2= static_cast<_edge2m_t*>(bedge->alloc_element());
	  n_edges++;
	  e2->n = cnode2;
	  e2->next = cnode3->e;
	  cnode3->e = e2;
	}

	e2 = e->next;
	bedge->free_element(e);
	n_edges--;
	e = e2;
      }

      for(m = mm; m <= mmax; m++) {
        if (!nnode[m]->e) {
            bnode->free_element(nnode[m]);
            n_nodes--;
        } else {
            break;
        }
      }

      if(m <= mmax) {
	cnode2 = cnode->next;
	*cnode = *(nnode[m]);

	UPDATE_WINDOWS;

	    cnode->ty &= 3;
	    cnode->ty |= 8;
	    pnode = &(cnode->next);
        bnode->free_element(nnode[m]);
	    n_nodes--;

	for(m++; m <= mmax; m++) {
	  if (!nnode[m]->e) {
	    bnode->free_element(nnode[m]);
	    n_nodes--;
	  } else {
	    ptable->occ[nnode[m]->j[0]]++;
	    ptable->occ[nnode[m]->j[1]]++;
	    ptable->occ[nnode[m]->j[2]]++;
	    *pnode = nnode[m];
	        (*pnode)->ty &= 3;
	    pnode = &(nnode[m]->next);
	  }
	}

	*pnode = cnode2;
      } else {
        cnode->e = nullptr;
        cnode->ty |= 4;
        pnode = &(cnode->next);
      }
    }

    if(t + 3*prob->pmax <= prob->graph->Tmax + 1) {
      pnode = node + (t + 3*prob->pmax);
      while (*pnode) {
        if ((*pnode)->ty & 4) {
            cnode = (*pnode)->next;
            bnode->free_element(*pnode);
            *pnode = cnode;
            n_nodes--;
        } else {
            (*pnode)->ty &= 3;
            pnode = &((*pnode)->next);
        }
      }
    }
  }

  delete[] nnode;
  delete[] mtable;

  cnode = node[prob->graph->Tmin];
  cnode->v[0] = cnode->v[1] = dinf;
  cnode->n[0] = cnode->n[1] = nullptr;
  cnode->ty = 0;
  pe = &(cnode->e);
  while (*pe) {
    cnode2 = (*pe)->n;
    if (cnode2->ty & 4) {
      REMOVE_SINGLE_EDGE;
      continue;
    }
    if(cnode2->v[0] < cnode->v[0]) {
      cnode->v[0] = cnode2->v[0];
      cnode->n[0] = cnode2;
    }
    pe = &((*pe)->next);
  }

  if (!cnode->e) {
    prob->graph->n_nodes = n_nodes;
    prob->graph->n_edges = n_edges;
    return SIPS_INFEASIBLE;
  }

  for(t = min(prob->graph->Tmax, prob->graph->Tmin + 3*prob->pmax);
      t >= prob->graph->Tmin; t--) {
    pnode = node + t;
    while (*pnode) {
        if ((*pnode)->ty & 4) {
            cnode = (*pnode)->next;
            bnode->free_element(*pnode);
            *pnode = cnode;
            n_nodes--;
        } else {
            (*pnode)->ty &= 3;
            pnode = &((*pnode)->next);
        }
    }
  }
  prob->graph->n_nodes = n_nodes;
  prob->graph->n_edges = n_edges;
  return _lag2_shrink_horizon_head(u);
}

int _lag2_solve_LR2m_forward(double *u, double ub,
  unsigned char eflag)
{
int t, tt[2];
int i;
char ty, ty1, ty2;
double f, g;
double v1, v2;
unsigned int n_edges;
_ptable_t *ptable;
Benv *bedge;
_node2m_t **node, *cnode, *cnode2, *cnode3, **pnode, *pr1, *pr2;
_edge2m_t *e, *e2, **pe;

n_edges = prob->graph->n_edges;
node = (_node2m_t **) prob->graph->node2m;
ptable = prob->graph->ptable;
bedge = prob->graph->bedge;

memset(ptable->occ, 0, (prob->n + 2)*sizeof(unsigned int));
for(i = 0; i < prob->n; i++) {
ptable->window[i].s = prob->graph->Tmax + 1;
ptable->window[i].e = prob->graph->Tmin;
}
for(i = 0; i < prob->graph->fixed->nhead; i++) {
ptable->window[prob->graph->fixed->job[i]->no].s
= ptable->window[prob->graph->fixed->job[i]->no].e
= prob->graph->fixed->c[i];
}
for(i = 0; i < prob->graph->fixed->ntail; i++) {
ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].s
= ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].e
= prob->graph->fixed->c[prob->n - i - 1];
}

node[prob->graph->Tmin]->v[0] = prob->graph->fixed->fhead;
for(i = 0; i < prob->graph->fixed->nhead; i++) {
node[prob->graph->Tmin]->v[0] -= u[prob->graph->fixed->job[i]->no];
}
node[prob->graph->Tmin]->n[0] = node[prob->graph->Tmin];
node[prob->graph->Tmin]->ty = 0;

for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
for(pnode = node + t; *pnode; pnode = &(cnode->next)) {
cnode = *pnode;
if (cnode->ty & 4) {
REMOVE_EDGES(cnode);
continue;
}

if (prec_get_dom_ti(t, cnode->j[0])) {
REMOVE_EDGES(cnode);
 cnode->ty |= 4;
continue;
}
g = - u[cnode->j[0]] + prob->sjob[cnode->j[0]]->f[t];

tt[0] = tt[1] = t;
if (cnode->j[1] < prob->n) {
tt[0] -= prob->sjob[cnode->j[0]]->p;
if(prec_get_dom_ti(tt[0], cnode->j[1])) {
 REMOVE_EDGES(cnode);
     cnode->ty |= 4;
 continue;
}
g += - u[cnode->j[1]] + prob->sjob[cnode->j[1]]->f[tt[0]];

if (cnode->j[2] < prob->n) {
 tt[1] = tt[0] - prob->sjob[cnode->j[1]]->p;
 if (prec_get_dom_ti(tt[1], cnode->j[2])) {
   REMOVE_EDGES(cnode);
       cnode->ty |= 4;
   continue;
 }
 g += - u[cnode->j[2]] + prob->sjob[cnode->j[2]]->f[tt[1]];
}
}

v1 = v2 = dinf;
pr1 = pr2 = nullptr;
ty1 = ty2 = 0;

pe = &(cnode->e);
while (*pe) {
cnode2 = (*pe)->n;
if ((cnode2->ty & 4) || HAVE_COMMON_REAL_JOB(cnode, cnode2)) {
 REMOVE_SINGLE_EDGE;
 continue;
}

cnode3 = cnode2->n[0];
if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
 if (!cnode2->n[1]) {
   REMOVE_SINGLE_EDGE;
   continue;
 } else if (!cnode2->e->next->next) {
   cnode3 = cnode2->n[1];
   if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
     REMOVE_SINGLE_EDGE;
     continue;
   }
 }
 ty = 1;
 f = cnode2->v[1];
} else {
 ty = 0;
 f = cnode2->v[0];
}

if(eflag) {
 if(ub - (f + cnode->v[0]) < 1. - eps) {
   REMOVE_SINGLE_EDGE;
   continue;
 } else if(ub - (f + cnode->v[1]) < 1. - eps) {
   cnode3 = cnode->n[0];
   if(HAVE_COMMON_REAL_JOB(cnode2, cnode3)) {
     REMOVE_SINGLE_EDGE;
     continue;
   }
 }

 if(_check_adj_supernode_forward(cnode2, cnode, t)) {
   REMOVE_SINGLE_EDGE;
   continue;
 }
}

f += g;
if(f < v1) {
 v2 = v1;
 v1 = f;
 pr2 = pr1;
 pr1 = cnode2;
 ty2 = ty1;
 ty1 = ty;
} else if(f < v2) {
 v2 = f;
 pr2 = cnode2;
 ty2 = ty;
}

pe = &((*pe)->next);
}

if (!pr1) {
cnode->ty |= 4;
continue;
}

cnode->v[0] = v1;
cnode->v[1] = v2;
cnode->ty = CONV_TYPE(ty1, ty2);
cnode->n[0] = pr1;
cnode->n[1] = pr2;

cnode2 = cnode->e->n;
if(eflag && !cnode->e->next && cnode2->j[0] < prob->n && cnode->j[2] > prob->n && cnode2->j[2] > prob->n) {

if (cnode->j[1] < prob->n) {
if(cnode2->j[1] > prob->n) {
cnode->j[2] = cnode2->j[0];
} else {
goto _forward_next;
}
} else {
cnode->j[1] = cnode2->j[0];
cnode->j[2] = cnode2->j[1];
}

bedge->free_element(cnode->e);
n_edges--;

if(cnode->j[2] < prob->n && _check_supernode(cnode->j, t)) {
cnode->ty |= 4;
continue;
}

g = - u[cnode->j[0]] + prob->sjob[cnode->j[0]]->f[t];
tt[0] = t - prob->sjob[cnode->j[0]]->p;
g += - u[cnode->j[1]] + prob->sjob[cnode->j[1]]->f[tt[0]];
if (cnode->j[2] < prob->n) {
tt[1] = tt[0] - prob->sjob[cnode->j[1]]->p;
g += - u[cnode->j[2]] + prob->sjob[cnode->j[2]]->f[tt[1]];
}

v1 = v2 = dinf;
pr1 = pr2 = nullptr;
ty1 = ty2 = 0;
pe = &(cnode->e);
for(e = cnode2->e; e; e = e->next) {
cnode2 = e->n;
if(HAVE_REAL_JOB(cnode->j[2], cnode2)
|| HAVE_JOB(cnode->j[1], cnode2)
|| HAVE_JOB(cnode->j[0], cnode2)) {
continue;
}

cnode3 = cnode2->n[0];
if(HAVE_JOB(cnode->j[0], cnode3)
|| HAVE_JOB(cnode->j[1], cnode3)
|| HAVE_REAL_JOB(cnode->j[2], cnode3)) {
if (!cnode2->n[1]) {
continue;
} else if (!cnode2->e->next->next) {
cnode3 = cnode2->n[1];
if(HAVE_JOB(cnode->j[0], cnode3)
|| HAVE_JOB(cnode->j[1], cnode3)
|| HAVE_REAL_JOB(cnode->j[2], cnode3)) {
continue;
}
}
ty = 1;
f = cnode2->v[1] + g;
} else {
ty = 0;
f = cnode2->v[0] + g;
}

if(f < v1) {
v2 = v1;
v1 = f;
pr2 = pr1;
pr1 = cnode2;
ty2 = ty1;
ty1 = ty;
} else if(f < v2) {
v2 = f;
pr2 = cnode2;
ty2 = ty;
}
e2 = static_cast<_edge2m_t*>(bedge->alloc_element());

n_edges++;
e2->n = cnode2;
*pe = e2;
pe = &(e2->next);
}
*pe = nullptr;

if(!cnode->e) {
cnode->ty |= 4;
continue;
}

cnode->v[0] = v1;
cnode->v[1] = v2;
cnode->ty = CONV_TYPE(ty1, ty2);
cnode->n[0] = pr1;
cnode->n[1] = pr2;
}

_forward_next:
UPDATE_WINDOWS;
}
}

cnode = node[prob->graph->Tmax + 1];
cnode->v[0] = cnode->v[1] = dinf;
cnode->n[0] = cnode->n[1] = nullptr;
cnode->ty = 0;
pe = &(cnode->e);
while (*pe) {
cnode2 = (*pe)->n;
if (cnode2->ty & 4) {
REMOVE_SINGLE_EDGE;
continue;
}

if(cnode2->v[0] < cnode->v[0]) {
cnode->v[0] = cnode2->v[0];
cnode->n[0] = cnode2;
}
pe = &((*pe)->next);
}

prob->graph->n_edges = n_edges;

if (!cnode->e) return SIPS_INFEASIBLE;
_lag2_free_eliminated_nodes();
return(_lag2_shrink_horizon_tail(u));
}

int _lag2_solve_LR2m_backward(double *u, double ub, unsigned char eflag) {
  int t, tt[2];
  int i;
  char ty, ty1, ty2;
  double f, g;
  double v1, v2;
  unsigned int n_edges;
  _ptable_t *ptable;
  Benv* bedge;
  _node2m_t **node, *cnode, *cnode2, *cnode3, **pnode, *pr1, *pr2;
  _edge2m_t *e, *e2, **pe;

  n_edges = prob->graph->n_edges;
  node = (_node2m_t **) prob->graph->node2m;
  ptable = prob->graph->ptable;
  bedge = prob->graph->bedge;

  memset(ptable->occ, 0, (prob->n + 2)*sizeof(unsigned int));
  for(i = 0; i < prob->n; i++) {
  ptable->window[i].s = prob->graph->Tmax + 1;
  ptable->window[i].e = prob->graph->Tmin;
  }
  for(i = 0; i < prob->graph->fixed->nhead; i++) {
  ptable->window[prob->graph->fixed->job[i]->no].s
  = ptable->window[prob->graph->fixed->job[i]->no].e
  = prob->graph->fixed->c[i];
  }
  for(i = 0; i < prob->graph->fixed->ntail; i++) {
  ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].s
  = ptable->window[prob->graph->fixed->job[prob->n - i - 1]->no].e
  = prob->graph->fixed->c[prob->n - i - 1];
  }

  node[prob->graph->Tmax + 1]->v[0]  = prob->graph->fixed->ftail;
  for(i = 0; i < prob->graph->fixed->ntail; i++) {
  node[prob->graph->Tmax + 1]->v[0]
  -= u[prob->graph->fixed->job[prob->n - i - 1]->no];
  }
  node[prob->graph->Tmax + 1]->n[0] = node[prob->graph->Tmax + 1];
  node[prob->graph->Tmax + 1]->ty = 0;

  for(t = prob->graph->Tmax; t > prob->graph->Tmin; t--) {
  for(pnode = node + t; *pnode; pnode = &(cnode->next)) {
  cnode = *pnode;
  if (cnode->ty & 4) {
  REMOVE_EDGES(cnode);
  continue;
  }

  if (prec_get_dom_ti(t, cnode->j[0])) {
  REMOVE_EDGES(cnode);
  cnode->ty |= 4;
  continue;
  }
  g = - u[cnode->j[0]] + prob->sjob[cnode->j[0]]->f[t];

  tt[0] = tt[1] = t;
  if (cnode->j[1] < prob->n) {
  tt[0] -= prob->sjob[cnode->j[0]]->p;
  if (prec_get_dom_ti(tt[0], cnode->j[1])) {
  REMOVE_EDGES(cnode);
    cnode->ty |= 4;
  continue;
  }
  g += - u[cnode->j[1]] + prob->sjob[cnode->j[1]]->f[tt[0]];

  if (cnode->j[2] < prob->n) {
  tt[1] = tt[0] - prob->sjob[cnode->j[1]]->p;
  if (prec_get_dom_ti(tt[1], cnode->j[2])) {
    REMOVE_EDGES(cnode);
    cnode->ty |= 4;
    continue;
  }
  g += - u[cnode->j[2]] + prob->sjob[cnode->j[2]]->f[tt[1]];
  }
  }

  v1 = v2 = dinf;
  pr1 = pr2 = nullptr;
  ty1 = ty2 = 0;

  pe = &(cnode->e);
  while (*pe) {
  cnode2 = (*pe)->n;
  if (cnode2->ty & 4) {
  REMOVE_SINGLE_EDGE;
  continue;
  }

  cnode3 = cnode2->n[0];
  if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
  if(!cnode2->n[1]) {
    REMOVE_SINGLE_EDGE;
    continue;
  } else if(!cnode2->e->next->next) {
    cnode3 = cnode2->n[1];
    if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
      REMOVE_SINGLE_EDGE;
      continue;
    }
  }
  ty = 1;
  f = cnode2->v[1];
  } else {
  ty = 0;
  f = cnode2->v[0];
  }

  if(eflag) {
  if(ub - (f + cnode->v[0]) < 1. - eps) {
    REMOVE_SINGLE_EDGE;
    continue;
  } else if(ub - (f + cnode->v[1]) < 1. - eps) {
    cnode3 = cnode->n[0];
    if(HAVE_COMMON_REAL_JOB(cnode2, cnode3)) {
      REMOVE_SINGLE_EDGE;
      continue;
    }
  }

  if(_check_adj_supernode_backward(cnode, cnode2, t)) {
    REMOVE_SINGLE_EDGE;
    continue;
  }
  }

  f += g;
  if(f < v1) {
  v2 = v1;
  v1 = f;
  pr2 = pr1;
  pr1 = cnode2;
  ty2 = ty1;
  ty1 = ty;
  } else if(f < v2) {
  v2 = f;
  pr2 = cnode2;
  ty2 = ty;
  }

  pe = &((*pe)->next);
  }

  if (!pr1) {
  cnode->ty |= 4;
  continue;
  }

  cnode->v[0] = v1;
  cnode->v[1] = v2;
  cnode->ty = CONV_TYPE(ty1, ty2);
  cnode->n[0] = pr1;
  cnode->n[1] = pr2;

  UPDATE_WINDOWS;
  }
  }

  cnode = node[prob->graph->Tmin];
  cnode->v[0] = cnode->v[1] = dinf;
  cnode->n[0] = cnode->n[1] = nullptr;
  cnode->ty = 0;
  pe = &(cnode->e);
  while (*pe) {
  cnode2 = (*pe)->n;
  if (cnode2->ty & 4) {
  REMOVE_SINGLE_EDGE;
  continue;
  }

  if(cnode2->v[0] < cnode->v[0]) {
  cnode->v[0] = cnode2->v[0];
  cnode->n[0] = cnode2;
  }
  pe = &((*pe)->next);
  }
  prob->graph->n_edges = n_edges;
  if (!cnode->e) return SIPS_INFEASIBLE;
  _lag2_free_eliminated_nodes();
  return _lag2_shrink_horizon_head(u);
}

int _lag2_get_sol_LR2m_forward(double *u, _solution_t *sol, double *lb, unsigned int *o) {
  int t;
  int ty, pty;
  int ret;
  _node2m_t **node, *cnode;
  _fixed_t *fixed;

  node = (_node2m_t **) prob->graph->node2m;
  fixed = prob->graph->fixed;

  cnode = node[prob->graph->Tmax + 1];
  if (!cnode) {
    *lb = dinf;
    if (sol) {
      sol->f = inf;
    }
    return SIPS_INFEASIBLE;
  }

  *lb = cnode->v[0] + fixed->ftail;
  for (int i = 0; i < fixed->ntail; i++) {
    *lb -= u[fixed->job[prob->n - i - 1]->no];
  }

  if(*lb > dinf2) {
    if (sol) sol->f = inf;
    return SIPS_INFEASIBLE;
  }
  ty = 0;
  cnode = cnode->n[0];
  t = prob->graph->Tmax;

  ret = SIPS_OPTIMAL;
  if (!sol) {
    if(!o) return SIPS_NORMAL;

    memset(o, 0, prob->n*sizeof(unsigned int));

    for (int i = 0; i < fixed->nhead; i++) {
      o[fixed->job[i]->no]++;
    }
    for (int i = 0; i < fixed->ntail; i++) {
      o[fixed->job[prob->n - i - 1]->no]++;
    }

    while (cnode->j[0] < prob->n) {
      for (int i=0; i<3; ++i) 
        if (cnode->j[i] < prob->n) {
          o[cnode->j[i]]++;
        }
      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    for (int i = 0; i < prob->n; i++) {
      if (o[i] != 1) {
	      ret = SIPS_NORMAL;
	      break;
      }
    }

    return ret;
  }

  sol->n = 0;
  sol->f = fixed->ftail;
  for (int i = 0; i < fixed->ntail; i++) {
    sol->job[sol->n] = fixed->job[prob->n - i - 1];
    sol->c[sol->n++] = fixed->c[prob->n - i - 1];
  }
  if (o) {
    memset(o, 0, prob->n*sizeof(unsigned int));
    for (int i = 0; i < fixed->nhead; i++) {
      o[fixed->job[i]->no]++;
    }
    for (int i = 0; i < fixed->ntail; i++) {
      o[fixed->job[prob->n - i - 1]->no]++;
    }

    while (cnode->j[0] < prob->n) {
      for (int i=0; i<3; ++i)
        if (cnode->j[i] < prob->n) {
          sol->f += prob->sjob[cnode->j[i]]->f[t];
          o[cnode->j[i]]++;
          sol->job[sol->n] = prob->sjob[cnode->j[i]];
          sol->c[sol->n++] = t;
          t -= prob->sjob[cnode->j[i]]->p;
        } else break;
      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    for (int i = 0; i < prob->n; i++) {
      if (o[i] != 1) {
	      ret = SIPS_NORMAL;
	      break;
      }
    }
  } else {
      while (cnode->j[0] < prob->n) {
          for (int i=0; i<3; ++i) 
            if (cnode->j[i] < prob->n) {
              sol->f += prob->sjob[cnode->j[i]]->f[t];
              sol->job[sol->n] = prob->sjob[cnode->j[i]];
              sol->c[sol->n++] = t;
              t -= prob->sjob[cnode->j[i]]->p;       
            } else break;
          pty = UNCONV_TYPE(cnode);
          cnode = cnode->n[ty];
          ty = pty;
      }

    ret = SIPS_NORMAL;
  }

  sol->f += fixed->fhead;
  for (int i = fixed->nhead - 1; i >= 0; i--) {
    sol->job[sol->n] = fixed->job[i];
    sol->c[sol->n++] = fixed->c[i];
  }

  reverse_solution(sol);

  if(fixed->nhead + fixed->ntail == prob->n) return SIPS_OPTIMAL;
  return ret;
}

int _lag2_get_sol_LR2m_backward(double *u, _solution_t *sol,
				double *lb, unsigned int *o) {
  int t;
  int ty, pty;
  int ret;
  _node2m_t **node, *cnode;
  _fixed_t *fixed;

  node = (_node2m_t **) prob->graph->node2m;
  fixed = prob->graph->fixed;

  cnode = node[prob->graph->Tmin];
  if (!cnode) {
    *lb = dinf;
    if (sol) sol->f = inf;
    return SIPS_INFEASIBLE;
  }

  *lb = cnode->v[0] + fixed->fhead;
  for (int i = 0; i < fixed->nhead; i++) {
    *lb -= u[fixed->job[i]->no];
  }

  if (*lb > dinf2) {
    if (sol) sol->f = inf;
    return SIPS_INFEASIBLE;
  }
  ty = 0;
  cnode = cnode->n[0];
  t = prob->graph->Tmin;

  ret = SIPS_OPTIMAL;
  if (!sol) {
    if(!o) return SIPS_NORMAL;
    memset(o, 0, prob->n*sizeof(unsigned int));
    for (int i = 0; i < fixed->nhead; i++) {
      o[fixed->job[i]->no]++;
    }
    for (int i = 0; i < fixed->ntail; i++) {
      o[fixed->job[prob->n - i - 1]->no]++;
    }
    while (cnode->j[0] < prob->n) {
      for (int i=0; i<3; ++i)
        if (cnode->j[i] < prob->n) {
          o[cnode->j[i]]++;
        }
      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    for (int i = 0; i < prob->n; i++) {
      if (o[i] != 1) {
        ret = SIPS_NORMAL;
        break;
      }
    }
    return ret;
  }

  sol->n = 0;
  sol->f = fixed->fhead;
  for (int i = 0; i < fixed->nhead; i++) {
    sol->job[sol->n] = fixed->job[i];
    sol->c[sol->n++] = fixed->c[i];
  }

  if (o) {
    memset(o, 0, prob->n*sizeof(unsigned int));
    for (int i = 0; i < fixed->nhead; i++) {
      o[fixed->job[i]->no]++;
    }
    for (int i = 0; i < fixed->ntail; i++) {
      o[fixed->job[prob->n - i - 1]->no]++;
    }

    while (cnode->j[0] < prob->n) {
      for (int i=2; i>=0; --i)
        if (cnode->j[i] < prob->n) {
          t += prob->sjob[cnode->j[i]]->p;
          o[cnode->j[i]]++;
          sol->f += prob->sjob[cnode->j[i]]->f[t];
          sol->job[sol->n] = prob->sjob[cnode->j[i]];
          sol->c[sol->n++] = t;
        }
      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    for (int i = 0; i < prob->n; i++) {
      if (o[i] != 1) {
        ret = SIPS_NORMAL;
        break;
      }
    }
  } else {
    while (cnode->j[0] < prob->n) {
      for (int i=2; i>=0; --i)
        if (cnode->j[i] < prob->n) {
          t += prob->sjob[cnode->j[i]]->p;
          sol->f += prob->sjob[cnode->j[i]]->f[t];
          sol->job[sol->n] = prob->sjob[cnode->j[i]];
          sol->c[sol->n++] = t;
        }
      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    ret = SIPS_NORMAL;
  }
  sol->f += fixed->ftail;
  for (int i = fixed->ntail - 1; i >= 0; i--) {
    sol->job[sol->n] = fixed->job[prob->n - i - 1];
    sol->c[sol->n++] = fixed->c[prob->n - i - 1];
  }
  if (fixed->nhead + fixed->ntail == prob->n) return SIPS_OPTIMAL;
  return ret;
}


void lag2_reverse() {
  if(prob->graph->direction == SIPS_FORWARD) {
    _lag2_move_edges_tail();
    prob->graph->direction = SIPS_BACKWARD;
  } else {
    _lag2_move_edges_head();
    prob->graph->direction = SIPS_FORWARD;
  }
}

int lag2_solve_LR2m_reverse(double *u, double ub) {
  int ret;
  if(prob->graph->direction == SIPS_FORWARD) {
    _lag2_move_edges_tail();
    ret = _lag2_solve_LR2m_backward(u, ub, 1);
    prob->graph->direction = SIPS_BACKWARD;
  } else {
    _lag2_move_edges_head();
    ret = _lag2_solve_LR2m_forward(u, ub, 1);
    prob->graph->direction = SIPS_FORWARD;
  }
  return ret;
}

int lag2_solve_LR2m_reverse_without_elimination(double *u, double ub) {
  int ret;

  if(prob->graph->direction == SIPS_FORWARD) {
    _lag2_move_edges_tail();
    prob->graph->direction = SIPS_BACKWARD;
    ret = _lag2_solve_LR2m_backward(u, ub, 0);
  } else {
    _lag2_move_edges_head();
    prob->graph->direction = SIPS_FORWARD;
    ret = _lag2_solve_LR2m_forward(u, ub, 0);
  }
  return ret;
}


int _lag2_get_sol_LR2m(double *u, double ub, _solution_t *sol, double *lb, unsigned int *o) {
    int ret;
    if(prob->graph->direction == SIPS_FORWARD) {
        ret = _lag2_get_sol_LR2m_forward(u, sol, lb, o);
    } else {
        ret = _lag2_get_sol_LR2m_backward(u, sol, lb, o);
    }
    *lb = min(*lb, ub);
    return ret;
}

int lag2_solve_LR2m(double *u, double ub, _solution_t *sol, double *lb, unsigned int *o) {
  int ret;
  if (prob->graph->direction == SIPS_FORWARD) {
    ret = _lag2_solve_LR2m_forward(u, ub, 1);
  } else {  
    ret = _lag2_solve_LR2m_backward(u, ub, 1);
  }
  if (ret == SIPS_INFEASIBLE) *lb = ub;
  return(_lag2_get_sol_LR2m(u, ub, sol, lb, o));
}

int lag2_assign_modifiers(unsigned char type, int nmax, _mod_t *mod) {
  int i, j;
  int ty, pty;
  int n, nn, m;
  _ptable_t *ptable;
  _fixed_t *fixed;
  _node2m_t **node, *cnode;
  _occur_t *occ;

  if (_lag2_check_time_window() == SIPS_INFEASIBLE) return SIPS_INFEASIBLE;

  node = (_node2m_t **) prob->graph->node2m;
  fixed = prob->graph->fixed;
  occ = new _occur_t[prob->n + 2]{};

  ptable = prob->graph->ptable;
  for(i = 0; i < prob->n; i++) {
    occ[i].j = i;
    occ[i].nd = ptable->occ[i];
  }

  for(i = 0; i < fixed->nhead; i++) {
    mod->fl[fixed->job[i]->no] = 1;
  }
  for(i = 0; i < fixed->ntail; i++) {
    mod->fl[fixed->job[prob->n - i - 1]->no] = 1;
  }

  if(prob->graph->direction == SIPS_FORWARD) {
    ty = 0;
    cnode = node[prob->graph->Tmax + 1]->n[0];
    while (cnode->j[0] < prob->n) {
      occ[cnode->j[0]].no++;
      occ[cnode->j[1]].no++;
      occ[cnode->j[2]].no++;
      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }
  } else {
    ty = 0;
    cnode = node[prob->graph->Tmin]->n[0];
    while (cnode->j[0] < prob->n) {
      occ[cnode->j[2]].no++;
      occ[cnode->j[1]].no++;
      occ[cnode->j[0]].no++;
      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }
  }

  for(i = nn = 0; i < prob->n; i++) {
    if(mod->fl[occ[i].j] == 0) {
      occ[nn++] = occ[i];
    }
  }

  if(type == 0) {
    sort(occ, occ + nn, [](auto x, auto y) {
        if (x.no != y.no) return x.no < y.no;
        return x.nd < y.nd;
    });

    for(i = 0; i < nn && occ[i].no == 0; i++);
    for(n = 0, i--; n < nmax && i >= 0; n++, i--) {
      mod->jobs[n] = occ[i].j;
      mod->fl[occ[i].j] = 1;
    }

    if(n < nmax) {
      for(i = nn - 1; n < nmax && i >= 0 && occ[i].no > 1; n++, i--) {
	      mod->jobs[n] = occ[i].j;
	      mod->fl[occ[i].j] = 1;
      }
    }
  } else {
        sort(occ, occ + nn, [](auto x, auto y) {
            if (x.nd != y.nd) return x.nd < y.nd;
            return x.no < y.no;
        });

    for(n = 0;  n < nn && n < nmax; n++) {
      mod->jobs[n] = occ[n].j;
      mod->fl[occ[n].j] = 1;
    }
  }

  delete[] occ;
  mod->an = n;
  memset(mod->v[0], 0, (prob->n + 2)*8);
  memset(mod->m, 0, prob->n + 2);
  for(i = 0; i < mod->an; i++) {
    mod->m[mod->jobs[i]] = 1<<i;
  }
  //las mascaras validas 
  for(j = 0; j < prob->n; j++) {
    for(m = 0; m < 1<<mod->an; m++) {
      if(mod->m[j] && !(m & mod->m[j])) {
	      mod->v[j][m] = 1;
      }
    }
  }

  for(i = 0; i < mod->an; i++) {
    for(j = 0; j < prob->n; j++) {
      if(ptable->inc[mod->jobs[i]][j] == 1) {
        /* mod->job[i] -> j */
        for(m = 0; m < 1<<mod->an; m++) {
          if(!(m & (1<<i))) {
            mod->v[j][m] = 1;
          }
        }
      } else if(ptable->inc[mod->jobs[i]][j] == -1) {
        /* j -> mod->job[i] */
        for(m = 0; m < 1<<mod->an; m++) {
          if(m & (1<<i)) {
            mod->v[j][m] = 1;
          }
        }
      }
    }
  }
  mod->n += mod->an;
  return SIPS_OK;
}

int lag2_add_modifiers_LR2m(double *u, double ub, _solution_t *sol, double *lb, unsigned int *o, _mod_t *mod) {
    int ret;
    if (mod->an == 0) return SIPS_NORMAL;
    sol->n = 0;
    if(prob->graph->direction == SIPS_FORWARD) {
      ret = _lag2_add_modifiers_LR2m_forward(u, ub, mod, 1);
    } else {
      ret = _lag2_add_modifiers_LR2m_backward(u, ub, mod, 1);
    }
    if(ret == SIPS_INFEASIBLE) {
      *lb = ub;
    } else {
      ret = _lag2_get_sol_LR2m(u, ub, sol, lb, o);
    }
    return ret;
  }
  

  int _lag2_initialize_node_forward(double *u) {
    int t, tt;
    int i;
    char ty, ty1, ty2;
    double f, g;
    double v1, v2;
    unsigned int n_nodes, n_edges;
    _ptable_t *ptable;
    Benv *bnode, *bedge;
    _node2m_t **node, *cnode, *cnode2, *pr, *pr1, *pr2;
    _edge2m_t *e, *e2;
    char (*check_func)(int, int, _node2m_t *);
    check_func = _check_three_forward;

    n_nodes = 0;
    n_edges = prob->graph->n_edges;
    bnode = prob->graph->bnode;
    bedge = prob->graph->bedge;
    ptable = prob->graph->ptable;
    node = (_node2m_t **) prob->graph->node2m;
    prob->graph->direction = SIPS_FORWARD;

  memset(ptable->occ, 0, (prob->n + 2)*sizeof(unsigned int));

  memset((void *) node, 0, (prob->T + 2)*sizeof(_node2m_t *)); 
  node[prob->graph->Tmin] = cnode = static_cast<_node2m_t*>(bnode->alloc_element());

  n_nodes++;
  memset(cnode, 0, sizeof(_node2m_t));
  cnode->j[0] = prob->n;
  cnode->j[1] = prob->n + 1;
  cnode->j[2] = prob->n + 1;
  cnode->n[0] = cnode;

  for(tt = prob->graph->Tmin; tt < prob->graph->Tmax; tt++) {
    if(prec_get_sd_t(tt)) {
      continue;
    } else if (!node[tt]) {
      prec_set_sd_t(tt);
      continue;
    }

    for(i = 0; i < prob->n; i++) {
      t = tt + prob->sjob[i]->p;
      if(t > prob->graph->Tmax || prec_get_sd_t(t) || prec_get_dom_ti(t, i)) {
	      continue;
      }

      g = -u[i] + prob->sjob[i]->f[t];
      v1 = v2 = dinf;
      pr1 = pr2 = nullptr;
      ty1 = ty2 = 0;
      e = nullptr;
      for (cnode2 = node[tt]; cnode2; cnode2 = cnode2->next) {
        if(prec_get_adj_tij1(t, cnode2->j[0], i)) {
          continue;
        }

        if(check_func(t, i, cnode2)) {
          continue;
        }

        if(cnode2->n[0]->j[0] == i) {
          if (!cnode2->n[1]) continue;
          f = cnode2->v[1] + g;
          ty = 1;
        } else {
          f = cnode2->v[0] + g;
          ty = 0;
        }

        if(f < v1) {
          v2 = v1;
          v1 = f;
          pr2 = pr1;
          pr1 = cnode2;
          ty2 = ty1;
          ty1 = ty;
        } else if(f < v2) {
          v2 = f;
          pr2 = cnode2;
          ty2 = ty;
        }

        e2 = static_cast<_edge2m_t*>(bedge->alloc_element());

        n_edges++;
        e2->n = cnode2;
        e2->next = e;
        e = e2;
      }

      if (e) {
        cnode = static_cast<_node2m_t*>(bnode->alloc_element());

        ptable->occ[i]++;
        n_nodes++;
        cnode->j[0] = i;
        cnode->j[1] = prob->n + 1;
        cnode->j[2] = prob->n + 1;
        cnode->v[0] = v1;
        cnode->v[1] = v2;
        cnode->ty = CONV_TYPE(ty1, ty2);
        cnode->n[0] = pr1;
        cnode->n[1] = pr2;
        cnode->next = node[t];
        node[t] = cnode;
        cnode->e = e;
      } else {
	      prec_set_dom_ti(t, i);
      }
    }
  }

  node[prob->graph->Tmax + 1] = cnode = static_cast<_node2m_t*>(bnode->alloc_element());

  n_nodes++;
  cnode->j[0] = prob->n;
  cnode->j[1] = prob->n + 1;
  cnode->j[2] = prob->n + 1;
  cnode->next = nullptr;
  cnode->e = nullptr;
  f = dinf;
  pr = nullptr;
  for(cnode2 = node[prob->graph->Tmax]; cnode2; cnode2 = cnode2->next) {
    e = static_cast<_edge2m_t*>(bedge->alloc_element());
    n_edges++;
    e->n = cnode2;
    e->next = cnode->e;
    cnode->e = e;
    if(f > cnode2->v[0]) {
      f = cnode2->v[0];
      pr = cnode2;
    }
  }

  cnode->v[0] = f;
  cnode->v[1] = dinf;
  cnode->n[0] = pr;
  cnode->n[1] = nullptr;
  cnode->ty = 0;

  prob->graph->n_nodes = n_nodes;
  prob->graph->n_edges = n_edges;

  if (!cnode->e) return SIPS_INFEASIBLE;
  for(i = 0; i < prob->n; i++) {
    for(t = ptable->window[i].s; t <= ptable->window[i].e; t++) {
      if(!prec_get_dom_ti(t, i)) {
	      ptable->window[i].s = t;
	      break;
      }
    }
    if(t > ptable->window[i].e) return SIPS_INFEASIBLE;
    for(t = ptable->window[i].e; t >= ptable->window[i].s; t--) {
      if(!prec_get_dom_ti(t, i)) {
	      ptable->window[i].e = t;
	      break;
      }
    }
  }

  return _lag2_shrink_horizon_tail(u);
}


int _lag2_initialize_node(double *u) {
    if (!prob->graph->bedge) {
      prob->graph->n_nodes = prob->graph->n_edges = 0;
      prob->graph->bnode = new Benv(sizeof(_node2m_t));
      prob->graph->bedge = new Benv(sizeof(_edge2m_t));
      prob->graph->fixed = create_fixed();
      prob->graph->node2m = new _node2m_t[prob->T + 2]{};
      int ret = _lag2_initialize_node_forward(u);
      ptable_free_adj(prob->graph->ptable);
      if(ret == SIPS_NORMAL) ret = _lag2_check_time_window();
    }
    return SIPS_NORMAL;
}

int lag2_add_modifiers_LR2m_without_elimination(double *u, double ub, _solution_t *sol, double *lb,
  unsigned int *o, _mod_t *mod)
{
int ret;

if(mod->an == 0) return SIPS_NORMAL;
sol->n = 0;
if(prob->graph->direction == SIPS_FORWARD) {
ret = _lag2_add_modifiers_LR2m_forward(u, ub, mod, 0);
} else {
ret = _lag2_add_modifiers_LR2m_backward(u, ub, mod, 0);
}
if(ret == SIPS_INFEASIBLE) {
*lb = ub;
} else {
ret = _lag2_get_sol_LR2m(u, ub, sol, lb, o);
}
return(ret);
}

int lag2_solve_LR2m_without_elimination(double *u, double ub,
  _solution_t *sol, double *lb,
  unsigned int *o) {
int ret;
if(prob->graph->direction == SIPS_FORWARD) {
ret = _lag2_solve_LR2m_forward(u, ub, 0);
} else {
ret = _lag2_solve_LR2m_backward(u, ub, 0);
}
if(ret == SIPS_INFEASIBLE) *lb = ub;
return(_lag2_get_sol_LR2m(u, ub, sol, lb, o));
}


int lag2_initialize(double *u, double ub, _solution_t *sol, double *lb, unsigned int *o) {
    if (!prob->graph->bedge) {
        int ret = _lag2_initialize_node(u);
        if (ret != SIPS_INFEASIBLE) {
            ret = _lag2_get_sol_LR2m(u, ub, sol, lb, o);
        }
        return ret;
    } 
    return SIPS_NORMAL;
}

  