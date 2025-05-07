#pragma once
#include "models.hpp"
#include "utils/constants.hpp"
#include "utils/solution.hpp"
#include "dynasearch.hpp"
#include "heuristics.hpp"
#include "subgradient.hpp"
#include "lagrange_mod.hpp"

int stage0() {
    prob->sol->f = inf;
    _solution_t* sol2 = create_solution();
    /* spt */
    sol2->n = 0;
    insert_ordered_jobs_greedily(sol2, prob->n, prob->sjob);
    if (prob->sol->n < prob->n || sol2->f < prob->sol->f) {
        copy_solution(prob->sol, sol2);
        if (prob->sol->f <= 0) return SIPS_SOLVED;
    }
    std::vector<_job_t*> job(prob->n);
    /* lpt */
    for (int i = 0; i < prob->n; i++) {
        job[i] = prob->sjob[prob->n - i - 1];
    }
    sol2->n = 0;
    insert_ordered_jobs_greedily(sol2, prob->n, job);
    if (sol2->f < prob->sol->f) {
        copy_solution(prob->sol, sol2);
        if (prob->sol->f <= 0) return SIPS_SOLVED;
    }
    /* edd */
    sort(job.begin(), job.end(), [](auto p, auto q) {
        if (p->d != q->d) return p->d < q->d;
        if (p->p != q->p) return p->p < q->p;
        if (p->w != q->w) return p->w < q->w;
        return p->no < q->no;
    });
    sol2->n = 0;
    insert_ordered_jobs_greedily(sol2, prob->n, job);
    if (sol2->f < prob->sol->f) {
        copy_solution(prob->sol, sol2);
        if (prob->sol->f <= 0) return SIPS_SOLVED;
    }
    /* ldd */
    for (int i = 0, j = prob->n - 1; i < j; i++, j--) {
        swap(job[i], job[j]);
    }
    sol2->n = 0;
    insert_ordered_jobs_greedily(sol2, prob->n, job);
    if (sol2->f < prob->sol->f) {
        copy_solution(prob->sol, sol2);
        if (prob->sol->f <= 0) return SIPS_SOLVED;
    }
    free_solution(sol2);
    edynasearch(prob->sol);
    return prob->sol->f <= 0 ? SIPS_SOLVED : SIPS_UNSOLVED;
}

int stage1(double *lb) {  
    return subgradient_LR1(lb);
}

int stage2(double *lb) {
    return subgradient_LR2adj(lb);
}

int _stage3_loop(int *ub, double lboff, double *u) {
  unsigned int max_nedges, max_nnodes;
  unsigned char nmod;
  int ret;
  bool heavy;
  double lb, lb2, lbprev;
  double mb;
  unsigned int *o;
  _solution_t *tmpsol, *tmpsol2;
  _mod_t *mod;

  o = new unsigned int[prob->n]{};
  mod = new _mod_t;
  mod->m = new unsigned char[prob->n + 2]{};
  mod->fl = new unsigned char[prob->n]{};
  mod->jobs = new int[prob->n]{};
  mod->v = new unsigned char *[prob->n + 2]{};
  mod->v[0] = new unsigned char[(prob->n + 2)*8]{};
  for (int i = 1; i <= prob->n + 1; i++) {
    mod->v[i] = mod->v[i - 1] + 8;
  }

  tmpsol = create_solution();
  tmpsol2 = create_solution();

  mod->an = 0;
  mod->n = 0;

  max_nnodes = prob->graph->n_nodes;
  max_nedges = prob->graph->n_edges;

  lag2_reverse();

  lb2 = -dinf;
  ret = lag2_solve_LR2m(u, *ub - lboff, tmpsol, &lb, o);
  lb += lboff;

  if(ret == SIPS_OPTIMAL) {
    if (tmpsol->f < prob->sol->f) {
      copy_solution(prob->sol, tmpsol);
      *ub = min(*ub, tmpsol->f);
    }
    ret = SIPS_SOLVED;
  } else if (ret == SIPS_INFEASIBLE) {
    ret = SIPS_SOLVED;
  }

  mb = lag2_get_memory_in_MB() - prob->graph->copy->mem;
  lbprev = lb;
  nmod = 3;
  heavy = false;
  while (ret == SIPS_NORMAL) {
    lbprev = max(lbprev, lb);
    mb = (mem 
            - prob->graph->copy->mem
	          - prob->graph->ptable->mem)
      / (lag2_get_memory_in_MB() 
        - prob->graph->copy->mem
	      - prob->graph->ptable->mem);
        
    for(nmod = 0; mb >= 1.0 && nmod <= 3; mb /= 2.0, nmod++);
    nmod = max(1, nmod - 1);
    if (mod->n + nmod >= prob->n) {
      nmod = prob->n - mod->n - 1;
    }
    ret = lag2_assign_modifiers(1, nmod, mod);
    if (ret == SIPS_INFEASIBLE) {
        lb = *ub;
        ret = SIPS_SOLVED;
        break;
    }
    lag2_reverse();
    if (mod->an == 0) {
        ret = lag2_solve_LR2m(u, *ub - lboff, tmpsol, &lb, o);
    } else {
        ret = lag2_add_modifiers_LR2m(u, *ub - lboff, tmpsol, &lb, o, mod);
    }
    lb += lboff;
    max_nnodes = max(max_nnodes, prob->graph->n_nodes);
    max_nedges = max(max_nedges, prob->graph->n_edges);
    mb = (lag2_get_memory_in_MB() 
          - prob->graph->copy->mem
	        - prob->graph->ptable->mem) /
              ((double) mem 
                - prob->graph->copy->mem
	              - prob->graph->ptable->mem);
    if(nmod == 1 && mb >= 0.75) {
      heavy = true;
    } else if(nmod > 1 || mb < 0.5) {
      heavy = false;
    }

    if(heavy && ret == SIPS_NORMAL) {
      lag2_reverse();
      ret = lag2_solve_LR2m(u, *ub - lboff, tmpsol, &lb2, o);
      lb2 += lboff;
      lb = max(lb, lb2);
      max_nnodes = max(max_nnodes, prob->graph->n_nodes);
      max_nedges = max(max_nedges, prob->graph->n_edges);
    }

    if (*ub - lb < 1. - eps) {
      lb = *ub;
      ret = SIPS_SOLVED;
      break;
    }

    if (ret != SIPS_NORMAL) {
      if (ret == SIPS_OPTIMAL) {
        if (tmpsol->f < *ub) {
          copy_solution(prob->sol, tmpsol);
          *ub = tmpsol->f;
        } else if (tmpsol->f < prob->sol->f) {
          copy_solution(prob->sol, tmpsol);
        }
      } else if (ret == SIPS_INFEASIBLE) {
        lb = *ub;
      }
      ret = SIPS_SOLVED;
    } else if (lb2 > lb) {
        partialdp(tmpsol2);
	      edynasearch(tmpsol2);
        if (tmpsol2->f<*ub) {
            copy_solution(prob->sol, tmpsol2);
            *ub = tmpsol2->f;
            lag2_free_copy();
        } else if (tmpsol2->f<prob->sol->f) {
            copy_solution(prob->sol, tmpsol2);
        }
        if (*ub - lb2 < 1. - eps) {
          lb = *ub;
          ret = SIPS_SOLVED;
          break;
        }
    } else if(lb - lbprev > eps) {
        partialdp(tmpsol);
	      edynasearch(tmpsol);
        if (tmpsol->f < *ub) {
            copy_solution(prob->sol, tmpsol);
            *ub = tmpsol->f;
            lag2_free_copy();
        } else if (tmpsol->f < prob->sol->f) {
            copy_solution(prob->sol, tmpsol);
        }
    }
  }
  return ret;
}


void stage3() {
    double lb = 0., lboff;
    double r, lbprev;
    double* u = new double[prob->n + 1];
    lboff = 0.0;
    for (int i = 0; i < prob->n; i++) {
        u[i] = prob->sjob[i]->u;
        lboff += u[i];
    }
    _solution_t *tmpsol = create_solution();
    int ret = lag2_initialize(u, prob->sol->f - lboff, tmpsol, &lb, nullptr);
    lb += lboff;
    if (ret == SIPS_OPTIMAL && tmpsol->f < prob->sol->f) copy_solution(prob->sol, tmpsol);
    free_solution(tmpsol);
    if (ret == SIPS_OPTIMAL || ret == SIPS_INFEASIBLE) ret = SIPS_SOLVED;
    if (ret != SIPS_NORMAL) return;
    lag2_push_nodes();
    if (prob->graph->copy->mem >= 16) {
        lbprev = (prob->sol->f + lb) / 2.0;
        r = (prob->sol->f - lbprev) / 2.0;
    } else {
        lbprev = prob->sol->f;
        r = prob->sol->f;
    }
    double lbbest = lb;
    int f = prob->sol->f;
    for (int iter = 1;; iter++) {
        int ub = max(round(lbprev), ceil(lbbest));
        if (prob->sol->f <= ub) {
            ub = prob->sol->f;
            lag2_pop_nodes();
        } else lag2_recover_nodes();

        if (iter >= 3 && prob->sol->f==ub && prob->sol->f<=f) {
            edynasearch(prob->sol);
            ub = min(ub, prob->sol->f);
        }
        if (_stage3_loop(&ub, lboff, u) == SIPS_SOLVED) {
            if (prob->sol->f <= ub) break;
            lbbest = ub;
            lbprev += r;
        } else break;
    }
}

void ssdp() {
   //cout << "stage 0" << endl;
    int ret = stage0();
    if (ret == SIPS_SOLVED) return;
    double lb = -inf;
    //cout << "stage 1" << endl;
    ret = stage1(&lb);
    //cout << "current lb: "<< lb << endl;
    //cout << "current ub: "<<prob->sol->f <<endl;
    if (ret != SIPS_UNSOLVED) return;
    //cout << "stage 2" << endl;
    ret = stage2(&lb);
    //cout << SIPS_UNSOLVED << endl;
    if (ret != SIPS_UNSOLVED) return;
    //cout << "stage 3" << endl;
    stage3();
}