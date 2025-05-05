#pragma once

#include "heuristics.h"
#include "utils_dp.h"
#include "dynasearch.h"
#include "utils_io.h"
#include <algorithm>
#include <vector>
#include "subgradient.h"
#include "fixed.h"
#include <chrono>
#include "utils_graph.h"
#include "sips_defines.h"  // Incluir el nuevo archivo de constantes

bool is_fixed(int i) {
    return prob->s[i] == prob->e[i];
}


int stage3_loop(int& ub, double lboff) {
    return SIPS_OK;
/** 
  int i;
  unsigned int max_nedges, max_nnodes;
  unsigned char type;
  unsigned char nmod;
  int ret, heavy;
  _real lb, lb2, lbprev;
  _real mb;
  unsigned int *o;
  _real *ud;
  _solution_t *tmpsol, *tmpsol2;
  _mod_t *mod;

  o = (unsigned int *) xmalloc(prob->n*sizeof(unsigned int));
  mod = (_mod_t *) xmalloc(sizeof(_mod_t));
  mod->m = (unsigned char *) xmalloc(N_JOBS + 2);
  mod->fl = (unsigned char *) xcalloc(prob->n, 1);
  mod->jobs = (int *) xmalloc(prob->n*sizeof(int));
  mod->v = (unsigned char **) xmalloc((N_JOBS + 2)*sizeof(unsigned char *));
  mod->v[0] = (unsigned char *) xmalloc((N_JOBS + 2)*(1<<prob->param->mod));
  ud = (_real *) xcalloc(N_JOBS + 1, sizeof(_real));
  for(i = 1; i <= N_JOBS + 1; i++) {
    mod->v[i] = mod->v[i - 1] + (1<<prob->param->mod);
  }

  tmpsol = create_solution(prob);
  tmpsol2 = create_solution(prob);

  type = 0;
  mod->an = 0;
  mod->n = 0;

  max_nnodes = prob->graph->n_nodes;
  max_nedges = prob->graph->n_edges;

  lag2_reverse(prob);

  lb2 = - LARGE_REAL;
  ret = lag2_solve_LR2m(prob, u, ctod(*ub) - lboff, tmpsol, &lb, o);
  lb += lboff;

  if(ret == SIPS_OPTIMAL) {
    if(obj_lesser(tmpsol->f, prob->sol->f)) {
      copy_solution(prob, prob->sol, tmpsol);
      if(obj_lesser(tmpsol->f, *ub)) {
	    *ub = tmpsol->f;
      }
    }
    ret = SIPS_SOLVED;
  } else if(ret == SIPS_INFEASIBLE) {
    ret = SIPS_SOLVED;
  }

  mb = lag2_get_memory_in_MB(prob) - prob->graph->copy->mem;
  if(prob->param->modsw >= 0 && mb >= (_real) prob->param->modsw) {
    type = 1;
  } else {
    type = 0;
  }

  lbprev = lb;
  nmod = prob->param->mod;
  heavy = SIPS_FALSE;
  while(ret == SIPS_NORMAL) {
    lbprev = max(lbprev, lb);
    mb = ((_real) prob->param->mem - prob->graph->copy->mem
	  - prob->graph->ptable->mem)
      /(lag2_get_memory_in_MB(prob) - prob->graph->copy->mem
	- prob->graph->ptable->mem);
    for(nmod = 0; mb >= 1.0 && nmod <= prob->param->mod; mb /= 2.0, nmod++);
    nmod--;
    if(nmod == 0) {
      nmod = 1;
    }
    if(mod->n + nmod >= prob->n) {
      nmod = prob->n - mod->n - 1;
    }
    ret = lag2_assign_modifiers(prob, type, nmod, mod);

    if(ret == SIPS_INFEASIBLE) {
      goto _loop_end;
    }

    if(is_true(prob->param->zmplier)) {
      if(mod->an == 0) {
	      ret = lag2_solve_LR2m_without_elimination(prob, u, ctod(*ub) - lboff,
						  tmpsol, &lb, o);
      } else {
	      ret = lag2_add_modifiers_LR2m_without_elimination(prob, u,
							  ctod(*ub) - lboff,
							  tmpsol, &lb, o, mod);
      }
    } else {
      lag2_reverse(prob);
      if(mod->an == 0) {
	      ret = lag2_solve_LR2m(prob, u, ctod(*ub) - lboff, tmpsol, &lb, o);
      } else {
	      ret = lag2_add_modifiers_LR2m(prob, u, ctod(*ub) - lboff,
				      tmpsol, &lb, o, mod);
      }
    }
    lb += lboff;

    max_nnodes = max(max_nnodes, prob->graph->n_nodes);
    max_nedges = max(max_nedges, prob->graph->n_edges);

    mb = (lag2_get_memory_in_MB(prob) - prob->graph->copy->mem
	  - prob->graph->ptable->mem)
      /((_real) prob->param->mem - prob->graph->copy->mem
	- prob->graph->ptable->mem);
    if(nmod == 1 && mb >= 0.75) {
      heavy = SIPS_TRUE;
    } else if(nmod > 1 || mb < 0.5) {
      heavy = SIPS_FALSE;
    }

    if((is_true(heavy) || is_true(prob->param->zmplier))
       && ret == SIPS_NORMAL) {
      lag2_reverse(prob);
      ret = lag2_solve_LR2m(prob, u, ctod(*ub) - lboff, tmpsol, &lb2, o);
      lb2 += lboff;
      lb = max(lb, lb2);
      max_nnodes = max(max_nnodes, prob->graph->n_nodes);
      max_nedges = max(max_nedges, prob->graph->n_edges);
    }

    if(ret != SIPS_NORMAL || ctod(*ub) - lb < prob->param->lbeps) {
      goto _loop_end;
    }

    if(is_true(prob->param->zmplier)) {
      ret = lag2_solve_LR2m_without_elimination(prob, ud, ctod(*ub),
						tmpsol2, &lb2, o);
      max_nnodes = max(max_nnodes, prob->graph->n_nodes);
      max_nedges = max(max_nedges, prob->graph->n_edges);

      if(ret != SIPS_NORMAL || ctod(*ub) - lb2 < prob->param->lbeps) {
	copy_solution(prob, tmpsol, tmpsol2);
	lb = lb2;

	goto _loop_end;
      }

      lag2_reverse(prob);
      ret = lag2_solve_LR2m(prob, ud, ctod(*ub), tmpsol2, &lb2, o);
      max_nnodes = max(max_nnodes, prob->graph->n_nodes);
      max_nedges = max(max_nedges, prob->graph->n_edges);

      if(ret != SIPS_NORMAL || ctod(*ub) - lb2 < prob->param->lbeps) {
        copy_solution(prob, tmpsol, tmpsol2);
        lb = lb2;
        goto _loop_end;
      }
    }

    if(lb2 > lb) {
      partialdp(prob, u, tmpsol2);
	edynasearch(prob, tmpsol2);

      if(obj_lesser(tmpsol2->f, *ub)) {
	copy_solution(prob, prob->sol, tmpsol2);
	*ub = tmpsol2->f;
	if(!prob->param->bisec) {
	  lag2_free_copy(prob);
	}
      } else if(obj_lesser(tmpsol2->f, prob->sol->f)) {
	copy_solution(prob, prob->sol, tmpsol2);
      }
    } else if(lb - lbprev > prob->param->eps
	      || is_true(prob->param->ubupd3)) {
      partialdp(prob, u, tmpsol); 
	    edynasearch(prob, tmpsol);
      

      if(obj_lesser(tmpsol->f, *ub)) {
	copy_solution(prob, prob->sol, tmpsol);
	*ub = tmpsol->f;
	if(!prob->param->bisec) {
	  lag2_free_copy(prob);
	}
      } else if(obj_lesser(tmpsol->f, prob->sol->f)) {
	copy_solution(prob, prob->sol, tmpsol);
      }
    }
    if(ctod(*ub) - lb2 < prob->param->lbeps) {
      lb = lb2;
    }

_loop_end:
    if(ret == SIPS_OPTIMAL) {
      if(obj_lesser(tmpsol->f, *ub)) {
	copy_solution(prob, prob->sol, tmpsol);
	*ub = tmpsol->f;
      } else if(obj_lesser(tmpsol->f, prob->sol->f)) {
	copy_solution(prob, prob->sol, tmpsol);
      }
      ret = SIPS_SOLVED;
      break;
    } else if(ret == SIPS_INFEASIBLE
	      || ctod(*ub) - lb < prob->param->lbeps) {
      lb = ctod(*ub);
      ret = SIPS_SOLVED;
      break;
    }

  }


  lag2_free(prob);

  free_solution(tmpsol2);
  free_solution(tmpsol);

  xfree(ud);
  xfree(mod->v[0]);
  xfree(mod->v);
  xfree(mod->jobs);
  xfree(mod->fl);
  xfree(mod->m);
  xfree(mod);
  xfree(o);

  return(ret);
  **/
}

void reverse_solution(_solution_t *sol) {
  for(int i = 0, j = sol->n - 1; i < j; i++, j--) {
    int c = sol->c[i];
    sol->c[i] = sol->c[j];
    sol->c[j] = c;
    _job_t* job = sol->job[i];
    sol->job[i] = sol->job[j];
    sol->job[j] = job;
  }
}

std::pair<std::vector<int>, int> best_heuristic(info* prob, Tmap* tm, T2map* t2m) {
    auto j1 = greedy_fordward(prob, tm);
    auto j2 = greedy_backward(prob, tm);
    auto j3 = shortest_processing_time_order(prob);

    auto c1 = calc_cost(j1, t2m, prob);
    auto c2 = calc_cost(j2, t2m, prob);
    auto c3 = calc_cost(j3, t2m, prob);

    std::vector<std::pair<std::vector<int>, int>> res = {
        {j1, c1}, 
        {j2, c2}, 
        {j3, c3}
    };
    std::sort(res.begin(), res.end(), [](auto p, auto q) {
        return p.second < q.second;
    });
    return res[0];
}

int check_time_window() {
    for (int i = 0; i < prob->n; i++) {
        if (!is_fixed(i) && (prob->s[i] > prob->T || prob->e[i] < 0)) {
            return SIPS_INFEASIBLE;
        }
    }

    std::vector<int> orig_s(prob->n);
    std::vector<int> orig_e(prob->n);
    for (int i = 0; i < prob->n; i++) {
        orig_s[i] = prob->s[i];
        orig_e[i] = prob->e[i];
    }

    if (!check_dominance()) {
        return SIPS_INFEASIBLE;
    }

    for (int i = 0; i < prob->n; i++) {
        for (int t = orig_s[i]; t < prob->s[i]; t++) {
            delete_time(t);
        }
        for (int t = orig_e[i]; t > prob->e[i]; t--) {
            delete_time(t);
        }
    }

    return SIPS_NORMAL;
}

// Inicializa los nodos del grafo en dirección forward
int initialize_node_forward() {
    // Usar el array de nodos ya inicializado
    Node2m** node = (Node2m**)subl_graph->node;
    subl_graph->n_nodes = 0;
    subl_graph->n_edges = 0;
    subl_graph->direction = SIPS_FORWARD;

    Node2m* initial_node = new Node2m();
    initial_node->j[0] = 0;
    initial_node->j[1] = prob->n + 2;
    initial_node->j[2] = prob->n + 2;
    initial_node->m = 0;
    initial_node->ty[0] = 1;
    initial_node->v[0] = 0.0;
    initial_node->n[0] = initial_node;
    initial_node->e = nullptr;
    initial_node->next = nullptr;
    
    // Asignar nodo inicial al tiempo 0
    node[0] = initial_node;
    subl_graph->n_nodes++;
    
    // Para cada tiempo t y cada job j
    for (int tt = 1; tt <= prob->T; tt++) {
        if (!check_time(tt)) {
            continue;
        } else if (node[tt] == nullptr) {
            delete_time(tt);
            continue;
        }
        
        for (int j = 1; j <= prob->n; j++) {
            int t = tt + prob->p[j];
            if (t > prob->T || !check_time(t) || !check_node(t, j)) {
	            continue;
            }
            double g = -prob->u[j] + tmap[j].f[t];
            double v1 = 1e18;
            double v2 = 1e18;
            Node2m* pr1 = NULL;
            Node2m* pr2 = NULL;
            bool ty1 = 0;
            bool ty2 = 0;
            Edge2m* e = NULL;

            for (Node2m* it = node[tt]; it; it = it->next) {
                if (!check_edge(t, it->j[0], j)) {
                    continue;
                }

                //if (!check_func(t, j, it)) {
                //    continue;
                //}
                double f = 0;
                int ty = 0;
                if (it->n[0]->j[0] == j) {
                    if (!it->n[1]) continue;
                    f = it->v[1] + g;
                    ty = 1;
                } else {
                    f = it->v[0] + g;
                    ty = 0;
                }

                if(f < v1) {
                    v2 = v1;
                    v1 = f;
                    pr2 = pr1;
                    pr1 = it;
                    ty2 = ty1;
                    ty1 = ty;
                } else if(f < v2) {
                    v2 = f;
                    pr2 = it;
                    ty2 = ty;
                }

                auto e2 = new Edge2m();
                subl_graph->n_edges++;
                e2->n = it;
                e2->next = e;
                e = e2;
            }

            if(e) {
                auto jt = new Node2m();
                ptable->occ[j]++;
                subl_graph->n_nodes++;
                jt->j[0] = j;
                jt->j[1] = prob->n + 2;
                jt->j[2] = prob->n + 2;
                jt->v[0] = v1;
                jt->v[1] = v2;
                jt->ty[0] = ty1;
                jt->ty[1] = ty2;
                jt->n[0] = pr1;
                jt->n[1] = pr2;
                jt->next = node[t];
                node[t] = jt;
                jt->e = e;
            } else {
                delete_node(t, j);
            }
        }
    }

    auto fnode = new Node2m();
    subl_graph->n_nodes++;
    fnode->j[0] = prob->n + 1;
    fnode->j[1] = fnode->j[2] = prob->n + 2;
    fnode->next = nullptr;
    fnode->e = nullptr;
    node[prob->T + 1] = fnode;
    
    double f = 1e18;
    Node2m* pr = nullptr;

    for (auto it = node[prob->T]; it; it = it->next) {
        auto e = new Edge2m();
        subl_graph->n_edges++;
        e->n = it;
        e->next = fnode->e;
        fnode->e = e;
        if (it->v[0] < f) {
            f = it->v[0];
            pr = it;
        }
    }

    fnode->v[0] = f;
    fnode->v[1] = 1e18;
    fnode->n[0] = pr;
    fnode->n[1] = nullptr;
    fnode->ty[0] = false;
    fnode->ty[1] = false;

    if (fnode->e == nullptr) {
        return SIPS_INFEASIBLE;
    }

    for (int i=1; i <= prob->n; i++) {
        while (prob->s[i] <= prob->e[i] && !check_node(prob->s[i], i)) {
            prob->s[i]++;
        }
        if (prob->s[i] > prob->e[i]) {
            return SIPS_INFEASIBLE;
        }
        while (prob->s[i] <= prob->e[i] && !check_node(prob->e[i], i)) {
            prob->e[i]--;
        }
    }
    return SIPS_NORMAL;
}

int get_sol_LR2m_forward(double* lb, _solution_t* sol) {

    auto node = (Node2m**)subl_graph->node;
    auto& fixed = fixed_jobs;
    
    auto o = subl_graph->occ;

    auto it = node[prob->T + 1];
    if (it == nullptr) {
        *lb = 1e18;
        return SIPS_INFEASIBLE;
    }

    *lb = it->v[0] + fixed->ftail;

    for(int i = 0; i < fixed->ntail; i++) {
        *lb -= prob->u[fixed->job[prob->n - i - 1]->no];
    }

    if(*lb > 1e17) {
        return SIPS_INFEASIBLE;
    }
    
    int ty = 0;
    it = it->n[0];
    int t = prob->T;

    int ret = SIPS_OPTIMAL;
    if (sol == nullptr) {
        if (o == nullptr) {
            return SIPS_NORMAL;
        }
        for (int i = 1; i <= prob->n; i++) {
            o[i] = 0;
        }
        for (int i = 0; i < fixed->nhead; i++) {
            o[fixed->job[i]->no]++;
        }
        for (int i = 0; i < fixed->ntail; i++) {
            o[fixed->job[prob->n - i - 1]->no]++;
        }
        while (1 <= it->j[0] && it->j[0] <= prob->n) {
            o[it->j[0]]++;
            if (1 <= it->j[1] && it->j[1] <= prob->n) {
                o[it->j[1]]++;
            }
            if (1 <= it->j[2] && it->j[2] <= prob->n) {
                o[it->j[2]]++;
            }
            int pty = it->ty[ty];
            it = it->n[ty];
            ty = pty;
        }
        for (int i = 1; i <= prob->n; i++) {
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
    
    if(o != NULL) {
        for (int i = 1; i <= prob->n; i++) {
            o[i] = 0;
        }
        for (int i = 0; i < fixed->nhead; i++) {
            o[fixed->job[i]->no]++;
        }
        for (int i = 0; i < fixed->ntail; i++) {
            o[fixed->job[prob->n - i - 1]->no]++;
        }

        while(1 <= it->j[0] && it->j[0] <= prob->n) {
            for (int i = 0; i <= 2; ++i) {
                if (1 <= it->j[i] && it->j[i] <= prob->n) {
                    sol->f += tmap[it->j[i]].f[t];
                    o[it->j[i]]++;
                    sol->job[sol->n]->no = it->j[i];
                    sol->c[sol->n++] = t;
                    t -= prob->p[it->j[i]];
                }
            }
            int pty = it->ty[ty];
            it = it->n[ty];
            ty = pty;
        }

        for(int i = 1; i <= prob->n; i++) {
            if(o[i] != 1) {
                ret = SIPS_NORMAL;
                break;
            }
        }
    } else {
        while(1 <= it->j[0] && it->j[0] <= prob->n) {
            for (int i = 0; i <= 2; ++i) {
                if (1 <= it->j[i] && it->j[i] <= prob->n) {
                    sol->f += tmap[it->j[i]].f[t];
                    sol->job[sol->n]->no = it->j[i];
                    sol->c[sol->n++] = t;
                    t -= prob->p[it->j[i]];
                }
            }
            int pty = it->ty[ty];
            it = it->n[ty];
            ty = pty;
        }
        ret = SIPS_NORMAL;
    }

    sol->f += fixed->fhead;
    for(int i = fixed->nhead - 1; i >= 0; i--) {
        sol->job[sol->n] = fixed->job[i];
        sol->c[sol->n++] = fixed->c[i];
    }

    reverse_solution(sol);

    if(fixed->nhead + fixed->ntail == prob->n) {
        return SIPS_OPTIMAL;
    }

    return ret;
}


int get_sol_LR2m_backward(double* lb, _solution_t* sol) {
    Node2m** node = &subl_graph->node;
    auto& fixed = fixed_jobs;\

    auto o = subl_graph->occ;

    auto it = node[prob->Tmin];
    if(it == nullptr) {
        *lb = 1e18;
        if(sol != nullptr) {
        sol->f = 1e18;
        }
        return SIPS_INFEASIBLE;
    }

    *lb = it->v[0] + fixed->fhead;
    for(int i = 0; i < fixed->nhead; i++) {
        *lb -= prob->u[fixed->job[i]->no];
    }

    if(*lb > 1e17) {
        if(sol != nullptr) {
        sol->f = 1e18;
        }
        return SIPS_INFEASIBLE;
    }
    int ty = 0;
    it = it->n[0];
    int t = prob->Tmin;

    int ret = SIPS_OPTIMAL;
    if(sol == nullptr) {
        if(o == nullptr) {
            return SIPS_NORMAL;
        }
        for(int i = 1; i <= prob->n; i++) {
            o[i] = 0;
        }
        for(int i = 0; i < fixed->nhead; i++) {
            o[fixed->job[i]->no]++;
        }
        for(int i = 0; i < fixed->ntail; i++) {
            o[fixed->job[prob->n - i - 1]->no]++;
        }

        while (1 <= it->j[0] && it->j[0] <= prob->n) {
            for (int i = 0; i <= 2; ++i) {
                if (1 <= it->j[i] && it->j[i] <= prob->n) {
                    o[it->j[i]]++;
                }
            }
            int pty = it->ty[ty];
            it = it->n[ty];
            ty = pty;
        }

        for(int i = 1; i <= prob->n; i++) {
            if(o[i] != 1) {
                ret = SIPS_NORMAL;
                break;
            }
        }

        return ret;
    }

    sol->n = 0;
    sol->f = fixed->fhead;
    for(int i = 0; i < fixed->nhead; i++) {
        sol->job[sol->n] = fixed->job[i];
        sol->c[sol->n++] = fixed->c[i];
    }

    if(o != nullptr) {
        for (int i = 1; i <= prob->n; i++) {
            o[i] = 0;
        }
        for (int i = 0; i < fixed->nhead; i++) {
            o[fixed->job[i]->no]++;
        }
        for (int i = 0; i < fixed->ntail; i++) {
            o[fixed->job[prob->n - i - 1]->no]++;
        }
        while(1 <= it->j[0] && it->j[0] <= prob->n) {
            for (int i=2; i>=0; i--) {
                if(1 <= it->j[i] && it->j[i] <= prob->n) {
                    t += prob->p[it->j[i]];
                    o[it->j[i]]++;
                    sol->f += tmap[it->j[i]].f[t];
                    sol->job[sol->n]->no = it->j[i];
                    sol->c[sol->n++] = t;
                }
            }
            int pty = it->ty[ty];
            it = it->n[ty];
            ty = pty;
        }
        for(int i = 1; i <= prob->n; i++) {
            if(o[i] != 1) {
                ret = SIPS_NORMAL;
                break;
            }
        }
    } else {
        while(1 <= it->j[0] && it->j[0] <= prob->n) {
            for (int i=2; i>=0; i--) {
                if(1 <= it->j[i] && it->j[i] <= prob->n) {
                    t += prob->p[it->j[i]];
                    sol->f += tmap[it->j[i]].f[t];
                    sol->job[sol->n]->no = it->j[i];
                    sol->c[sol->n++] = t;
                }
            }
            int pty = it->ty[ty];
            it = it->n[ty];
            ty = pty;
        }
        ret = SIPS_NORMAL;
    }

    sol->f += fixed->ftail;
    for(int i = fixed->ntail - 1; i >= 0; i--) {
        sol->job[sol->n] = fixed->job[prob->n - i - 1];
        sol->c[sol->n++] = fixed->c[prob->n - i - 1];
    }

    if(fixed->nhead + fixed->ntail == prob->n) {
        return SIPS_OPTIMAL;
    }

    return ret;
}


int get_sol_LR2m(double ub, _solution_t* solution, double* lb) {
    int ret;
    if (subl_graph->direction == SIPS_FORWARD) {
        ret = get_sol_LR2m_forward(lb, solution);
    } else {
        ret = get_sol_LR2m_backward(lb, solution);
    }
    if (ret != SIPS_INFEASIBLE) {
        if (ub - *lb < 1.0 - prob->eps) {
            *lb = ub;
            return SIPS_OPTIMAL;    
        }
    }
    return ret;
}


int initialize_node() {
    int ret;

    if (subl_graph == nullptr) {
        subl_graph = new SublGraph();
        subl_graph->n_nodes = 0;
        subl_graph->n_edges = 0;
        fixed_jobs = create_fixed();
        subl_graph->node = new Node2m[prob->T + 2]();  // Inicializa a nullptr
        ret = initialize_node_forward();
        
        if (ret == SIPS_NORMAL) {
            ret = check_time_window();
        }
    } else {
        ret = SIPS_NORMAL;
    }

    return ret;
}


int lag2_initialize(double ub, double* lb, _solution_t* sol) {
    int ret;
    if (subl_graph == nullptr) {
        ret = initialize_node();
        if (ret != SIPS_MEMLIMIT && ret != SIPS_INFEASIBLE) {
            ret = get_sol_LR2m(ub, sol, lb);
        }
    } else {
        ret = SIPS_NORMAL;
    }

    return ret;
}

bool check_func(int t, int a, int b) {
    // Verificar si podemos agregar aristas desde nodos anteriores
    bool has_valid_edge = false;
    
    // Verificar aristas desde nodos anteriores
    for (int prev_t = 0; prev_t < t; prev_t++) {
        Node2m* prev_node = subl_graph->node;
        while (prev_node != nullptr) {
            // Solo considerar nodos que terminan en tiempo prev_t
            if (prev_t + prob->p[prev_node->j[0]] == t && check_edge(t, prev_node->j[0], b)) {
                // Crear arista
                Edge2m* edge = new Edge2m();
                edge->n = prev_node;
                edge->next = nullptr;
                
                // Agregar arista al nodo actual
                if (subl_graph->node->e == nullptr) {
                    subl_graph->node->e = edge;
                } else {
                    Edge2m* last = subl_graph->node->e;
                    while (last->next != nullptr) {
                        last = last->next;
                    }
                    last->next = edge;
                }
                
                subl_graph->n_edges++;
                has_valid_edge = true;
            }
            prev_node = prev_node->next;
        }
    }
    
    return has_valid_edge;
}

// Libera la memoria del grafo de sublimación
void free_subl_graph() {
    if (!subl_graph) {
        return;
    }
    
    // Liberar todos los nodos y aristas
    Node2m* node = subl_graph->node;
    while (node) {
        // Liberar aristas
        Edge2m* edge = node->e;
        while (edge) {
            Edge2m* next_edge = edge->next;
            delete edge;
            edge = next_edge;
        }
        
        Node2m* next_node = node->next;
        delete node;
        node = next_node;
    }
    
    // Liberar el grafo
    delete subl_graph;
    subl_graph = nullptr;
}

int lag2_push_nodes() {
    delete subl_graph_copy->node;
    //ptable_free(ptable);
    free_fixed(fixed_jobs);
    subl_graph_copy->node = subl_graph->node;
    ptable_copy = ptable;
    fixed_jobs_copy = fixed_jobs;

    subl_graph_copy->direction = subl_graph->direction;
    subl_graph_copy->n_nodes = subl_graph->n_nodes;
    subl_graph_copy->n_edges = subl_graph->n_edges;
    subl_graph_copy->Tmin = subl_graph->Tmin;
    subl_graph_copy->Tmax = subl_graph->Tmax;

    subl_graph->node = nullptr;
    ptable = nullptr;
    fixed_jobs = nullptr;
    subl_graph->Tmin = 0;
    subl_graph->Tmax = 0;
    subl_graph->n_nodes = 0;
    subl_graph->n_edges = 0;
    return SIPS_OK;
}

int lag2_pop_nodes() {
  delete subl_graph->node;
  //ptable_free(ptable);
  free_fixed(fixed_jobs);

  subl_graph->node = subl_graph_copy->node;
  ptable = ptable_copy;
  fixed_jobs = fixed_jobs_copy;
  subl_graph->direction = subl_graph_copy->direction;
  subl_graph->n_nodes = subl_graph_copy->n_nodes;
  subl_graph->n_edges = subl_graph_copy->n_edges;
  subl_graph->Tmin = subl_graph_copy->Tmin;
  subl_graph->Tmax = subl_graph_copy->Tmax;

  subl_graph_copy->node = nullptr;
  ptable_copy = nullptr;
  fixed_jobs_copy = nullptr;
  subl_graph_copy->Tmin = 0;
  subl_graph_copy->Tmax = 0;
  subl_graph_copy->n_nodes = 0;
  subl_graph_copy->n_edges = 0;
  return SIPS_OK;
}
void lag2_recover_nodes_forward() {
    return;
    /** 
    int t, tt, tt2;
  int i;
  _node2m_t **snode, **node, ***ntable;
  _node2m_t *csnode, **pnode, *cnode;
  _edge2m_t *e, **pe;
  _benv_t *bnode, *bedge;

  snode = (_node2m_t **) prob->graph->copy->node2;
  node = (void *) xcalloc(prob->T + 2, sizeof(_node2m_t *));
  bnode = create_benv(sizeof(_node2m_t));
  bedge = create_benv(sizeof(_edge2m_t));

  ntable = (_node2m_t ***) xmalloc((prob->pmax + 1)*sizeof(_node2m_t **));
  ntable[0] = (_node2m_t **) xmalloc((prob->pmax + 1)*(N_JOBS + 1)
				     *sizeof(_node2m_t *));
  for(i = 1; i <= prob->pmax; i++) {
    ntable[i] = ntable[i - 1] + N_JOBS + 1;
  }

  node[prob->graph->Tmin] = cnode = alloc_memory(bnode);
  memset((void *) cnode, 0, sizeof(_node2m_t));
  cnode->j[0] = snode[prob->graph->Tmin]->j[0];
  cnode->j[1] = snode[prob->graph->Tmin]->j[1];
  cnode->j[2] = snode[prob->graph->Tmin]->j[2];
  cnode->n[0] = cnode;
  memset((void *) ntable[prob->graph->Tmin%(prob->pmax + 1)], 0,
	 (N_JOBS + 1)*sizeof(_node2m_t *));
  ntable[prob->graph->Tmin%(prob->pmax + 1)][cnode->j[0]] = cnode;

  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
    pnode = node + t;
    tt = t%(prob->pmax + 1);
    memset((void *) ntable[tt], 0, (N_JOBS + 1)*sizeof(_node2m_t *));

    for(csnode = snode[t]; csnode != NULL; csnode = csnode->next) {
      tt2 = (t - prob->sjob[csnode->j[0]]->p)%(prob->pmax + 1);

      *pnode = cnode = alloc_memory(bnode);
      cnode->j[0] = csnode->j[0];
      cnode->j[1] = csnode->j[1];
      cnode->j[2] = csnode->j[2];
      cnode->ty = csnode->ty;
      cnode->v[0] = csnode->v[0];
      cnode->v[1] = csnode->v[1];

      if(csnode->n[0] != NULL) {
	cnode->n[0] = ntable[tt2][csnode->n[0]->j[0]];
	if(csnode->n[1] != NULL) {
	  cnode->n[1] = ntable[tt2][csnode->n[1]->j[0]];
	} else {
	  cnode->n[1] = NULL;
	}
      } else {
	cnode->n[0] = NULL;
	cnode->n[1] = NULL;
      }

      for(pe = &(cnode->e), e = csnode->e; e != NULL;
	  e = e->next, pe = &((*pe)->next)) {
	*pe = alloc_memory(bedge);
	(*pe)->n = ntable[tt2][e->n->j[0]];
      }
      *pe = NULL;

      ntable[tt][cnode->j[0]] = cnode;
      pnode = &(cnode->next);      
    }
    *pnode = NULL;
  }

  tt2 = prob->graph->Tmax%(prob->pmax + 1);
  csnode = snode[prob->graph->Tmax + 1];

  node[prob->graph->Tmax + 1] = cnode = alloc_memory(bnode);
  cnode->j[0] = csnode->j[0];
  cnode->j[1] = csnode->j[1];
  cnode->j[2] = csnode->j[2];
  cnode->ty = csnode->ty;
  cnode->v[0] = csnode->v[0];
  cnode->v[1] = csnode->v[1];
  cnode->n[0] = ntable[tt2][csnode->n[0]->j[0]];
  cnode->n[1] = NULL;
  cnode->next = NULL;

  for(pe = &(cnode->e), e = csnode->e; e != NULL;
      e = e->next, pe = &((*pe)->next)) {
    *pe = alloc_memory(bedge);
    (*pe)->n = ntable[tt2][e->n->j[0]];
  }
  *pe = NULL;

  prob->graph->node2 = (void **) node;
  prob->graph->bnode = bnode;
  prob->graph->bedge = bedge;

  xfree(ntable[0]);
  xfree(ntable);
  **/
}

int lag2_recover_nodes() {
    return SIPS_OK;
    /** 
    if(subl_graph_copy->node == nullptr) {
        return SIPS_FAIL;
    }
    delete subl_graph->node;
    ptable_free(ptable);
    free_fixed(fixed_jobs);

    subl_graph->n_nodes = subl_graph_copy->n_nodes;
    subl_graph->n_edges = subl_graph_copy->n_edges;
    subl_graph->Tmin = subl_graph_copy->Tmin;
    subl_graph->Tmax = subl_graph_copy->Tmax;
    subl_graph->direction = subl_graph_copy->direction;
    ptable = duplicate_ptable(ptable_copy);
    fixed_jobs = duplicate_fixed(fixed_jobs_copy);
    
    lag2_recover_nodes_forward();
    return SIPS_OK;
    **/
}

void stage3() {
    
}



int stage32() {
    if (prob->verbose >= 2) {
        std::cout << "Iniciando fase 3 del algoritmo SSDP" << std::endl;
    }
    std::vector<double> u(prob->n + 2);
    for (int i = 0; i <= prob->n + 1; i++) {
        u[i] = prob->u[i];
    }
    double lboff = prob->su;       
    double lb;
    _solution_t* tmpsol = new _solution_t();
    int ret = lag2_initialize(prob->ub - lboff, &lb, tmpsol);
    lb += lboff;
    

    if(ret == SIPS_OPTIMAL) {
        if (tmpsol->f < prob->ub) {
            //copy_solution(prob, prob->sol, tmpsol);
        }
    }

    //free_solution(tmpsol);
    if(ret == SIPS_OPTIMAL || ret == SIPS_INFEASIBLE) {
        ret = SIPS_SOLVED;
    }
    if(ret != SIPS_NORMAL) return ret;
    lag2_push_nodes();
    double lbprev;
    double r;
    if (subl_graph->n_edges >= 1e7) {
        lbprev = (prob->ub + lb) / 2.0;
        r = (prob->ub - lbprev) / 2.0;
    } else {
        lbprev = prob->ub;
        r = prob->ub;
    }

    double lbbest = lb;
    double f = prob->ub;
    int iter = 1;
    while (true) {
        int tub = std::max(std::round(lbprev + prob->eps), std::ceil(lbbest + prob->eps));
        if(prob->ub <= tub) {
            tub = prob->ub;
            ret = lag2_pop_nodes();
        } else {
            ret = lag2_recover_nodes();
        }
        if (iter >= 3 && prob->ub == tub && prob->ub <= f) {
            std::vector<int> order(prob->n);
            for (int i = 0; i < prob->n; i++) {
                order[i] = prob->sol[i];
            }
            auto [dsol, cost] = dynasearch(order);
            if (cost < prob->ub) {
                prob->ub = cost;
                for (int i = 0; i < prob->n; i++) {
                    prob->sol[i] = dsol[i];
                }
            }
            if (prob->ub < tub) {
                tub = prob->ub;
            }
        }

        ret = stage3_loop(tub, lboff);

        if(ret == SIPS_SOLVED) {
            if (prob->ub <= tub) break;
            lbbest = tub;
            lbprev += r;
        } 
        else {
            break;
        }
        iter++;
    }
    return ret;
}

int _lag2_solve_LR2m_forward(double ub, unsigned char eflag) {
    return SIPS_OK;
/** 
  int t, tt[2];
  int i;
  int dlen;
  char ty, ty1, ty2;
  _real f, g;
  _real v1, v2;
  unsigned int n_edges;
  _ptable_t *ptable;
  _benv_t *bedge;
  _node2m_t **node, *cnode, *cnode2, *cnode3, **pnode, *pr1, *pr2;
  _edge2m_t *e, *e2, **pe;

  n_edges = prob->graph->n_edges;
  node = (_node2m_t **) prob->graph->node2;
  ptable = prob->graph->ptable;
  bedge = prob->graph->bedge;

  memset((void *) ptable->occ, 0, (N_JOBS + 2)*sizeof(unsigned int));
  for(i = 0; i < N_JOBS; i++) {
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

  node[prob->graph->Tmin]->v[0] = ctod(prob->graph->fixed->fhead);
  for(i = 0; i < prob->graph->fixed->nhead; i++) {
    node[prob->graph->Tmin]->v[0] -= u[prob->graph->fixed->job[i]->no];
  }
  node[prob->graph->Tmin]->n[0] = node[prob->graph->Tmin];
  node[prob->graph->Tmin]->ty = 0;

  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
    for(pnode = node + t; *pnode != NULL; pnode = &(cnode->next)) {
      cnode = *pnode;
      if(ELIMINATED_NODE(cnode)) {
      	REMOVE_EDGES(cnode);
	      continue;
      }

      if(IS_REAL_JOB(cnode->j[0]) && prec_get_dom_ti(t, cnode->j[0])) {
	      REMOVE_EDGES(cnode);
	      ELIMINATE_NODE(cnode);
	      continue;
      }
      g = - u[cnode->j[0]] + ctod(prob->sjob[cnode->j[0]]->f[t]);

      tt[0] = tt[1] = t;
      if(IS_JOB(cnode->j[1])) {
	      tt[0] -= prob->sjob[cnode->j[0]]->p;
	      if(IS_REAL_JOB(cnode->j[1]) && prec_get_dom_ti(tt[0], cnode->j[1])) {
	        REMOVE_EDGES(cnode);
	        ELIMINATE_NODE(cnode);
	        continue;
	      }
	      g += - u[cnode->j[1]] + ctod(prob->sjob[cnode->j[1]]->f[tt[0]]);

        if(IS_JOB(cnode->j[2])) {
          tt[1] = tt[0] - prob->sjob[cnode->j[1]]->p;
          if(IS_REAL_JOB(cnode->j[2]) && prec_get_dom_ti(tt[1], cnode->j[2])) {
            REMOVE_EDGES(cnode);
            ELIMINATE_NODE(cnode);
            continue;
          }
          g += - u[cnode->j[2]] + ctod(prob->sjob[cnode->j[2]]->f[tt[1]]);
        }
      }

      v1 = v2 = LARGE_REAL;
      pr1 = pr2 = NULL;
      ty1 = ty2 = 0;

      pe = &(cnode->e);
      while(*pe != NULL) {
        cnode2 = (*pe)->n;
	      if(ELIMINATED_NODE(cnode2) || HAVE_COMMON_REAL_JOB(cnode, cnode2)) {
	        REMOVE_SINGLE_EDGE;
	        continue;
      	}

	      cnode3 = cnode2->n[0];
	      if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
	        if(cnode2->n[1] == NULL) {
	          REMOVE_SINGLE_EDGE;
	          continue;
	        } else if(cnode2->e->next->next == NULL) {
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
	        if(ub - (f + cnode->v[0]) < prob->param->lbeps) {
	          REMOVE_SINGLE_EDGE;
	          continue;
	        } else if(ub - (f + cnode->v[1]) < prob->param->lbeps) {
	          cnode3 = cnode->n[0];
	          if(HAVE_COMMON_REAL_JOB(cnode2, cnode3)) {
	            REMOVE_SINGLE_EDGE;
	            continue;
	          }
	        }

	        if(_check_adj_supernode_forward(prob, cnode2, cnode, t)) {
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

      if(pr1 == NULL) {
	ELIMINATE_NODE(cnode);
	continue;
      }

      cnode->v[0] = v1;
      cnode->v[1] = v2;
      cnode->ty = CONV_TYPE(ty1, ty2);
      cnode->n[0] = pr1;
      cnode->n[1] = pr2;

      cnode2 = cnode->e->n;
#ifdef SIPSI
      if(eflag && cnode->e->next == NULL && IS_JOB(cnode2->j[0])) {
	if(IS_JOB(cnode->j[2])) {
	  if(IS_REAL_JOB(cnode->j[2]) || IS_REAL_JOB(cnode2->j[0])
	     || IS_JOB(cnode2->j[1])) {
	    goto _forward_next;
	  }
	  dlen = cnode->j[2] + cnode2->j[0] + 2 - 2*prob->n;
	  if(dlen > prob->dn) {
	    goto _forward_next;
	  }
	  cnode->j[2] = (unsigned short) (prob->n + dlen - 1);
	} else if(IS_JOB(cnode->j[1])) {
	  if(IS_JOB(cnode2->j[2])) {
	    goto _forward_next;
	  } else if(IS_REAL_JOB(cnode->j[1]) || IS_REAL_JOB(cnode2->j[0])) {
	    if(IS_JOB(cnode2->j[1])) {
	      goto _forward_next;
	    }
	    cnode->j[2] = cnode2->j[0];
	  } else {
	    dlen = cnode->j[1] + cnode2->j[0] + 2 - 2*prob->n;
	    if(dlen > prob->dn) {
	      if(IS_JOB(cnode2->j[1])) {
		goto _forward_next;
	      }
	      cnode->j[1] = (unsigned short) (prob->n + prob->dn - 1);
	      cnode->j[2]
		= (unsigned short) (prob->n + dlen - prob->dn - 1);
	    } else {
	      cnode->j[1] = (unsigned short) (prob->n + dlen - 1);
	      cnode->j[2] = cnode2->j[1];
	    }
	  }
	} else if(IS_REAL_JOB(cnode->j[0]) || IS_REAL_JOB(cnode2->j[0])) {
	  if(IS_JOB(cnode2->j[2])) {
	    goto _forward_next;
	  }
	  cnode->j[1] = cnode2->j[0];
	  cnode->j[2] = cnode2->j[1];
	} else {
	  dlen = cnode->j[0] + cnode2->j[0] + 2 - 2*prob->n;
	  if(dlen > prob->dn) {
	    if(IS_JOB(cnode2->j[2])) {
	      goto _forward_next;
	    }
	    cnode->j[0] = (unsigned short) (prob->n + prob->dn - 1);
	    cnode->j[1]
	      = (unsigned short) (prob->n + dlen - prob->dn - 1);
	    cnode->j[2] = cnode2->j[1];
	  } else {
	    cnode->j[0] = (unsigned short) (prob->n + dlen - 1);
	    cnode->j[1] = cnode2->j[1];
	    cnode->j[2] = cnode2->j[2];
	  }
	}

	free_memory(bedge, cnode->e);
	n_edges--;

	if(IS_JOB(cnode->j[2]) && _check_supernode(prob, cnode->j, t)) {
	  ELIMINATE_NODE(cnode);
	  continue;
	}

	pe = &(cnode->e);
	if(NOT_JOB(cnode->j[1])) {
	  cnode->v[0] = cnode2->v[0] + g;
	  cnode->v[1] = cnode2->v[1] + g;
	  cnode->n[0] = cnode2->n[0];
	  cnode->n[1] = cnode2->n[1];
	  cnode->ty = cnode2->ty;
	    
	  for(e = cnode2->e; e != NULL; e = e->next) {
	    e2 = alloc_memory(bedge);
	    n_edges++;
	    e2->n = e->n;
	    *pe = e2;
	    pe = &(e2->next);
	  }
	  *pe = NULL;
	} else {
	  g = - u[cnode->j[0]] + ctod(prob->sjob[cnode->j[0]]->f[t]);
	  tt[0] = tt[1] = t;
	  if(IS_JOB(cnode->j[1])) {
	    tt[0] -= prob->sjob[cnode->j[0]]->p;
	    g += - u[cnode->j[1]] + ctod(prob->sjob[cnode->j[1]]->f[tt[0]]);
	    if(IS_JOB(cnode->j[2])) {
	      tt[1] = tt[0] -  prob->sjob[cnode->j[1]]->p;
	      g += - u[cnode->j[2]] + ctod(prob->sjob[cnode->j[2]]->f[tt[1]]);
	    }
	  }

	  v1 = v2 = LARGE_REAL;
	  pr1 = pr2 = NULL;
	  ty1 = ty2 = 0;
	  for(e = cnode2->e; e != NULL; e = e->next) {
	    cnode2 = e->n;
	    if(HAVE_REAL_JOB(cnode->j[2], cnode2)
	       || HAVE_REAL_JOB(cnode->j[1], cnode2)
	       || HAVE_REAL_JOB(cnode->j[0], cnode2)) {
	      continue;
	    }

	    cnode3 = cnode2->n[0];
	    if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
	      if(cnode2->n[1] == NULL) {
		continue;
	      } else if(cnode2->e->next->next == NULL) {
		cnode3 = cnode2->n[1];
		if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
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
	    
	    e2 = alloc_memory(bedge);
	    n_edges++;
	    e2->n = cnode2;
	    *pe = e2;
	    pe = &(e2->next);
	  }
	  *pe = NULL;

	  if(cnode->e == NULL) {
	    ELIMINATE_NODE(cnode);
	    continue;
	  }

	  cnode->v[0] = v1;
	  cnode->v[1] = v2;
	  cnode->ty = CONV_TYPE(ty1, ty2);
	  cnode->n[0] = pr1;
	  cnode->n[1] = pr2;
	}
      }
#else
  if(eflag && cnode->e->next == NULL && IS_JOB(cnode2->j[0]) && NOT_JOB(cnode->j[2]) && NOT_JOB(cnode2->j[2])) {
	    
    if(IS_JOB(cnode->j[1])) {
	    if(NOT_JOB(cnode2->j[1])) {
	      cnode->j[2] = cnode2->j[0];
	    } else {
	      goto _forward_next;
	    }
    } else {
      cnode->j[1] = cnode2->j[0];
      cnode->j[2] = cnode2->j[1];
    }

	    free_memory(bedge, cnode->e);
    	n_edges--;

	  if(IS_JOB(cnode->j[2]) && _check_supernode(prob, cnode->j, t)) {
	    ELIMINATE_NODE(cnode);
	    continue;
	  }

	g = - u[cnode->j[0]] + ctod(prob->sjob[cnode->j[0]]->f[t]);
	tt[0] = t - prob->sjob[cnode->j[0]]->p;
	g += - u[cnode->j[1]] + ctod(prob->sjob[cnode->j[1]]->f[tt[0]]);
	if(IS_JOB(cnode->j[2])) {
	  tt[1] = tt[0] - prob->sjob[cnode->j[1]]->p;
	  g += - u[cnode->j[2]] + ctod(prob->sjob[cnode->j[2]]->f[tt[1]]);
	}

	v1 = v2 = LARGE_REAL;
	pr1 = pr2 = NULL;
	ty1 = ty2 = 0;
	pe = &(cnode->e);
	for(e = cnode2->e; e != NULL; e = e->next) {
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
	    if(cnode2->n[1] == NULL) {
	      continue;
	    } else if(cnode2->e->next->next == NULL) {
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

	  e2 = alloc_memory(bedge);
	  n_edges++;
	  e2->n = cnode2;
	  *pe = e2;
	  pe = &(e2->next);
	}
	*pe = NULL;

	if(cnode->e == NULL) {
	  ELIMINATE_NODE(cnode);
	  continue;
	}

        cnode->v[0] = v1;
        cnode->v[1] = v2;
        cnode->ty = CONV_TYPE(ty1, ty2);
        cnode->n[0] = pr1;
        cnode->n[1] = pr2;
      }
#endif

    _forward_next:
      UPDATE_WINDOWS;
    }
  }

  cnode = node[prob->graph->Tmax + 1];
  cnode->v[0] = cnode->v[1] = LARGE_REAL;
  cnode->n[0] = cnode->n[1] = NULL;
  cnode->ty = 0;
  pe = &(cnode->e);
  while(*pe != NULL) {
    cnode2 = (*pe)->n;
    if(ELIMINATED_NODE(cnode2)) {
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

  if(cnode->e == NULL) {
    return(SIPS_INFEASIBLE);
  }

  _lag2_free_eliminated_nodes(prob);

  return(_lag2_shrink_horizon_tail(prob, u));
  **/
}

/*
 * _lag2_solve_LR2m_backward(prob, u, ub, eflag)
 *   solves (LR2m) by backward DP.
 *        u: Lagrangian multipliers
 *       ub: upper bound
 *    eflag: perform state elimination (1)
 *
 */
int _lag2_solve_LR2m_backward(double ub, unsigned char eflag) {
    return SIPS_OK;
/** 
  int t, tt[2];
  int i;
  char ty, ty1, ty2;
  _real f, g;
  _real v1, v2;
  unsigned int n_edges;
  _ptable_t *ptable;
  _benv_t *bedge;
  _node2m_t **node, *cnode, *cnode2, *cnode3, **pnode, *pr1, *pr2;
  _edge2m_t *e, *e2, **pe;

  n_edges = prob->graph->n_edges;
  node = (_node2m_t **) prob->graph->node2;
  ptable = prob->graph->ptable;
  bedge = prob->graph->bedge;

  memset((void *) ptable->occ, 0, (N_JOBS + 2)*sizeof(unsigned int));
  for(i = 0; i < N_JOBS; i++) {
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

  node[prob->graph->Tmax + 1]->v[0]  = ctod(prob->graph->fixed->ftail);
  for(i = 0; i < prob->graph->fixed->ntail; i++) {
    node[prob->graph->Tmax + 1]->v[0]
      -= u[prob->graph->fixed->job[prob->n - i - 1]->no];
  }
  node[prob->graph->Tmax + 1]->n[0] = node[prob->graph->Tmax + 1];
  node[prob->graph->Tmax + 1]->ty = 0;

  for(t = prob->graph->Tmax; t > prob->graph->Tmin; t--) {
    for(pnode = node + t; *pnode != NULL; pnode = &(cnode->next)) {
      cnode = *pnode;
      if(ELIMINATED_NODE(cnode)) {
	      REMOVE_EDGES(cnode);
	      continue;
      }

      if(IS_REAL_JOB(cnode->j[0]) && prec_get_dom_ti(t, cnode->j[0])) {
	      REMOVE_EDGES(cnode);
	      ELIMINATE_NODE(cnode);
	      continue;
      }
      g = - u[cnode->j[0]] + ctod(prob->sjob[cnode->j[0]]->f[t]);

      tt[0] = tt[1] = t;
      if(IS_JOB(cnode->j[1])) {
	      tt[0] -= prob->sjob[cnode->j[0]]->p;
	      if(IS_REAL_JOB(cnode->j[1]) && prec_get_dom_ti(tt[0], cnode->j[1])) {
	        REMOVE_EDGES(cnode);
	        ELIMINATE_NODE(cnode);
	        continue;
	      }
	      g += - u[cnode->j[1]] + ctod(prob->sjob[cnode->j[1]]->f[tt[0]]);

        if(IS_JOB(cnode->j[2])) {
          tt[1] = tt[0] - prob->sjob[cnode->j[1]]->p;
          if(IS_REAL_JOB(cnode->j[2]) && prec_get_dom_ti(tt[1], cnode->j[2])) {
            REMOVE_EDGES(cnode);
            ELIMINATE_NODE(cnode);
            continue;
          }
          g += - u[cnode->j[2]] + ctod(prob->sjob[cnode->j[2]]->f[tt[1]]);
        }
      }

      v1 = v2 = LARGE_REAL;
      pr1 = pr2 = NULL;
      ty1 = ty2 = 0;

      pe = &(cnode->e);
      while(*pe != NULL) {
        cnode2 = (*pe)->n;
	      if(ELIMINATED_NODE(cnode2)) {
	        REMOVE_SINGLE_EDGE;
	        continue;
	      }

	cnode3 = cnode2->n[0];
        if(HAVE_COMMON_REAL_JOB(cnode, cnode3)) {
          if(cnode2->n[1] == NULL) {
            REMOVE_SINGLE_EDGE;
            continue;
          } else if(cnode2->e->next->next == NULL) {
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
          if(ub - (f + cnode->v[0]) < prob->param->lbeps) {
            REMOVE_SINGLE_EDGE;
            continue;
          } else if(ub - (f + cnode->v[1]) < prob->param->lbeps) {
            cnode3 = cnode->n[0];
            if(HAVE_COMMON_REAL_JOB(cnode2, cnode3)) {
              REMOVE_SINGLE_EDGE;
              continue;
            }
          }

          if(_check_adj_supernode_backward(prob, cnode, cnode2, t)) {
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

      if(pr1 == NULL) {
	      ELIMINATE_NODE(cnode);
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
  cnode->v[0] = cnode->v[1] = LARGE_REAL;
  cnode->n[0] = cnode->n[1] = NULL;
  cnode->ty = 0;
  pe = &(cnode->e);
  while(*pe != NULL) {
    cnode2 = (*pe)->n;
    if(ELIMINATED_NODE(cnode2)) {
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

  if(cnode->e == NULL) {
    return(SIPS_INFEASIBLE);
  }

  _lag2_free_eliminated_nodes(prob);

  return(_lag2_shrink_horizon_head(prob, u));
  **/
}

int lag2_assign_modifiers(unsigned char type, int nmax)
{
    return SIPS_OK;
/** 
  int i, j;
  int ty, pty;
  int n, nn, m;
  _ptable_t *ptable;
  _fixed_t *fixed;
  _node2m_t **node, *cnode;
  _occur_t *occ;

  if(_lag2_check_time_window(prob) == SIPS_INFEASIBLE) {
    return(SIPS_INFEASIBLE);
  }

  node = (_node2m_t **) prob->graph->node2;
  fixed = prob->graph->fixed;

  occ = (_occur_t *) xcalloc(sizeof(_occur_t), N_JOBS + 2);

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
    while(IS_JOB(cnode->j[0])) {
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
    while(IS_JOB(cnode->j[0])) {
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
    qsort((void *) occ, nn, sizeof(_occur_t), _compare_occur);

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
    qsort((void *) occ, nn, sizeof(_occur_t), _compare_occur2);

    for(n = 0;  n < nn && n < nmax; n++) {
      mod->jobs[n] = occ[n].j;
      mod->fl[occ[n].j] = 1;
    }
  }

  xfree(occ);

  mod->an = n;
  memset((void *) mod->v[0], 0, (N_JOBS + 2)*(1<<prob->param->mod));
  memset((void *) mod->m, 0, N_JOBS + 2);
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
        for(m = 0; m < 1<<mod->an; m++) {
          if(!(m & (1<<i))) {
            mod->v[j][m] = 1;
          }
        }
      } else if(ptable->inc[mod->jobs[i]][j] == -1) {
        for(m = 0; m < 1<<mod->an; m++) {
          if(m & (1<<i)) {
            mod->v[j][m] = 1;
          }
        }
      }
    }
  }
  mod->n += mod->an;

  return(SIPS_OK);
  **/
}

int lag2_solve_LR2m(double ub, _solution_t *sol, double* lb)
{
  int ret;

  if(subl_graph->direction == SIPS_FORWARD) {
    ret = _lag2_solve_LR2m_forward(ub, 1);
  } else {
    ret = _lag2_solve_LR2m_backward(ub, 1);
  }

  if(ret == SIPS_INFEASIBLE) {
    *lb = ub;
  }

  //return(_lag2_get_sol_LR2m(prob, u, ub, sol, lb, o));
  return SIPS_OK;
}

/*
 * _lag2_move_edges_head(prob)
 *   moves edges from their source nodes to their destination nodes.
 *
 */
void _lag2_move_edges_head()
{
  return;
  /** 
  int t;
  _node2m_t **node;
  _node2m_t *cnode;
  _edge2m_t *e, *e2, *e3;

  node = (_node2m_t **) prob->graph->node2;
  for(t = prob->graph->Tmax; t >= prob->graph->Tmin; t--) {
    for(cnode = node[t]; cnode != NULL; cnode = cnode->next) {
      for(e = cnode->e; e != NULL;) {
	e3 = e->next;
	e2 = e->n->e;
	e->n->e = e;
	e->n = cnode;
	e->next = e2;
	e = e3;
      }

      cnode->e = NULL;
    }
  }
  **/
}

/*
 * _lag2_move_edges_tail(prob)
 *   moves edges from their destination nodes to their source nodes.
 *
 */
void _lag2_move_edges_tail()
{
  return;
  /** 
  int t;
  _node2m_t **node;
  _node2m_t *cnode;
  _edge2m_t *e, *e2, *e3;

  node = (_node2m_t **) prob->graph->node2;
  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax + 1; t++) {
    for(cnode = node[t]; cnode != NULL; cnode = cnode->next) {
      for(e = cnode->e; e != NULL;) {
	e3 = e->next;
	e2 = e->n->e;
	e->n->e = e;
	e->n = cnode;
	e->next = e2;
	e = e3;
      }
      cnode->e = NULL;
    }
  }
  **/
}

void lag2_reverse() {
  return;
  /** 
  if(prob->graph->direction == SIPS_FORWARD) {
    _lag2_move_edges_tail(prob);
    prob->graph->direction = SIPS_BACKWARD;
  } else {
    _lag2_move_edges_head(prob);
    prob->graph->direction = SIPS_FORWARD;
  }
  **/
}

void ssdp(info* prob, Tmap* tm, T2map* t2m) {
    prob->stage = 0;
    print_stage(prob);
    auto [jobs, cst] = best_heuristic(prob, tm, t2m);
    auto [ejobs, ecst] = dynasearch(jobs);
    update(ejobs, ecst, prob);
    prob->stage = 1;
    print_stage(prob);
    subgradient_LR1();
    
    prob->stage = 2;
    print_stage(prob);
    subgradient_LR2adj();

    prob->stage = 3;
    print_stage(prob);
    stage3();        
    print_result(prob);
}