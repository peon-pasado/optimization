#pragma once

#include <vector>
#include "models.h"

std::pair<std::vector<int>, int> dynasearch(std::vector<int> order) {
    auto tm = tmap;
    int n = prob->n;
    std::vector<int> c(n);
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(3));
    std::vector<int> job(n);
    int final_cost = 0;

    int iter = 0;
    while (true) {
        iter++;
        c[0] = prob->p[order[0]];
        for (int i = 1; i < n; ++i)
            c[i] = c[i - 1] + prob->p[order[i]];

        dp[0][0] = 0;
        dp[0][1] = -1;
        dp[0][2] = 0;

        for (int i = 1; i <= n; i++) {
            dp[i][0] = dp[i - 1][0] + tm[c[i-1]].f[order[i - 1]];
            dp[i][1] = i - 1;
            dp[i][2] = 0;

            // Regla PI
            for (int j = 0; j <= i - 2; j++) {
                int cx = (j > 0) ? c[j - 1] + prob->p[order[i - 1]] : prob->p[order[i - 1]];
                int vv = dp[j][0] + tm[cx].f[order[i - 1]];
                for (int k = j + 1; k <= i - 2; k++) {
                    cx += prob->p[order[k]];
                    vv += tm[cx].f[order[k]];
                }
                cx += prob->p[order[j]];
                vv += tm[cx].f[order[j]];
                if (vv < dp[i][0]) {
                    dp[i][0] = vv;
                    dp[i][1] = j;
                    dp[i][2] = 1;
                }
            }

            // Regla EBSR
            for (int j = 0; j <= i - 3; j++) {
                int cx = (j > 0) ? c[j - 1] + prob->p[order[i - 1]] : prob->p[order[i - 1]];
                int vv = dp[j][0] + tm[cx].f[order[i - 1]];
                for (int k = j; k <= i - 2; k++) {
                    cx += prob->p[order[k]];
                    vv += tm[cx].f[order[k]];
                }
                if (vv < dp[i][0]) {
                    dp[i][0] = vv;
                    dp[i][1] = j;
                    dp[i][2] = 2;
                }
            }

            // Regla EFSR
            for (int j = 0; j <= i - 3; j++) {
                int cx = (j > 0) ? c[j - 1] : 0;
                int vv = dp[j][0];
                for (int k = j + 1; k <= i - 1; k++) {
                    cx += prob->p[order[k]];
                    vv += tm[cx].f[order[k]];
                }
                cx += prob->p[order[j]];
                vv += tm[cx].f[order[j]];
                if (vv < dp[i][0]) {
                    dp[i][0] = vv;
                    dp[i][1] = j;
                    dp[i][2] = 3;
                }
            }

            // Regla RS (Reverse Segment)
            if (iter <= 2) {
                for (int j = 0; j <= i - 2; j++) {
                    int cx = (j > 0) ? c[j - 1] : 0;
                    int vv = dp[j][0];
                    for (int k = i - 1; k >= j; k--) {
                        cx += prob->p[order[k]];
                        vv += tm[cx].f[order[k]];
                    }
                    if (vv < dp[i][0]) {
                        dp[i][0] = vv;
                        dp[i][1] = j;
                        dp[i][2] = 4;
                    }
                }
            }
        }


        // Reconstrucción de la solución
        for (int i = n, j = n - 1; i > 0;) {
            switch (dp[i][2]) {
                case 0:
                    job[j--] = order[dp[i][1]];
                    break;
                case 1: {
                    job[j--] = order[dp[i][1]];
                    for (int k = i - 2; j > dp[i][1]; j--, k--)
                        job[j] = order[k];
                    job[j--] = order[i - 1];
                    break;
                }
                case 2: {
                    for (int k = i - 2; j > dp[i][1]; j--, k--)
                        job[j] = order[k];
                    job[j--] = order[i - 1];
                    break;
                }
                case 3: {
                    job[j--] = order[dp[i][1]];
                    for (int k = i - 1; j >= dp[i][1]; j--, k--)
                        job[j] = order[k];
                    break;
                }
                case 4: {
                    for (int k = dp[i][1]; k < i; k++)
                        job[j--] = order[k];
                    break;
                }
            }
            i = dp[i][1];
        }
        //std::cout << "final cost: " << dp[n][0] << std::endl;
        final_cost = dp[n][0];
        if (job == order)
            break;
        order.swap(job);
    }
    return make_pair(order, final_cost);
}
