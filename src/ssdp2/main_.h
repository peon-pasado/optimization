#include <bits/stdc++.h>
//#include "bitset.h"
#include "subgradient.h"
using namespace std;

namespace ssdp2 {

const int maxN = 152;
const int maxT = 20005;
int ha[maxN];
using ll = int32_t;//long long;
ll UB;
int nids = 0;
int pt[310];
int T;
vector<double> mu;
ll n_edges = 0;
vector<int> opt_jobs;
vector<int> p, w, d;
int n;
const double EPS = 1e-6;

using uint = unsigned long long;
const int B = 64;

struct bset {
    vector<uint> s;

    inline int size() const {
        return s.size();
    }

    inline void resize(uint n) {
        s.resize((n + B - 1) / B);
    }

    inline bool empty() const {
        return s.empty();
    }

    inline bool at(uint pos) const {
        return (s[pos / B] >> (pos % B)) & 1;
    }

    inline void set(uint pos) {
        s[pos / B] |= 1ull << (pos % B);
    }

    inline void unset(uint pos) {
        s[pos / B] &= ~(1ull << (pos % B));
    }

    int next(uint pos) const {
        if (s.empty()) return -1;
        uint x = pos / B, y = pos % B;
        if (x >= s.size()) return -1;

        uint mask = s[x] >> y;
        if (mask) return (x * B) + y + __builtin_ctzll(mask);

        for (++x; x < s.size(); ++x) {
            if (s[x]) return (x * B) + __builtin_ctzll(s[x]);
        }

        return -1;
    }
};


struct Node {
    int i;
    Node() {}
    Node(int i): i(i) {
        for (int i=0; i<2; ++i) {
            for (int j=0; j<2; ++j) {
                value[i][j] = 1e12;
                no[i][j] = -1;
            }
        }
        indeg=0; 
        outdeg=0; 
    }
    double value[2][2];
    int no[2][2];

    int indeg;
    int outdeg;
    
    bset adj;
} *root, *broot;


void add_edge(Node* x, Node* y, int pos) {
    n_edges++;
    x->outdeg++;
    y->indeg++;
    x->adj.set(pos);
}

void erase_edge(Node* x, Node* y, int pos) {
    n_edges--;
    x->outdeg--;
    y->indeg--;
    x->adj.unset(pos);
}

vector<Node*> at[maxT];
vector<int> ids;
int od[maxN];
double h1[maxN][maxT];
double H1[maxN][maxT];

int f(int i, int T) {
    //if (vis2[i][T]) return memo[i][T];
    //vis2[i][T] = 1;
    //return memo[i][T] = (i >= 1 && i <= n && T > d[i]) ? w[i] * (T - d[i]) : 0;
    return (i >= 1 && i <= n && T > d[i]) ? w[i] * (T - d[i]) : 0;
}

inline int calc_cost(const vector<int>& sol) {
    int t = 0;
    int res = 0;
    for (int i : sol) {
        t += p[i];
        res += f(i, t);
    }
    return res;
}


void print_ub() {
    cout << UB << endl;
    ////cout << UB << endl;
    for (int i : opt_jobs) {
        ////cout << i << ' ';
    }
    ////cout << endl;
}

vector<int> greedy_fordward() {
    set<int> next_elements;
    for (int i=1; i<=n; ++i) {
        next_elements.insert(i);
    }
    vector<int> sol;
    int t = 0;
    for (int i=0; i<n; ++i) {
        int idx = -1;
        for (int j : next_elements) {
            if (idx == -1 || f(idx, t + p[idx]) > f(j, t + p[j])) {
                idx = j;
            }
        }
        sol.push_back(idx);
        t += p[idx];
        next_elements.erase(idx);
    }
    return sol;
}

vector<int> greedy_backward() {
    set<int> next_elements;
    for (int i=1; i<=n; ++i) {
        next_elements.insert(i);
    }
    vector<int> sol;
    int T = accumulate(p.begin(), p.end(), 0);    
    for (int i=0; i<n; ++i) {
        int idx=-1;
        for (int j : next_elements) {
            if (idx == -1 || f(idx, T) > f(j, T)) {
                idx = j;
            }
        }
        sol.push_back(idx);
        T -= p[idx];
        next_elements.erase(idx);
    }
    reverse(sol.begin(), sol.end());
    return sol;
}


vector<int> shortest_processing_time_order() {
    vector<int> id(n);
    iota(id.begin(), id.end(), 1);
    sort(id.begin(), id.end(), [](int i, int j) {
        return od[i] < od[j];
    });
    return id;   
}

vector<int> dynasearch(vector<int> order, int lw=0, int nit=10000) {

    for (int s=0; s<nit; ++s) {

    vector<vector<int32_t>> dp(n + 1, vector<int32_t>(3));
    int32_t i, j, k;
    int vv;
    int cx;

    vector<int> c(n);
    int t=0;
    for (int32_t i=0; i<(int32_t)n; ++i) {
        t += p[order[i]];
        c[i] = t;
    }

    if (lw == 0) {
        dp[0][0] = 0; // f
        dp[0][1] = -1;        // j
        dp[0][2] = 0;         // ty
    }

    vector<int> job(n);
    for (i = lw + 1; i <= (int32_t)n; i++) {
        dp[i][1] = i - 1; // j
        dp[i][0] = dp[i - 1][0] + f(order[i - 1], c[i - 1]); // f
        dp[i][2] = 0; // ty

        // PI
        for (j = 0; j <= i - 2; j++) {
            if (j > 0) {
                cx = c[j - 1] + p[order[i - 1]];
            } else {
                cx = p[order[i - 1]];
            }
            vv = dp[j][0] + f(order[i - 1], cx);
            for (k = j + 1; k <= i - 2; k++) {
                cx += p[order[k]];
                vv += f(order[k], cx);
            }
            cx += p[order[j]];
            vv += f(order[j], cx);

            if (vv < dp[i][0]) {
                dp[i][0] = vv; // f
                dp[i][1] = j; // j
                dp[i][2] = 1; // ty
            }
        }

        // EBSR
        for (j = 0; j <= i - 3; j++) {
            if (j > 0) {
                cx = c[j - 1] + p[order[i - 1]];
            } else {
                cx = p[order[i - 1]];
            }
            vv = dp[j][0] + f(order[i - 1], cx);
            for (k = j; k <= i - 2; k++) {
                cx += p[order[k]];
                vv += f(order[k], cx);
            }

            if (vv < dp[i][0]) {
                dp[i][0] = vv; // f
                dp[i][1] = j; // j
                dp[i][2] = 2; // ty
            }
        }

        // EFSR
        for (j = 0; j <= i - 3; j++) {
            if (j > 0) {
                cx = c[j - 1];
            } else {
                cx = 0;
            }
            vv = dp[j][0]; // f

            for (k = j + 1; k <= i - 1; k++) {
                cx += p[order[k]];
                vv += f(order[k], cx);
            }
            cx += p[order[j]];
            vv += f(order[j], cx);

            if (vv < dp[i][0]) {
                dp[i][0] = vv; // f
                dp[i][1] = j; // j
                dp[i][2] = 3; // ty
            }
        }
    }

    for (i = n, j = n - 1; i > 0;) {
        switch (dp[i][2]) {
            case 0:
                job[j--] = order[dp[i][1]];
                break;
            case 1: // PI
                job[j--] = order[dp[i][1]];
                for (k = i - 2; j > dp[i][1]; j--, k--) {
                    job[j] = order[k];
                }
                job[j--] = order[i - 1];
                break;
            case 2: // EBSR
                for (k = i - 2; j > dp[i][1]; j--, k--) {
                    job[j] = order[k];
                }
                job[j--] = order[i - 1];
                break;
            case 3: // EFSR
                job[j--] = order[dp[i][1]];
                for (k = i - 1; j >= dp[i][1]; j--, k--) {
                    job[j] = order[k];
                }
                break;
        }

        i = dp[i][1];
    }
        if (job == order) break;
       //if (calc_cost(job) == calc_cost(order)) break;
       order = job;
    }

    //for (lw = 0; lw < sol->n; lw++) {
    //    if (job[lw]->no != sol->job[lw]->no) {
    //        break;
    //    }
    //}
    //return lw;
    return order;
}

vector<int> greedy_sp() {
    vector<int> s;
    int t = T;
    vector<bool> vis(n + 1); 
    for (int i=0; i<n; ++i) {
        int idx = -1;
        for (int i=1; i<=n; ++i) {
            if (vis[i]) continue;
            if (idx == -1 || h1[i][t] < h1[idx][t]) {
                idx = i;
            }       
        }
        s.push_back(idx);
        vis[idx] = 1;
        t -= p[idx];
    }
    return s;
}

mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
vector<int> greedy_sp_l2() {
    vector<int> s;
    int v = -1;
    int t = T;
    vector<bool> vis(n + 1);
    vector<int> id(n);
    iota(id.begin(), id.end(), 1);
    shuffle(id.begin(), id.end(), rng);
    for (int i=1; i<=n; ++i) {
        double val = 1e13;
        for (auto jt : at[t]) {
            if (jt->no[0][0] == -1) continue;
            if (vis[jt->i]) continue;
            if (jt->value[0][0] < val) {
                val = jt->value[0][0];
                v = jt->i;
            }
        }        
        if (val > 1e12) {
            vector<int> r;
            for (int i=1; i<=n; ++i) {
                if (vis[i]) continue;
                r.push_back(i);
                break;
            }
            v = r[0];
        }
        vis[v] = 1;
        s.push_back(v);
        t -= p[v];
    }
    reverse(s.begin(), s.end());
    return s;
}

vector<int> greedy_sp_l2_backward() {
    vector<int> s;
    int v = -1;
    int t = 0;
    vector<bool> vis(n + 1);
    vector<int> id(n);
    iota(id.begin(), id.end(), 1);
    shuffle(id.begin(), id.end(), rng);
    for (int i = 1; i <= n; ++i)
    {
        double val = 1e12;
        for (auto jt : at[t])
        {
            for (int k=0; k<2; ++k) {
                if (jt->no[1][k] == -1)
                    continue;
                if (vis[jt->no[1][k]])
                    continue;
                if (jt->value[1][k] < val)
                {
                    val = jt->value[1][k];
                    v = jt->no[1][k];
                }
            }
        }
        if (val > 1e8 || v == (int32_t)n+1) {
            for (int i = 1; i <= n; ++i) {
               if (vis[i])
                    continue;
                v = i;
                break;
            }
        }
        vis[v] = 1;
        s.emplace_back(v);
        t += p[v];
    }
    //reverse(s.begin(), s.end());
    return s;
}

vector<int> upper_bound_refine(vector<int>& quasi_solution, int lim=12, bool ford=1, bool L2 = 1) { //FORDWARD
    
    //elements 1, 2, ..., n
    vector<int> order_; //order from left
    vector<int> cnt(n); //count contribution
    if (!ford) {
        reverse(quasi_solution.begin(), quasi_solution.end());
    }
    for (auto s_i : quasi_solution) {
        if (cnt[s_i - 1] == 0) {
            order_.push_back(s_i - 1); 
        }
        cnt[s_i - 1] += 1;
    }
    if (!ford) {
        reverse(order_.begin(), order_.end());
        reverse(quasi_solution.begin(), quasi_solution.end());
    }

    //0-base from here
    int N1 = order_.size();
    int N2 = n - N1;

    if (N2 == 0) {
        ////cout << "n2 found solution " << ford << endl;
        opt_jobs = quasi_solution;
        print_ub();
        exit(0);
    }

    if (N2 <= lim) { //dynamic programming process
        ////cout << "limits: " << N1 << ", " << N2 << endl;
        auto fixed_item = order_;
        vector<int> free_item;
        for (int i=0; i<n; ++i) {
            if (cnt[i] == 0) {
                free_item.push_back(i);
            }
        }
        vector<int> sum_subset(1<<N2); //[0] = 0
        for (int s=1; s<(1<<N2); ++s) {
            int first_idx = 31 - __builtin_clz(s & -s);
            sum_subset[s] = sum_subset[s ^ (1 << first_idx)] + p[free_item[first_idx]];
        }
        vector<int> sum_prefix(N1 + 1);
        for (int i=1; i<=N1; ++i) {
            sum_prefix[i] = sum_prefix[i-1] + p[fixed_item[i - 1]];
        }
        vector<vector<long long>> dp(N1 + 1, vector<long long>(1<<N2, 1e18));
        vector<vector<int>> tr(N1 + 1, vector<int>(1<<N2, -1));
        dp[0][0] = 0;
        for (int i=1; i<=N1; ++i) {
            int T = sum_prefix[i];
            if (dp[i][0] > dp[i-1][0] + f(fixed_item[i-1], T)) {
                dp[i][0] = dp[i-1][0] + f(fixed_item[i-1], T);
                tr[i][0] = -i;
            }
        }
        for (int s=1; s<(1<<N2); ++s) {
            int T = sum_subset[s];
            int first_idx = 31 - __builtin_clz(s & -s);
            long long temp = dp[0][s ^ (1<<first_idx)] + f(free_item[first_idx], T);
            if (dp[0][s] > temp) {
                dp[0][s] = temp;
                tr[0][s] = first_idx;
            }
        }
        for (int i=1; i<=N1; ++i)
            for (int s=1; s<(1<<N2); ++s) {
                int T = sum_subset[s] + sum_prefix[i];
                if (dp[i][s] > dp[i-1][s] + f(fixed_item[i-1], T)) {
                    dp[i][s] = dp[i-1][s] + f(fixed_item[i-1], T);
                    tr[i][s] = -i;
                }
                for (int j=0; j<N2; ++j) {
                    if (s & (1<<j)) {
                        auto temp = dp[i][s^(1<<j)] + f(free_item[j], T);
                        if (dp[i][s] > temp) {
                            dp[i][s] = temp;
                            tr[i][s] = j;
                        }
                    }
                }
            }
        int i = N1, s = (1<<N2)-1;
        vector<int> ub_solution;
        while (i != 0 || s != 0) {
            int r = tr[i][s];
            if (r < 0) {
                ub_solution.push_back(fixed_item[-r - 1] + 1);
                i -= 1;
            } else {
                ub_solution.push_back(free_item[r] + 1);
                s ^= (1<<r);
            }
        }        
        return ub_solution;
    }
    vector<int> sol = L2 ? (ford ? greedy_sp_l2() : greedy_sp_l2_backward()) : greedy_sp();
    return  sol;
}

double calc_cost_lb(const vector<int>& s, const vector<double>& mu) {
    double sum = 0;
    int t = 0;
    for (auto i : s) {
        t += p[i];
        sum += f(i, t) - mu[i];
    }
    for (int i=1; i<=(int)n; ++i) {
        sum += mu[i];
    }
    return sum; 
}

double dnorm = 1;
vector<double> du(300);
void update_multipliers(vector<double>& mu, double LB, double UB, const vector<int>& dual_solution, double g) {
    vector<int> cnt_elem(n + 1);
    for (int i : dual_solution) {
        cnt_elem[i]++;
    }
    double norm = 0;
    for (int i=1; i <= n; ++i) {
        norm += (1 - cnt_elem[i]) * (1 - cnt_elem[i]); 
    }
    if (norm < 0.5) {
        opt_jobs = dual_solution;
        print_ub();
        exit(0);
    }

    double ksi = sqrt(norm / dnorm);

    for (int i = 1; i <= n; ++i) {
        du[i] = (1 - cnt_elem[i]) + ksi * du[i];
    }

    dnorm = 0.0;
    for(int i = 1; i <= n; i++) {
        dnorm += du[i] * du[i];
    }

    if(dnorm < EPS) {
        dnorm = norm;
        for(int i = 1; i <= n; ++i) {
            du[i] = (1 - cnt_elem[i]);
        }
    }

    double tk = g * (UB - LB) / dnorm;
    for (int i=1; i <= n; ++i) {
        mu[i] += tk * du[i];
    }

        

    //gmu = mu;
}

//gamma_init, delta_T, delta_S, eps, k_S, k_E
/** 
tuple<vector<double>, double> subgradient_L1(double g_0=2, int d_T=n/4, int d_S=4, double eps=0.02, double k_S=0.75, double k_E=2) {
    vector<double> mu(n + 2, 0.);
    auto best_mu = mu;
    vector<double> G(d_T + 1);
    double best = 0;
    double cur = 0;
    int iter = 0;   
    int id_best = 0;
    double g = g_0;
    int niter = 0;
    while (true) {
        niter++;
        int pre = iter;
        iter = (iter + 1) % (d_T + 1);
        int last = (iter + 1) % (d_T + 1);

        L1_fordward(mu);
        auto sol_lb = obtain_fordward_L1();
        cur = calc_cost_lb(sol_lb, mu);

        G[iter] = max(G[pre], cur);
        
        if (niter > 1 * d_T + 1 
            && G[iter] * (1 - eps) < G[last]
            && (G[iter] - G[pre]) < (UB - G[last]) * eps) {
            break;
        } 

        if (cur > best + EPS) {
            best = cur;
            best_mu = mu;
            id_best = niter;
            if (iter != 1) g *= k_E;              
        }
        
        if (niter - id_best > d_S) {
            g *= k_S;
        }

        if (g > 2.5) g = 2;
        if (g < 1e-4) break;
        if (UB - cur < 1) break;
        update_multipliers(mu, cur, UB, sol_lb, g);
    }
    L1_fordward(best_mu);
    auto sol_lb = obtain_fordward_L1();
    auto lb = calc_cost_lb(sol_lb, best_mu);
    return tuple<vector<double>, double>(best_mu, lb);
}
**/


vector<int> new_pos[maxT];
void shrink(bool bck=1, bool repeat=0) {    
    ////cout << "init shrink" << endl;
    int m = 0;
    auto can_erase = [bck](Node* it) {
        if (it == root || it == broot) return 0;
        if (it->no[0][0] == -1) return 1;
        if (bck && it->no[1][0] == -1) return 1;
        if (it->outdeg == 0 || it->indeg == 0) return 1;
        return 0;
    };

    /** 
    queue<pair<int, int>> Q;
    for (int i=T+1; i>=0; --i) {
        for (int j=0; j<(int)at[i].size(); ++j) {
            if (can_erase(at[i][j])) {
                Q.push({i, j});
            }
        }
    }

    while (!Q.empty()) {
        auto [i, j] = Q.front(); Q.pop();
        auto q = at[i][j];
        if (q == nullptr) continue;
        int tk = i - p[q->i];
        for (int k=q->adj.next(0); ~k; k=q->adj.next(k+1)) {
            if (at[tk][k] != nullptr) {
                erase_edge(q, at[tk][k], k);
                if (can_erase(at[tk][k])) {
                    Q.push({tk, k});
                }
            } else {
                q->adj.unset(k);
                q->outdeg--;
            }
            m++;
        }
        delete at[i][j];
        at[i][j] = nullptr;
    }**/

   //double smu = accumulate(mu.begin(), mu.end(), 0.d);

    for (int i=T+1; i>=0; --i) {
        for (int j=0; j<(int)at[i].size(); ++j) {
            if (at[i][j] == nullptr) continue;
            if (can_erase(at[i][j])) {
                int tk = i - p[at[i][j]->i];
                for (int k=at[i][j]->adj.next(0); ~k; k=at[i][j]->adj.next(k+1)) {
                    if (at[tk][k] != nullptr) {
                        erase_edge(at[i][j], at[tk][k], k);
                    } else {
                        n_edges--;
                        at[i][j]->adj.unset(k);
                        at[i][j]->outdeg--;
                    }
                    m++;
                }
                delete at[i][j];
                at[i][j] = nullptr;
            }
        }
    }
    //cout << "debug3" << endl;


    for (int i=0; i<=T+1; ++i) {
        for (int j=0; j<(int)at[i].size(); ++j) {
            if (at[i][j] == nullptr) continue;
            if (can_erase(at[i][j])) {
                int tk = i - p[at[i][j]->i];
                for (int k=at[i][j]->adj.next(0); ~k; k=at[i][j]->adj.next(k+1)) {
                    if (at[tk][k] != nullptr) {
                        erase_edge(at[i][j], at[tk][k], k);
                    } else {
                        n_edges--;
                        at[i][j]->adj.unset(k);
                        at[i][j]->outdeg--;
                    }
                    m++;
                }
                delete at[i][j];
                at[i][j] = nullptr;
            }
        }
    }
    //cout << "debug4" << endl;

/** 
    for (int i=T+1; i>=0; --i) {
        for (int j=0; j<(int)at[i].size(); ++j) {
            if (at[i][j] == nullptr) continue;
            if (can_erase(at[i][j])) {
                int tk = i - p[at[i][j]->i];
                for (int k=at[i][j]->adj.next(0); ~k; k=at[i][j]->adj.next(k+1)) {
                    if (at[tk][k] != nullptr) {
                        erase_edge(at[i][j], at[tk][k], k);
                    } else {
                        at[i][j]->adj.unset(k);
                        at[i][j]->outdeg--;
                    }
                    m++;
                }
                delete at[i][j];
                at[i][j] = nullptr;
            }
        }
    }
    **/

    for (int i=0; i<=T+1; ++i) {
        new_pos[i].resize(at[i].size(), -1);
        int k = 0;
        for (int j=0; j<(int)at[i].size(); ++j) {
            if (at[i][j] != nullptr) {
                new_pos[i][j] = k;
                if (j != k) swap(at[i][j], at[i][k]);
                ++k;
            }
        }
        at[i].resize(k);
        if (i == 0) continue;
        for (int j=0; j<(int)at[i].size(); ++j) {
            if (at[i][j] == nullptr) continue;
            int tk = i - p[at[i][j]->i];
            for (int k=at[i][j]->adj.next(0); ~k; k=at[i][j]->adj.next(k+1)) {
                at[i][j]->adj.unset(k);
                if (new_pos[tk][k] != -1) {
                    at[i][j]->adj.set(new_pos[tk][k]);
                } else {
                    m++;
                    at[i][j]->outdeg--;
                    n_edges--;
                }
            }
            at[i][j]->adj.resize(at[tk].size());
        }
    }

    for (int i=0; i<=T+1; ++i) {
        new_pos[i].clear();
    }

    //cout << "debug5" << endl;
    ////cout << "erased edges: " << m << endl;
    if (repeat && m > 0) shrink(bck, repeat);
    //if (m > 0) shrink(bck, repeat);
}


int eval(int s[], const vector<int> id, int t) {
    int res = 0;
    for (int i : id) {
        t += p[s[i]];
        res += f(s[i], t);
    }
    return res;
}

int fr[5];
bool check_dominance(int s[], int t) {
    for (int i = 0; i < 4; ++i) fr[i] = s[i];
    vector<int> id(4);
    iota(id.begin(), id.end(), 0);
    int F = eval(fr, id, t);
    for (int i = 1; i <= 23; ++i) {
        next_permutation(id.begin(), id.end());
        int G = eval(fr, id, t);
        if (F > G) return 0;// || (k == 3 && (F == G && od[fr[k - 3]] > od[fr[id[k - 3]]]))) return 0;
    }
    return 1;

}

bool vis[maxN];
int seq[5];
void dfs_back(Node* it, int pos, int t, bool& dom, bool& n4) {
    if (dom) return;
    if (pos == -1) {
        n4 = 1;
        dom |= check_dominance(seq, t);
        return;
    }
    vis[it->i] = 1;
    for (int j=it->adj.next(0); ~j; j=it->adj.next(j+1)) {
        if (dom) break;
        auto jt = at[t][j];
        if (pos > 0 && t == (int32_t)p[jt->i]) continue;
        if (vis[jt->i]) continue;
        seq[pos] = jt->i;
        dfs_back(jt, pos-1, t - p[jt->i], dom, n4);
    }
    vis[it->i] = 0;
}


int fs = 0;
void four_sucessive_dominance() {
    //int mm = 0;

    /** 
    for (int t = T-1; t >= 1; --t) {
        for (auto it : at[t]) {
            vis[it->i] = 1;
            seq[0] = it->i;
            for (auto jt = it->adj[0].begin(); jt != it->adj[0].end();) {
                bool dom = 0;
                need4 = 0;
                seq[1] = jt->to->i;
                dfs(jt->to, 2, t + p[jt->to->i], dom, 0, 3);                                                                                                                                                                                                                                                                                                                                                            
                if (!dom && need4) {
                    mm++;
                    jt->to->adj[1].erase(jt->back);
                    jt = it->adj[0].erase(jt);
                }
                else { 
                    dom = 0;
                    need4 = 0;
                    dfs(jt->to, 2, t + p[jt->to->i], dom, 0, 4);
                    if (!dom && need4) {
                        mm++;
                        jt->to->adj[0].erase(jt->back);
                        jt = it->adj[1].erase(jt);
                    } else {
                        ++jt;
                    }
                }
            }
            vis[it->i] = 0;
        }
    }
    **/
    //shrink();
    int mm = 0;

    for (int t=1; t<=T; ++t) {
        for (auto it : at[t]) {
            if ((int)p[it->i] == t) continue;
            seq[3] = it->i;
            vis[it->i] = 1;
            for (int j=it->adj.next(0); ~j; j=it->adj.next(j+1)) {
                auto jt = at[t - p[it->i]][j];
                if ((int)(p[it->i] + p[jt->i]) == t) continue;
                seq[2] = jt->i;
                bool dom = 0, need4 = 0;
                dfs_back(jt, 1, t - p[it->i] - p[jt->i], dom, need4);
                if (!dom && need4) {
                    erase_edge(it, jt, j);
                    ++mm;
                } 
            }
            vis[it->i] = 0;
        }
    }

    fs++;
    
    
    ////cout << "erased edges fordward: " << mm << endl;

    shrink();
    //shrink();
}



void update(int id, Node* no, int i, double v) {
    if (v > 1e10) return;
    if (no->no[id][0] == -1) {
        no->no[id][0] = i;
        no->value[id][0] = v;
    } else if (no->no[id][0] == i) {
        no->value[id][0] = min(no->value[id][0], v);
    } else if (no->no[id][1] == -1) {
        no->no[id][1] = i;
        no->value[id][1] = v;
        if (no->value[id][0] > no->value[id][1]) {
            swap(no->no[id][0], no->no[id][1]);
            swap(no->value[id][0], no->value[id][1]);
        }   
    } else if (no->no[id][1] == i) {  
        no->value[id][1] = min(no->value[id][1], v);
        if (no->value[id][0] > no->value[id][1]) {
            swap(no->no[id][0], no->no[id][1]);
            swap(no->value[id][0], no->value[id][1]);
        }   
    } else if (no->value[id][0] >= v) {
        no->no[id][1] = no->no[id][0];
        no->value[id][1] = no->value[id][0];
        no->no[id][0] = i;
        no->value[id][0] = v;
    } else if (no->value[id][1] > v) {
        no->no[id][1] = i;
        no->value[id][1] = v;
    }
}


void L2_fordward(const vector<double>& mu) {
    for (auto& noi : at[0]) {
        noi->value[0][0] = noi->value[0][1] = 1e12;
        noi->no[0][0] = noi->no[0][1] = -1;
    }
    root->value[0][0] = 0;
    root->no[0][0] = 0;
    for (int t=1; t<=T+1; ++t) {
        if (!check_time(t)) continue;
        for (auto& noi : at[t]) {
            if (!check_node(t, noi->i)) continue;
            noi->value[0][0] = noi->value[0][1] = 1e12;
            noi->no[0][0] = noi->no[0][1] = -1;
            for (int j = noi->adj.next(0); ~j; j = noi->adj.next(j + 1)) {
                auto noj = at[t - p[noi->i]][j];
                //if (noj->no[0][noi->i == noj->no[0][0]] == -1) continue;
                update(0, noi, noj->i, noj->value[0][noi->i == noj->no[0][0]] + f(noi->i, t) - mu[noi->i]);
            }
        }
    }
    
    
    ////cout << "last node: " << broot->no[0][0] << endl;
    
    
    ////cout << accumulate(mu.begin(), mu.end(), 0.d) << endl;
    
    
    ////cout << "l2 fordward lb: " << broot->value[0][1] + accumulate(mu.begin(), mu.end(), 0.d) << endl;
    
    
    //cout << "l2 fordward lb: " << broot->value[0][0] + accumulate(mu.begin(), mu.end(), 0.) << endl;
}

void L2_backward(const vector<double>& mu) {
    for (int t=0; t<=T+1; ++t) {
        for (auto& noi : at[t]) {
            noi->value[1][0] = noi->value[1][1] = 1e12;
            noi->no[1][0] = noi->no[1][1] = -1;
        }
    }
    broot->value[1][0] = 0;
    broot->no[1][0] = n+1;
    for (int t=T+1; t>0; --t) {
        if (!check_time(t)) continue;
        for (auto& noi : at[t]) {
            if (!check_node(t, noi->i)) continue;
            for (int j = noi->adj.next(0); ~j; j = noi->adj.next(j + 1)) {
                //assert(noi->adj.at(j));
                auto noj = at[t - p[noi->i]][j];
                //if (noi->no[1][noj->i == noi->no[1][0]] == -1) continue; 
                update(1, noj, noi->i, noi->value[1][noj->i == noi->no[1][0]] + f(noi->i, t) - mu[noi->i]);
            }
        }
    }    
    
    
    ////cout << "last node: " << root->no[1][0] << endl;
    
    
    //cout << "l2 backward lb: " << root->value[1][0] + accumulate(mu.begin(), mu.end(), 0.) << endl;
}

vector<int> compute_L2_solution_fordward() {
    
    
    ////cout << "compute l2 sol" << endl;
    vector<int> s;
    int last = n+1;
    int cur = n+1;
    auto it = broot;
    int t = T;
    while (it != root) {
        int nxt = it->no[0][0];
        if (last == nxt) nxt = it->no[0][1];
        for (int j=it->adj.next(0); ~j; j=it->adj.next(j+1)) {
            auto jt = at[t][j];
            if (jt->i == nxt) {
                it = jt;
                assert(it->i == nxt);
                break;
            }
        }
        if (nxt == 0) break;
        s.push_back(nxt);
        t -= p[nxt];
        last = cur;
        cur = nxt;
    }
    
    
    ////cout << "finish" << endl;
    reverse(s.begin(), s.end());
    return s;
}

vector<int> compute_L2_solution_backward() {
    vector<int> s;
    int last = 0;
    int cur = 0;
    int ps = 0;
    auto it = root;
    int t = 0;
    while (it != broot) {
        int nxt = it->no[1][0];
        if (last == nxt) nxt = it->no[1][1];
        for (int i=0; i<(int)at[t+p[nxt]].size(); ++i) {
            auto jt = at[t + p[nxt]][i]; 
            if (jt->i == nxt && jt->adj.at(ps)) {
                it = jt;
                ps = i;
                break;
            }
        }
        if (nxt == (int)n+1) break;
        s.push_back(nxt);
        t += p[nxt];
        last = cur;
        cur = nxt;
    }
    return s;
}

void elimine_states_L2L2(const vector<double>& mu) {
    
    
    ////cout << "elimine states l2 l2" << endl;
    double smu = accumulate(mu.begin(), mu.end(), 0.);
    for (int t=0; t<=T+1; ++t) {
        for (const auto& it : at[t]) {
            if (it->no[0][0] == -1 || it->no[1][0] == -1) {
                for (int j = it->adj.next(0); ~j; j = it->adj.next(j + 1)) {
                    erase_edge(it, at[t - p[it->i]][j], j);
                }
                continue;
            }
              

            int k = 0;
            for (int j = it->adj.next(0); ~j; j = it->adj.next(j + 1)) {
                
                ++k;
                auto jt = at[t - p[it->i]][j];
                
                if (jt->no[0][0] == -1 || jt->no[1][0] == -1) {
                    erase_edge(it, jt, j);
                    k--;
                } else {
                    int l1 = jt->no[0][0] == it->i, r1 = it->no[1][0] == jt->i;
                    if (UB - 1 + EPS < jt->value[0][l1] + it->value[1][r1] + f(it->i, t) - mu[it->i] + smu) {
                        erase_edge(it, jt, j);
                        k--;
                    }
                }
            }
        }
    }

    shrink();
}


set<int> select_ids;
int n_sublimation = 0;
ll max_edges = 1e7;
bool sublimeL2(const vector<int>& lb_solution) {
    
    
    ////cout << "numero de sublimations: " << ++n_sublimation << endl;
    
    vector<int> new_ids;
    vector<int> cnt(n + 1);
    vector<int> occ(n + 1);
    for (int t=1; t<=T; ++t) {
        for (auto it : at[t]) {
            occ[it->i]++;
        }
    }

    if (n_edges <= (max_edges >> 6)) {
        for (int i=1; i<=n; ++i) if (!select_ids.count(i)) new_ids.emplace_back(i);
        sort(new_ids.begin(), new_ids.end(), [&](int i, int j) {
            return occ[i] < occ[j];
        });
        new_ids.resize(min((int)new_ids.size(), 3));
    } else {
        for (auto x : lb_solution) cnt[x]++;
        for (int i=1; i<=n; ++i) {
            if (select_ids.count(i)) continue;
            if (cnt[i] != 1) {
                new_ids.emplace_back(i);
            }
        }
        sort(new_ids.begin(), new_ids.end(), [&](int i, int j)->bool {
            if (cnt[i] == 0 && cnt[j] == 0) return occ[i] < occ[j];
            if (cnt[i] == 0) return 1;
            if (cnt[j] == 0) return 0;
            return occ[i] < occ[j];
        });

        int n_select = 1;
        if (n_edges <= (max_edges >> 3)) n_select = 3;
        else if (n_edges <= (max_edges >> 2)) n_select = 2;
        new_ids.resize(min((int)new_ids.size(), n_select));
    }

    if (new_ids.empty()) {
        opt_jobs = lb_solution;
        UB = calc_cost(lb_solution);
        print_ub();
        return false;
    }

    for (auto id : new_ids) {
        select_ids.insert(id);

        for (int t=0; t<=T+1; ++t) {
            int m = at[t].size();
            for (int j=0; j<m; ++j) {
                auto it = at[t][j];
                 
                Node* no = new Node(it->i);
                for (int i=0; i<2; ++i) 
                    for (int j=0; j<2; ++j) {
                        no->no[i][j] = it->no[i][j];
                        no->value[i][j] = it->value[i][j];
                    }   

                at[t].emplace_back(no);

                if (t == 0) continue;

                it->adj.resize(at[t - p[it->i]].size()); //CHANGE TO REDUCE MEMORY
                no->adj.resize(at[t - p[it->i]].size());
                if (it->i == id) {
                    for (int k = it->adj.next(0); ~k; k = it->adj.next(k + 1)) {
                        add_edge(no, at[t - p[id]][k], k);
                        erase_edge(it, at[t - p[id]][k], k);
                    }
                } else {
                    int q = at[t - p[it->i]].size() / 2;
                    for (int k = it->adj.next(0); ~k; k = it->adj.next(k + 1)) {
                        if (at[t - p[it->i]][k]->i == id) {
                            erase_edge(it, at[t - p[it->i]][k], k);
                        }
                        add_edge(no, at[t - p[it->i]][q + k], q + k);
                    }
                }
            }
        }
        broot = at[T+1].back();
    }

    shrink();
    shrink();
    return true;
}


void solve() {
    
    
    ////cout << "STAGE 1" << endl;
    //STAGE 1
    //set initial upper bound
    //auto upper_bound_1 = greedy_fordward();
    //auto upper_bound_2 = greedy_backward();
    //auto upper_bound_3 = shortest_processing_time_order();
    //opt_jobs = upper_bound_1;
    //if (calc_cost(opt_jobs) > calc_cost(upper_bound_2)) {
    //    opt_jobs = upper_bound_2;
    //}
    //if (calc_cost(opt_jobs) > calc_cost(upper_bound_3)) {
    //    opt_jobs = upper_bound_3;
    //}
    //print_ub();   
    //UB = calc_cost(opt_jobs);
    //cin>>UB;
    
    
    ////cout << "UB: " << UB << endl;  
    //auto dyna_sol = dynasearch(opt_jobs);    
    //auto dyna_UB = calc_cost(dyna_sol);

    //if (dyna_UB < UB) {
    //    opt_jobs = dyna_sol;
    //    UB = dyna_UB;
    //}

    
    ////cout << "FIRST UPPER BOUND: " << UB << endl;
    //print_ub();

    //auto [xmu, LB] = subgradient_L1();

    
    ////cout << "FIRST LOWER BOUND: " << LB << endl;

    //L1_fordward(mu);
    //auto sol_ford = obtain_fordward_L1();
    
    //dyna_sol = dynasearch(upper_bound_refine(sol_ford, 12, 1, 0));
    //dyna_UB = calc_cost(dyna_sol);

    //if (dyna_UB < UB) {
    //    opt_jobs = dyna_sol;
    //    UB = dyna_UB;
    //}


    
    
    ////cout << "SECOND UPPER BOUND: " << UB << endl;

    //if (UB - LB < 1) return; //cur solution in opt_jobs

    //L1_backward(mu);
    //auto sol_back = obtain_backward_L1();
    
    
    ////cout << "make graph" << endl;
    //make_and_solve_graph_L2(mu);
    
    
    ////cout << "finish" << ' ' << broot->no[0][0] << endl;
    //if (broot->no[0][0] == -1) {
    //    print_ub();
    //    exit(0);
    //}
    //STAGE 2
    
    
    ////cout << "STAGE 2" << endl;    

   // L2_fordward(mu);
    //if (broot->no[0][0] == -1) {
    //    print_ub();
    //    exit(0);
    //}
    //auto [mu2, LB2] = subgradient_L2(mu);

    
    
    ////cout << "SECOND LOWER BOUND 2: " << LB2 << endl;

    //cout << T << endl;
    //STAGE 3:
    auto mu2 = mu;

    ////cout << "STAGE 3" << endl;
    //cout << "debug0" << endl;
    L2_fordward(mu2);
    //cout << "debug1" << endl;
    L2_backward(mu2);
    //cout << "debug2" << endl;
    //cout << root->value[0][0] << ' ' << UB << endl;
    auto lb_sol = compute_L2_solution_fordward();
    //cout << "debug6" << endl;
    auto lb = calc_cost_lb(lb_sol, mu2);
    //cout << "debug7" << endl;
    elimine_states_L2L2(mu2); 
    //cout << "debug8" << endl;
    four_sucessive_dominance();
    //cout << "debug9" << endl;
    ////cout << "init four sucessive domunance" << endl;
    
    
    ////cout << endl;
    int iter = 1;
    while (true) {
        if (!sublimeL2(lb_sol)) {
                //cout << "found solution by lower bound" << endl;
                exit(0);
        }
        
        
        //cout << "current upper bound:" << UB << endl;
        four_sucessive_dominance();
        //cout << "finish 4 sucessive dom" << endl;
        vector<int> inner_sol;
        if (iter % 2 == 0) {
            L2_fordward(mu2);
            if (broot->no[0][0] == -1) {
                print_ub();
                exit(0);
            }
            inner_sol = compute_L2_solution_fordward();
        } else {
            L2_backward(mu2);
            if (root->no[1][0] == -1) {
                print_ub();
                exit(0);
            }
            inner_sol = compute_L2_solution_backward();
        }
        auto lb2 = calc_cost_lb(inner_sol, mu2);
        //cout << "current status: " << lb2 << ' ' << UB << endl;
        if (UB - lb2 + EPS < 1) break;        
        if (lb2 > lb || iter % 5 == 0) {
            auto ub_sol2 = dynasearch(upper_bound_refine(inner_sol, 12, iter % 2 == 0));   
            auto dyna_UB = calc_cost(ub_sol2);

            if (dyna_UB < UB) {
                opt_jobs = ub_sol2;
                UB = dyna_UB;
            }

            lb = max(lb, lb2);
            if (lb == lb2)
                lb_sol = inner_sol;
            cout << "edges: " << n_edges << " LB: " << lb << ' ' << "UB: " << UB << endl;
        }        
        if (UB - lb < 1) break;
        elimine_states_L2L2(mu2); 
        iter++;
    }

    //check memory occupation ratio M
}

};


/** 
int main() {
    
    
    ////cout.precision(10);
    
    
    ////cout<<fixed;
    cin>>n;
    vector<tuple<int, int, int>> ord;
    p.resize(n + 2);
    d.resize(n + 2);
    w.resize(n + 2) ;
    for (int i=1; i<=n; ++i) {
        //cin>>p[i]>>d[i]>>ha[i]>>w[i];
        cin>>p[i]>>d[i]>>w[i];
        T += p[i];
        ord.emplace_back(d[i], p[i], i);
    }
    sort(ord.begin(), ord.end());
    for (int i=0; i<n; ++i) {
        auto [a, b, c] = ord[i];
        od[c] = i;
    }
    od[0] = -1;
    od[n+1] = n;
    
    
    ////cout << "cur T: " << T << endl;
    p[0] = 0;
    p[n+1] = 1;
    //cin>>UB;
    solve();
    
    
    ////cout  << "solution: ";
    print_ub();
    return 0;
}
**/