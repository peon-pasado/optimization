#pragma once

#include "models.h"


int _lag2_get_sol_LR2m_forward(sips *prob, _real *u, _solution_t *sol,
  _real *lb, unsigned int *o)
{
int t;
int i;
int ty, pty;
int ret;
_node2m_t **node, *cnode;
_fixed_t *fixed;

node = (_node2m_t **) prob->graph->node2;
fixed = prob->graph->fixed;

cnode = node[prob->graph->Tmax + 1];
if(cnode == NULL) {
*lb = LARGE_REAL;
if(sol != NULL) {
sol->f = LARGE_COST;
}
return(SIPS_INFEASIBLE);
}

*lb = cnode->v[0] + ctod(fixed->ftail);
for(i = 0; i < fixed->ntail; i++) {
*lb -= u[fixed->job[prob->n - i - 1]->no];
}

if(*lb > LARGE_REAL2) {
if(sol != NULL) {
sol->f = LARGE_COST;
}
return(SIPS_INFEASIBLE);
}
ty = 0;
cnode = cnode->n[0];
t = prob->graph->Tmax;

ret = SIPS_OPTIMAL;
if(sol == NULL) {
if(o == NULL) {
return(SIPS_NORMAL);
}

memset((void *) o, 0, prob->n*sizeof(unsigned int));

for(i = 0; i < fixed->nhead; i++) {
o[fixed->job[i]->no]++;
}
for(i = 0; i < fixed->ntail; i++) {
o[fixed->job[prob->n - i - 1]->no]++;
}

while(IS_JOB(cnode->j[0])) {
#ifdef SIPSI
if(IS_REAL_JOB(cnode->j[0])) {
o[cnode->j[0]]++;
}
if(IS_REAL_JOB(cnode->j[1])) {
o[cnode->j[1]]++;
}
if(IS_REAL_JOB(cnode->j[2])) {
o[cnode->j[2]]++;
}
#else /* SIPSI */
o[cnode->j[0]]++;
if(IS_JOB(cnode->j[1])) {
o[cnode->j[1]]++;
}
if(IS_JOB(cnode->j[2])) {
o[cnode->j[2]]++;
}
#endif /* SIPSI */

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
sol->f = fixed->ftail;
for(i = 0; i < fixed->ntail; i++) {
sol->job[sol->n] = fixed->job[prob->n - i - 1];
sol->c[sol->n++] = fixed->c[prob->n - i - 1];
}

if(o != NULL) {
memset((void *) o, 0, prob->n*sizeof(unsigned int));

for(i = 0; i < fixed->nhead; i++) {
o[fixed->job[i]->no]++;
}
for(i = 0; i < fixed->ntail; i++) {
o[fixed->job[prob->n - i - 1]->no]++;
}

while(IS_JOB(cnode->j[0])) {
sol->f += prob->sjob[cnode->j[0]]->f[t];
if(IS_REAL_JOB(cnode->j[0])) {
o[cnode->j[0]]++;
sol->job[sol->n] = prob->sjob[cnode->j[0]];
sol->c[sol->n++] = t;
}
t -= prob->sjob[cnode->j[0]]->p;

if(IS_JOB(cnode->j[1])) {
sol->f += prob->sjob[cnode->j[1]]->f[t];
if(IS_REAL_JOB(cnode->j[1])) {
o[cnode->j[1]]++;
sol->job[sol->n] = prob->sjob[cnode->j[1]];
sol->c[sol->n++] = t;
}
t -= prob->sjob[cnode->j[1]]->p;

if(IS_JOB(cnode->j[2])) {
sol->f += prob->sjob[cnode->j[2]]->f[t];
if(IS_REAL_JOB(cnode->j[2])) {
o[cnode->j[2]]++;
sol->job[sol->n] = prob->sjob[cnode->j[2]];
sol->c[sol->n++] = t;
}
t -= prob->sjob[cnode->j[2]]->p;
}
}

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
while(IS_JOB(cnode->j[0])) {
if(IS_REAL_JOB(cnode->j[0])) {
sol->f += prob->sjob[cnode->j[0]]->f[t];
sol->job[sol->n] = prob->sjob[cnode->j[0]];
sol->c[sol->n++] = t;
}
t -= prob->sjob[cnode->j[0]]->p;

if(IS_JOB(cnode->j[1])) {
if(IS_REAL_JOB(cnode->j[1])) {
sol->f += prob->sjob[cnode->j[1]]->f[t];
sol->job[sol->n] = prob->sjob[cnode->j[1]];
sol->c[sol->n++] = t;
}
t -= prob->sjob[cnode->j[1]]->p;

if(IS_JOB(cnode->j[2])) {
if(IS_REAL_JOB(cnode->j[2])) {
 sol->f += prob->sjob[cnode->j[2]]->f[t];
 sol->job[sol->n] = prob->sjob[cnode->j[2]];
 sol->c[sol->n++] = t;
}
t -= prob->sjob[cnode->j[2]]->p;
}
}

pty = UNCONV_TYPE(cnode);
cnode = cnode->n[ty];
ty = pty;
}

ret = SIPS_NORMAL;
}

sol->f += fixed->fhead;
for(i = fixed->nhead - 1; i >= 0; i--) {
sol->job[sol->n] = fixed->job[i];
sol->c[sol->n++] = fixed->c[i];
}

reverse_solution(sol);

if(fixed->nhead + fixed->ntail == prob->n) {
return(SIPS_OPTIMAL);
}

return(ret);
}

int _lag2_get_sol_LR2m_backward(sips *prob, _real *u, _solution_t *sol,
  _real *lb, unsigned int *o)
{
int t;
int i;
int ty, pty;
int ret;
_node2m_t **node, *cnode;
_fixed_t *fixed;

node = (_node2m_t **) prob->graph->node2;
fixed = prob->graph->fixed;

cnode = node[prob->graph->Tmin];
if(cnode == NULL) {
*lb = LARGE_REAL;
if(sol != NULL) {
sol->f = LARGE_COST;
}
return(SIPS_INFEASIBLE);
}

*lb = cnode->v[0] + ctod(fixed->fhead);
for(i = 0; i < fixed->nhead; i++) {
*lb -= u[fixed->job[i]->no];
}

if(*lb > LARGE_REAL2) {
if(sol != NULL) {
sol->f = LARGE_COST;
}
return(SIPS_INFEASIBLE);
}
ty = 0;
cnode = cnode->n[0];
t = prob->graph->Tmin;

ret = SIPS_OPTIMAL;
if(sol == NULL) {
if(o == NULL) {
return(SIPS_NORMAL);
}

memset((void *) o, 0, prob->n*sizeof(unsigned int));

for(i = 0; i < fixed->nhead; i++) {
o[fixed->job[i]->no]++;
}
for(i = 0; i < fixed->ntail; i++) {
o[fixed->job[prob->n - i - 1]->no]++;
}

while(IS_JOB(cnode->j[0])) {
#ifdef SIPSI
if(IS_REAL_JOB(cnode->j[0])) {
o[cnode->j[0]]++;
}
if(IS_REAL_JOB(cnode->j[1])) {
o[cnode->j[1]]++;
}
if(IS_REAL_JOB(cnode->j[2])) {
o[cnode->j[2]]++;
}
#else /* SIPSI */
o[cnode->j[0]]++;
if(IS_JOB(cnode->j[1])) {
o[cnode->j[1]]++;
}
if(IS_JOB(cnode->j[2])) {
o[cnode->j[2]]++;
}
#endif /* SIPSI */

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
sol->f = fixed->fhead;
for(i = 0; i < fixed->nhead; i++) {
sol->job[sol->n] = fixed->job[i];
sol->c[sol->n++] = fixed->c[i];
}

if(o != NULL) {
memset((void *) o, 0, prob->n*sizeof(unsigned int));

for(i = 0; i < fixed->nhead; i++) {
o[fixed->job[i]->no]++;
}
for(i = 0; i < fixed->ntail; i++) {
o[fixed->job[prob->n - i - 1]->no]++;
}

while(IS_JOB(cnode->j[0])) {
if(IS_JOB(cnode->j[2])) {
t += prob->sjob[cnode->j[2]]->p;
if(IS_REAL_JOB(cnode->j[2])) {
o[cnode->j[2]]++;
sol->f += prob->sjob[cnode->j[2]]->f[t];
sol->job[sol->n] = prob->sjob[cnode->j[2]];
sol->c[sol->n++] = t;
}
}
if(IS_JOB(cnode->j[1])) {
t += prob->sjob[cnode->j[1]]->p;
if(IS_REAL_JOB(cnode->j[1])) {
o[cnode->j[1]]++;
sol->f += prob->sjob[cnode->j[1]]->f[t];
sol->job[sol->n] = prob->sjob[cnode->j[1]];
sol->c[sol->n++] = t;
}
}
t += prob->sjob[cnode->j[0]]->p;
if(IS_REAL_JOB(cnode->j[0])) {
o[cnode->j[0]]++;
sol->f += prob->sjob[cnode->j[0]]->f[t];
sol->job[sol->n] = prob->sjob[cnode->j[0]];
sol->c[sol->n++] = t;
}

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
while(IS_JOB(cnode->j[0])) {
if(IS_JOB(cnode->j[2])) {
t += prob->sjob[cnode->j[2]]->p;
sol->f += prob->sjob[cnode->j[2]]->f[t];
if(IS_REAL_JOB(cnode->j[2])) {
sol->job[sol->n] = prob->sjob[cnode->j[2]];
sol->c[sol->n++] = t;
}
}
if(IS_JOB(cnode->j[1])) {
t += prob->sjob[cnode->j[1]]->p;
sol->f += prob->sjob[cnode->j[1]]->f[t];
if(IS_REAL_JOB(cnode->j[1])) {
sol->job[sol->n] = prob->sjob[cnode->j[1]];
sol->c[sol->n++] = t;
}
}
t += prob->sjob[cnode->j[0]]->p;
sol->f += prob->sjob[cnode->j[0]]->f[t];
if(IS_REAL_JOB(cnode->j[0])) {
sol->job[sol->n] = prob->sjob[cnode->j[0]];
sol->c[sol->n++] = t;
}

pty = UNCONV_TYPE(cnode);
cnode = cnode->n[ty];
ty = pty;
}

ret = SIPS_NORMAL;
}

sol->f += fixed->ftail;
for(i = fixed->ntail - 1; i >= 0; i--) {
sol->job[sol->n] = fixed->job[prob->n - i - 1];
sol->c[sol->n++] = fixed->c[prob->n - i - 1];
}

if(fixed->nhead + fixed->ntail == prob->n) {
return(SIPS_OPTIMAL);
}

return(ret);
}


int _lag2_get_sol_LR2m(sips *prob, _real *u, _real ub, _solution_t *sol,
  _real *lb, unsigned int *o)
{
int ret;

if(prob->graph->direction == SIPS_FORWARD) { /* forward */
ret = _lag2_get_sol_LR2m_forward(prob, u, sol, lb, o);
} else { /* backward */
ret = _lag2_get_sol_LR2m_backward(prob, u, sol, lb, o);
}

if(*lb > ub) {
*lb = ub;
}

return(ret);
}

int LRm2_initialize_node_forward() {  
  auto check_func = _check_three_forward;

  n_nodes = 0;
  n_edges = subl_graph->n_edges;
  ptable = subl_graph->ptable;
  node = subl_graph->node;
  subl_graph->direction = SIPS_FORWARD;

  memset((void *) ptable->occ, 0, (N_JOBS + 2)*sizeof(unsigned int));

  memset((void *) node, 0, (prob->T + 2)*sizeof(_node2m_t *));

  node[prob->graph->Tmin] = cnode = (_node2m_t *) alloc_memory(bnode);
  n_nodes++;
  memset((void *) cnode, 0, sizeof(_node2m_t));
  cnode->j[0] = (unsigned short) N_JOBS;
  cnode->j[1] = (unsigned short) (N_JOBS + 1);
  cnode->j[2] = (unsigned short) (N_JOBS + 1);
  cnode->n[0] = cnode;

  for(tt = prob->graph->Tmin; tt < prob->graph->Tmax; tt++) {
    if(prec_get_sd_t(tt)) {
      continue;
    } else if(node[tt] == NULL) {
      prec_set_sd_t(tt);
      continue;
    }

    for(i = 0; i < prob->n; i++) {
      t = tt + prob->sjob[i]->p;
      if(t > prob->graph->Tmax || prec_get_sd_t(t) || prec_get_dom_ti(t, i)) {
	      continue;
      }

      g = - u[i] + ctod(prob->sjob[i]->f[t]);
      v1 = v2 = LARGE_REAL;
      pr1 = pr2 = NULL;
      ty1 = ty2 = 0;
      e = NULL;
      for(cnode2 = node[tt]; cnode2 != NULL; cnode2 = cnode2->next) {
        if(prec_get_adj_tij1(t, cnode2->j[0], i)) {
          continue;
        }

        if(check_func(prob, t, i, cnode2)) {
          continue;
        }

        if(cnode2->n[0]->j[0] == i) {
          if(cnode2->n[1] == NULL) {
            continue;
          }
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

        e2 = (_edge2m_t *) alloc_memory(bedge);
        n_edges++;
        e2->n = cnode2;
        e2->next = e;
        e = e2;
      }

      if(e != NULL) {
        cnode = (_node2m_t *) alloc_memory(bnode);
        ptable->occ[i]++;
        n_nodes++;
        cnode->j[0] = (unsigned short) i;
        cnode->j[1] = (unsigned short) (N_JOBS + 1);
        cnode->j[2] = (unsigned short) (N_JOBS + 1);
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

    if(lag_get_memory_in_MB(prob) + lag2_get_real_memory_in_MB(prob) > (_real) prob->param->mem) {
      prob->graph->n_nodes = n_nodes;
      prob->graph->n_edges = n_edges;
      return(SIPS_MEMLIMIT);
    }
  }

  node[prob->graph->Tmax + 1] = cnode = (_node2m_t *) alloc_memory(bnode);
  n_nodes++;
  cnode->j[0] = (unsigned short) N_JOBS;
  cnode->j[1] = (unsigned short) (N_JOBS + 1);
  cnode->j[2] = (unsigned short) (N_JOBS + 1);
  cnode->next = NULL;
  cnode->e = NULL;
  f = LARGE_REAL;
  pr = NULL;
  for(cnode2 = node[prob->graph->Tmax]; cnode2 != NULL; cnode2 = cnode2->next) {
    e = (_edge2m_t *) alloc_memory(bedge);
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
  cnode->v[1] = LARGE_REAL;
  cnode->n[0] = pr;
  cnode->n[1] = NULL;
  cnode->ty = 0;

  prob->graph->n_nodes = n_nodes;
  prob->graph->n_edges = n_edges;

  if(cnode->e == NULL) {
    return(SIPS_INFEASIBLE);
  } else if(lag_get_memory_in_MB(prob) + lag2_get_real_memory_in_MB(prob) > (_real) prob->param->mem) {
    return(SIPS_MEMLIMIT);
  }

  for(i = 0; i < prob->n; i++) {
    for(t = ptable->window[i].s; t <= ptable->window[i].e; t++) {
      if(!prec_get_dom_ti(t, i)) {
	      ptable->window[i].s = t;
	      break;
      }
    }

    if(t > ptable->window[i].e) {
      return(SIPS_INFEASIBLE);
    }

    for(t = ptable->window[i].e; t >= ptable->window[i].s; t--) {
      if(!prec_get_dom_ti(t, i)) {
	      ptable->window[i].e = t;
	      break;
      }
    }
  }

  return(_lag2_shrink_horizon_tail(prob, u));
}


int LRm2_initialize_node() {
  int ret;
  if(subl_graph == nullptr) {
    subl_graph->n_nodes = subl_graph->n_edges = 0;
    subl_graph->fixed = create_fixed();
    subl_graph->node = new Node2m[prob->T + 2];
    ret = LRm2_initialize_node_forward(prob, u);
    ptable_free_adj(prob->graph->ptable);

    if(ret == SIPS_NORMAL) {
      ret = _lag2_check_time_window(prob);
    }
  } else {
    ret = SIPS_NORMAL;
  }

  return(ret);
}

int LRm2_initialization() {
  if (subl_graph == nullptr) {
    int ret = LRm2_initialize_node();
    if(ret != SIPS_MEMLIMIT && ret != SIPS_INFEASIBLE) {
      ret = _lag2_get_sol_LR2m(prob, u, ub, sol, lb, o);
    }
    return ret;
  } 
  return SIPS_NORMAL;
}


void sublimation() {
  auto u = new double[prob->n + 2];
  for (int i=1; i<=prob->n; ++i) {
    u[i] = prob->u[i];
  }
  double su = prob->su;
  

  tmpsol = create_solution(prob);

  int ret = LRm2_initialization();


  ret = lag2_initialize(prob, u, ctod(prob->sol->f) - lboff, tmpsol, &lb, NULL);
  lb += lboff;

  _get_time(prob);

  if(ret == SIPS_OPTIMAL) {
    if(obj_lesser(tmpsol->f, prob->sol->f)) {
      PRINT_UBUPDATED(stdout, 0, tmpsol);
      copy_solution(prob, prob->sol, tmpsol);
    }
  }

  free_solution(tmpsol);

  if(ret == SIPS_OPTIMAL || ret == SIPS_INFEASIBLE) {
#ifdef LB_DEBUG
    if(ret != SIPS_OPTIMAL) {
      fprintf(stderr, "Feasibility error.\n");
      exit(1);
    } else {
      fprintf(stdout, "[%3u] Feasible.\n", 0);
    }
#endif /* LB_DEBUG */
    if(prob->param->verbose >= 2) {
      fprintf(stdout, "[%3u] Solved.\n", 0);
    }
    ret = SIPS_SOLVED;
  } else if(ret == SIPS_MEMLIMIT) {
    if(prob->param->verbose >= 2) {
      PRINT_NETWORKSIZE(stdout, 0);
      PRINT_MAXMEMORY(stdout, 0);
    } else if(prob->param->verbose >= 1) {
      PRINT_MAXMEMORY2(stdout);
    }
  }

  if(ret != SIPS_NORMAL) {
    xfree(u);
    lag2_free(prob);
    return(ret);
  }

  lag2_push_nodes(prob);

  if(prob->param->secsw >= 0 && prob->graph->copy->mem >= (_real) prob->param->secsw) {
    if(prob->param->secratio <= prob->param->eps) {
      lbprev = (ctod(prob->sol->f) + lb)/2.0;
      r = (ctod(prob->sol->f) - lbprev)/2.0;
    } else {
      r = ctod(prob->sol->f)*prob->param->secratio;
      if(prob->param->seciniratio > prob->param->eps) {
	      lbprev = lb + ctod(prob->sol->f)*prob->param->seciniratio;
      } else {
	      lbprev = lb + r;
      }

      if(lbprev >= ctod(prob->sol->f)) {
	      lbprev = ctod(prob->sol->f);
      }
    }
  } else {
    lbprev = ctod(prob->sol->f);
    r = ctod(prob->sol->f);
  }

  lbbest = lb;
  f = prob->sol->f;
  for(iter = 1;; iter++) {
    ub = (cost_t) (lbprev + 0.5 + prob->param->eps);
    while(ub <= (cost_t) (lbbest + prob->param->eps)) {
      ub++;
    }

    if(obj_lesser_equal(prob->sol->f, ub)) {
      ub = prob->sol->f;
      if(prob->param->bisec) {
	      ret = lag2_recover_nodes(prob);
      } else {
	      ret = lag2_pop_nodes(prob);
      }
    } else {
      ret = lag2_recover_nodes(prob);
    }

    if(ret == SIPS_MEMLIMIT) {
      if(prob->param->verbose >= 1) {
	      PRINT_MAXMEMORY2(stdout);
      }
      break;
    }

    if(prob->param->ls == SIPS_LS_COMBINED_A && iter >= 3 && obj_equal(prob->sol->f, ub) && obj_lesser_equal(prob->sol->f, f)) {
      edynasearch(prob, prob->sol);
      if(obj_lesser(prob->sol->f, ub)) {
        if(prob->param->verbose == 1) {
          fprintf(stdout, "UB=");
          print_cost(stdout, prob->sol->f + prob->off);
          fprintf(stdout, "\n");
        }
        ub = prob->sol->f;
      }
    }

    if(prob->param->verbose >= 2) {
      fprintf(stdout, "Iteration %d: tentative UB=", iter);
      print_cost(stdout, ub + prob->off);
      fprintf(stdout, ", true UB=");
      print_current_objective(stdout);
      fprintf(stdout, "\n");
    }

    ret = _stage3_loop(prob, &ub, lboff, u);

    if(ret == SIPS_SOLVED) {
      if(obj_lesser_equal(prob->sol->f, ub)) {
	      break;
      }
      lbbest = ctod(ub);
      lbprev += r;
    } else if(ret == SIPS_MEMLIMIT) {
      if(!prob->param->bisec) {
	      break;
      }
      if(obj_lesser_equal(prob->sol->f, ub)) {
	      lbprev = ctod(ub);
      }
      if(lbprev - lbbest <= 1.0) {
	      break;
      }
      r = (lbprev - lbbest)/2.0;
      lbprev = lbbest + r;
    } else {
      break;
    }
  }

  xfree(u);

  lag2_free_copy(prob);
  lag2_free(prob);

  if(ret != SIPS_SOLVED) {
    if(prob->param->verbose >= 2) {
      fprintf(stdout, "      Lower bound: ");
      print_real(stdout, .4, lbbest + (_real) prob->off);
      fprintf(stdout, "\n");
    }
  } else {
    if(prob->param->verbose >= 2) {
      fprintf(stdout, "    UB Iterations: %d\n", iter);
    }
  }

  return(ret);

}


