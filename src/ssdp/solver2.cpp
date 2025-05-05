#pragma GCC optimize("Ofast")
// #pragma GCC target("avx2")
#include <bits/stdc++.h>
using namespace std;
#define double float

const int maxN = 202;
const int maxT = 10105;
int n;
int w[maxN], ha[maxN];
int p[maxN], d[maxN];
using ll = long long;
int UB;
vector<int> UB_sol;
int nids = 0;
int pt[310];
int T;
struct Node
{
    int i;
    Node() {}
    Node(int i) : i(i)
    {
        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 2; ++j)
            {
                value[i][j] = 0;
                no[i][j] = -1;
            }
        }
    }
    double value[2][2];
    int no[2][2];
    list<Node *> adj[2];
    Node *up;
};
list<Node *> at[maxT];
Node *root;
Node *broot;
vector<int> ids;
double h1[maxN][maxT];
double H1[maxN][maxT];

int f(int i, int T)
{
    return (i >= 1 && i <= n && T > d[i]) ? w[i] * (T - d[i]) : 0;
    // return ha[i] * max(d[i] - T, 0) + w[i] * max(T - d[i], 0);
}

int calc_cost(const vector<int> &sol)
{
    int t = 0;
    int res = 0;
    for (int i : sol)
    {
        t += p[i];
        res += f(i, t);
    }
    return res;
}

void print_ub()
{cout << calc_cost(UB_sol) << endl;
    for (int i : UB_sol) {
        cout << i << ' ';
    }
    cout << endl;
}

vector<int> greedy_fordward()
{
    set<int> next_elements;
    for (int i = 1; i <= n; ++i)
    {
        next_elements.emplace(i);
    }
    vector<int> sol;
    int t = 0;
    for (int i = 0; i < n; ++i)
    {
        int idx = -1;
        for (int j : next_elements)
        {
            if (idx == -1 || f(idx, t + p[idx]) > f(j, t + p[j]))
            {
                idx = j;
            }
        }
        sol.emplace_back(idx);
        t += p[idx];
        next_elements.erase(idx);
    }
    return sol;
}

vector<int> greedy_backward()
{
    set<int> next_elements;
    for (int i = 1; i <= n; ++i)
    {
        next_elements.emplace(i);
    }
    vector<int> sol;
    int t = T;
    for (int i = 0; i < n; ++i)
    {
        int idx = -1;
        for (int j : next_elements)
        {
            if (idx == -1 || f(idx, t) > f(j, t))
            {
                idx = j;
            }
        }
        sol.emplace_back(idx);
        t -= p[idx];
        next_elements.erase(idx);
    }
    reverse(sol.begin(), sol.end());
    return sol;
}

vector<int> shortest_processing_time_order()
{
    vector<int> id(n);
    iota(id.begin(), id.end(), 1);
    sort(id.begin(), id.end(), [](int i, int j)
         { return d[i] < d[j] || (d[i] == d[j] && (p[i] < p[j] || (p[i] == p[j] && i < j))); });
    return id;
}

int dp[maxN][3];
vector<int> dynasearch(vector<int> order, int lw = 0, int nit = 30)
{

    for (int s = 0; s < nit; ++s)
    {

        //vector<vector<int>> dp(n + 1, vector<int>(3));
        int i, j, k;
        int vv;
        int cx;

        vector<int> c(n);
        int t = 0;
        for (int i = 0; i < n; ++i)
        {
            t += p[order[i]];
            c[i] = t;
        }

        if (lw == 0)
        {
            dp[0][0] = 0;  // f
            dp[0][1] = -1; // j
            dp[0][2] = 0;  // ty
        }

        vector<int> job(n);
        for (i = lw + 1; i <= n; i++)
        {
            dp[i][1] = i - 1;                                    // j
            dp[i][0] = dp[i - 1][0] + f(order[i - 1], c[i - 1]); // f
            dp[i][2] = 0;                                        // ty

            // PI
            for (j = 0; j <= i - 2; j++)
            {
                if (j > 0)
                {
                    cx = c[j - 1] + p[order[i - 1]];
                }
                else
                {
                    cx = p[order[i - 1]];
                }
                vv = dp[j][0] + f(order[i - 1], cx);
                for (k = j + 1; k <= i - 2; k++)
                {
                    cx += p[order[k]];
                    vv += f(order[k], cx);
                }
                cx += p[order[j]];
                vv += f(order[j], cx);

                if (vv < dp[i][0])
                {
                    dp[i][0] = vv; // f
                    dp[i][1] = j;  // j
                    dp[i][2] = 1;  // ty
                }
            }

            // EBSR
            for (j = 0; j <= i - 3; j++)
            {
                if (j > 0)
                {
                    cx = c[j - 1] + p[order[i - 1]];
                }
                else
                {
                    cx = p[order[i - 1]];
                }
                vv = dp[j][0] + f(order[i - 1], cx);
                for (k = j; k <= i - 2; k++)
                {
                    cx += p[order[k]];
                    vv += f(order[k], cx);
                }

                if (vv < dp[i][0])
                {
                    dp[i][0] = vv; // f
                    dp[i][1] = j;  // j
                    dp[i][2] = 2;  // ty
                }
            }

            // EFSR
            for (j = 0; j <= i - 3; j++)
            {
                if (j > 0)
                {
                    cx = c[j - 1];
                }
                else
                {
                    cx = 0;
                }
                vv = dp[j][0]; // f

                for (k = j + 1; k <= i - 1; k++)
                {
                    cx += p[order[k]];
                    vv += f(order[k], cx);
                }
                cx += p[order[j]];
                vv += f(order[j], cx);

                if (vv < dp[i][0])
                {
                    dp[i][0] = vv; // f
                    dp[i][1] = j;  // j
                    dp[i][2] = 3;  // ty
                }
            }
        }

        for (i = n, j = n - 1; i > 0;)
        {
            switch (dp[i][2])
            {
            case 0:
                job[j--] = order[dp[i][1]];
                break;
            case 1: // PI
                job[j--] = order[dp[i][1]];
                for (k = i - 2; j > dp[i][1]; j--, k--)
                {
                    job[j] = order[k];
                }
                job[j--] = order[i - 1];
                break;
            case 2: // EBSR
                for (k = i - 2; j > dp[i][1]; j--, k--)
                {
                    job[j] = order[k];
                }
                job[j--] = order[i - 1];
                break;
            case 3: // EFSR
                job[j--] = order[dp[i][1]];
                for (k = i - 1; j >= dp[i][1]; j--, k--)
                {
                    job[j] = order[k];
                }
                break;
            }

            i = dp[i][1];
        }

        if (calc_cost(job) == calc_cost(order)) break;
        order = job;
    }

    // for (lw = 0; lw < sol->n; lw++) {
    //     if (job[lw]->no != sol->job[lw]->no) {
    //         break;
    //     }
    // }
    // return lw;
    return order;
}

vector<int> greedy_sp() {
    vector<int> s;
    int t = T;
    vector<bool> vis(n + 1);
    for (int i = 0; i < n; ++i) {
        int idx = -1;
        for (int i = 1; i <= n; ++i) {
            if (vis[i])
                continue;
            if (idx == -1 || h1[i][t] < h1[idx][t]) {
                idx = i;
            }
        }
        s.emplace_back(idx);
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
    int l = 0;
    for (int i = 1; i <= n; ++i)
    {
        double val = 1e9;
        for (auto jt : at[t])
        {
            if (jt->no[0][0] == -1)
                continue;
            if (vis[jt->i])
                continue;
            if (jt->value[0][0] < val)
            {
                val = jt->value[0][0];
                v = jt->i;
            }
        }
        if (val > 1e8) {
            for (l = 0; l < n; ++l) {
               if (vis[id[l]])
                    continue;

                v = id[l];
                break;
            }
        }
        vis[v] = 1;
        s.emplace_back(v);
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
        double val = 1e9;
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
        if (val > 1e8 || v == n+1) {
            for (int i = 0; i < n; ++i) {
               if (vis[id[i]])
                    continue;
                v = id[i];
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


vector<int> upper_bound_refine(vector<int> &quasi_solution, int lim = 12, bool ford = 1, bool L2 = 1)
{ // FORDWARD

    // elements 1, 2, ..., n
    vector<int> order_; // order from left
    vector<int> cnt(n); // count contribution
    if (!ford)
    {
        reverse(quasi_solution.begin(), quasi_solution.end());
    }
    for (auto s_i : quasi_solution)
    {
        // assert(s_i != 0);
        if (cnt[s_i - 1] == 0)
        {
            order_.emplace_back(s_i - 1);
        }
        cnt[s_i - 1] += 1;
    }
    if (!ford)
    {
        reverse(order_.begin(), order_.end());
        reverse(quasi_solution.begin(), quasi_solution.end());
    }

    // 0-base from here
    int N1 = order_.size();
    int N2 = n - N1;

    if (N2 == 0)
    {
        cout << "Upper bound: " << UB << endl; 
        cout << "n2 found solution " << endl;
        UB_sol = quasi_solution;
        print_ub();
        exit(0);
    }

    if (N2 <= lim)
    { // dynamic programming process
        cout << "limits: " << N1 << ", " << N2 << endl;
        auto fixed_item = order_;
        vector<int> free_item;
        for (int i = 0; i < n; ++i)
        {
            if (cnt[i] == 0)
            {
                free_item.emplace_back(i);
            }
        }
        vector<int> sum_subset(1 << N2);
        for (int s = 1; s < (1 << N2); ++s)
        {
            int first_idx = __builtin_ctz(s & -s);
            sum_subset[s] = sum_subset[s ^ (1 << first_idx)] + p[free_item[first_idx]];
        }
        vector<int> sum_prefix(N1 + 1);
        for (int i = 1; i <= N1; ++i)
        {
            sum_prefix[i] = sum_prefix[i - 1] + p[fixed_item[i - 1]];
        }
        vector<vector<int>> dp(N1 + 1, vector<int>(1 << N2, 1e9));
        vector<vector<short>> tr(N1 + 1, vector<short>(1 << N2, -1));
        dp[0][0] = 0;
        for (int i = 1; i <= N1; ++i)
        {
            int T = sum_prefix[i];
            if (dp[i][0] > dp[i - 1][0] + f(fixed_item[i - 1], T))
            {
                dp[i][0] = dp[i - 1][0] + f(fixed_item[i - 1], T);
                tr[i][0] = -i;
            }
        }
        for (int s = 1; s < (1 << N2); ++s)
        {
            int T = sum_subset[s];
            int first_idx = 31 - __builtin_clz(s & -s);
            int temp = dp[0][s ^ (1 << first_idx)] + f(free_item[first_idx], T);
            if (dp[0][s] > temp)
            {
                dp[0][s] = temp;
                tr[0][s] = first_idx;
            }
        }
        for (int i = 1; i <= N1; ++i)
            for (int s = 1; s < (1 << N2); ++s)
            {
                int T = sum_subset[s] + sum_prefix[i];
                int temp = dp[i - 1][s] + f(fixed_item[i - 1], T);
                if (dp[i][s] > temp)
                {
                    dp[i][s] = temp;
                    tr[i][s] = -i;
                }
                for (int j = 0; j < N2; ++j)
                {
                    if (s & (1 << j))
                    {
                        temp = dp[i][s ^ (1 << j)] + f(free_item[j], T);
                        if (dp[i][s] > temp)
                        {
                            dp[i][s] = temp;
                            tr[i][s] = j;
                        }
                    }
                }
            }
        int i = N1, s = (1 << N2) - 1;
        vector<int> ub_solution;
        while (i != 0 || s != 0)
        {
            int r = tr[i][s];
            if (r < 0)
            {
                ub_solution.emplace_back(fixed_item[-r - 1] + 1);
                i -= 1;
            }
            else
            {
                ub_solution.emplace_back(free_item[r] + 1);
                s ^= (1 << r);
            }
        }
        return ub_solution;
    }
    auto sol = L2 ? (ford ? greedy_sp_l2() : greedy_sp_l2_backward()): greedy_sp();
    return sol;
}

vector<pair<int, int>> best(maxT);
void L1_fordward(const vector<double> &mu) {
    for (int i=1; i<=n+1; ++i) h1[i][0] = 1e9;
    h1[0][0] = 0.;
    fill(best.begin(), best.end(), make_pair(-1, -1));
    best[0] = {0, -1};
    for (int t = 1; t <= T; ++t) {
        for (int i = 1; i <= n; ++i) {
            h1[i][t] = 1e9;
            if (p[i] > t)
                continue;
            auto [s, r] = best[t - p[i]];

            if (s == -1) {
                continue;
            } else if (s != i) {
                h1[i][t] = h1[s][t - p[i]] + f(i, t) - mu[i];
            } else if (r != -1) {
                h1[i][t] = h1[r][t - p[i]] + f(i, t) - mu[i];
            } else {
                continue;
            }

            auto &[u, v] = best[t];

            if (u == -1 || h1[u][t] >= h1[i][t]) {
                v = u;
                u = i;
            } else if (v == -1 || h1[v][t] > h1[i][t]) {
                v = i;
            }
        }
    }
}

int sL1[maxT];
int nL1;
void obtain_fordward_L1() {
    int t = T;
    int last_idx = -1;
    nL1 = 0;
    while (t != 0) {
        int idx = (best[t].first == last_idx) ? best[t].second : best[t].first;
        t -= p[idx];
        sL1[nL1++] = idx;
        last_idx = idx;
    }    
    reverse(sL1, sL1+nL1); 
}

void L1_backward(const vector<double> &mu) {
    for (int i=0; i<=n; ++i) H1[i][T+1] = 1e9;
    H1[n + 1][T + 1] = 0.;
    fill(best.begin(), best.end(), make_pair(-1, -1));
    best[T] = {n + 1, -1};
    for (int t = T; t >= 0; --t) {
        for (int i = 1; i <= n; ++i) {
            H1[i][t] = 1e9;
            if (p[i] > t)
                continue;
            auto [s, r] = best[t];
            if (s == -1)
                continue;
            if (s != i) {
                H1[i][t] = H1[s][t + p[s]] + f(i, t) - mu[i];
            }
            else if (r == -1) {
                continue;
            }
            else {
                H1[i][t] = H1[r][t + p[r]] + f(i, t) - mu[i];
            }
            auto &[u, v] = best[t - p[i]];
            if (u == -1)
            {
                u = i;
            }
            else if (H1[u][t - p[i] + p[u]] >= H1[i][t])
            {
                v = u;
                u = i;
            }
            else if (v == -1 || H1[v][t - p[i] + p[v]] > H1[i][t])
            {
                v = i;
            }
        }
    }

    H1[0][0] = H1[best[0].first][p[best[0].first]];
}

vector<int> obtain_backward_L1() {
    int t = 0;
    vector<int> s;
    int last_idx = -1;
    while (t < T) {
        int idx = best[t].first;
        if (idx == last_idx) idx = best[t].second;
        t += p[idx];
        s.emplace_back(idx);
        last_idx = idx;
    }
    return s;
}

double calc_cost_lb(const vector<int> &s, const vector<double> &mu) {
    double sum = 0;
    int t = 0;
    for (auto i : s) {
        t += p[i];
        sum += f(i, t) - mu[i];
    }
    for (int i = 1; i <= n; ++i) {
        sum += mu[i];
    }
    return sum;
}

void update_multipliers(vector<double> &mu, double LB, double UB, const vector<int> &dual_solution, double g)
{
    vector<int> cnt_elem(n + 1);
    for (int i : dual_solution) {
        cnt_elem[i]++;
    }
    int sqr_from_1 = 0;
    for (int i = 1; i <= n; ++i) {
        sqr_from_1 += (1 - cnt_elem[i]) * (1 - cnt_elem[i]);
    }
    if (sqr_from_1 == 0)
    {
        cout << "mu found solution: " << endl;
        UB_sol = dual_solution;
        print_ub();
        exit(0);
    }

    for (int i = 1; i <= n; ++i)
    {
        if (cnt_elem[i] != 1)
        {
            mu[i] += g * (1 - cnt_elem[i]) * (UB - LB) / sqr_from_1;
        }
    }
}

// gamma_init, delta_T, delta_S, eps, k_S, k_E
tuple<vector<double>, double> subgradient_L1(double g_0 = 2, int d_T = n / 4, int d_S = 4, double eps = 0.002, double k_S = 0.75, double k_E = 2)
{
    vector<double> mu(n + 2, 0.);
    auto best_mu = mu;
    vector<double> G(d_T + 1);
    double bst = 0;
    double cur = 0;
    int iter = 0;
    int id_best = 0;
    double g = g_0;
    int niter = 0;
    double fi = 0;
    double smu = 0;
    int tree = 20;
    while (++iter) {
        smu = accumulate(mu.begin(), mu.end(), 0.d);
        L1_fordward(mu);
        obtain_fordward_L1();
        cur = h1[best[T].first][T] + smu;
        
        if (iter == 1)
            fi = cur;

        G[iter % (d_T + 1)] = max(G[(iter + d_T) % (d_T + 1)], cur);

        if (bst > fi && iter > d_T * 20 + 15 && G[iter % (d_T + 1)] * (1 - eps) < G[(iter + d_T) % (d_T + 1)] && (G[iter % (d_T + 1)] - G[(iter + d_T) % (d_T + 1)]) < (UB - G[(iter + d_T) % (d_T + 1)]) * eps)
        {
            // niter++;
            // if (niter == 5 * n)
            break;
        }
        else
        {
            niter = 0;
        }

        if (iter > 1 && cur - bst > 0) { // 1e-4) {
            bst = cur;
            best_mu = mu;
            id_best = iter;
            if (iter != 1)
                g *= k_E;
        }

        if (iter - id_best > d_S) {
            g *= k_S;
        }

        if (g > 2.5)
            g = 2;
        if (g < 1e-5) {
            if (tree-- == 0) break;
            g = 0.5;
            //break;
        }
        if (UB - cur < 1)
            break;

        //update_multipliers(mu, cur, UB, sol_lb, g);

        vector<int> cnt_elem(n + 1);
        for (int i = 0; i<nL1; ++i) {
            cnt_elem[sL1[i]]++;
        }
        int sqr_from_1 = 0;
        for (int i = 1; i <= n; ++i) {
            sqr_from_1 += (1 - cnt_elem[i]) * (1 - cnt_elem[i]);
        }

        if (sqr_from_1 == 0) {
            cout << "mu found solution: " << endl;
            //UB_sol = dual_solution;
            UB_sol.clear();
            for (int i=0; i<n; ++i) UB_sol.emplace_back(sL1[i]);
            print_ub();
            exit(0);
        }

        for (int i = 1; i <= n; ++i) {
            if (cnt_elem[i] != 1) {
                mu[i] += g * (1 - cnt_elem[i]) * (UB - ceil(cur)) / sqr_from_1;
            }
        }
    }
    L1_fordward(best_mu);
    //auto sol_lb = obtain_fordward_L1();
    auto lb = h1[best[T].first][T] + accumulate(best_mu.begin(), best_mu.end(), 0.d);//calc_cost_lb(sol_lb, best_mu);
    return tuple<vector<double>, double>(best_mu, lb);
}

void make_graph_L2(const vector<double> &mu)
{
    root = new Node(0);
    at[0].emplace_back(root);
    double smu = accumulate(mu.begin(), mu.end(), 0.d);
    for (int t = 1; t <= T; ++t)
    {
        for (int i = 1; i <= n; ++i)
        {
            if (p[i] > t)
                continue;
            if (UB - 1 < h1[i][t] + H1[i][t] - (f(i, t) - mu[i]) + smu) {
                continue;
            }
            at[t].emplace_back(new Node(i));
            for (auto jt = at[t - p[i]].begin(); jt != at[t - p[i]].end(); ++jt)
            {
                if ((*jt)->i == i)
                    continue;
                if (p[(*jt)->i] + p[i] > t)
                    continue;

                int dji = f((*jt)->i, t - p[i]) + f(i, t);
                int dij = f(i, t - p[(*jt)->i]) + f((*jt)->i, t);
                auto tji = make_tuple(dji, d[(*jt)->i], p[(*jt)->i], (*jt)->i);
                auto tij = make_tuple(dij, d[i], p[i], i);
                if (tji < tij)
                {
                    at[t].back()->adj[1].emplace_back(*jt);
                    (*jt)->adj[0].emplace_back(at[t].back());
                }
            }
        }
    }
    broot = new Node(n + 1);
    at[T + 1].emplace_back(broot);
    for (auto it = at[T].begin(); it != at[T].end(); ++it)
    {
        broot->adj[1].emplace_back(*it);
        (*it)->adj[0].emplace_back(broot);
    }
}

void elimine_states_L1L2(int UB, const vector<double> &mu)
{
    //return;
    double smu = accumulate(mu.begin(), mu.end(), 0.d);
    //
    //for (int t=0; t<=T; ++t) {
    //    for (auto& it : at[t]) {
    //        if (it->no[0][0] == -1) {
    //            it->i = -1;
    //        }
    //    }
   //}
    for (int t = 0; t <= T; ++t)
    {
        for (auto it = at[t].begin(); it != at[t].end(); ++it)
        {   
            //if ((*it)->i == -1) continue;
            for (auto jt = (*it)->adj[0].begin(); jt != (*it)->adj[0].end();)
            {
                // i -> j
                int x = (*it)->no[0][0] == (*jt)->i;
                //if ((*jt)->i == -1)
                //{
                //    jt = (*it)->adj[0].erase(jt);
                //} else 
                if ((*it)->no[0][x] == -1){
                    ++jt;
                    //jt = (*it)->adj[0].erase(jt);
                }
                else if (UB - 1 < (*it)->value[0][x] + H1[(*jt)->i][t + p[(*jt)->i]] + smu)
                {
                    jt = (*it)->adj[0].erase(jt);
                }
                else
                {
                    ++jt;
                }
            }
        }
    }
    for (int t = T + 1; t > 0; --t)
    {
        for (auto it = at[t].begin(); it != at[t].end(); ++it)
        {
            //if ((*it)->i == -1) continue;
            for (auto jt = (*it)->adj[1].begin(); jt != (*it)->adj[1].end();)
            {
                // j <- i
                int x = (*jt)->no[0][0] == (*it)->i;
                //if ((*jt)->i == -1)
                //{
                //    jt = (*it)->adj[1].erase(jt);
                //} else 
                if ((*jt)->no[0][x] == -1) {
                //    jt = (*it)->adj[0].erase(jt);
                    ++jt;
                }
                else if (UB - 1 < (*jt)->value[0][x] + H1[(*it)->i][t] + smu)
                {
                    jt = (*it)->adj[1].erase(jt);
                }
                else
                {
                    ++jt;
                }
            }
        }
    }
 /** 
    for (int t=0; t<=T+1; ++t) {
        for (auto it = at[t].begin(); it != at[t].end(); ) {
            if ((*it)->i == -1) {
                delete *it;
                *it = nullptr;
                it = at[t].erase(it);
            }
            else ++it;
        }
    }
    **/
}

void update(int id, Node *no, int i, double v)
{
    if (no->no[id][0] == -1)
    {
        no->no[id][0] = i;
        no->value[id][0] = v;
    }
    else if (no->no[id][0] == i)
    {
        no->value[id][0] = min(no->value[id][0], v);
    }
    else if (no->no[id][1] == -1)
    {
        no->no[id][1] = i;
        no->value[id][1] = v;
        if (no->value[id][0] > no->value[id][1])
        {
            swap(no->no[id][0], no->no[id][1]);
            swap(no->value[id][0], no->value[id][1]);
        }
    }
    else if (no->no[id][1] == i)
    {
        no->value[id][1] = min(no->value[id][1], v);
        if (no->value[id][0] > no->value[id][1])
        {
            swap(no->no[id][0], no->no[id][1]);
            swap(no->value[id][0], no->value[id][1]);
        }
    }
    else if (no->value[id][0] >= v)
    {
        no->no[id][1] = no->no[id][0];
        no->value[id][1] = no->value[id][0];
        no->no[id][0] = i;
        no->value[id][0] = v;
    }
    else if (no->value[id][1] > v)
    {
        no->no[id][1] = i;
        no->value[id][1] = v;
    }
}

void L2_fordward(const vector<double> &mu)  {
    cout << "fordward" << endl;
    for (auto &noi : at[0])
    {
        noi->value[0][0] = noi->value[0][1] = 0;
        noi->no[0][0] = noi->no[0][1] = -1;
    }
    root->value[0][0] = 0;
    root->no[0][0] = 0;
    for (int t = 1; t <= T + 1; ++t)
    {
        for (auto &noi : at[t])
        {
            noi->value[0][0] = noi->value[0][1] = 0;
            noi->no[0][0] = noi->no[0][1] = -1;
            for (const auto &noj : noi->adj[1])
            {
                for (int k = 0; k < 2; ++k)
                {
                    if (noj->no[0][k] == -1)
                        break;
                    if (noi->i != noj->no[0][k])
                    {
                        update(0, noi, noj->i, noj->value[0][k] + f(noi->i, t) - mu[noi->i]);
                        break;
                    }
                }
            }
        }
    }
    if (broot->no[0][0] == -1) {
        cout << "not paths, find solution by Upper Bound - 1" << endl;
        print_ub();
        exit(0);
    }
    cout << "finish: " << broot->value[0][0] + accumulate(mu.begin(), mu.end(), 0.d) << endl;
}

void L2_backward(const vector<double> &mu)
{
    cout << "backward" << endl;
    for (auto &noi : at[T + 1])
    {
        noi->value[1][0] = noi->value[1][1] = 0;
        noi->no[1][0] = noi->no[1][1] = -1;
    }
    broot->value[1][0] = 0;
    broot->no[1][0] = n + 1;
    for (int t = T; t >= 0; --t)
    {
        for (auto &noi : at[t])
        {
            noi->value[1][0] = noi->value[1][1] = 0;
            noi->no[1][0] = noi->no[1][1] = -1;
            for (const auto &noj : noi->adj[0])
            {
                for (int k = 0; k < 2; ++k)
                {
                    if (noj->no[1][k] == -1)
                        break;
                    if (noi->i != noj->no[1][k])
                    {
                        update(1, noi, noj->i, noj->value[1][k] + f(noj->i, t + p[noj->i]) - mu[noj->i]);
                        break;
                    }
                }
            }
        }
    }
    cout << "finish: " << root->value[1][0] + accumulate(mu.begin(), mu.end(), 0.d) << endl;
}

vector<int> compute_L2_solution_fordward()
{
    vector<int> s;
    int last = n + 1;
    int cur = n + 1;
    auto it = broot;
    int t = T;
    double tt = broot->value[0][0];
    while (it != root)
    {
        int nxt = it->no[0][0];
        if (last == nxt)
            nxt = it->no[0][1];

        int ok = 0;
        for (auto jt : it->adj[1])
        {
            if (jt->i == nxt)
            {
                ok = 1;
                it = jt;
                break;
            }
        }
        if (!ok) {
            print_ub();
            exit(0);
        }
        if (nxt <= 0)
            break;
        s.push_back(nxt);
        t -= p[nxt];
        last = cur;
        cur = nxt;
    }
    reverse(s.begin(), s.end());
    return s;
}

vector<int> compute_L2_solution_backward() {
    vector<int> s;
    int last = 0;
    int cur = 0;
    auto it = root;
    int t = 0;
    while (it != broot) {
        int nxt = it->no[1][0];
        if (last == nxt)
            nxt = it->no[1][1];
        bool ok = 0;
        for (auto jt : it->adj[0]) {
            if (jt->i == nxt) {
                ok = 1;
                it = jt;
                break;
            }
        }
        if (!ok) {
            print_ub();
            exit(0);
        }
        if (nxt == n + 1)
            break;
        s.push_back(nxt);
        t += p[nxt];
        last = cur;
        cur = nxt;
    }
    return s;
}

void elimine_states_L2L2(int UB, const vector<double> &mu, int st=3)
{


    double smu = accumulate(mu.begin(), mu.end(), 0.f);
    for (int t = 1; t <= T + 1; ++t)
    {
        for (auto &it : at[t])
        {
            for (auto jt = it->adj[1].begin(); jt != it->adj[1].end();)
            {
                int l1 = (*jt)->no[0][0] == it->i, r1 = it->no[1][0] == (*jt)->i;
                if ((*jt)->no[0][l1] == -1 || it->no[1][r1] == -1) {
                   
                   ++jt;
                   // jt = it->adj[1].erase(jt);
                }
                else if (UB - 1 < (*jt)->value[0][l1] + it->value[1][r1] + f(it->i, t) - mu[it->i] + smu)
                {
                    jt = it->adj[1].erase(jt);
                }
                else
                {
                    ++jt;
                }
            }
        }
    }
    for (int t = 0; t <= T; ++t)
    {
        for (auto &it : at[t])
        {
            for (auto jt = it->adj[0].begin(); jt != it->adj[0].end();)
            {
                int l1 = it->no[0][0] == (*jt)->i, r1 = (*jt)->no[1][0] == it->i;
                if (it->no[0][l1] == -1 || (*jt)->no[1][r1] == -1) { 
                //    jt = it->adj[0].erase(jt);   
                    ++jt;
                }
                else if (UB - 1 < it->value[0][l1] + (*jt)->value[1][r1] + f((*jt)->i, t + p[(*jt)->i]) - mu[(*jt)->i] + smu)
                {
                    jt = it->adj[0].erase(jt);
                }
                else
                {
                    ++jt;
                }
            }
        }
    }

    for (int t = 0; t <= T+1; ++t) {
        for (auto it = at[t].begin(); it != at[t].end(); ++it) {
            if ((*it)->no[0][0] == -1 && (*it)->no[1][0] == -1 && st == 3) {
                //(*it)->adj[0].clear();
                //(*it)->adj[1].clear();
            }
            if ((*it)->adj[0].empty() && (*it)->adj[1].empty()) {
                (*it)->i = -1;
            }
        }
    }

    for (int t = 0; t <= T + 1; ++t)
    {
        for (auto &it : at[t])
        {
            for (int k = 0; k < 2; ++k)
            {
                for (auto jt = it->adj[k].begin(); jt != it->adj[k].end();)
                {
                    if ((*jt)->i == -1) {
                        jt = it->adj[k].erase(jt);
                    }
                    else
                    {
                        ++jt;
                    }
                }
            }
        }
    }

    for (int t = 0; t <= T + 1; ++t)
    {
        for (auto it = at[t].begin(); it != at[t].end();)
        {
            if ((*it)->i == -1) {
                delete *it;
                *it = nullptr;
                it = at[t].erase(it);
            } else {
                ++it;
            }
        }
    }



}

tuple<vector<double>, double> subgradient_L2(vector<double> &mu, double g_0 = 2, int d_T = n / 4, int d_S = 4, double eps = 0.002, double k_S = 0.8, double k_E = 2)
{
    auto best_mu = mu;
    L2_fordward(mu);
    auto s = compute_L2_solution_fordward();
    double cur = calc_cost_lb(s, mu);
    double best = 0;
    int iter = 0;
    int id_best = 0;
    int niter = 0;
    double g = g_0;
    vector<double> G(d_T + 1);
    G[0] = cur;
    double fi = 0;
    int tree = 10;
    int itt=0;
    while (++iter)
    {
        double smu = accumulate(mu.begin(), mu.end(), 0.);
        L2_fordward(mu);
        if (iter == 1) {
            elimine_states_L1L2(UB, mu);
        }

        L2_fordward(mu);
        auto sol_lb = compute_L2_solution_fordward();
        cur = broot->value[0][0] + smu;

        int UB2 = UB;
        vector<int> s = UB_sol;
        if (iter % 5 == 0)
        {
            s = dynasearch(upper_bound_refine(sol_lb, 15));
            UB2 = calc_cost(s);
        }

        if (cur > best || UB2 < UB) {
            if (UB2 == UB) {
                if (itt++ % 2 == 0) {
                    L2_backward(mu);
                    sol_lb = compute_L2_solution_backward();   
                }

                s = dynasearch(upper_bound_refine(sol_lb, 15, 0));
                UB2 = calc_cost(s);
                cout << "try to update UB: " << UB2 << endl;
            }
            if (UB > UB2) {
                UB = UB2;
                UB_sol = s;
            }
            L2_backward(mu);
            elimine_states_L2L2(UB, mu);
            cur = max(cur, root->value[1][0] + smu);
        }

        if (iter == 1)
            fi = cur;

        G[iter % (d_T + 1)] = max(G[(iter + d_T) % (d_T + 1)], cur);

        if (best > fi && iter > 5 * d_T && G[iter % (d_T + 1)] * (1 - eps) < G[(iter + d_T) % (d_T + 1)] && (G[iter % (d_T + 1)] - G[(iter + d_T) % (d_T + 1)]) < (UB - G[(iter + d_T) % (d_T + 1)]) * eps)
        {
            break;
        }
        else
        {
            niter = 0;
        }

        if (UB - cur < 1)
        {
            cout << cur << ' ' << UB << endl;
            cout << "**found solution: " << endl;
            //sort(UB_sol.begin(), UB_sol.end());
            print_ub();
            exit(0);
        }

        if (iter > 1 && cur - best > 0) { // 1e-4) {
            best = cur;
            best_mu = mu;
            id_best = iter;
            if (iter != 1)
                g *= k_E; 
        }

        if (iter - id_best >= d_S) {
            g *= k_S;
        }

        if (g > 3)
            g = 2;
        
        if (g < 1e-8) {
            if (tree-- == 0) break;
            g = 2;
        }
        if (UB - cur < 1)
            break;

        update_multipliers(mu, best, UB, sol_lb, g);
    }

    L2_fordward(best_mu);
    auto sol_lb = compute_L2_solution_fordward();
    auto dyna_sol = dynasearch(upper_bound_refine(sol_lb, 15));
    auto dyna_UB = calc_cost(dyna_sol);

    if (dyna_UB < UB)
    {
        UB_sol = dyna_sol;
        UB = dyna_UB;
    }

    auto lb = calc_cost_lb(sol_lb, best_mu);
    return tuple<vector<double>, double>(best_mu, lb);
}

int n_sublimation = 0;
bool sublimeL2(const vector<int> &lb_solution)
{
    cout << "numero de sublimations: " << ++n_sublimation << endl;
    vector<int> cnt(n + 1);
    for (auto x : lb_solution) {
        //assert(x >= 1 && x <= n);
        cnt[x] += 1;
    }
    //set<int> sids(ids.begin(), ids.end());

    vector<int> new_ids;
    for (int i = 1; i <= n; ++i) {
        if (cnt[i] == 0) {
            // assert(sids.count(i) == 0);
            new_ids.emplace_back(i);
            break;
        }
    }
    if (new_ids.empty()) {
        UB_sol = lb_solution;
        UB = calc_cost(lb_solution);
        print_ub();
        return false;
    }
    // shuffle(new_ids.begin(), new_ids.end(), rng);
    new_ids.resize(1);

    for (auto id : new_ids)
    {
        int pot = (1 << ids.size());
        ids.push_back(id);
        pt[id] = pot;

        int m = at[T + 1].size();
        for (auto it = at[T + 1].begin(); m--; ++it)
        {
            Node *no = new Node((*it)->i);
            for (int i = 0; i < 2; ++i)
                for (int j = 0; j < 2; ++j)
                {
                    no->no[i][j] = (*it)->no[i][j];
                    no->value[i][j] = (*it)->value[i][j];
                }
            (*it)->up = no;
            at[T + 1].push_back(no);
        }
        broot = broot->up;
        for (int t = T; t >= 0; --t)
        {
            int m = at[t].size();
            for (auto it = at[t].begin(); m--; ++it)
            {
                Node *no = new Node((*it)->i);
                for (int i = 0; i < 2; ++i)
                    for (int j = 0; j < 2; ++j)
                    {
                        no->no[i][j] = (*it)->no[i][j];
                        no->value[i][j] = (*it)->value[i][j];
                    }
                
                if ((*it)->i == id)
                {
                    swap(no->adj[0], (*it)->adj[0]);
                    for (auto& jt : no->adj[0]) {
                       jt = jt->up;
                    }
                }
                else
                {
                    for (auto &jt : (*it)->adj[0])
                    {
                        if (jt->i == id)
                        {
                            jt = jt->up;
                        }
                        else
                        {
                            no->adj[0].emplace_back(jt->up);
                        }
                    }
                }

                (*it)->up = no;
                at[t].push_back(no);
            }
        }
        for (int t = 1; t <= T + 1; ++t)
        {
            int m = at[t].size() / 2;
            for (auto it = at[t].begin(); m--; ++it)
            {
                if ((*it)->i == id)
                {
                    swap((*it)->up->adj[1], (*it)->adj[1]);
                }
                else
                {
                    int q = (*it)->adj[1].size();
                    for (auto jt = (*it)->adj[1].begin(); q--;)
                    {
                        if ((*jt)->i == id)
                        {
                            (*it)->up->adj[1].push_back((*jt)->up);
                            jt = (*it)->adj[1].erase(jt);
                        }
                        else
                        {
                            (*it)->up->adj[1].push_back((*jt)->up);
                            ++jt;
                        }
                    }
                }
            }
        }
    }

    return true;
}

int eval4(int s[], const vector<int> &id, int t)
{
    int res = 0;
    for (int i : id) {
        //assert(i >= 0 && i <= 3);
        t += p[s[i]];
        res += f(s[i], t);
    }
    return res;
}

bool check_dominance(int s[], int t, int ty) {
    int f[5];
    for (int i = 0; i < 4; ++i) {
        f[i] = s[i];
    }
    if (ty) reverse(f, f + 4);
    else {
        for (int i=0; i<4; ++i) {
            t -= p[f[i]];
        }
    }
    vector<int> id(4);
    iota(id.begin(), id.end(), 0);
    int F = eval4(f, id, t);
    next_permutation(id.begin(), id.end());
    for (int i = 1; i <= 23; ++i, next_permutation(id.begin(), id.end())) {
        //if (ty && id[0] == 1 && id[1] == 0 && id[2] == 2) continue;
        //if (!ty && id[0] == 0 && id[1] == 1 && id[2] == 3) continue;
        int G = eval4(f, id, t);
        if (F > G) return 0;
    }
    return 1;

}

bool vis[maxN];
int seq[5];
bool need4;
void dfs(Node *it, int pos, int t, bool &domination, int ty) {
    if (domination)
        return;
    if (pos == 4) {
        need4 = 1;
        domination |= check_dominance(seq, t, ty);
        return;
    }
    if (ty && t == 0) return;
    if (!ty && t >= T) return;
    vis[it->i] = 1;
    for (auto &jt : it->adj[ty]) {
        if (vis[jt->i]) continue;
        seq[pos] = jt->i;
        dfs(jt, pos + 1, t - (2 * ty - 1) * p[jt->i], domination, ty);
        if (domination) break;
    }
    vis[it->i] = 0;
}

set<pair<int, int>> b[maxN];
void four_sucessive_dominance() {
    int mm = 0;
    for (int t = 1; t <= T; ++t) {
        for (auto it : at[t]) {
            vis[it->i] = 1;
            seq[0] = it->i;
            for (auto jt = it->adj[1].begin(); jt != it->adj[1].end();) {
                if ((*jt)->i == 0) {
                    ++jt;
                    continue;
                }

                bool dom = 0;
                need4 = 0;
                seq[1] = (*jt)->i;
                dfs(*jt, 2, t - p[it->i] - p[(*jt)->i], dom, 1);                                                                                                                                                                                                                                                                                                                                                            
                if (!dom && need4)
                {
                    mm++;
                    b[(*jt)->i].emplace(t - p[it->i], it->i);
                    jt = it->adj[1].erase(jt);
                }
                else
                {
                    ++jt;
                }
            }
            vis[it->i] = 0;
        }
    }
    cout << "erased edges fordward: " << mm << endl;

    for (auto t = 0; t <= T; ++t)
    {
        for (auto it : at[t])
        {

            for (auto jt = it->adj[0].begin(); jt != it->adj[0].end();)
            {
                if (b[it->i].count({t, (*jt)->i}))
                {
                    jt = it->adj[0].erase(jt);
                }
                else
                {
                    ++jt;
                }
            }

        }
    }

    for (int i = 0; i <= n+1; ++i)
        b[i].clear();

    mm = 0;
    for (int t = T-1; t >= 1; --t) {
        for (auto it : at[t]) {
            vis[it->i] = 1;
            seq[0] = it->i;
            for (auto jt = it->adj[0].begin(); jt != it->adj[0].end();) {
                bool dom = 0;
                need4 = 0;
                seq[1] = (*jt)->i;
                dfs(*jt, 2, t + p[(*jt)->i], dom, 0);                                                                                                                                                                                                                                                                                                                                                            
                if (!dom && need4)
                {
                    mm++;
                    b[(*jt)->i].emplace(t, it->i);
                    jt = it->adj[0].erase(jt);
                }
                else
                {
                    ++jt;
                }
            }
            vis[it->i] = 0;
        }
    }
    cout << "erased edges backward: " << mm << endl;

    for (auto t = 0; t <= T; ++t) {
        for (auto it : at[t]) {
            for (auto jt = it->adj[1].begin(); jt != it->adj[1].end();) {
                if (b[it->i].count({t - p[it->i], (*jt)->i})) {
                    jt = it->adj[1].erase(jt);
                } else {
                    ++jt;
                }
            }
        }
    }

    for (int i = 0; i <= n+1; ++i)
        b[i].clear();


}

void solve()
{
    cout << "STAGE 1" << endl;
    // STAGE 1
    // set initial upper bound
    auto upper_bound_1 = greedy_fordward();
    auto upper_bound_2 = greedy_backward();
    auto upper_bound_3 = shortest_processing_time_order();
    UB_sol = upper_bound_1;
    if (calc_cost(UB_sol) > calc_cost(upper_bound_2))
    {
        UB_sol = upper_bound_2;
    }
    if (calc_cost(UB_sol) > calc_cost(upper_bound_3))
    {
        UB_sol = upper_bound_3;
    }
    // print_ub();
    UB = calc_cost(UB_sol);
    cout << "UB: " << UB << endl;
    auto dyna_sol = dynasearch(UB_sol);
    auto dyna_UB = calc_cost(dyna_sol);

    if (dyna_UB < UB)
    {
        UB_sol = dyna_sol;
        UB = dyna_UB;
    }

    cout << "FIRST UPPER BOUND: " << UB << endl;
    //print_ub();

    auto [mu, LB] = subgradient_L1();

    cout << "FIRST LOWER BOUND: " << LB << endl;

    L1_fordward(mu);
    obtain_fordward_L1();
    vector<int> sol_ford;
    for (int i=0; i<nL1; ++i) sol_ford.emplace_back(sL1[i]);

    dyna_sol = dynasearch(upper_bound_refine(sol_ford, 12, 1, 0));
    dyna_UB = calc_cost(dyna_sol);

    if (dyna_UB < UB)
    {
        UB_sol = dyna_sol;
        UB = dyna_UB;
    }

    cout << "SECOND UPPER BOUND: " << UB << endl;

    if (UB - LB < 1)
        return; // cur solution in UB_sol

    L1_backward(mu);
    auto sol_back = obtain_backward_L1();
    cout << "make graph" << endl;
    make_graph_L2(mu);

    // STAGE 2
    cout << "STAGE 2" << endl;

    auto [mu2, LB2] = subgradient_L2(mu);

    cout << "SECOND LOWER BOUND 2: " << LB2 << endl;

    // STAGE 3:
    cout << "STAGE 3" << endl;

    L2_fordward(mu2);
    L2_backward(mu2);

    auto lb_sol = compute_L2_solution_fordward();
    auto lb = calc_cost_lb(lb_sol, mu2);

    elimine_states_L2L2(UB, mu2);
    four_sucessive_dominance();
    cout << "init four sucessive dominance" << endl;
    cout << endl;
    int iter = 1;
    while (true)
    {
        if (!sublimeL2(lb_sol))
        {
            cout << "found solution by lower bound" << endl;
            exit(0);
        }
        four_sucessive_dominance();
        vector<int> inner_sol;

        if (iter % 2 == 0)
        {
            cout << "fordward" << endl;
            L2_fordward(mu2);
            if (root->no[0][0] == -1) {
                print_ub();
                exit(0);
            }
            inner_sol = compute_L2_solution_fordward();
        }
        else
        {
            cout << "backward" << endl;
            L2_backward(mu2);
            if (broot->no[1][0] == -1) {
                print_ub();
                exit(0);
            }
            inner_sol = compute_L2_solution_backward();
        }
        auto lb2 = calc_cost_lb(inner_sol, mu2);
        cout << "current lower bound: " << lb2 << endl;
        elimine_states_L2L2(UB, mu2, iter % 2 == 0 ? 1 : 2);

        if (lb2 > lb || iter % 5 == 0)
        {
            
            auto ub_sol2 = dynasearch(upper_bound_refine(inner_sol, 15, iter % 2 == 0));
            auto dyna_UB = calc_cost(ub_sol2);

            if (dyna_UB < UB)
            {
                UB_sol = ub_sol2;
                UB = dyna_UB;
            }

            cout << "current Upper bound: " << UB << endl;
            lb = max(lb, lb2);
            if (lb == lb2)
                lb_sol = inner_sol;
        }
        if (UB - lb < 1)
            break;
        iter++;
    }

    // check memory occupation ratio M
}

// int main(int argc, char* argv[]) {
int main()
{
    cin >> n;
    for (int i = 1; i <= n; ++i)
    {
        // cin>>p[i]>>d[i]>>ha[i]>>w[i];
        cin >> p[i] >> d[i] >> w[i];
        T += p[i];
    }
    cout << "cur T: " << T << endl;
    p[0] = 0;
    p[n + 1] = 1;
    solve();
    cout << "solution: ";
    print_ub();
    return 0;
}