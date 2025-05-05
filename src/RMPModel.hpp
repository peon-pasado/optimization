#pragma once
#include "gurobi_c++.h"
#include "Problem.hpp"
#include <algorithm>
#include <vector>
#include <tuple>
#include <utility>
#include <set>
#define PRECISION_PATH 1e-10

const double eps = 1e-8;
const double inf = 1e8;

//restricted price problem model
struct RMPModel {

     GRBEnv env;
     GRBModel* model;
     std::set<int> nodes;
     std::set<std::pair<int, int>>& edges;
     std::set<std::pair<int, int>> total_edges;
     /**
      * description: init the model for environment,
      * we add extra features if needed.
     **/
     RMPModel(std::set<std::pair<int, int>>& edges, std::set<std::pair<int, int>> total_edges): 
          edges(edges),  total_edges(total_edges) {
          std::cout << "init new restricted master problem model" << std::endl;
          try {
               std::cout << "init environment" << std::endl;
               GRBEnv env = GRBEnv(true);
               //env.set("LogFile", "branch_and_price.log");
               //env.set(GRB_IntParam_OutputFlag, 0);
               env.start();

               std::cout << "create empty model" << std::endl;
               model = new GRBModel(env); 

               //model.getEnv().set(GRB_DoubleParam_FeasibilityTol, eps);
               //model.getEnv().set(GRB_DoubleParam_OptimalityTol, eps); 
          } catch(GRBException e) {
               std::cout << "Error code = " << e.getErrorCode() << std::endl;
               std::cout << e.getMessage() << std::endl;
          } catch(...) {
               std::cout << "Exception during optimization" << std::endl;
          }
     }

     void add_objective_function(const Problem& problem) {
          //adding objective var z
          model->addVar(0, GRB_INFINITY, 1, GRB_CONTINUOUS, problem.x(problem.W, 0));
          model->update();
     }

     void add_dummy_edges(const Problem& problem) {
          for (int d = 1; d <= problem.W; ++d) {
               if (d > 1 && problem.b[d] == 0) continue;
               std::cout << "add dummy edge for weight " << d << std::endl;
               /**
                * create var A_d with cost infinity
                * min z + ... oo x A_d ...
                * 0 <= A_d < oo
               **/  
               GRBLinExpr A_d = model->addVar(0, GRB_INFINITY, inf, GRB_CONTINUOUS);
               model->update();
               /**
                * add restriction A_d >= b_d
               **/
               model->addConstr(A_d >= problem.b[d], problem.demand(d));
          }
          model->update();
     }

     void add_nodes(std::vector<int> new_nodes) {
          nodes.insert(new_nodes.begin(), new_nodes.end());
     }

     void add_flow_constraint(const Problem& problem, GRBVar& x, int node, int coeff) {
          std::cout << "add flow constraint (" << node << ")" << std::endl;
          if (!nodes.count(node)) {
               nodes.emplace(node);
               model->addConstr(coeff * x == 0, problem.flow(node));
               model->update();
          } else {
               model->chgCoeff(model->getConstrByName(problem.flow(node)), x, coeff);
          }
          std::cout << "[finish]" << std::endl;
     }

     //check yourself that new edges are "new" edges to model
     void add_edges(const Problem& problem, std::set<std::pair<int, int>> new_edges) {
          try {
               for (auto [i, j] : new_edges) {
                    edges.emplace(i, j);
                    GRBVar var;
                    
                    if (i < j) {
                         var = model->addVar(0, GRB_INFINITY, 0, GRB_CONTINUOUS, problem.x(i, j));
                    } else {
                         var = model->getVarByName(problem.x(problem.W, 0));
                    }

                    model->update();

                    std::cout << "add edges constraints" << std::endl;
                    add_flow_constraint(problem, var, i, -1);
                    add_flow_constraint(problem, var, j, 1);
                    std::cout << "<finish>" << std::endl;

                    if (problem.has_branch_lower(i, j)) {
                         std::cout << "add lower constaints" << std::endl;
                         model->addConstr(var >= problem.get_lower(i, j), problem.l(i, j));
                         std::cout << "[finish]" << std::endl;
                    }
                    if (problem.has_branch_upper(i, j)) {
                         std::cout << "add upper constaints" << std::endl;
                         model->addConstr(var <= problem.get_upper(i, j), problem.r(i, j));
                         std::cout << "[finish]" << std::endl;
                    }
                    if (i > j) continue;
                    std::cout << "change demand coefficient " << i << ' ' << j << std::endl;
                    model->chgCoeff(model->getConstrByName(problem.demand(j - i)), var, 1);
                    std::cout << "[finish]" << std::endl;
               }
               model->update();
          } catch(GRBException e) {
               std::cout << "Error code = " << e.getErrorCode() << std::endl;
               std::cout << e.getMessage() << std::endl;
          } catch(...) {
               std::cout << "Exception during optimization" << std::endl;
          }
     }

     std::vector<int> price_path(const Problem& problem) {
          int n = problem.W;
          std::vector<std::vector<std::pair<int, double>>> g(n + 1);
          std::cout << "find reduced cost" << std::endl;
          try {
               for (auto [i, j] : total_edges) {
                    if (i > j) continue;
                    double c = -model->getConstrByName(problem.demand(j - i)).get(GRB_DoubleAttr_Pi);
                    if (edges.count(std::make_pair(i, j))) {
                         if (problem.has_branch_lower(i, j)) c += model->getConstrByName(problem.l(i, j)).get(GRB_DoubleAttr_Pi);
                         if (problem.has_branch_upper(i, j)) c -= model->getConstrByName(problem.r(i, j)).get(GRB_DoubleAttr_Pi);
                    }
                    std::cout << "edge ("<<i<<","<<j<<","<<-c<<")"<<std::endl;
                    g[i].emplace_back(j, -c);
               }
          } catch(GRBException e) {
               std::cout << "Error code = " << e.getErrorCode() << std::endl;
               std::cout << e.getMessage() << std::endl;
          } catch(...) {
               std::cout << "Exception during optimization" << std::endl;
          }
          std::cout << "build dp" << std::endl;
          std::vector<double> dp(n + 1, -inf);
          std::vector<int> nxt(n + 1, -1);
          dp[n] = 0;
          for (int i=n-1; i>=0; --i) {
               for (auto [j, c] : g[i]) {
                    if (dp[j] == -inf) continue;
                    if (dp[i] < dp[j] + c) {
                         dp[i] = dp[j] + c;
                         nxt[i] = j;
                    } 
               }
          }
          std::cout << "[finish]" << std::endl;
          std::cout << "dp value: " << dp[0] << std::endl;
          //10^-16
          if (dp[0] - PRECISION_PATH <= 1) {
               return std::vector<int>();
          } 
          std::cout << "get best path from dp" << std::endl;
          std::vector<int> nodes;
          int s = 0;
          while (s != -1) {
               nodes.emplace_back(s);
               s = nxt[s];
          }
          return nodes;
     }

     /**
      * build model with dummy edges
      * 
      * add new edges to price restricted model, that's 
      * included lower/upper restrictions and demand
      * restrictions.
      * 
      *  
     **/ 
     void build(const Problem& problem) {   
          std::cout << "- add objective function" << std::endl;      
          add_objective_function(problem);
          std::cout << "- add dummy edges" << std::endl;
          add_dummy_edges(problem);
          //adding z var
          edges.emplace(problem.W, 0);
          std::cout << "- add edges to model" << std::endl;
          add_edges(problem, edges);
     }

     void update(const Problem& problem, std::vector<int> path) {
          std::set<std::pair<int, int>> new_edges;
          for (int i=1; i<(int)path.size(); ++i) {
               auto e = std::make_pair(path[i-1], path[i]);
               if (edges.count(e)) continue;
               new_edges.emplace(e);
          }
          std::cout << "- add edges" << std::endl;
          add_edges(problem, new_edges);
     }

     void solve_simplex() {
          std::cout << "calculate optimum" << std::endl;
          model->optimize();
          std::cout << "[finish]" << std::endl;
     }

     double opt() {
          solve_simplex();
          return model->get(GRB_DoubleAttr_ObjVal); 
     }     

     std::vector<std::tuple<int, int, double>> empty_val() {
          return std::vector<std::tuple<int, int, double>>();
     }

     //find primal values in model for RMP problem
     std::vector<std::tuple<int, int, double>> get_opt_val(const Problem& problem) {
          //solve_simplex();
          std::vector<std::tuple<int, int, double>> opt_val;
          for (auto [i, j] : edges) {
               double xij = model->getVarByName(problem.x(i, j)).get(GRB_DoubleAttr_X);
               if (xij > eps) {
                    opt_val.emplace_back(i, j, xij);
               }
          }
          return opt_val;
     }

     std::pair<double, std::vector<std::tuple<int, int, double>>> solve(const Problem& problem, int z_LB) {
          std::cout << "solve column generation" << std::endl;
          double z_w = -1;
          std::cout << "build initial model..." << std::endl;
          build(problem);
          std::cout << "[finish]" << std::endl;
          while (true) {
               z_w = opt();
               if (z_w <= z_LB + eps) break;
               std::cout << "get price path" << std::endl;
               auto path = price_path(problem);
               for (auto p : path) std::cout << p << ' ';
               std::cout << std::endl;
               if (path.empty()) {
                    if (z_LB < 0) break;
                    return make_pair(z_w, empty_val());
               }    
               std::cout << "update restricted price model" << std::endl;
               update(problem, path);
          }
          std::cout << "[finish by break]" << std::endl;
          return make_pair(z_w, get_opt_val(problem));
     }
};


