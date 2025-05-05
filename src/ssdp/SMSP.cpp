#include <bits/stdc++.h>
using namespace std;

const int maxn = 52;
const double inf = 1e18;
int n;
int h[maxn]; 
int w[maxn];
int d[maxn];
int p[maxn];

int g(int i, int C) {
    return h[i] * max(d[i] - C, 0) + w[i] * max(C - d[i], 0); 
}

//ssdp
struct Node {
    int T;
    int i, j;
};

vector<float> ssdp[3e5 + 1];   

unordered_map<Node, int> id;


void init() {
    for ()

}


struct F1 {

    vector<double> dp;
    vector<int> tr;
    int T;

    void init(int T) {
        this->T = T;
        dp.resize(T + 1);
        tr.resize(T + 1, -1);
    }

    void solve(const vector<double>& lambda) {
        dp[0] = 0;
        for (int t=1; t<=T; ++t) {
            double& ans = dp[t] = inf;
            for (int i=0; i<n; ++i)  {
                if (t < p[i]) continue;
                double cst = dp[t - p[i]] + g(i, t) + lambda[i];
                if (ans > cst) {
                    tr[t] = i;
                    ans = cst;
                }
            }
        }
    }

    int greedy() {
        int t = 0;
        long long mask = (1ll<<n) - 1;
        int ans = 0;
        while (t != T) {
            int val = 1e9;
            int idx=-1;
            for (int i=0; i<n; ++i) {
                if (mask & (1ll<<i)) {
                    if (val > g(i, t + p[i])) {
                        val = g(i, t + p[i]);
                        idx = i;
                    }
                }
            }
            ans += val;
            mask ^= (1ll<<idx);
            t += p[idx];
        }
        //  cout << "help" << endl;
        int ans2 = 0;
        vector<int> id(n);
        iota(id.begin(), id.end(), 0);
        sort(id.begin(), id.end(), [](int i, int j) {
            return d[i] < d[j];
        });
        t = 0;
        for (int i=0; i<n; ++i) {
            t += p[id[i]];
            ans2 += g(id[i], t);
        }
        return min(ans, ans2);
    }


    int dp_ub[1<<9][102];
    int sumTfr[1<<9];
    int sumTfi[102];
    int UB() {
        vector<int> ids, ms(n); 
        for (int t=T; t>0; t -= p[tr[t]]) {
            ms[tr[t]]++;
            ids.push_back(tr[t]);
        }
        reverse(ids.begin(), ids.end());
        vector<int> _free, _fixed;
        vector<bool> is_fixed(n);
        for (int i=0; i < (int)ids.size(); ++i) {
            if (ms[ids[i]] == 1) is_fixed[ids[i]] = 1;
            else if (i + 1 < (int)ids.size() && ms[ids[i]] == 2 && ids[i] == ids[i+1]) {
                is_fixed[ids[i]] = 1;
            }
        }
        //cout << "u2" << endl;
        for (int i=0; i<n; ++i) {
            if (is_fixed[i]) _fixed.push_back(i);
            else _free.push_back(i);
        }
        if (_free.size() > 9) {
            cout << "upper bound is so complex!" << endl;
            return 1e9;
        }
        //cout << "u3" << endl;
        int nfr = _free.size();
        int nfi = _fixed.size();

        sumTfr[0] = 0;
        for (int s=1; s<(1<<nfr); ++s) {
            int lso = 31 - __builtin_clz(s & -s);
            sumTfr[s] = sumTfr[s ^ (1<<lso)] + p[_free[lso]];  
        }
        sumTfi[0] = 0;
        for (int i=1; i<=nfi; ++i) {
            sumTfi[i] = sumTfi[i-1] + p[_fixed[i - 1]];
        }
        for (int s=0; s<(1<<nfr); ++s) {
            for (int i=0; i<=nfi; ++i) {
                if (s == 0 && i == 0) dp_ub[s][i] = 0;
                else {
                    int T = sumTfr[s] + sumTfi[i];
                    dp_ub[s][i] = 1e9;
                    if (s > 0) {
                        for (int j=0; j<nfr; ++j) {
                            if (s & (1<<j)) {
                                dp_ub[s][i] = min(dp_ub[s][i], dp_ub[s ^ (1<<j)][i] + g(_free[j], T));  
                            }
                        }
                    }
                    if (i > 0) {
                        dp_ub[s][i] = min(dp_ub[s][i], dp_ub[s][i-1] + g(_fixed[i - 1], T));
                    }
                }
            }
        }
        cout << "upper bound: " << dp_ub[(1<<nfr) - 1][nfi] << endl;
        return dp_ub[(1<<nfr) - 1][nfi];
    }

    void subgradient() {


            
    }

    tuple<bool, int, int> subgradient(int iter, int k, int cnt) {
        vector<double> lambda(n, 0.);
        vector<double> f(3);
        //cout << "d1" << endl;
        solve(lambda);
        //cout << "d1.5" << endl;
//                    cout << "debug" << endl;

        f[0] = min(greedy(), UB());
        //cout << "debug" << endl;
        //cout << f[0] << endl;
        //cout << "d2" << endl;
        vector<double> lb1(k);
        double max_lb1 = lb1[0] = LB1(lambda);   
        vector<int> m(n, 0);
        int C = cnt;
        for (int it=0; it<iter; ++it) {
            int t = T;
            while (t > 0) {
                int id = tr[t];
                m[id]++;
                t -= p[id];
            }
            int n2 = 0.;
            for (int i=0; i<n; ++i) {
                n2 += (m[i] - 1) * (m[i] - 1);
            }

            if (abs(n2) < 1e-10) {
                return tuple<bool, int, int>(true, lb1[it % k], it);
            }

            for  (int i=0; i<n; ++i) {
               lambda[i] += (f[it % 3] - lb1[it % k]) * (m[i] - 1) / n2;
            }

            if (lb1[it % k] >= f[it % 3]) {
                f[(it + 1) % 3] = lb1[it % k] + abs(f[(it + 1) % 3] - f[it % 3]) * 0.5;  
                C--;
            } else if(it > k && max_element(lb1.begin(), lb1.end()) - lb1.begin() == (it + 1) % k) {
                f[(it + 1) % 3] = (f[it % 3] + max_lb1) * 0.5;
                C--;
            } else {
                f[(it + 1) % 3] = f[it % 3];
                
                C = cnt;
            }
            if (it % 5 == 1) {
                auto ex = UB();
                if (ex < 1e9)
                    f[(it + 1) % 3] = ex; //min(f[(it + 1) % 3], (double)UB());
            }
            if (C == -1) return tuple<bool, int, int>(false, max_lb1, it + 1);
            solve(lambda);
            lb1[(it + 1) % k] = LB1(lambda);
            max_lb1 = max(max_lb1, lb1[(it + 1) % k]);
            cout << lb1[(it + 1) % k] << endl;
        }
        return tuple<bool, int, int>(false, max_lb1, iter);
    }

    int LB1(const vector<double>& lambda) {
        return ceil(dp[T] - accumulate(lambda.begin(), lambda.end(), 0.d));
    }

};


int main() {
    cin>>n;
    //p h w d
    for (int i=0; i<n; ++i) {
        cin>>p[i]>>d[i]>>h[i]>>w[i];
    }
    F1 solver;
    int T = 0;
    for (int i=0; i<n; ++i) T += p[i];
    cout << "T: " << T << endl;
    solver.init(T);
    auto [ok, lb, it] = solver.subgradient(10000, 7, 5);
    cout << lb << ' ' << it << endl;
    return 0;
}
