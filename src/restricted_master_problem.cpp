#pragma once
#include "gurobi_c++.h"
#include <algorithm>
#include <vector>
#include <tuple>
#include <utility>

using v_x = std::vector<std::tuple<int, int, double>>;
using Edges = std::set<std::pair<int, int>>;


std::pair<double, v_x> restricted_master(int z_LB, Edges& edges_id, GRBEnv& env) {
          GRBModel model = GRBModel(env);
          //model.getEnv().set(GRB_DoubleParam_FeasibilityTol, eps);
          //model.getEnv().set(GRB_DoubleParam_OptimalityTol, eps);
          set<int> nodes;
          make_model(model, edges_id, nodes, {}, 1);
          double z_w = -1;
          while ((z_w = solve(model)) - 1e-8 > z_LB) {//z^w > z_LB
               auto path = gen_column(model, edges_id);
               if (path.empty()) {
                    if (z_LB < 0) break;
                    return make_pair(z_w, v_x());
               }
               make_model(model, edges_id, nodes, path, 0);
          }
          return make_pair(z_w, get_solution(model, edges_id));
     };