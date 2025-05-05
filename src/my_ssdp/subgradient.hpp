#pragma once
#include "models.hpp"
#include "utils/solution.hpp"
#include "utils/constants.hpp"
#include "heuristics.hpp"
#include "dynasearch.hpp"
#include <cmath>
#include "lagrange.hpp"
#include <iostream>
#include <numeric>
using namespace std;

const double tratio1 = 0.02;
const double tratio2 = 0.002;
const double expand1 = 1.25;
const double expand2 = 1.30;
const double shrink1 = 0.9;
const double shrink2 = 0.9;

void update_multipliers(double& dnorm, int* o, double* d, double cl, double lk, double* u) {
    int osq = accumulate(o, o + prob->n, 0, [](int sum, int oi) {return sum + (1-oi)*(1-oi);});
    double ksi = sqrt(osq / dnorm);
    transform(o, o + prob->n, d, d, [ksi](double oi, double di) {return (1 - oi) + ksi*di;});
    dnorm = accumulate(d, d + prob->n, 0., [](double sum, double di) {return sum + di*di;});
    if (dnorm < eps) {
      dnorm = osq;
      transform(o, o + prob->n, d, [](int x) {return 1. - x;});
    }
    double tk = lk * (prob->sol->f - cl) / dnorm;
    transform(u, u + prob->n, d, u, [tk](double ui, double di) {return ui + tk*di;});
}

int subgradient_LR1(double *lb) {
    double cl;    
    int titer = ceil(prob->n / 4.);
    int ititer = ceil(prob->n / 3.);
    int *o = new int[prob->n]{};
    double *u = new double[2 * (prob->n + 1)];
    double *ubest = u + (prob->n + 1);
    double *d = new double[prob->n]{};
    double *gap_table = nullptr;
    if(titer > 0) gap_table = new double[titer]{};
    _solution_t *csol = create_solution();
    _solution_t *tmpsol = create_solution();
    fill(gap_table, gap_table+titer, -dinf);
    fill(u, u+prob->n, 0.);
    fill(ubest, ubest+prob->n, 0.);
    int iter = 0;
    int updated = 0;
    int nupdated = 0;
    int nsiter = 0;
    double lmax = *lb;
    double lmax2 = -dinf;
    double lk = 1.0;
    int pos = 0;
    int ret = SIPS_UNSOLVED;
    double dnorm = 1.0;
    while (++iter) {
        double off = accumulate(u, u+prob->n, 0.);
        int ret2 = lag_solve_LR1(u, prob->sol->f - off, tmpsol, &cl, o);
        cl += off;
        if (ret2 == SIPS_FAIL) {
            ret = ret2;
            break;
        }
        if (cl - lmax > eps) {
            updated = nsiter = 0;
            nupdated++;
            lmax = cl;
            copy(u, u + prob->n, ubest);
            if(iter > 1) lk = min(1.0, lk * expand1);
        } else if(lmax2 >= cl - 1 + eps) updated++;

        if(iter > 1) lmax2 = max(lmax2, cl);
        if (prob->sol->f - lmax < 1 - eps) {
            ret = SIPS_SOLVED;
            break;
        }
        if(ret2 != SIPS_NORMAL) {
            if(ret2 == SIPS_OPTIMAL && prob->sol->f > tmpsol->f) {	
              copy_solution(prob->sol, tmpsol);	
            }
            ret = SIPS_SOLVED;
            break;
        }

        if(!titer) break;

        if(nupdated <= 1) {
            if(iter > ititer) break;
        } else if(tratio1*(prob->sol->f - gap_table[pos]) 
                > lmax - gap_table[pos] 
                && tratio1*lmax + eps 
                > lmax - gap_table[pos] 
                && (nupdated*10 >= prob->n || nsiter >= prob->n)) {
            break;
        }

      if(iter == 1) lag_solve_LR1_backward(u, prob->sol->f - off);
      if (updated >= 2) {
        nsiter += updated;
        updated = 0;
        lk *= shrink1;
        if (lk < 1.0e-4) break;
      }
      gap_table[pos++] = lmax;
      pos %= titer;
      update_multipliers(dnorm, o, d, cl, lk, u);
    }

  if(ret == SIPS_UNSOLVED) {
      double off = accumulate(ubest, ubest+prob->n, 0.0);
      lag_solve_LR1(ubest, prob->sol->f - off, tmpsol, &cl, o);
      cl += off;
      if (prob->sol->f - cl < 1 - eps) ret = SIPS_SOLVED;
      else {
          copy_solution(csol, tmpsol);
          partialdp(csol);
          edynasearch(csol);
          if (prob->sol->f > csol->f) copy_solution(prob->sol, csol);
          if (prob->sol->f - cl < 1.0 - eps) ret = SIPS_SOLVED;
          else lag_solve_LR1_backward(ubest, prob->sol->f - off);
      }
    lmax = cl;
  }
  for (int i = 0; i < prob->n; i++) prob->sjob[i]->u = ubest[i];
  delete[] d;
  delete[] u;
  delete[] o;
  delete[] gap_table;
  free_solution(tmpsol);
  free_solution(csol);
  *lb = lmax;
  return ret;
}

int subgradient_LR2adj(double *lb) {
  int i;
  int ititer, titer, iter;
  int updated, nupdated, nsiter, pos;
  int ret, ret2;
  unsigned int osq, bosq;
  char pflag;
  double dnorm;
  double lk, cl, off, lmax, lmax2;
  int *o;
  double *u, *ubest, *d;
  double *gap_table = nullptr;
  _solution_t *tmpsol, *csol;
  titer = prob->n / 4.;
  ititer = prob->n;
  o = new int[prob->n]{};
  u = new double[2 * (prob->n + 1)]{};
  ubest = u + (prob->n + 1);
  d = new double[prob->n]{};
  if(titer > 0) gap_table = new double[titer]{};
  csol = create_solution();
  tmpsol = create_solution();
  for (i = 0; i < prob->n; i++) {
    u[i] = ubest[i] = prob->sjob[i]->u;
  }
  for(i = 0; i < titer; i++) {
    gap_table[i] = -dinf;
  }
  iter = updated = nupdated = nsiter = 0;
  pflag = 0;
  lmax = *lb;
  lmax2 = -dinf;
  lk = 1.0;
  pos = 0;
  ret = SIPS_UNSOLVED;
  bosq = inf;
  dnorm = 1.0;

  while (++iter) {
    pflag = 0;
    off = accumulate(u, u+prob->n, 0.);
    ret2 = lag_solve_LR2adj(u, prob->sol->f - off, tmpsol, &cl, o);
    cl += off;

    if(iter == 1) pflag = 1;
    if(cl - lmax > eps) {
      updated = nsiter = 0;
      nupdated++;
      pflag = 1;
      lmax = cl;
      memcpy(ubest, u, prob->n * sizeof(double));

      if(iter > 1) {
	      lk *= expand2;
	      if(lk > 1.0) {
	        lk = 1.0;
	      }
      }
    } else if(lmax2 >= cl - 1. + eps) updated++;

    if(iter > 1) lmax2 = max(lmax2, cl);
    if(ret2 != SIPS_NORMAL) {
      if(ret2 == SIPS_OPTIMAL && prob->sol->f > tmpsol->f) {
        copy_solution(prob->sol, tmpsol);
      }
      ret = SIPS_SOLVED;
      break;
    }
    if (prob->sol->f - lmax < 1. - eps) {
      ret = SIPS_SOLVED;
      break;
    }
    osq = 0;
    for(i = 0; i < prob->n; i++) {
      osq += (1 - o[i])*(1 - o[i]);
    }
    if(bosq > osq) {
      bosq = osq;
      copy_solution(csol, tmpsol);
    }
    if (iter % 10 == 1) {
      partialdp(csol);
	    edynasearch(csol);
      bosq = inf;
    } else csol->f = inf;

    if (prob->sol->f > csol->f || pflag) {
      if (iter > 1 && prob->sol->f > csol->f && csol->f > lmax) {
	      lk *= (prob->sol->f - lmax) / (csol->f - lmax);
	      if (lk > 1.0) lk = 1.0;
      }
      if (prob->sol->f > csol->f) 
        copy_solution(prob->sol, csol);
      if (prob->sol->f - lmax < 1. - eps) {
        ret = SIPS_SOLVED;
        break;
      }

      ret = lag_solve_LR2adj_backward(u, prob->sol->f - off);
      if(ret == SIPS_INFEASIBLE) {
        lmax = *lb = prob->sol->f;
        ret = SIPS_SOLVED;
        break;
      } else {
	      ret = SIPS_UNSOLVED;
      }
    }

    if(!titer) break;
    if(nupdated <= 1) {
      if(iter > ititer)  break;
    } else if (tratio2 * (prob->sol->f - gap_table[pos])
	      > lmax - gap_table[pos]
	      && tratio2 * lmax + eps
	      > lmax - gap_table[pos]
	      && (nupdated * 10 >= prob->n
		  || nsiter >= prob->n)) {
      break;
    }

    if(updated >= 2) {
      nsiter += updated;
      updated = 0;
      lk *= shrink2;
      if (lk < 1.0e-4) break;
    }

    gap_table[pos++] = lmax;
    pos %= titer;

    update_multipliers(dnorm, o, d, cl, lk, u);
  }

  if(ret == SIPS_UNSOLVED) {
    tmpsol->f = prob->sol->f;
    edynasearch(prob->sol);
    if (prob->sol->f - lmax < 1.0 - eps) {
      ret = SIPS_SOLVED;
    }
    if(ret == SIPS_UNSOLVED && prob->sol->f < tmpsol->f) {
      off = 0.0;
      for(i = 0; i < prob->n; i++) {
	    off += ubest[i];
      }

      ret2 = lag_solve_LR2adj(ubest, prob->sol->f - off, tmpsol, &cl, o);
      cl += off;

      if(ret2 != SIPS_NORMAL) {
	    ret = SIPS_SOLVED;
      } else {
	    ret2 = lag_solve_LR2adj_backward(ubest, prob->sol->f - off);
	    if  (ret2 == SIPS_INFEASIBLE) {
	        lmax = *lb = prob->sol->f;
	        ret = SIPS_SOLVED;
	    }
      }
    }
  }

  for(i = 0; i < prob->n; i++) {
    prob->sjob[i]->u = ubest[i];
  }

  delete[] d;
  delete[] u;
  delete[] o;
  delete[] gap_table;
  free_solution(tmpsol);
  free_solution(csol);
  *lb = lmax;
  return ret;
}
