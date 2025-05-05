#pragma once
#include "models.hpp"
#include "utils/solution.hpp"
#include "utils/graph.hpp"
#include "ssdp.hpp"

void solve() {
    prob->sol = create_solution();
    problem_create_graph();
    ssdp();
}