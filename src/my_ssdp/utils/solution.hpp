#pragma once
#include "../models.hpp"
#include <cassert>
using namespace std;


void objective(_solution_t *sol) {
  sol->f = 0;
  for (int i = 0; i < sol->n; i++) {
    sol->f += sol->job[i]->f[sol->c[i]];
  }
}

_solution_t* create_solution() {
  _solution_t* sol = new _solution_t();
  sol->n = 0;
  sol->f = 0;
  sol->c = new int[prob->T / prob->pmin]{};
  sol->job = new _job_t*[prob->T / prob->pmin]{};
  return sol;
}

void copy_solution(_solution_t *dest, _solution_t *src) {
  if (!src || !dest) return;
  dest->f = src->f;
  dest->n = src->n;
  memcpy(dest->c, src->c, src->n * sizeof(int));
  memcpy(dest->job, src->job, src->n * sizeof(_job_t *));
}

void reverse_solution(_solution_t *sol) {
  for (int i = 0, j = sol->n - 1; i < j; i++, j--) {
    swap(sol->c[i], sol->c[j]);
    swap(sol->job[i], sol->job[j]);
  }
}

void free_solution(_solution_t *sol) {
  if (sol) {
    delete[] sol->c;
    delete[] sol->job;
    delete sol;
  }
}

void solution_set_c(_solution_t* sol) {
    if (sol->n > 0) {
        sol->c[0] = sol->job[0]->p;
        for (int i = 1; i < sol->n; ++i) {
            sol->c[i] = sol->c[i - 1] + sol->job[i]->p;
        }
        objective(sol);
    }
}