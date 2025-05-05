#pragma once

#include "heuristics.h"
#include "utils_dp.h"
#include "dynasearch.h"
#include "utils_io.h"
#include <algorithm>
#include <vector>
#include "subgradient.h"
//#include "sublimation.h"
#include "main_.h"

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

void stage3() {
    ssdp2::n = prob->n;
    ssdp2::T = prob->T;
    ssdp2::p.resize(prob->n + 2);
    ssdp2::w.resize(prob->n + 2);
    ssdp2::d.resize(prob->n + 2);
    for (int i=1; i<=prob->n; ++i) {
        ssdp2::p[i] = prob->p[i];
        ssdp2::w[i] = prob->w[i];
        ssdp2::d[i] = prob->d[i];
    }
    ssdp2::p[prob->n + 1] = 1;
    ssdp2::p[0] = 0;
    ssdp2::UB = prob->ub;
    ssdp2::mu.resize(prob->n + 2);
    for (int i = 0; i <= prob->n + 1; i++) {
        ssdp2::mu[i] = prob->u[i];
    }
    ssdp2::opt_jobs.resize(prob->n);
    for (int i = 0; i < prob->n; i++) {
        ssdp2::opt_jobs[i] = prob->ord[i];
    }
    ssdp2::root = new ssdp2::Node(0);
    ssdp2::root->no[0][0] = 0;
    ssdp2::root->value[0][0] = 0;
    ssdp2::at[0].push_back(ssdp2::root);
    double smu = accumulate(ssdp2::mu.begin(), ssdp2::mu.end(), 0.0);
    for (int t=1; t<=prob->T; ++t) {
        if (!check_time(t)) continue;
        for (uint32_t i=1; i<=prob->n; ++i) {
            if ((int)ssdp2::p[i] > t) continue;
            if (!check_node(t, i)) continue;
            auto no = new ssdp2::Node(i);
            for (int j=0; j<(int)ssdp2::at[t - ssdp2::p[i]].size(); ++j) {
                auto jt = ssdp2::at[t - ssdp2::p[i]][j];
                if (jt->i == (int)i) continue;
                if (!check_edge(t, jt->i, i)) continue;
                int x = jt->no[0][0] == i;
                int y = 0;//x == 0 ? jt->sno[0][0][0] == i : 0;
                ssdp2::update(0, no, jt->i, jt->value[0][x] + tmap[t].f[i] - ssdp2::mu[i]);
                if (no->outdeg == 0) no->adj.resize(ssdp2::at[t-ssdp2::p[i]].size());
                ssdp2::add_edge(no, jt, j);
            }
            if (no->outdeg) {
                ssdp2::at[t].emplace_back(no);
            } else {
                delete no;
            }
        }
    }
    ssdp2::broot = new ssdp2::Node(ssdp2::n+1);
    ssdp2::broot->adj.resize(ssdp2::at[prob->T].size());
    ssdp2::at[prob->T+1].push_back(ssdp2::broot);
    for (int i=0; i<(int)ssdp2::at[prob->T].size(); ++i) {
        if (!check_node(prob->T, ssdp2::at[prob->T][i]->i)) continue;
        ssdp2::update(0, ssdp2::broot, ssdp2::at[prob->T][i]->i, ssdp2::at[prob->T][i]->value[0][0]);
        ssdp2::add_edge(ssdp2::broot, ssdp2::at[prob->T][i], i);
    }
    ssdp2::shrink(0);
}


void ssdp(info* prob, Tmap* tm, T2map* t2m) {
    prob->stage = 0;
    print_stage(prob);
    auto [jobs, cst] = best_heuristic(prob, tm, t2m);
    auto [ejobs, ecst] = dynasearch(prob, tm, jobs);
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
    time_t start = clock();
    ssdp2::solve();
    time_t end = clock();
    //if (initialize_subl_graph()) {
     //   sublimation();    
    //}
    print_result(prob);
}