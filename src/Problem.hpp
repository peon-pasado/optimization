#pragma once
#include <vector>
#include <stack>
#include <utility>
#include <set>

const int INF = 1e9;

struct Problem {

    std::vector<std::stack<int>> L;
    std::vector<std::stack<int>> R;
    std::vector<int> b;
    std::set<std::pair<int, int>> edges;
    int W;
    int sum;

    Problem(int W, std::vector<std::pair<int, int>> obj): b(W + 1), W(W) {
        L.resize((W + 1) * (W + 1) + 1); 
        R.resize((W + 1) * (W + 1) + 1);

        for (auto [weight, count] : obj) {
            b[weight] = count;
        }
        sum = 0;
        for (int i=0; i<=W; ++i) {
            sum += i * b[i];
        }
        b[0] = b[1]; //save information for objects with weight = 1

        std::vector<bool> used_unit_edge(W + 1);
        auto create_edge = [&](int i, int j) {
            if (j - i == 1) used_unit_edge[i] = 1;
            edges.emplace(i, j);
        };

        std::vector<int> head(W + 1);
        head[0] = INF;
        for (int d = W; d >= 1; --d) {
            if (b[d] == 0) continue;
            for (int i=0; i<d; ++i) {
                int c = 0;
                for (int j=0; i + (j + 1)*d <= W; ++j) {
                        if (head[i + j * d] > d) c += 1;
                        if (j >= b[d] && head[i + (j - b[d]) * d] > d) c -= 1; 
                        if (c > 0) {
                            head[i + (j + 1) * d] = std::max(head[i + (j + 1) * d], d);
                            create_edge(i + j * d, i + (j + 1) * d);
                        }
                }
            }
        }         

        int min_w = min_element(obj.begin(), obj.end())->first;
        for (int i=W-1; i >= min_w; --i) {
            if (used_unit_edge[i]) continue;
            create_edge(i, i+1);
        }
    }

    std::string val(int x) const {
        static const std::string dic = "0123456789abcdef";
        std::string s = "";
        while (x > 0) {
            s += dic[x & 15];
            x >>= 4;
        }
        return s;
    }

    std::string x(int i, int j) const {
        return val(i * (W + 1) + j);
    }

    std::string l(int i, int j) const {
        return val((W + 1) * (W + 1) + i * (W + 1) + j);
    }

    std::string r(int i, int j) const {
        return val(2 * (W + 1) * (W + 1) + i * (W + 1) + j);
    }

    std::string demand(int d) const {
        return val(3 * (W + 1) * (W + 1) + d);
    }

    std::string flow(int i) const {
        return val(3 * (W + 1) * (W + 1) + (W + 1) + i);
    }

    int get_sum() const {
        return sum;
    }

    int id_edge(int i, int j) const {
        return i * (W + 1) + j;
    }

    bool has_branch_lower(int i, int j) const {
        return !L[id_edge(i, j)].empty();
    }

    bool has_branch_upper(int i, int j) const {
        return !R[id_edge(i, j)].empty();
    } 

    int get_lower(int i, int j) const {
        return L[id_edge(i, j)].top();
    }

    int get_upper(int i, int j) const {
        return R[id_edge(i, j)].top();
    }

    void add_lower(int i, int j, int l) {
        L[id_edge(i, j)].push(l);
    }

    void pop_lower(int i, int j) {
        L[id_edge(i, j)].pop();
    }

    void add_upper(int i, int j, int u) {
        R[id_edge(i, j)].push(u);
    }

    void pop_upper(int i, int j) {
        R[id_edge(i, j)].pop();
    }

};