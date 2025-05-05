#pragma once

#include <vector>
#include <numeric>
#include <string>
#include <algorithm>
#include "models.h"
#include "utils_io.h"

std::vector<int> greedy_fordward(info* prob, Tmap* tm) {
    std::vector<bool> taked(prob->n + 1);
    std::vector<int> sol(prob->n);
    int t = 0;
    for (int i=0; i<prob->n; ++i) {
        int idx = -1;
        for (int j=1; j<=prob->n; ++j) {
            if (taked[j]) continue;
            if (idx == -1) {
                idx = j;
            } else {
                int64_t f1 = tm[t + prob->p[j]].f[j];//* prob->p[idx];
                int64_t f2 = tm[t + prob->p[idx]].f[idx];// * prob->p[j];
                if (std::make_pair(f1, prob->ord[j]) < std::make_pair(f2, prob->ord[idx])) {
                    idx = j;
                }
            }
        }
        taked[idx] = 1;
        t += prob->p[idx];
        sol[i] = idx;
    }
    return sol;
}

std::vector<int> greedy_backward(info* prob, Tmap* tm) {
    std::vector<bool> taked(prob->n + 1);
    std::vector<int> sol(prob->n);
    int t = prob->T;
    for (int i=prob->n - 1; i>=0; --i) {
        int idx = -1;
        for (int j=1; j<=prob->n; ++j) {
            if (taked[j]) continue;
            if (idx == -1) {
                idx = j;
            } else {
                int64_t f1 = tm[t].f[j];// * prob->p[idx];
                int64_t f2 = tm[t].f[idx];// * prob->p[j];
                if (std::make_pair(f1, prob->ord[j]) < std::make_pair(f2, prob->ord[idx])) {
                    idx = j;
                }
            }
        }
        taked[idx] = 1;
        t -= prob->p[idx];
        sol[i] = idx;
    }
    return sol;
}

std::vector<int> shortest_processing_time_order(info* prob) {
    std::vector<int> jobs(prob->n);
    std::iota(jobs.begin(), jobs.end(), 1);
    std::sort(jobs.begin(), jobs.end(), [&](int i, int j) {
        return prob->ord[i] < prob->ord[j];
    });
    return jobs;   
}

void update(std::vector<int> jobs, int cst, info* prob) {
    int n = jobs.size();
    if (prob->fobj > cst) {
        prob->fobj = cst;
        prob->ub = cst;
        for (int i=0; i<n; ++i) {
            prob->sol[i] = jobs[i];
        }
    }
}

std::vector<int> partial_dp(info* prob, Tmap* tm, std::vector<int>& fixed_item, std::vector<int>& free_item) {
    for (auto& i : fixed_item) i -= 1;
    for (auto& i : free_item) i -= 1;
    int N1 = fixed_item.size();
    int N2 = free_item.size();

    int freeCount = N2;
    int subsetSize = 1 << freeCount;
    std::vector<int> sum_subset(subsetSize, 0);
    // sum_subset[0] ya es 0; para cada subconjunto s, se obtiene el índice del bit menos significativo activo
    for (int s = 1; s < subsetSize; ++s) {
        int bit = __builtin_ctz(s); // índice del primer bit activo
        sum_subset[s] = sum_subset[s ^ (1 << bit)] + prob->p[free_item[bit]];
    }
    
    // Precalcular sum_prefix: suma acumulada de p para los elementos fijos.
    std::vector<int> sum_prefix(N1 + 1, 0);
    for (int i = 1; i <= N1; ++i) {
        sum_prefix[i] = sum_prefix[i - 1] + prob->p[fixed_item[i - 1]];
    }
    
    // Inicializar DP y vector de reconstrucción.
    const int INF = 1e9;
    // dp[i][s]: costo mínimo usando i elementos fijos y el subconjunto s (de free_item).
    std::vector<std::vector<int>> dp(N1 + 1, std::vector<int>(subsetSize, INF));
    std::vector<std::vector<int>> tr(N1 + 1, std::vector<int>(subsetSize, -1));
    dp[0][0] = 0;
    
    // Caso base: solo fijos, s == 0.
    for (int i = 1; i <= N1; ++i) {
        int T = sum_prefix[i];
        int candidate = dp[i - 1][0] + tm[T].f[fixed_item[i - 1]];
        if (candidate < dp[i][0]) {
            dp[i][0] = candidate;
            tr[i][0] = -i;  // usamos -i para indicar que se tomó fixed_item[i-1]
        }
    }
    
    // Caso base: usar solo free_item (fijos = 0).
    for (int s = 1; s < subsetSize; ++s) {
        int T = sum_subset[s];
        int bit = __builtin_ctz(s);
        int candidate = dp[0][s ^ (1 << bit)] + tm[T].f[free_item[bit]];
        if (candidate < dp[0][s]) {
            dp[0][s] = candidate;
            tr[0][s] = bit;
        }
    }
    
    // Rellenar DP combinando elementos fijos y libres.
    for (int i = 1; i <= N1; ++i) {
        for (int s = 1; s < subsetSize; ++s) {
            int T = sum_prefix[i] + sum_subset[s];
            // Opción 1: agregar siguiente fixed_item.
            int candidate = dp[i - 1][s] + tm[T].f[fixed_item[i - 1]];
            if (candidate < dp[i][s]) {
                dp[i][s] = candidate;
                tr[i][s] = -i;
            }
            // Opción 2: agregar algún free_item (para cada bit activo en s).
            for (int j = 0; j < freeCount; ++j) {
                if (s & (1 << j)) {
                    candidate = dp[i][s ^ (1 << j)] + tm[T].f[free_item[j]];
                    if (candidate < dp[i][s]) {
                        dp[i][s] = candidate;
                        tr[i][s] = j;
                    }
                }
            }
        }
    }
    
    // Reconstrucción de la solución.
    int i = N1, s = subsetSize - 1;
    std::vector<int> ub_solution;
    while (i != 0 || s != 0) {
        int r = tr[i][s];
        if (r < 0) {
            // Se usó un elemento fijo.
            ub_solution.push_back(fixed_item[-r - 1] + 1);
            i--;
        } else {
            // Se usó un free_item.
            ub_solution.push_back(free_item[r] + 1);
            s ^= (1 << r);
        }
    }
    
    return ub_solution;
}


std::vector<int> greedy_insert(info* prob, Tmap* tm, std::vector<int>& psol, std::vector<int>& isol) {
    std::vector<int> sequence = psol;
    for (int job : isol) {
        int best_pos = 0;
        int64_t best_cost = std::numeric_limits<int64_t>::max();
        for (int pos = 0; pos <= (int)sequence.size(); pos++) {
            sequence.insert(sequence.begin() + pos, job);
            int64_t current_cost = 0;
            int t = 0;
            for (int j : sequence) {
                t += prob->p[j];
                current_cost += tm[t].f[j];
            }
            if (current_cost < best_cost) {
                best_cost = current_cost;
                best_pos = pos;
            }
            sequence.erase(sequence.begin() + pos);
        }
        sequence.insert(sequence.begin() + best_pos, job);
    }
    return sequence;
}

std::vector<int> upper_bound_refine(info* prob, Tmap* tm, std::vector<int>& quasi_solution, double lb, int lim=12, bool ford=1) { //FORDWARD
    //elements 1, 2, ..., n
    std::vector<int> order_; //order from left
    std::vector<int> cnt(prob->n + 1); //count contribution
    if (!ford) {
        std::reverse(quasi_solution.begin(), quasi_solution.end());
    }
    for (auto s_i : quasi_solution) {
        if (cnt[s_i] == 0) {
            order_.push_back(s_i); 
        }
        cnt[s_i] += 1;
    }
    if (!ford) {
        std::reverse(order_.begin(), order_.end());
        std::reverse(quasi_solution.begin(), quasi_solution.end());
    }
    if ((int)order_.size() == prob->n) {
        update(quasi_solution, round(lb), prob);
        print_result(prob, true);
    }   
    std::vector<int> fixed_item = order_;
    std::vector<int> free_item;
    for (int i = 1; i <= prob->n; ++i) {
        if (cnt[i] == 0) {
            free_item.push_back(i);
        }
    }
    if ((int)free_item.size() <= lim) return partial_dp(prob, tm, fixed_item, free_item);
    return greedy_insert(prob, tm, fixed_item, free_item);
}