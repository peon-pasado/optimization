#include <bits/stdc++.h>
using namespace std;

const int step = 2654208;//eps = 1 / step
const int maxn = 96;
const int L = 2;
int sz[maxn], inv[maxn];
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

double dp_1D[maxn][maxn];

//

void preprocess() {
    for (int i=0; i<n; ++i) {
        for (int j=i; j<n; ++j) {
            //dp_1D[i][j] = 1 / inv[i] + t * 1 / inv[j] <= 1
            long long t = (inv[i] - 1) * 1ll* inv[j] / inv[i];
            dp_1D[i][j] = (double)step / inv[i] + (double)t * step / inv[j];
        }
    }
}

double dp_2D[maxn][step + 1];
long long dp() {
    for (int i=0; i<n; +++i)
        for (int j=0; j<=step; ++j)
            dp_2D[i][j] = 1e15;
    
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
    cout << "final bin: " << sz[idx_final] << " / " << step << endl;
    return ans;
}

int m;
double solve_upper() {
    vector<int> di = {1};
    for (int i=2; i*i<=m; ++i) {
        if (m % i == 0) {
            di.push_back(i);
            if (i != m/i) di.push_back(m / i);
        }
    }
    sort(di.rbegin(), di.rend());
    n = di.size();
    for (int i=0; i<n; ++i) {
        sz[i] = di[i];
        inv[i] = m / di[i];
    } 
    preprocess();
    return 1.d * (dp() - area(sz[0])) / area(step);
}

int main() {
    cout.precision(8);
    cout << fixed;
    m = step;
    cout << solve_upper() << endl;
    return 0;
}