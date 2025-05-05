#pragma once

#include <vector>
#include "models.h"


int calc_cost(std::vector<int> jobs, T2map* t2m, info* prob) {
    int t = 0;
    int total_cost = 0; 
    for (auto j : jobs) {
        t += prob->p[j];
        total_cost += t2m->tm[t].f[j];
    }
    return total_cost;
}   

int calc_cost(int* jobs, int n, T2map* t2m, info* prob) {
    int t = 0;
    int total_cost = 0;
    for (int i=0; i<n; ++i) { 
        int j = jobs[i];
        t += prob->p[j];
        total_cost += t2m->tm[t].f[j];
    }
    return total_cost;
}