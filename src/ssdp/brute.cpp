#include <bits/stdc++.h>
using namespace std;

const int maxn = 102;
int p[maxn];
int h[maxn];
int w[maxn];
int d[maxn];
int n;
int T;

int g(int i, int C) {
    return h[i] * max(d[i] - C, 0) + w[i] * max(C - d[i], 0); 
}

const int maxs = 1<<29;
short memo[maxs];
bitset<maxs> vis;
int dp(long long mask, int t) {
    if (mask+1 == (1ll<<(n-1))) return 0;
    if (vis[mask]) return memo[mask];
    int ans = 1e9;    
    for (int i=0; i<n-1; ++i) {
        if ((~mask) & (1ll<<i)) {
            ans = min(ans, dp(mask | (1ll<<i), t + p[i]) + g(i, t + p[i]));
        }
    }
    vis[mask] = 1;
    memo[mask] = ans;
    return ans;
}

int main() {    
    cin>>n;
    for (int i=0; i<n; ++i) {
        cin>>p[i]>>h[i]>>w[i]>>d[i];
        T += p[i];
    }
    int ans = dp(0, 0) + g(n-1, T);
    for (int i=0; i<n-1; ++i) {
        swap(p[i], p[n-1]);
        swap(h[i], h[n-1]);
        swap(w[i], w[n-1]);
        swap(d[i], d[n-1]);
        vis.reset();
        ans = min(ans, dp(0, 0) + g(n-1, T));
    }
    cout << ans << '\n';
    return 0;
}