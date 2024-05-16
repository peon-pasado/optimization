#include <iostream>
#include <queue>
#include "RMPModel.hpp"
#include "BranchAndPrice.hpp"
#include "Problem.hpp"

void print_solution(std::vector<std::vector<int>> bins) {
    std::cout << "SOLUTION: " << std::endl;
    for (int i=0; i<bins.size(); ++i) {
        std::cout << "Bin (" << i + 1 << "):";
        for (int j=0; j<bins[i].size(); ++j) {
            std::cout << " " << bins[i][j];
        }
        std::cout << std::endl;
    }
}

bool verify_and_reduce(int W, std::vector<std::vector<int>>& bins, std::vector<std::pair<int, int>> obj) {
    std::vector<int> b(W + 1);
    for (auto [item, count] : obj) {
        b[item] = count;
    }
    std::vector<std::vector<int>> new_bins;
    for (auto bin : bins) {
        std::vector<int> new_bin;
        for (int o : bin) {
            if (b[o] > 0) {
                new_bin.push_back(o);
                b[o]--;
            } else if (o != 1) {
                return 0;
            }
        }
        new_bins.emplace_back(new_bin);
    }   
    bins = new_bins;
    return 1;
}

//bfs algorithm, return parent array from bfs
std::vector<int> bfs(std::vector<std::vector<std::pair<int, int>>>& graph, int s, int t) {
    int W = (int)graph.size() - 1;
    std::vector<bool> vis(graph.size());
    std::vector<int> parent(graph.size(), -1);
    vis[s] = 1;
    std::queue<int> Q;
    Q.push(s);
    while (!Q.empty()) {
        int q = Q.front();
        Q.pop();
        //existe al menos un camino.
        if (q == W) return parent;
        for (auto [v, f] : graph[q]) {
            if (f == 0 || vis[v]) continue;
            vis[v] = 1;
            parent[v] = q;
            Q.push(v);
        }
    }

    return {};
} 

std::vector<std::vector<int>> recover_bins(int W, std::vector<std::tuple<int, int, int>> flows_in_edges) {
    std::vector<std::vector<std::pair<int, int>>> graph(W + 1);
    for (auto [u, v, flow] : flows_in_edges) {
        graph[u].emplace_back(v, flow);
    }
    std::vector<std::vector<int>> bins;
    while (true) {
        auto bck = bfs(graph, 0, W);
        if (bck.empty()) break;
        int v = W;
        std::vector<int> bin;
        while (v != 0) {
            bin.emplace_back(v - bck[v]);
            for (auto& [u, f] : graph[bck[v]]) {
                if (u == v) {
                    f -= 1;
                    break;
                }               
            }
            v = bck[v];
        }
        bins.emplace_back(bin);
    }
    return bins;
}

int main() {
    int n, W;
    std::cin>>n>>W;
    std::vector<std::pair<int, int>> obj;
    for (int i=0; i<n; ++i) {
        int x, y;
        std::cin>>x>>y;
        obj.emplace_back(x, y);    
    }
    std::cout << "generate dp graph" << std::endl;
    Problem cutting_stock(W, obj);
    BranchAndPrice solver(cutting_stock);
    std::vector<std::tuple<int, int, int>> flows_in_edges;
    std::cout << "invoke branch and bound" << std::endl;
    int n_bins = solver.solve(flows_in_edges);
    std::cout << "Number of bins in branch and bounds: " << n_bins << std::endl;
    auto bins = recover_bins(W, flows_in_edges);
    std::cout << "Number of bins when recover paths: " << bins.size() << std::endl;
    if (verify_and_reduce(W, bins, obj)) {
        std::cout << "The number of items is [correct]" << std::endl;
    } else {
        std::cout << "The number of items is [incorrect]" << std::endl; 
    }
    print_solution(bins);
    return 0;
}