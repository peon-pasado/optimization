#include <bits/stdc++.h>
using namespace std;

const int step = 1<<23;//eps = 1 / step
const int maxn = 23;
const int L = 2;
int sz[maxn];
int n;

//1 -> step
//1/k ~~ ceil(step / k) 
int n_bins_ceil(int k) { // 1 / k
    return (step + k - 1) / k;
}

int n_bins_floor(int k) { // 1 / k
    return step / k;
}


long long area(int height) {
    return 1ll * height * height;
}

long long dp_1D[maxn][maxn];
void preprocess() {
    const long long inf = 1e18;
    vector<long long> min_area(step + 1);

    for (int i=0; i<n; ++i) {
        fill(min_area.begin(), min_area.end(), inf);
        min_area[0] = 0;
        for (int j=i; j<n; ++j) {
            
            for (int k = sz[j]; k<=step; ++k) {
                min_area[k] = min(min_area[k], min_area[k - sz[j]] + area(sz[j]));
            }

            auto& ans = dp_1D[i][j] = inf;

            for (int k = step - sz[j] - sz[i] + 1; k<=step; ++k) {
                ans = min(ans, min_area[k] + area(sz[i]));
            }

        } 
    }
}


long long dp_2D[maxn][step + 1];
long long dp() {
    memset(dp_2D, 1, sizeof dp_2D);
    dp_2D[0][sz[0]] = 0; 
    for (int s=sz[0] + 1; s<=step; ++s) {
        for (int j=0; j<n; ++j) {
            for (int i=0; i<=j; ++i) {
                dp_2D[j][s] = min(dp_2D[j][s], dp_2D[i][s - sz[j]] + dp_1D[i][j]);
            } 
        }
    }
    long long ans = 1e18;
    int idx_final = -1;
    for (int i=0; i<n; ++i) {
        for (int j=i; j<n; ++j) {
            for (int k=step-sz[j]+1; k<=step; ++k) {
                if (ans > dp_2D[i][k] + dp_1D[i][j] + area(sz[j])) {
                    ans = dp_2D[i][k] + dp_1D[i][j] + area(sz[j]);
                    idx_final = j;
                }
            }
        }
    }
    //cout << "final bin: " << sz[idx_final] << " / " << step << endl;
    return ans;
}

double solve_lower() {
    for (int i=0; i<n; ++i) {
        sz[i] = n_bins_ceil(i + L);
    }
    preprocess();
    return 1.d * (dp() - area(sz[0])) / area(step);
}

double solve_upper() {
    for (int i=0; i<n; ++i) {
        sz[i] = (step >> (i + 1));//n_bins_floor(i + L);
    }
    preprocess();
    return 1.d * (dp() - area(sz[0])) / area(step);
}

int main() {
    cout.precision(8);
    cout << fixed;
    for (int i=1; i<=23; ++i) {
        n = i;
        //cout << "lower: " << solve_lower() << endl;
        cout << i << ": " << solve_upper() << endl;
    }
    return 0;
}