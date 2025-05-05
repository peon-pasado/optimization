#pragma once
#include "models.hpp"
#include "utils/solution.hpp"
#include <algorithm>

typedef struct {
    int j;
    int f;
    unsigned char ty;
} _dyna_dp_t;


int _edynasearch_internal(_solution_t *sol, int lw, _job_t **job, _dyna_dp_t *dp) {
    int vv;
    int c;

    if(lw == 0) {
        dp[0].f = 0;
        dp[0].j = -1;
        dp[0].ty = 0;
    }

    for(int i = lw + 1; i <= sol->n; ++i) {
        dp[i].j = i - 1;
        dp[i].f = dp[i - 1].f + sol->job[i - 1]->f[sol->c[i - 1]];
        dp[i].ty = 0;

        /* PI */
        for (int j = 0; j <= i - 2; ++j) {
            if(j > 0) {
                c = sol->c[j - 1] + sol->job[i - 1]->p;
            } else {
                c = sol->job[i - 1]->p;
            }
            vv = dp[j].f + sol->job[i - 1]->f[c];
            for (int k = j + 1; k <= i - 2; ++k) {
                c += sol->job[k]->p;
                vv += sol->job[k]->f[c];
            }
            c += sol->job[j]->p;
            vv += sol->job[j]->f[c];

            if (vv < dp[i].f) {
                dp[i].f = vv;
                dp[i].j = j;
                dp[i].ty = 1;
            }
        }

        /* EBSR */
        for (int j = 0; j <= i - 3; j++) {
            if(j > 0) {
                c = sol->c[j - 1] + sol->job[i - 1]->p;
            } else {
                c = sol->job[i - 1]->p;
            }
            vv = dp[j].f + sol->job[i - 1]->f[c];
            for (int k = j; k <= i - 2; k++) {
                c += sol->job[k]->p;
                vv += sol->job[k]->f[c];
            }

            if(vv < dp[i].f) {
                dp[i].f = vv;
                dp[i].j = j;
                dp[i].ty = 2;
            }
        }

        /* EFSR */
        for (int j = 0; j <= i - 3; j++) {
            if(j > 0) {
                c = sol->c[j - 1];
            } else {
                c = 0;
            }
            vv = dp[j].f;
            for (int k = j + 1; k <= i - 1; k++) {
                c += sol->job[k]->p;
                vv += sol->job[k]->f[c];
            }
            c += sol->job[j]->p;
            vv += sol->job[j]->f[c];
            if (vv < dp[i].f) {
                dp[i].f = vv;
                dp[i].j = j;
                dp[i].ty = 3;
            }
        }

        for (int j = 0; j <= i - 2; j++) {
            int c = (j > 0) ? sol->c[j - 1] : 0;
            int vv = dp[j].f;
            for (int k = i - 1; k >= j; k--) {
                c += sol->job[k]->p;
                vv += sol->job[k]->f[c];
            }
            if (vv < dp[i].f) {
                dp[i].f = vv;
                dp[i].j = j;
                dp[i].ty = 4;
            }
        }

    }

    for(int i = sol->n, j = sol->n - 1; i > 0;) {
        switch(dp[i].ty) {
            case 0:
                job[j--] = sol->job[dp[i].j];
                break;
            case 1: /* PI */
                job[j--] = sol->job[dp[i].j];
                for (int k = i - 2; j > dp[i].j; j--, k--) {
                    job[j] = sol->job[k];
                }
                job[j--] = sol->job[i - 1];
                break;
            case 2: /* EBSR */
                for (int k = i - 2; j > dp[i].j; j--, k--) {
                    job[j] = sol->job[k];
                }
                job[j--] = sol->job[i - 1];
                break;
            case 3: /* EFSR */
                job[j--] = sol->job[dp[i].j];
                for (int k = i - 1; j >= dp[i].j; j--, k--) {
                    job[j] = sol->job[k];
                }
                break;
            case 4:
                for (int k = dp[i].j; k < i; k++)
                    job[j--] = sol->job[k];
                break;
        }
        i = dp[i].j;
    }

    for(lw = 0; lw < sol->n; lw++) {
        if(job[lw]->no != sol->job[lw]->no) {
            break;
        }
    }

    copy_n(job, sol->n, sol->job);
    solution_set_c(sol);
    return lw;
}

void edynasearch(_solution_t *sol) {
    int i;
    int f;
    _dyna_dp_t *dp;
    _job_t **job;

    dp = new _dyna_dp_t[sol->n + 1];
    job = new _job_t*[sol->n];

    f = sol->f;
    i = 0;
    while (1) {
        i = _edynasearch_internal(sol, i, job, dp);
        if (f == sol->f) break;
        f = sol->f;
    }

    delete[] job;
    delete[] dp;
}