#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models.hpp"
#include <algorithm>
#include <iostream>
#include "sips.hpp"
using namespace std;

void objective_initialize() {
    for (int i = 0; i < prob->n; i++) {
        for (int t = 0; t < prob->job[i].p; t++) prob->job[i].f[t] = inf;
        int tard = prob->T - prob->job[i].d;
        int fmin = prob->job[i].w * max(tard, 0);
        tard = prob->job[i].p - prob->job[i].d;
        int f = prob->job[i].w * max(tard, 0);
        if (fmin > f) fmin = f;
        if (fmin > 0) fmin = 0;
        for (int t = prob->job[i].p; t <= prob->T; t++, tard++) {
            prob->job[i].f[t] = prob->job[i].w * max(tard, 0) - fmin;
        }
    }
    for (int i=0; i<=prob->T+1; ++i) prob->job[prob->n].f[i] = 0;
}

void setup_problem() {
  prob->psum = 0;
  prob->pmin = 1e9;
  prob->pmax = 0;
  for (int i=0; i<prob->n; ++i) {
    prob->job[i].d = prob->job[i].rd;
    prob->psum += prob->job[i].p;
    prob->pmax = max(prob->pmax, prob->job[i].p);
    prob->pmin = min(prob->pmin, prob->job[i].p);
  }
  prob->T = prob->psum;
  prob->dmin = 1e9;
  prob->dmax = -1e9;
  for(int i = 0; i < prob->n; ++i) {
    prob->dmax = max(prob->dmax, prob->job[i].d);
    prob->dmin = min(prob->dmin, prob->job[i].d);
  }
  prob->job[0].f = new int[(prob->T + 1) * (prob->n + 1)];
  for (int i=1; i<=prob->n; ++i) {
    prob->job[i].f = prob->job[i-1].f + (prob->T + 1);
  }
  prob->job[prob->n + 1].f = prob->job[prob->n].f;
  objective_initialize();
  prob->sjob = new _job_t*[prob->n + 2];
  for (int i=0; i<=prob->n+1; ++i) {
    prob->sjob[i] = &(prob->job[i]);
  }
  sort(prob->sjob, prob->sjob + prob->n, [](auto x, auto y) {
    if (x->p != y->p) return x->p < y->p;
    if (x->d != y->d) return x->d < y->d;
    if (x->w != y->w) return x->w < y->w;
    return x->no < y->no;
  });
  for (int i = 0; i < prob->n; i++) {
    prob->sjob[i]->no = i;
  }
}

void read_problem() {
  prob = new sips();
  cin>>prob->n;
  prob->job = new _job_t[prob->n];
  for (int i=0; i<prob->n; ++i) {
    prob->job[i].no = prob->job[i].rno = i;
    std::cin>>prob->job[i].p;
    std::cin>>prob->job[i].rd;
    std::cin>>prob->job[i].w;
  }
}




int main(int argc, char **argv) {
  int verbosity = 0;  // Valor por defecto

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-v") {
            if (i + 1 < argc) {
                verbosity = std::atoi(argv[i + 1]);
                if (verbosity < 0 || verbosity > 2) {
                    std::cerr << "Error: valor para -v debe ser 0, 1 o 2.\n";
                    return 1;
                }
                ++i; // Saltamos el número
            } else {
                std::cerr << "Error: falta valor para -v.\n";
                return 1;
            }
        }
    }

    //std::cout << "Nivel de verbosidad: " << verbosity << "\n";

  read_problem();
  setup_problem();
  solve();
  auto s = prob->sol;
  cout << s->f << '\n';
  //for (int i=0; i<s->n; ++i) {
  //  cout << s->job[i]->rno << ",\n"[i+1==s->n];
  //}
  return 0;
}