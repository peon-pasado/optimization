#include <bits/stdc++.h>
using namespace std;




float h2[maxN][maxN][maxT];
pair<int, int> besth2[maxN][maxT];
void L2_fordward(const vector<double>& mu) {
    h2[0][0][0] = 0.;
    memset(besth2, -1, sizeof besth2);
    besth2[0][0] = 0;
    for (int t=1; t<=T; ++t) {
        for (int i=0; i<=n; ++i) {
            for (int j=1; j<=n; ++j) {
                if (i == j) continue;
                if (p[i] + p[j] > t) continue;
                auto [s, r] = besth2[i][t - p[j]];
                if (s != -1 && s != i) {
                    h2[i][j][t] = h2[s][i][t - p[j]] + f(j, t) - mu[j];
                } else if (r != -1) {
                    h2[i][j][t] = h2[r][i][t - p[j]] + f(j, t) - mu[j];
                }

                auto& [u, v] = besth2[j][t];
                if (u == -1) {
                    u = i;
                }
                else if (h2[u][j][t] > h2[i][j][t]) {
                    v = u;
                    u = i;
                } else if (h2[v][j][t] > h2[i][j][t]) {
                    v = i;
                }
            }
        }
    }
}

float H2[maxN][maxN][maxT];
pair<int, int> bestH2[maxN][maxT];
void L2_backward(const vector<double>& mu) {
    H2[0][0][T] = 0.;
    memset(besth2, -1, sizeof bestH2);
    bestH2[0][T] = 0;
    for (int t=T-1; t>=0; --t) {
        for (int i=0; i<=n; ++i) {
            for (int j=1; j<=n; ++j) {
                if (i == j) continue;
                if (t + p[i] + p[j] > T) continue;
                auto [s, r] = bestH2[j][t + p[i]];
                if (s != -1 && s != i) {
                    H2[i][j][t] = H2[j][s][t + p[i]] + f(i, t + p[i]) - mu[i];
                } else if (r != -1) {
                    H2[i][j][t] = H2[j][r][t + p[i]] + f(i, t + p[i]) - mu[i];
                }

                auto& [u, v] = bestH2[i][t];
                if (u == -1) {
                    u = j;
                }
                else if (H2[i][u][t] > H2[i][j][t]) {
                    v = u;
                    u = j;
                } else if (H2[i][v][t] > H2[i][j][t]) {
                    v = j;
                }
            }
        }
    }
}

int main() {


    return 0;
}