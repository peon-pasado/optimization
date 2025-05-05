#pragma once

#include "RMPModel.hpp"

const double Eps = 1e-5;
inline bool isInteger(float x) { 
     return abs(floor(x) - x) <= Eps;
}

struct BranchAndPrice {

     Problem problem;
     std::set<std::pair<int, int>> total_edges;
     bool found;

     BranchAndPrice(Problem problem): 
          problem(problem), total_edges(problem.edges), found(false) {}

     void reduce_loss(std::set<std::pair<int, int>>& edges, int z) {
          std::cout << "Calculate reduce loss" << std::endl;
          int l_min = z * problem.W - problem.get_sum();
          std::set<std::pair<int, int>> to_erase;
          for (auto [i, j] : total_edges) {
               if (i + 1 == j && i < problem.W - l_min) {
                    to_erase.emplace(i, j);
               }
          }
          total_edges.erase(to_erase.begin(), to_erase.end());
          edges.erase(to_erase.begin(), to_erase.end());
          problem.b[1] = problem.b[0] + l_min;
          std::cout << "[finish]" << std::endl;
     }

     void bandp(int z_LB, std::set<std::pair<int, int>>& edges, std::vector<std::tuple<int, int, int>>& int_solution) {
          std::cout << "Init branch and bound node" << std::endl;
          if (found) {
               std::cout << "[finish by found]" << std::endl;
               return;
          }
          RMPModel rmp_model = RMPModel(edges, total_edges);     
          auto [z, solution] = rmp_model.solve(problem, z_LB);
          if (solution.empty()) {
               std::cout << "Not solution" << std::endl;
               return;
          }
          std::cout << "look for not integer edge" << std::endl;
          int id = -1, d = -1;
          for (int i=0; i<(int)solution.size(); ++i) {
               auto [u, v, w] = solution[i];
               if (!isInteger(w) && d < v - u) {
                    id = i;
                    d = v - u;
               } 
          }
          std::cout << "[finish]" << std::endl;
          if (id == -1) {
               std::cout << "found a solution" << std::endl;
               found = true;
               for (auto [i, j, c] : solution) {
                    int_solution.emplace_back(i, j, round(c));
               }
               return;
          }

          std::cout << "Branch upper" << std::endl;
          auto [u, v, w] = solution[id];
          problem.add_upper(u, v, floor(w));
          bandp(z_LB, edges, int_solution);
          problem.pop_upper(u, v);

          std::cout << "Branch lower" << std::endl;
          problem.add_lower(u, v, floor(w) + 1);
          bandp(z_LB, edges, int_solution);
          problem.pop_lower(u, v);
     } 

     int solve(std::vector<std::tuple<int, int, int>>& int_solution) {
          std::set<std::pair<int, int>> edges;
          RMPModel rmp_model = RMPModel(edges, total_edges);     
          auto [z, solution] = rmp_model.solve(problem, -1);
          int z_LP = ceil(z - Eps);
          while (!found) {
               //reduce_loss(edges, z_LP);
               std::set<std::pair<int, int>> edges_copy = edges;
               bandp(z_LP, edges_copy, int_solution);
               z_LP++;
          }
          return z_LP - 1;
     }  
};