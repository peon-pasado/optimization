#include "gurobi_c++.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <cassert>
#include <cmath>
#include <set>
#include <utility>
using namespace std;

const double EPS = 1e-5;
inline bool isInteger(float x) { return abs(floor(x) - x) <= EPS; }

const int maxn = 1010;
int b[maxn];
int n, W;
int w[maxn];

vector<pair<int, int>> edges;
bool used[maxn];
int head[maxn];

string val(int x) {
     static const string dic = "0123456789abcdef";
     string s = "";
     while (x > 0) {
          s += dic[x & 15];
          x >>= 4;
     }
     return s;
}

string x(int i, int j) {
     return val(i * (W + 1) + j);
}

string l(int i, int j) {
     return val((W + 1) * (W + 1) + i * (W + 1) + j);
}

string r(int i, int j) {
     return val(2 * (W + 1) * (W + 1) + i * (W + 1) + j);
}

string demand(int d) {
     return val(3 * (W + 1) * (W + 1) + d);
}

string flow(int i) {
     return val(3 * (W + 1) * (W + 1) + (W + 1) + i);
}

int main(int argc, char *argv[]) {
     cin>>n>>W;
     int minW = W;
     int sum = 0;
     for (int i=1; i<=n; ++i) {
          int c;
          cin>>w[i]>>c;
          b[w[i]] = c;
          sum += c * w[i];
          minW = min(minW, w[i]);
     }
     
  try {

     GRBEnv env = GRBEnv(true);
     env.set("LogFile", "branch_and_price.log");
     env.set(GRB_IntParam_OutputFlag, 0);
     env.start();
    
     GRBModel model = GRBModel(env);    

     {
          auto create = [&](int i, int j) {
               if (j - i == 1) used[i] = 1;
               edges.emplace_back(i, j);
               //model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, x(i, j));
          };

          head[0] = 1e9;
          for (int d = W; d >= 1; --d) {
               if (b[d] == 0) continue;
               for (int i=0; i<d; ++i) {
                    int c = 0;
                    for (int j=0; i + (j + 1)*d <= W; ++j) {
                         if (head[i + j * d] > d) c += 1;
                         if (j >= b[d] && head[i + (j - b[d]) * d] > d) c -= 1; 
                         if (c > 0) {
                              head[i + (j + 1) * d] = max(head[i + (j + 1) * d], d);
                              create(i + j * d, i + (j + 1) * d);
                         }
                    }
               }
          }         

          for (int i=W-1; i >= minW; --i) {
               if (used[i]) continue;
               create(i, i+1);
          }

          GRBVar z = model.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS, x(W, 0));
          for (int d = 1; d <= W; ++d) {
               if (d > 1 && b[d] == 0) continue;
               GRBLinExpr Ad = model.addVar(0.0, GRB_INFINITY, 1e9, GRB_CONTINUOUS);
               model.update();
               model.addConstr(Ad >= b[d], demand(d));
          }
          model.addConstr(z == 0, flow(0));
          model.addConstr(-z == 0, flow(W));
     }
     model.update();

     auto get_solution = [&](GRBModel& model, set<pair<int, int>>& edges_id) {
          int u=0, v=0;
          float w=0, z = model.get(GRB_DoubleAttr_ObjVal);
          for (auto [i, j] : edges_id) {
               float r = model.getVarByName(x(i, j)).get(GRB_DoubleAttr_X);
               if (r > 1e-8 && !isInteger(r)) {
                    u = i; v = j; w = r;
                    break;
               }
          }
          return tuple<float, float, int, int>(z, w, u, v);
     };
     auto modify = [&](GRBModel& model, const vector<int>& nodes, set<pair<int, int>>& edges_id) {
          for (int i=1; i<(int)nodes.size(); ++i) {
               int u = nodes[i-1], v = nodes[i];
               if (edges_id.count({u, v})) continue;
               edges_id.insert({u, v});
               GRBVar var;
               //try {
               //     var = model.getVarByName(x(u, v));
               //} catch(...) {
                    var = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, x(u, v));
                    model.update();
               //}
               
               try {
                    model.chgCoeff(model.getConstrByName(flow(u)), var, -1);
               }  catch (GRBException e) {
                    model.addConstr(-var == 0, flow(u));
                    model.update();
               }

               try {
                    model.chgCoeff(model.getConstrByName(flow(v)), var, 1);
               } catch (GRBException e) {
                    model.addConstr(var == 0, flow(v));
                    model.update();
               }

               auto constr = model.getConstrByName(demand(v - u));
               model.chgCoeff(constr, var, 1);
          }
          model.update();
     };
     vector<vector<pair<int, double>>> G(W+1);
     vector<float> dp(W+1);
     vector<int> nxt(W+1);
     auto create_graph = [&](GRBModel& model) {
          //vector<vector<pair<int, double>>> g(W+1);
          for (auto [i, j] : edges) {
               float c = -model.getConstrByName(demand(j - i)).get(GRB_DoubleAttr_Pi);
               try {
                    c += model.getConstrByName(l(i, j)).get(GRB_DoubleAttr_Pi);
               } catch(...) {

               }
               try {
                    c += -model.getConstrByName(r(i, j)).get(GRB_DoubleAttr_Pi);
               } catch(...) {

               }
               //if (c > -1e-5) continue;
               G[i].emplace_back(j, -c);
          }
          return G;
     };
     auto longest_path = [&G, &nxt, &dp]() {
          int n = G.size();
          const float inf = 1e18;
          //vector<float> dp(n, -inf);
          //vector<int> nxt(n, -1);
          fill(dp.begin(), dp.end(), -inf);
          fill(nxt.begin(), nxt.end(), -1);
          dp[n-1] = 0;
          for (int i=n-2; i>=0; --i) {
               for (auto [j, w] : G[i]) {
                    if (dp[j] == -inf) continue;
                    if (dp[i] < dp[j] + w) {
                         dp[i] = dp[j] + w;
                         nxt[i] = j;
                    }
               }
          }
          for (int i=0; i<n; ++i) {
               G[i].clear();
          }
          if (dp[0] <= 1 + 1e-5) {
               return vector<int>();
          }
          vector<int> nodes;
          int s = 0;
          while (s != -1) {
               nodes.push_back(s);
               s = nxt[s];
          }
          return nodes;
     };
     auto gen_column = [&](GRBModel& model) {
          create_graph(model);
          return longest_path();
     };
     auto solve = [](GRBModel& model) {
          model.update();
          model.optimize();
          return model.get(GRB_DoubleAttr_ObjVal);
     };
     auto restricted_master = [&](GRBModel& model, int z_LB, set<pair<int, int>>& edges_id) {
          double z_w = -1;
          while ((z_w = solve(model)) > z_LB) {//z^w > z_LB
               auto nodes = gen_column(model);
               if (nodes.empty()) {
                    if (z_LB == -1) break;
                    return tuple<float, float, int, int>(z_w, -1, -1, -1);
               }
               modify(model, nodes, edges_id);
          }
          return get_solution(model, edges_id);
     };
     auto reduce_loss = [&](GRBModel& model, int z, set<pair<int, int>>& edges_id) {
          int lmin = z * W - sum;
          auto constr = model.getConstrByName(demand(1));
          for (auto [u, v] : edges) {
               if (u + 1 == v) {
                    if (!edges_id.count({u, v})) continue;
                    if (u < W - lmin) {
                         edges_id.erase({u, v});
                         model.getVarByName(x(u, v)).set(GRB_DoubleAttr_Obj, GRB_INFINITY);
                    } else {
                         model.chgCoeff(constr, model.getVarByName(x(u, v)), 1);
                    }
               }
          }
          constr.set(GRB_DoubleAttr_RHS, max(b[1], lmin));
          model.update();
     };
     bool found = false;
     vector<vector<pair<int, int>>> g(W + 1);
     auto bandp = [&](auto&& bandp, GRBModel& model, int z_LB, set<pair<int, int>> edges_id) {
          if (found) return;
          auto [z, w, u, v] = restricted_master(model, z_LB, edges_id);
          if (v == -1) return;          
          if (z_LB == -1) {
               int z_LP = ceil(z - 1e-8);
               while (!found) {
                    //reduce_loss(model, z_LP, edges_id);
                    GRBModel cmodel = model;
                    bandp(bandp, cmodel, z_LP++, edges_id);
               }
          } else {
               if (v == 0) {
                    found = true;
                    for (auto [i, j] : edges_id) {
                         float r = model.getVarByName(x(i, j)).get(GRB_DoubleAttr_X);
                         if (r > 1e-8) {
                              g[i].emplace_back(j, round(r));
                         }
                    }
                    return;
               }

               GRBModel model_ = model;
               int L = floor(w) + 1;
               try {
                    model_.getConstrByName(l(u, v)).set(GRB_DoubleAttr_RHS, L);
               } catch (GRBException e) {
                    model_.addConstr(model.getVarByName(x(u, v)) >= L, l(u, v));
                    model_.update();
               }
               bandp(bandp, model_, z_LB, edges_id);
               int R = floor(w);
               try {
                    model.getConstrByName(r(u, v)).set(GRB_DoubleAttr_RHS, R);
               } catch (GRBException e) {
                    model.addConstr(model.getVarByName(x(u, v)) <= R, r(u, v));
                    model.update(); 
               }
               bandp(bandp, model, z_LB, edges_id);               
               
          }
     };
     set<pair<int, int>> s;
     bandp(bandp, model, -1, s);
     vector<int> bin;
     vector<vector<int>> bins;
     auto dfs = [&](auto&& dfs, int v, int last)->bool {
          if (v != 0) bin.push_back(v - last);
          if (v == W) {
               bins.push_back(bin);
               return true;
          }
          for (auto& [u, c] : g[v]) {
               if (c == 0) continue;
               c--;
               if (dfs(dfs, u, v)) {
                    bin.pop_back();
                    return true;
               }
               c++;
          }
          if (v != 0) bin.pop_back();
          return false;
     };
     while (dfs(dfs, 0, -1));
     for (int i=0; i<(int)bins.size(); ++i) {
          cout << "bin " << i + 1 << ": ";
          for (auto w : bins[i]) {
               if (w == 1) {
                    if (b[1] == 0) continue;
                    b[1]--;
               }
               cout << w << " ";
          }
          cout << '\n';
     }
  } catch(GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch(...) {
    cout << "Exception during optimization" << endl;
  }

  return 0;
}