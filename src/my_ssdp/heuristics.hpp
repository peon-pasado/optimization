#pragma once
#include <stdlib.h>
#include "models.hpp"
#include <algorithm>

#define ELIMINATED (prob->n)

int _count_bit(int k) {
    return __builtin_popcount(k);
}

struct _partdp_t {
    int f;
    unsigned short j;
    struct _partdp_t *pr;
};

void _insert_jobs_optimally( _solution_t *sol, _solution_t *psol, _solution_t *isol) {
    int n;
    int cpsum;
    int f;
    _partdp_t *cdp;

    int nsize = 1 << isol->n;

    _partdp_t** dp = new _partdp_t*[psol->n + 1];
    dp[0] = new _partdp_t[(psol->n + 1) * nsize];
    for (int i = 1; i <= psol->n; i++) {
        dp[i] = dp[i - 1] + nsize;
    }

    int* nn = new int[nsize];
    int* ppsum = new int[psol->n + 1];
    int* ipsum = new int[nsize];

    cpsum = 0;
    ipsum[0] = 0;
    for (int i = 1, m = 0; i < nsize; i++) {
        nn[i] = _count_bit(i);

        int j = i ^ (i - 1);
        int k = 1 << (isol->n - 1);
        int l=0;
        for (; (k & j) == 0; l++, k >>= 1) {
            ;
        }
        j = isol->n - l - 1;
        /* m: Gray code */
        m ^= k;
        if(m & k) {
            cpsum += isol->job[j]->p;
        } else {
            cpsum -= isol->job[j]->p;
        }
        ipsum[m] = cpsum;
    }

    ppsum[0] = 0;
    for (int i = 0; i < psol->n; i++) {
        ppsum[i + 1] = ppsum[i] + psol->job[i]->p;
    }


    for (int i = 0; i <= psol->n; i++) {
        cdp = dp[i];
        if (i > 0) {
            cdp->j = psol->job[i - 1]->no;
            cdp->f = dp[i - 1][0].f + psol->job[i - 1]->f[ppsum[i]];
            cdp->pr = &(dp[i - 1][0]);
        } else {
            cdp->j = prob->n;
            cdp->f = 0;
            cdp->pr = nullptr;
        }

        for (int j = 1; j <= isol->n; j++) {
            for (int k = 1; k < nsize; k++) {
                cdp = dp[i] + k;
                if (j == nn[k]) {
                    if(i > 0) {
                        cdp->j = psol->job[i - 1]->no;
                        cdp->f = dp[i - 1][k].f + psol->job[i - 1]->f[ppsum[i] + ipsum[k]];
                        cdp->pr = &(dp[i - 1][k]);
                    } else {
                        cdp->j = prob->n;
                        cdp->f = inf;
                        cdp->pr = nullptr;
                    } 

                    for (int l = 0, m = 1; l < isol->n; l++, m <<= 1) {
                        if(k & m) { 
                            n = k ^ m;
                            f = dp[i][n].f + isol->job[l]->f[ppsum[i] + ipsum[k]];

                            if(f < cdp->f) {
                                cdp->j = isol->job[l]->no;
                                cdp->f = f;
                                cdp->pr = &(dp[i][n]);
                            }
                        }   
                    }
                }    
            }
        }
    }

    sol->n = 0;
    cdp = &(dp[psol->n][nsize - 1]);
    for (int i = 0; i < prob->n; i++) {
        sol->job[prob->n - i - 1] = prob->sjob[cdp->j];
        cdp = cdp->pr;
    }
    sol->n = prob->n;

    delete[] ipsum;
    delete[] ppsum;
    delete[] nn;
    delete[] dp[0];
    delete[] dp;
    solution_set_c(sol);
}

void insert_jobs_greedily(_solution_t *sol, int in, _job_t **ijob) {
int i, j, k;
int argmin, argmini;
int c;
int prevf, f, minf;

solution_set_c(sol);
if(in == 0) {
return;
}

while(in > 0) {
argmini = 0;
argmin = 0;
minf = inf;
for(i = 0; i < in; i++) {
c = ijob[i]->p;
f = ijob[i]->f[c];
for(j = 0; j < sol->n; j++) {
c += sol->job[j]->p;
f += sol->job[j]->f[c];
}
if (f < minf) {
argmini = i;
argmin = 0;
minf = f;
}

prevf = 0;

for(j = 1; j <= sol->n; j++) {
c = sol->c[j - 1];
prevf += sol->job[j - 1]->f[c];

c += ijob[i]->p;
f = prevf + ijob[i]->f[c];

for(k = j; k < sol->n; k++) {
c += sol->job[k]->p;
f += sol->job[k]->f[c];
}

if(f < minf) {
argmini = i;
argmin = j;
minf = f;
}
}
}

c = ijob[argmini]->p;
for(j = sol->n; j > argmin; j--) {
sol->c[j] = sol->c[j - 1] + c;
sol->job[j] = sol->job[j - 1];
}

if(argmin == 0) {
sol->c[argmin] = c;
} else {
sol->c[argmin] = sol->c[argmin - 1] + c;
}
sol->job[argmin] = ijob[argmini];

sol->n++;
sol->f = minf;

in--;
for(j = argmini; j < in; j++) {
ijob[j] = ijob[j + 1];
}
}  

return;
}

void insert_ordered_jobs_greedily(_solution_t *sol, int in, std::vector<_job_t*>& ijob) {
    solution_set_c(sol);
    if (in == 0) return;
    for (int i = 0; i < in; i++) {
      int argmin = 0;
      int c = ijob[i]->p;
      int minf = ijob[i]->f[c];
      for (int k = 0; k < sol->n; k++) {
        c += sol->job[k]->p;
        minf += sol->job[k]->f[c];
      }
      int prevf = 0;
      for (int j = 1; j <= sol->n; j++) {
        c = sol->c[j - 1];
        prevf += sol->job[j - 1]->f[c];
        c += ijob[i]->p;
        int f = prevf + ijob[i]->f[c];
        for (int k = j; k < sol->n; k++) {
          c += sol->job[k]->p;
          f += sol->job[k]->f[c];
        }
        if (f < minf) {
          argmin = j;
          minf = f;
        }
      }
      c = ijob[i]->p;
      for (int j = sol->n; j > argmin; j--) {
        sol->c[j] = sol->c[j - 1] + c;
        sol->job[j] = sol->job[j - 1];
      }
      sol->c[argmin] = (argmin > 0 ? sol->c[argmin - 1] : 0) + ijob[i]->p;
      sol->job[argmin] = ijob[i];
      sol->f = minf;
      sol->n++;
    }  
}

void partialdp(_solution_t *sol)
{
  int i;
  int *e;
  _solution_t *psol, *isol;

  e = new int[prob->n]{};
  psol = create_solution();
  isol = create_solution();

  for(i = 0; i < sol->n; i++) {
    e[sol->job[i]->no]++;
  }
  if(prob->graph->hdir == SIPS_FORWARD) { /* forward */
    for(i = 0; i < sol->n; i++) {
      if(e[sol->job[i]->no] >= 1) {
        psol->job[psol->n++] = sol->job[i];
        e[sol->job[i]->no] = -1;
      }
    }
  } else { /* backward */
    for(i = sol->n - 1; i >= 0; i--) {
      if(e[sol->job[i]->no] >= 1) {
        psol->job[psol->n++] = sol->job[i];
        e[sol->job[i]->no] = -1;
      }
    }
    reverse_solution(psol);
  }

  for(i = prob->n - 1; i >= 0; i--) {
    if(e[i] != -1) {
      isol->job[isol->n++] = prob->sjob[i];
    }
  }
  delete[] e;

  if(isol->n > 12) {
    copy_solution(sol, psol);
    insert_jobs_greedily(sol,isol->n, isol->job);
  } else if(isol->n > 0) {
    _insert_jobs_optimally(sol, psol, isol);
  } else {
    copy_solution(sol, psol);
    solution_set_c(sol);
  }

  free_solution(isol);
  free_solution(psol);

  prob->graph->hdir = 1 - prob->graph->hdir; /* reverse the direction */

  return;
}
