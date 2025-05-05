#include <bits/stdc++.h>
using namespace std;

const int maxn = 410;
const int N = 100'000;
const int M = N + N;//1000;
int w[maxn];
double v[maxn];
double dp[N + 5];
double tr[N + 5];
double f[maxn][maxn], g[maxn][maxn];
double dp2[M][maxn];
double tr2[M][maxn];

void calc1(int r) {
    for (int i=2; i<=r; ++i) {
        f[i][r] = floor((1 - 1./i + 1e-8) * r) / r / r + 1. / i / i;
    }
    for (int i=1; i<=w[2]; ++i) {
        dp[i] = 1e18;
        tr[i] = 2;
    }
    dp[0] = tr[0] = 0;
    int L = max(3, (int)(r));
    for (int i=r; i>=L; --i) {
        for (int k=w[i]; k<=2 * w[2]; ++k) {
            double tmp = dp[k-w[i]] + v[i];
            if (dp[k] > tmp) {
                dp[k] = tmp;
                tr[k] = tr[k - w[i]] + 1. / i;
                for (int j=2; j<=i; ++j) {
                    int rr = (1 - tr[k] - 1./j + 1e-8) * r;
                    f[j][r] = min(f[j][r], dp[k] + 1. * rr / r / r + 1. / j / j);
                }
            } 
        }
    }
}

int n;
double calc2() {
    for (int i=0; i<N + 1000; ++i)
        for (int j=2; j<=n; ++j) {
            dp2[i][j] = 1e18;
            tr2[i][j] = 2;           
        }
    dp2[w[2]][2] = 0;
    tr2[w[2]][2] = 0.5; 
    for (int i=w[2] + 1; i<N + 2 * w[n]; ++i) {
        for (int u=2; u<=n; ++u) {
            for (int v=u; v<=n; ++v) {
                assert(i - w[v] >= 0);
                double tmp = dp2[i - w[v]][u] + f[u][v];
                if (tmp < dp2[i][v]) {
                    dp2[i][v] = tmp;
                    tr2[i][v] = tr2[i-w[v]][u] + 1./v;
                }
            }
        }
    }
    double res = 1e18;
    for (int i=N - w[2]; i<N + w[2]; ++i) {
        for (int j=2; j<=n; ++j) {
            if (tr2[i][j] > 1 + 1e-8 && tr2[i][j] - 1./j <= 1 + 1e-8) {
                res = min(res, dp2[i][j]);// + 1./j/j);
            }
        }
    }
    return res - 0.25;
}

int main() {
    cin>>n;
    for (int i=2; i<=n; ++i) {
        w[i] = (N + i - 1) / i;
        v[i] = 1./i/i;
    }
    for (int i=2; i<=n; ++i) {
        calc1(i);
        //_calc1(i);
        //cout << i << '\n';
        //for (int j=2; j<=i; ++j) {
        //    assert(abs(f[j][i] - g[j][i]) < 1e-6);
            //cout << f[j][i] << " \n"[j==i];
        //}
    }
    cout.precision(10);
    cout << fixed;
    cout << calc2() << '\n';
    return 0;
}   