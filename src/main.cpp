#include "gurobi_c++.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <cassert>
#include <cmath>
#include <set>
#include <utility>
#include <stack>
using namespace std;

const double EPS = 1e-7;
inline bool isInteger(float x) { return abs(floor(x) - x) <= EPS; }
using v_x = vector<tuple<int, int, double>>;
using setg = set<pair<int, int>, greater<pair<int, int>>>;


const int maxn = 1110;
int b[maxn];
int n, W;
int w[maxn];

set<pair<int, int>> edges;
bool used[maxn];
int head[maxn];
stack<int> L[maxn * maxn], R[maxn * maxn];
const double eps = 1e-6;

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
     const int INF = 1e8;

     {
          b[0] = b[1];
          auto create = [&](int i, int j) {
               if (j - i == 1) used[i] = 1;
               edges.emplace(i, j);
               L[i * (W + 1) + j].push(0);
               R[i * (W + 1) + j].push(INF); 
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

          //2 
          for (int i=W-1; i >= minW; --i) {
               if (used[i]) continue;
               create(i, i+1);
          }
     }
     auto make_model = [&](GRBModel& model, setg& edges_id, set<int>& nodes, vector<int> col, bool is_new) {
          if (is_new) {
               GRBVar z = model.addVar(0, GRB_INFINITY, 1, GRB_CONTINUOUS, x(W, 0));
               for (int d = 1; d <= W; ++d) {
                    if (d > 1 && b[d] == 0) continue;
                    GRBLinExpr Ad = model.addVar(0, GRB_INFINITY, INF, GRB_CONTINUOUS);
                    model.update();
                    model.addConstr(Ad >= b[d], demand(d));
               }
               nodes.emplace(0); nodes.emplace(W);
               model.addConstr(z == 0, flow(0));
               model.addConstr(-z == 0, flow(W));
               model.update();
          }

          vector<pair<int, int>> new_edges;
          if (is_new) {
               for (auto e : edges_id) new_edges.emplace_back(e);
          } else {
               for (int k = 1; k < (int)col.size(); ++k) {
                    int i = col[k - 1];
                    int j = col[k];
                    if (edges_id.count({i, j})) continue;
                    edges_id.insert({i, j});
                    new_edges.emplace_back(i, j);
               }
          }
          for (auto [i, j] : new_edges) {
               int v = i * (W + 1) + j;
               int ll = L[v].top(), rr = R[v].top();
               GRBVar var = model.addVar(0, GRB_INFINITY, 0, GRB_CONTINUOUS, x(i, j));
               model.update();

               if (!nodes.count(i)) {
                    nodes.emplace(i);
                    model.addConstr(-var == 0, flow(i));
                    model.update();
               } else {
                    model.chgCoeff(model.getConstrByName(flow(i)), var, -1);
               }

               if (!nodes.count(j)) {
                    nodes.emplace(j);                  
                    model.addConstr(var == 0, flow(j));
                    model.update();
               } else {
                    model.chgCoeff(model.getConstrByName(flow(j)), var, 1);
               }

               model.addConstr(var >= ll, l(i, j));
               model.addConstr(var <= rr, r(i, j));
               model.chgCoeff(model.getConstrByName(demand(j - i)), var, 1);
          }
          model.update();
     };

     auto get_solution = [](GRBModel& model, setg& edges_id) {
          v_x vars;
          for (auto [i, j] : edges_id) {
               double r = model.getVarByName(x(i, j)).get(GRB_DoubleAttr_X);
               if (r > eps) {
                    vars.emplace_back(i, j, r);
               }
          }
          return vars;
     };

     auto longest_path = [&](GRBModel& model, setg& edges_id) {
          int n = W+1;
          const double inf = 1e8;
          vector<vector<pair<int, double>>> G(n);
          for (auto [i, j] : edges) {
               double c = 0;
               c -= model.getConstrByName(demand(j - i)).get(GRB_DoubleAttr_Pi);
               if (edges_id.count({i, j})) {                        
                    if (L[i * (W + 1) + j].size() > 1) {
                         c += model.getConstrByName(l(i, j)).get(GRB_DoubleAttr_Pi);
                    }
                    if (R[i * (W + 1) + j].size() > 1) {
                         c += -model.getConstrByName(r(i, j)).get(GRB_DoubleAttr_Pi);
                    }
               }
               //if (c > 1e-8) continue;
               G[i].emplace_back(j, -c);
          }

          /**
          for (int i=0; i<=W; ++i) {
               if (G[i].empty()) continue;
               cout << i << ": ";
               for (auto [j, w] : G[i]) {
                    cout << "(" << j << "," << w << ") ";
               }
               cout << endl;
          }**/
          vector<double> dp(n, -inf);
          vector<int> nxt(n, -1);
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
          if (dp[0] - 1e-10 <= 1) {
               return vector<int>();
          } 
          vector<int> nodes;
          int s = 0;
          while (s != -1) {
               nodes.emplace_back(s);
               s = nxt[s];
          }
          return nodes;
     };
     auto gen_column = [&](GRBModel& model, setg& edges_id) {
          return longest_path(model, edges_id);
     };
     auto solve = [](GRBModel& model) {
          model.optimize();
          return model.get(GRB_DoubleAttr_ObjVal);
     };
     auto restricted_master = [&](int z_LB, setg& edges_id) {
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

     auto addUpper = [&](int i, int j, int r) {
          int v = i * (W + 1) + j;
          R[v].push(min(R[v].top(), r));
     };

     auto popUpper = [&](int i, int j) {
          R[i * (W + 1) + j].pop();
     };

     auto addLower = [&](int i, int j, int l) {
          int v = i * (W + 1) + j;
          L[v].push(max(L[v].top(), l));
     };

     auto popLower = [&](int i, int j) {
          L[i * (W + 1) + j].pop();
     };

     auto reduce_loss = [&](int z, setg& edges_id) {
          int lmin = z * W - sum;
          vector<pair<int, int>> to_erase;
          for (auto [u, v] : edges) {
               if (u + 1 == v) {
                    if (u < W - lmin) {
                         to_erase.emplace_back(u, v);
                    } else {
                         //edges_id.emplace(u, v);
                    }
               }
          }
          for (auto e : to_erase) {
               edges.erase(e);
               edges_id.erase(e);
          }
          b[1] = b[0] + lmin;
     };

     vector<vector<pair<int, int>>> g(W + 1);
     bool found = false;
     auto bandp = [&](auto&& bandp, int z_LB, setg edges_id) {
          if (found) return;
          auto [z, solution] = restricted_master(z_LB, edges_id);
                  
          if (z_LB == -1) {
               int z_LP = ceil(z - 1e-5);
               while (!found) {
                    reduce_loss(z_LP, edges_id);
                    bandp(bandp, z_LP++, edges_id);
               }
               return;
          } 
          if (solution.empty()) return;  

          int id = -1, d = -1;
          for (int i=0; i<(int)solution.size(); ++i) {
               auto [u, v, w] = solution[i];
               if (!isInteger(w) && d < v - u) {
                    id = i;
                    d = v - u;
               } 
          }

          if (id == -1) {
               found = true;
               for (auto [i, j, c] : solution) {
                    g[i].emplace_back(j, round(c));
               }
               return;
          }

          auto [u, v, w] = solution[id];
          addUpper(u, v, floor(w));
          bandp(bandp, z_LB, edges_id);
          popUpper(u, v);

          addLower(u, v, floor(w) + 1);
          bandp(bandp, z_LB, edges_id);
          popLower(u, v);
     };
     setg s;
     bandp(bandp, -1, s);
     vector<int> bin;
     vector<vector<int>> bins;
     auto dfs = [&](auto&& dfs, int v, int last)->bool {
          if (v != 0) bin.emplace_back(v - last);
          if (v == W) {
               bins.emplace_back(bin);
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
     
     cout << bins.size() << endl;
     
     b[1] = b[0];
     for (int i=0; i<(int)bins.size(); ++i) {
          cout << "bin " << i + 1 << ": ";
          for (auto w : bins[i]) {
               if (b[w] == 0) continue;
               b[w]--;
               cout << w << " ";
          }
          cout << '\n';
     }
     for (int i=1; i<=W; ++i) assert(b[i] == 0);
     
  } catch(GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch(...) {
    cout << "Exception during optimization" << endl;
  }

  return 0;
}