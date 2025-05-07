#pragma once
#include "models.hpp"
#include "utils/constants.hpp"
#include "utils/solution.hpp"
#include "ptable.hpp"
#include "constraint.hpp"
#include <iostream>

#define CONV_TYPE(a, b)	((a) | ((b) << 1))
#define UNCONV_TYPE(n) ((int) ((ty & 1)?((n->ty & 0x2) >> 1):(n->ty & 0x1)))


void lag_initialize_LR1_solver() {
  if (!prob->graph->node1) {
    prob->graph->node1 = new _node1_t[prob->T + 1]{};
    prob->graph->Tmax = prob->T;
    prob->graph->Tmin = 0;
    ptable_initialize();
  }
}

void lag_initialize_LR2adj_solver() {
  if (!prob->graph->node2) {
    ptable_initialize_adj(prob->graph->ptable);
    prob->graph->node2 = new _node2_t[prob->T + 2]{};
    prob->graph->bnode = new Benv(sizeof(_node2_t));
  }
}

void lag_free_LR1_solver() {
  if (prob->graph->node1) {
    delete[] prob->graph->node1;
    prob->graph->node1 = nullptr;
  }
}

void lag_free_LR2adj_solver() {
  if (!prob->graph->node2) {
    delete prob->graph->bnode;
    prob->graph->bnode = nullptr;
    delete[] prob->graph->node2;
    prob->graph->bnode = nullptr;
    prob->graph->node2 = nullptr;
  }
}

double lag_get_memory_in_MB() {
  double use_mem = 0.0;
  if (prob->graph->node1) {
    use_mem += (prob->T + 1)* sizeof(_node1_t);
  }
  if (prob->graph->node2) {
    use_mem += (prob->T + 2) * sizeof(_node2_t *);
    use_mem += prob->graph->bnode->used_memory();
  }
  use_mem /= (1<<20);
  return use_mem;
}

void lag_free() {
    lag_free_LR1_solver();
    lag_free_LR2adj_solver();
    ptable_free(prob->graph->ptable);
    prob->graph->ptable = nullptr;
  }

  int _lag_check_time_window() {
    int t, s, e;
    int i;
    _window_t *w;
    _ptable_t *ptable;
  
    ptable = prob->graph->ptable;
  
    for(i = 0; i < prob->n; i++) {
      s = prob->graph->Tmax + 1;;
      for(t = max(ptable->window[i].s, prob->graph->Tmin);
      t <= ptable->window[i].e && t <= prob->graph->Tmax; t++) {
        if(!prec_get_dom_ti(t, i)) {
      s = t;
      break;
        }
      }
  
      if(s == prob->graph->Tmax + 1) {
        return SIPS_INFEASIBLE;
      }
  
      e = s;
      for(t = min(ptable->window[i].e, prob->graph->Tmax); t > s; t--) {
        if(!prec_get_dom_ti(t, i)) {
          e = t;
          break;
        }
      }
  
      ptable->window[i].s = s;
      ptable->window[i].e = e;
    }
  
    w = new _window_t[prob->n]{};
    memcpy(w, ptable->window, prob->n*sizeof(_window_t));
  
    if (constraint_propagation(w) == SIPS_INFEASIBLE) {
      delete[] w;
      return SIPS_INFEASIBLE;
    }
  
    for(i = 0; i < prob->n; i++) {
      for(t = ptable->window[i].s; t < w[i].s; t++) {
        prec_set_dom_ti(t, i);
      }
      for(t = ptable->window[i].e; t > w[i].e; t--) {
        prec_set_dom_ti(t, i);
      }
    }
  
    memcpy(ptable->window, w, prob->n*sizeof(_window_t));
    delete[] w;
    return SIPS_NORMAL;
  }

void _lag_solve_LR1_forward_with_domain(double *u) {
  int t, tt;
  int i;
  int j1, j2;
  char ty, ty1, ty2;
  double vmin1, vmin2;
  double f;
  _ptable_t *ptable;
  _node1_t *node1;
  _node1_t *pr;

  ptable = prob->graph->ptable;
  node1 = prob->graph->node1;

  t = prob->graph->Tmin;
  node1[t].v[0] = node1[t].v[1] = 0.0;
  node1[t].j[0] = prob->n;
  node1[t].j[1] = prob->n + 1;
  node1[t].ty[0] = node1[t].ty[1] = 0;

  for (t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
    vmin1 = vmin2 = dinf;
    j1 = j2 = prob->n + 1;
    ty1 = ty2 = 0;

    for(i = 0; i < prob->n; i++) {
      if(t < prob->sjob[i]->p) {
	    continue;
      }

      tt = t - prob->sjob[i]->p;
      pr = node1 + tt;
      if(pr->j[0] > prob->n) {
        prec_set_dom_ti(t, i);
        continue;
      }

      if(i != pr->j[0]) {
        f = pr->v[0];
        ty = 0;
      } else if (pr->j[1] > prob->n) {
        prec_set_dom_ti(t, i);
        continue;
      } else {
        f = pr->v[1];
        ty = 1;
      }
      f += - u[i] + prob->sjob[i]->f[t];

      if(f < vmin1) {
        vmin2 = vmin1;
        vmin1 = f;
        j2 = j1;
        j1 = i;
        ty2 = ty1;
        ty1 = ty;
      } else if(f < vmin2) {
        vmin2 = f;
        j2 = i;
        ty2 = ty;
      }
    }

    node1[t].v[0] = vmin1;
    node1[t].v[1] = vmin2;
    node1[t].j[0] = j1;
    node1[t].j[1] = j2;
    node1[t].ty[0] = ty1;
    node1[t].ty[1] = ty2;

    if (node1[t].j[0] > prob->n) {
      prec_set_sd_t(t);
    }
  }
}


void _lag_solve_LR1_forward(double *u) {
  int t, tt;
  int i;
  int j1, j2;
  char ty, ty1, ty2;
  double vmin1, vmin2;
  double f;
  _ptable_t *ptable;
  _node1_t *node1;
  _node1_t *pr;

  ptable = prob->graph->ptable;
  node1 = prob->graph->node1;

  t = prob->graph->Tmin;
  node1[t].v[0] = node1[t].v[1] = 0.0;
  node1[t].j[0] = prob->n;
  node1[t].j[1] = prob->n + 1;
  node1[t].ty[0] = node1[t].ty[1] = 0;

  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
    if(prec_get_sd_t(t)) {
      continue;
    }
    vmin1 = vmin2 = dinf;
    j1 = j2 = prob->n + 1;
    ty1 = ty2 = 0;

    for(i = 0; i < prob->n; i++) {
      if(prec_get_dom_ti(t, i)) {
	    continue;
      }

      tt = t - prob->sjob[i]->p;
      pr = node1 + tt;
      if (pr->j[0] > prob->n) {
	    continue;
      }

      if(i != pr->j[0]) {
        f = pr->v[0];
        ty = 0;
      } else if (pr->j[1] > prob->n) {
	    continue;
      } else {
        f = pr->v[1];
        ty = 1;
      }
      f += - u[i] + prob->sjob[i]->f[t];

      if(f < vmin1) {
        vmin2 = vmin1;
        vmin1 = f;
        j2 = j1;
        j1 = i;
        ty2 = ty1;
        ty1 = ty;
      } else if(f < vmin2) {
        vmin2 = f;
        j2 = i;
        ty2 = ty;
      }
    }

    node1[t].v[0] = vmin1;
    node1[t].v[1] = vmin2;
    node1[t].j[0] = j1;
    node1[t].j[1] = j2;
    node1[t].ty[0] = ty1;
    node1[t].ty[1] = ty2;
  }
}

void lag_solve_LR1_backward(double *u, double ub) {
  int t, tt;
  int i;
  int j1, j2;
  char ty, ty1, ty2;
  double vmin1, vmin2;
  double f, g;
  _ptable_t *ptable;
  _node1_t *node1;
  _node1_t *pr;

  ptable = prob->graph->ptable;
  node1 = prob->graph->node1;
  t = prob->graph->Tmax;
  node1[t].v[0] = node1[t].v[1] = 0.0;
  node1[t].j[0] = prob->n;
  node1[t].j[1] = prob->n + 1;
  node1[t].ty[0] = node1[t].ty[1] = 0;

  for(t = prob->graph->Tmax - 1; t >= prob->graph->Tmin; t--) {
    if (prec_get_sd_t(t)) {
      continue;
    }
    if (node1[t].j[0] > prob->n) {
      for(i = 0; i < prob->n; i++) {
        if((tt = t + prob->sjob[i]->p) <= prob->graph->Tmax) {
          prec_set_dom_ti(tt, i);
        }
      }
      prec_set_sd_t(t);
      continue;
    }
    vmin1 = vmin2 = dinf;
    j1 = j2 = prob->n + 1;
    ty1 = ty2 = 0;
    for(i = 0; i < prob->n; i++) {
      if((tt = t + prob->sjob[i]->p) > prob->graph->Tmax) {
	      continue;
      }
      if(prec_get_dom_ti(tt, i)) {
	      continue;
      }
      pr = node1 + tt;
      if (pr->j[0] > prob->n) {
	      prec_set_dom_ti(tt, i);
	      continue;
      }

      if(i != pr->j[0]) {
	      f = pr->v[0];
	      ty = 0;
      } else if (pr->j[1] > prob->n) {
	      prec_set_dom_ti(tt, i);
	      continue;
      } else {
	      f = pr->v[1];
	      ty = 1;
      }
      f += - u[i] + prob->sjob[i]->f[tt];

      if(i != node1[t].j[0]) {
	      g = node1[t].v[0];
      } else if (node1[t].j[1] > prob->n) {
	      prec_set_dom_ti(tt, i);
	      continue;
      } else {
	      g = node1[t].v[1];
      }

      if(ub - (f + g) < 1. - eps) {
	      prec_set_dom_ti(tt, i);
	      continue;
      }

      if(f < vmin1) {
        vmin2 = vmin1;
        vmin1 = f;
        j2 = j1;
        j1 = i;
        ty2 = ty1;
        ty1 = ty;
      } else if(f < vmin2) {
        vmin2 = f;
        j2 = i;
        ty2 = ty;
      }
    }

    node1[t].v[0] = vmin1;
    node1[t].v[1] = vmin2;
    node1[t].j[0] = j1;
    node1[t].j[1] = j2;
    node1[t].ty[0] = ty1;
    node1[t].ty[1] = ty2;

    if (node1[t].j[0] > prob->n) {
      prec_set_sd_t(t);
    }
  }
}

void _lag_solve_LR2adj_forward(double *u) {
  int t, tt;
  int i, nn;
  char ty, ty1, ty2;
  double f, g;
  double v1, v2;
  unsigned int n_nodes;
  Benv *bnode;
  _ptable_t *ptable;
  _node2_t **node;
  _node2_t *cnode, **pnode;
  _node2_t *pr1, *pr2, *pr;

  bnode = prob->graph->bnode;
  ptable = prob->graph->ptable;
  node = (_node2_t **) prob->graph->node2;
  bnode->reset_memory();
  memset(node, 0, (prob->T + 2)*sizeof(_node2_t *));
  node[prob->graph->Tmin] = cnode = static_cast<_node2_t*>(bnode->alloc_element());
  n_nodes = 0;

  cnode->j = prob->n;
  cnode->v[0] = 0.0;
  cnode->v[1] = dinf;
  cnode->n[0] = cnode;
  cnode->n[1] = NULL;
  cnode->ty = 0;
  cnode->next = NULL;
  n_nodes++;

  nn = prob->n;
  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
    if(prec_get_sd_t(t)) {
      continue;
    }
    pnode = node + t;
    for(i = 0; i < nn; i++) {
      if(prec_get_dom_ti(t, i)) {
	    continue;
      }
      if((tt = t - prob->sjob[i]->p) < prob->graph->Tmin) {
	    prec_set_dom_ti(t, i);
	    continue;
      }

      v1 = v2 = dinf;
      pr1 = pr2 = NULL;
      ty1 = ty2 = 0;
      g = - u[i] + prob->sjob[i]->f[t];
      for (pr = node[tt]; pr; pr = pr->next) {
	    if(prec_get_adj_tij1(t, pr->j, i)) {
	        continue;
	    }
	    if (i == pr->n[0]->j) {
	        if (!pr->n[1]) {
	            prec_set_adj_tij(t, pr->j, i);
	            continue;
	        }
            ty = 1;
            f = g + pr->v[1];
        } else {
            ty = 0;
            f = g + pr->v[0];
        }

        if(f < v1) {
            v2 = v1;
            v1 = f;
            pr2 = pr1;
            pr1 = pr;
            ty2 = ty1;
            ty1 = ty;
        } else if(f < v2) {
            v2 = f;
            pr2 = pr;
            ty2 = ty;
        }
      }

      if (!pr1) {
        prec_set_dom_ti(t, i);
        continue;
      }

      *pnode = cnode = static_cast<_node2_t*>(bnode->alloc_element());
      cnode->j = i;
      cnode->v[0] = v1;
      cnode->v[1] = v2;
      cnode->n[0] = pr1;
      cnode->n[1] = pr2;
      cnode->ty = CONV_TYPE(ty1, ty2);
      cnode->next = nullptr;
      pnode = &(cnode->next);
      n_nodes++;
    }

    if(node[t] == NULL) {
      prec_set_sd_t(t);
    }
  }

  if (!node[prob->graph->Tmax]) {
    prob->graph->n_nodes = n_nodes;
    return;
  }

  v1 = dinf;
  pr1 = NULL;
  for(pr = node[prob->graph->Tmax]; pr; pr = pr->next) {
    if(pr->v[0] < v1) {
      v1 = pr->v[0];
      pr1 = pr;
    }
  }

  node[prob->graph->Tmax + 1] = cnode = static_cast<_node2_t*>(bnode->alloc_element());
  cnode->j = prob->n;
  cnode->v[0] = v1;
  cnode->v[1] = dinf;
  cnode->n[0] = pr1;
  cnode->n[1] = nullptr;
  cnode->ty = 0;
  cnode->next = nullptr;
  n_nodes++;

  prob->graph->n_nodes = n_nodes;
}

int lag_solve_LR2adj_backward(double *u, double ub) {
  int t, tt;
  int i;
  char ty, ty1, ty2;
  double f, g, fr;
  double v1, v2;
  unsigned int n_nodes;
  _ptable_t *ptable;
  _node2_t **node;
  _node2_t *cnode, *cnode2, **pnode;
  _node2_t *pr1, *pr2, *pr;

  n_nodes = prob->graph->n_nodes;
  ptable = prob->graph->ptable;
  node = (_node2_t **) prob->graph->node2;
  if (!node[prob->graph->Tmax + 1]) return SIPS_INFEASIBLE;
  pr = node[prob->graph->Tmin];
  node[prob->graph->Tmin] = nullptr;
  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
    cnode = node[t];
    node[t] = nullptr;
    while (cnode) {
      tt = t - prob->sjob[cnode->j]->p;
      cnode2 = cnode->next;
      cnode->next = node[tt];
      node[tt] = cnode;
      cnode = cnode2;
    }
  }

  node[prob->graph->Tmax] = node[prob->graph->Tmax + 1];
  node[prob->graph->Tmax]->v[0] = 0.0;
  node[prob->graph->Tmax]->n[0] = node[prob->graph->Tmax];
  node[prob->graph->Tmax + 1] = pr;

  for (t = prob->graph->Tmax - 1; t >= prob->graph->Tmin; t--) {
    pnode = node + t;
    while (*pnode) {
      cnode = *pnode;
      i = cnode->j;
      tt = t + prob->sjob[i]->p;

      v1 = v2 = dinf;
      pr1 = pr2 = nullptr;
      ty1 = ty2 = 0;
      g = - u[i] + prob->sjob[i]->f[tt];

      for(pr = node[tt]; pr; pr = pr->next) {
	    if(prec_get_adj_tij2(tt + prob->sjob[pr->j]->p, i, pr->j)) {
	        continue;
	    } else if(i == pr->n[0]->j) {
            if (!pr->n[1]) {
                prec_set_adj_tij(tt + prob->sjob[pr->j]->p, i, pr->j);
                continue;
            }
            ty = 1;
            f = pr->v[1];
        } else {
            ty = 0;
            f = pr->v[0];
        }

        if (pr->j == cnode->n[0]->j) {
            fr = cnode->v[1];
        } else {
            fr = cnode->v[0];
        }

        if(ub - (f + fr) < 1.0 - eps) {
            if(pr->j != prob->n) {
                prec_set_adj_tij(tt + prob->sjob[pr->j]->p, i, pr->j);
            }
            continue;
        }

	f += g;

	if(f < v1) {
	  v2 = v1;
	  v1 = f;
	  pr2 = pr1;
	  pr1 = pr;
	  ty2 = ty1;
	  ty1 = ty;
	} else if(f < v2) {
	  v2 = f;
	  pr2 = pr;
	  ty2 = ty;
	}
      }

      if (!pr1) {
        prec_set_dom_ti(tt, i);
        *pnode = cnode->next; 
        n_nodes--;
        continue;
      }

      cnode->v[0] = v1;
      cnode->v[1] = v2;
      cnode->n[0] = pr1;
      cnode->n[1] = pr2;
      cnode->ty = CONV_TYPE(ty1, ty2);

      pnode = &(cnode->next);
    }
  }

  prob->graph->n_nodes = n_nodes;

  if(_lag_check_time_window() == SIPS_INFEASIBLE) {
    return SIPS_INFEASIBLE;
  }
  return SIPS_NORMAL;
}

int _lag_solve_LR2adj_forward_with_elimination(double *u, double ub) {
  int t, tt;
  int i, nn;
  char ty, ty1, ty2;
  double f, g, fr;
  double v1, v2;
  unsigned int n_nodes;
  Benv* bnode;
  _ptable_t *ptable;
  _node1_t *node1;
  _node2_t **node;
  _node2_t *cnode, **pnode;
  _node2_t *pr1, *pr2, *pr;

  bnode = prob->graph->bnode;
  ptable = prob->graph->ptable;
  node1 = (_node1_t *) prob->graph->node1;
  node = (_node2_t **) prob->graph->node2;

  memset(node, 0, (prob->T + 2)*sizeof(_node2_t *));
  node[prob->graph->Tmin] = cnode = static_cast<_node2_t*>(bnode->alloc_element());
  n_nodes = 0;

  cnode->j = prob->n;
  cnode->v[0] = 0.0;
  cnode->v[1] = dinf; 
  cnode->n[0] = cnode;
  cnode->n[1] = nullptr;
  cnode->ty = 0;
  cnode->next = nullptr;
  n_nodes++;

  nn = prob->n;
  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
    if(prec_get_sd_t(t)) {
      continue;
    }
    pnode = node + t;
    for(i = 0; i < nn; i++) {
      if(prec_get_dom_ti(t, i)) {
	      continue;
      }
      tt = t - prob->sjob[i]->p;

      v1 = v2 = dinf;
      pr1 = pr2 = nullptr;
      ty1 = ty2 = 0;
      g = - u[i] + prob->sjob[i]->f[t];
      for(pr = node[tt]; pr; pr = pr->next) {
        if(prec_get_adj_tij1(t, pr->j, i)) {
          continue;
        }
        if (i == pr->n[0]->j) {
          if(pr->n[1] == NULL) {
            prec_set_adj_tij(t, pr->j, i);
            continue;
          }	
          ty = 1;
          f = g + pr->v[1];
        } else {
          ty = 0;
          f = g + pr->v[0];
        }

        if (i == node1[t].j[0]) {
          fr = node1[t].v[1];
        } else {
          fr = node1[t].v[0];
        }

        if(ub - (f + fr) < 1. - eps) {
          if (pr->j == prob->n) {
            prec_set_dom_ti(t, i);
          } else {
            prec_set_adj_tij(t, pr->j, i);
          }
          continue;
        }

        if(f < v1) {
          v2 = v1;
          v1 = f;
          pr2 = pr1;
          pr1 = pr;
          ty2 = ty1;
          ty1 = ty;
        } else if(f < v2) {
          v2 = f;
          pr2 = pr;
          ty2 = ty;
        }
      }

      if(pr1 == NULL) {
	      prec_set_dom_ti(t, i);
	      continue;
      }
      *pnode = cnode = static_cast<_node2_t*>(bnode->alloc_element());

      cnode->j = (unsigned short) i;
      cnode->v[0] = v1;
      cnode->v[1] = v2;
      cnode->n[0] = pr1;
      cnode->n[1] = pr2;
      cnode->ty = CONV_TYPE(ty1, ty2);
      cnode->next = NULL;
      pnode = &(cnode->next);
      n_nodes++;
    }
    if(node[t] == NULL) {
      prec_set_sd_t(t);
    }
  }

  if (!node[prob->graph->Tmax]) {
    prob->graph->n_nodes = n_nodes;
    return SIPS_INFEASIBLE;
  }

  v1 = dinf;
  pr1 = nullptr;
  for (pr = node[prob->graph->Tmax]; pr; pr = pr->next) {
    if(pr->v[0] < v1) {
      v1 = pr->v[0];
      pr1 = pr;
    }
  }

  node[prob->graph->Tmax + 1] = cnode = static_cast<_node2_t*>(bnode->alloc_element());

  cnode->j = prob->n;
  cnode->v[0] = v1;
  cnode->v[1] = dinf;
  cnode->n[0] = pr1;
  cnode->n[1] = NULL;
  cnode->ty = 0;
  cnode->next = NULL;
  n_nodes++;
  prob->graph->n_nodes = n_nodes;
  return SIPS_NORMAL;
}

int _lag_get_sol_LR1_forward(_solution_t *sol, double *lb, int *o)
{
  int t;
  int i;
  int ty, ret;
  _node1_t *node1;

  node1 = (_node1_t *) prob->graph->node1;
  *lb = node1[prob->graph->Tmax].v[0];
  if (node1[prob->graph->Tmax].j[0] > prob->n) {
    *lb = dinf;
    return SIPS_INFEASIBLE;
  }

  ty = 0;
  ret = SIPS_OPTIMAL;
  if (!sol) {
    if(!o) {
      return SIPS_NORMAL;
    }

    memset(o, 0, prob->n*sizeof(unsigned int));

    for(t = prob->graph->Tmax; t > prob->graph->Tmin;) {
        i = node1[t].j[ty];
        ty = node1[t].ty[ty];
        t -= prob->sjob[i]->p;
	    o[i]++;
    }

    for(i = 0; i < prob->n; i++) {
      if(o[i] != 1) {
	    ret = SIPS_NORMAL;
	    break;
      }
    }

    return ret;
  }

  sol->n = 0;
  sol->f = 0;
  if(o) {
    memset(o, 0, prob->n*sizeof(int));

    for(t = prob->graph->Tmax; t > prob->graph->Tmin;) {
        i = node1[t].j[ty];
        ty = node1[t].ty[ty];
        o[i]++;
        sol->f += prob->sjob[i]->f[t];
        sol->job[sol->n] = prob->sjob[i];
        sol->c[sol->n++] = t;
        t -= prob->sjob[i]->p;
    }

    for(i = 0; i < prob->n; i++) {
      if(o[i] != 1) {
	      ret = SIPS_NORMAL;
	      break;
      }
    }
  } else {
    for(t = prob->graph->Tmax; t > prob->graph->Tmin;) {
      i = node1[t].j[ty];
      ty = node1[t].ty[ty];
        sol->f += prob->sjob[i]->f[t];
        sol->job[sol->n] = prob->sjob[i];
        sol->c[sol->n++] = t;
      t -= prob->sjob[i]->p;
    }

    ret = SIPS_NORMAL;
  }

  reverse_solution(sol);

  return(ret);
}

int _lag_get_sol_LR2adj_forward(_solution_t *sol, double *lb, int *o)
{
  int t;
  int i;
  int ty, pty;
  int ret;
  _node2_t **node, *cnode;

  node = (_node2_t **) prob->graph->node2;
  cnode = node[prob->graph->Tmax + 1];
  if (!cnode) {
    *lb = dinf2;
    if (sol) {
      sol->f = inf;
    }
    return SIPS_INFEASIBLE;
  }
  *lb = cnode->v[0];
  if (*lb > dinf2) {
    if (sol) {
      sol->f = inf;
    }
    return SIPS_INFEASIBLE;
  }
  ty = 0;
  cnode = cnode->n[0];
  t = prob->graph->Tmax;

  ret = SIPS_OPTIMAL;
  if (!sol) {
    if (!o) {
      return SIPS_NORMAL;
    }

    memset(o, 0, prob->n*sizeof(int));

    while (cnode->j < prob->n) {
	    o[cnode->j]++;
        pty = UNCONV_TYPE(cnode);
        cnode = cnode->n[ty];
        ty = pty;
    }

    for(i = 0; i < prob->n; i++) {
      if(o[i] != 1) {
	ret = SIPS_NORMAL;
	break;
      }
    }

    return(ret);
  }

  sol->n = 0;
  sol->f = 0;

  if (o) {
    memset(o, 0, prob->n*sizeof(int));

    while (cnode->j < prob->n) {
        o[cnode->j]++;
        sol->f += prob->sjob[cnode->j]->f[t];
        sol->job[sol->n] = prob->sjob[cnode->j];
        sol->c[sol->n++] = t;
      t -= prob->sjob[cnode->j]->p;

      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    for(i = 0; i < prob->n; i++) {
      if(o[i] != 1) {
	ret = SIPS_NORMAL;
	break;
      }
    }
  } else {
    while (cnode->j < prob->n) {
	sol->f += prob->sjob[cnode->j]->f[t];
	sol->job[sol->n] = prob->sjob[cnode->j];
	sol->c[sol->n++] = t;
      t -= prob->sjob[cnode->j]->p;

      pty = UNCONV_TYPE(cnode);
      cnode = cnode->n[ty];
      ty = pty;
    }

    ret = SIPS_NORMAL;
  }

  reverse_solution(sol);

  return(ret);
}

int lag_solve_LR1(double *u, double ub, _solution_t *sol, double *lb, int *o) {
  if (!prob->graph->node1) {
    lag_initialize_LR1_solver();
    _lag_solve_LR1_forward_with_domain(u);
  } else {
    _lag_solve_LR1_forward(u);
  }
  int ret = _lag_get_sol_LR1_forward(sol, lb, o);
  if(*lb > ub) *lb = ub;
  return ret;
}


int lag_solve_LR2adj(double *u, double ub, _solution_t *sol,
    double *lb, int *o) {

    if (!prob->graph->node2) {
        lag_initialize_LR2adj_solver();
        _lag_solve_LR2adj_forward_with_elimination(u, ub);
        lag_free_LR1_solver();
    } else {
        _lag_solve_LR2adj_forward(u);
    }
    int ret = _lag_get_sol_LR2adj_forward(sol, lb, o);
    if(*lb > ub) *lb = ub;
    return ret;
}