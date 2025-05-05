#pragma once

#include "models.h"
#include "utils_graph.h"
#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>
#include "dominance.h"

std::pair<std::vector<int>, double> solve_LR1_fordward(bool first_time = false) {
    Tmap* tm = t2m->tm;
    tm[0].val[0][0] = 0;
    tm[0].node[0][0] = 0;
    for (int t = 1; t <= prob->T; t++) {
        tm[t].val[0][0] = tm[t].val[0][1] = inf;
        tm[t].node[0][0] = tm[t].node[0][1] = prob->n + 2;
        if (!check_time(t)) continue;
        for (int j = 1; j <= prob->n; j++) {
            if (first_time) delete_edge(t, j, j);
            if (!check_node(t, j)) continue;
            
            int tj = t - prob->p[j];
            if (tm[tj].node[0][0] > prob->n + 1 || (tm[tj].node[0][0] == j && tm[tj].node[0][1] > prob->n + 1)) {
                delete_node(t, j);
                continue;
            }

            double path_cost = tm[tj].val[0][tm[tj].node[0][0] == j] + tm[t].f[j] - prob->u[j];

            if (path_cost < tm[t].val[0][0]) {
                tm[t].val[0][1] = tm[t].val[0][0];
                tm[t].node[0][1] = tm[t].node[0][0];
                
                tm[t].val[0][0] = path_cost;
                tm[t].node[0][0] = j;
            }
            else if (path_cost < tm[t].val[0][1]) {
                tm[t].val[0][1] = path_cost;
                tm[t].node[0][1] = j;
            }
        }
        if (tm[t].node[0][0] > prob->n + 1) {
            delete_time(t);
        }
    }
    //std::cout << "finish solve_LR1_fordward" << std::endl;
    double total_cost = tm[prob->T].val[0][0];
    //std::cout << "total_cost: " << total_cost + prob->su << std::endl;
    if (total_cost > inf / 1.1) {
        std::vector<int> empty_seq;
        return {empty_seq, inf};
    }
    std::vector<int> sequence;
    int current_t = prob->T;
    int v = tm[prob->T].node[0][0];
    while (current_t > 0) {
        sequence.push_back(v);
        current_t -= prob->p[v];
        v = tm[current_t].node[0][tm[current_t].node[0][0] == v];
    }
    
    std::reverse(sequence.begin(), sequence.end());
    double lagrangian_cost = total_cost + prob->su;
    return {sequence, lagrangian_cost};
}

std::pair<std::vector<int>, double> solve_LR1_backward() {
    Tmap* tm = t2m->tm;
    for (int t = 0; t <= prob->T + 1; t++) {
        tm[t].val[1][0] = tm[t].val[1][1] = inf;
        tm[t].node[1][0] = tm[t].node[1][1] = prob->n + 2;
    }
    tm[prob->T + 1].val[1][0] = 0;
    tm[prob->T + 1].node[1][0] = prob->n + 1;
    tm[prob->T].val[1][0] = 0;
    tm[prob->T].node[1][0] = prob->n + 1;
    for (int t = prob->T; t > 0; t--) {
        if (!check_time(t)) continue;

        if (tm[t].node[1][0] > prob->n + 1) {
            delete_time(t);
            continue;
        }

        for (int j = 1; j <= prob->n; j++) { //(tj, i) <- (t, j) 
            if (!check_node(t, j)) continue;
            int tj = t - prob->p[j];

            if (tm[t].node[1][0] == j && tm[t].node[1][1] > prob->n + 1) {
                delete_node(t, j);
                continue;
            }

            if (tm[tj].node[0][0] > prob->n + 1 || (tm[tj].node[0][0] == j && tm[tj].node[0][1] > prob->n + 1)) {
                delete_node(t, j);
                continue;
            }
                
            double path_cost = tm[t].val[1][tm[t].node[1][0] == j] + tm[t].f[j] - prob->u[j];            

            if (path_cost + prob->su + tm[tj].val[0][tm[tj].node[0][0] == j] > prob->ub - 1 + prob->eps) {
                delete_node(t, j);
                continue;
            }

            if (path_cost < tm[tj].val[1][0]) {
                tm[tj].val[1][1] = tm[tj].val[1][0];
                tm[tj].node[1][1] = tm[tj].node[1][0];
                
                tm[tj].val[1][0] = path_cost;
                tm[tj].node[1][0] = j;
            }
            else if (path_cost < tm[tj].val[1][1]) {
                tm[tj].val[1][1] = path_cost;
                tm[tj].node[1][1] = j;
            }
        }

    }    
    double total_cost = tm[0].val[1][0];
    if (total_cost > inf / 1.1) {
        std::vector<int> empty_seq;
        return {empty_seq, inf};
    }
    
    std::vector<int> sequence;
    int current_t = 0;
    int v = tm[0].node[1][0];
    while (current_t < prob->T) {
        assert(v >= 1 && v <= prob->n);
        sequence.push_back(v);
        current_t += prob->p[v];
        v = tm[current_t].node[1][tm[current_t].node[1][0] == v];
    }
    double lagrangian_cost = total_cost + prob->su;
    return {sequence, lagrangian_cost};
}

/** 
bool check_sequence(info* prob, T2map* t2m, std::vector<int>& sequence, double lb) {
    int current_t = 0;
    double current_cost = 0;
    for (int i = 0; i < sequence.size(); i++) {
        int j = sequence[i];
        current_t += prob->p[j];
        if (current_t > prob->T) return false;
        current_cost += t2m->tm[current_t].f[j] - prob->u[j];
    }
    current_cost += prob->su;
    return fabs(current_cost - lb) < 1e-6;
}**/


std::pair<std::vector<int>, double> solve_LR2_fordward(info* prob, T2map* t2m, bool first_time=false) {
    Tmap* tm = t2m->tm;
    if (first_time) {
        add_node(tm[0].at, 0);
        for (int t=1; t<=prob->T; ++t) {
            if (!check_time(t)) continue;

            if (tm[t].node[0][0] > prob->n + 1 || tm[t].node[1][0] > prob->n + 1) {
                delete_time(t);
                continue;
            }

            for (int i=prob->n; i>=1; --i) {
                if (!check_node(t, i)) continue;

                if (tm[t].node[1][0] == i && tm[t].node[1][1] > prob->n + 1) {
                    delete_node(t, i);
                    continue;
                }

                int tj = t - prob->p[i];

                if (tm[tj].node[0][0] == i && tm[tj].node[0][1] > prob->n + 1) {
                    delete_node(t, i);
                    continue;
                }        

                bool has_edge = true;
                if (tj > 0) {
                    has_edge = false;
                    for (Node* jt = tm[tj].at; jt; jt = jt->next) {
                        if (!check_edge(t, jt->i, i)) continue;

                        if (!check_node(t - prob->p[jt->i], i)) {
                            delete_edge(t, i, jt->i);
                        }

                        int fji = tm[tj].f[jt->i] + tm[t].f[i];
                        int fij = tm[t - prob->p[jt->i]].f[i] + tm[t].f[jt->i];
                        if (fji > fij || (fji == fij && prob->ord[i] < prob->ord[jt->i])) {
                            delete_edge(t, jt->i, i);
                        } else {
                            has_edge = true;
                        }
                    }   
                } else {
                    if (!check_edge(t, 0, i)) {
                        has_edge = false;
                    }
                }
                if (has_edge) add_node(tm[t].at, i);
                else delete_node(t, i); 
            }
        }
        for (int i = 1; i <= prob->n; i++) {
            while (prob->s[i] <= prob->T && !check_node(prob->s[i], i)) {
                prob->s[i]++;
            }
            while (prob->e[i] >= 0 && !check_node(prob->e[i], i)) {
                prob->e[i]--;
            }
        }
        add_node(tm[prob->T+1].at, prob->n + 1);
    }
    tm[0].at->val[0][0] = 0;
    tm[0].at->val[0][1] = inf;
    tm[0].at->node[0][0] = 0;
    tm[0].at->node[0][1] = prob->n + 2;
    tm[0].at->fnode[0][0] = 0;
    tm[0].at->node_ptr[0][0] = nullptr;
    for (int t=1; t<=prob->T + 1; ++t) {
        if (!check_time(t)) continue;
        for (Node** it = &(tm[t].at); *it;) {
            if (!check_node(t, (*it)->i)) {
                delete_node_ptr(it);
                continue;
            }    
            (*it)->val[0][0] = (*it)->val[0][1] = inf;
            (*it)->node[0][0] = (*it)->node[0][1] = prob->n + 2;
            int tj = t - prob->p[(*it)->i];
            if (!check_time(tj)) {
                delete_node_ptr(it);
                continue;
            }      
            for (Node** jt = &(tm[tj].at); *jt;) {
                if (!check_node(tj, (*jt)->i)) {
                    delete_node_ptr(jt);
                    continue;
                }

                if (!check_edge(t, (*jt)->i, (*it)->i)) {
                    jt = &((*jt)->next);
                    continue;
                }
                if ((*jt)->node[0][0] > prob->n + 1) {
                    delete_edge(t, (*jt)->i, (*it)->i);
                    jt = &((*jt)->next);
                    continue;
                } 
                if ((*jt)->node[0][0] == (*it)->i && (*jt)->node[0][1] > prob->n + 1) {
                    delete_edge(t, (*jt)->i, (*it)->i);
                    jt = &((*jt)->next);
                    continue;
                }

                bool first_is_bad = (*jt)->node[0][0] == (*it)->i;
                double value_path = (*jt)->val[0][first_is_bad] + tm[t].f[(*it)->i] - prob->u[(*it)->i];

                if (first_time) {
                    if (t != prob->T + 1 && prob->ub - 1 + prob->eps 
                            < value_path + tm[t].val[1][tm[t].node[1][0] == (*it)->i] + prob->su) {
                        delete_edge(t, (*jt)->i, (*it)->i);
                        jt = &((*jt)->next);
                        continue;
                    }
                } 

                if (value_path < (*it)->val[0][0]) {
                    (*it)->val[0][1] = (*it)->val[0][0];
                    (*it)->node[0][1] = (*it)->node[0][0];
                    (*it)->fnode[0][1] = (*it)->fnode[0][0];
                    (*it)->node_ptr[0][1] = (*it)->node_ptr[0][0];

                    (*it)->val[0][0] = value_path;
                    (*it)->node[0][0] = (*jt)->i;
                    (*it)->fnode[0][0] = first_is_bad;
                    (*it)->node_ptr[0][0] = *jt;
                } else if (value_path < (*it)->val[0][1]) {
                    (*it)->val[0][1] = value_path;
                    (*it)->node[0][1] = (*jt)->i;
                    (*it)->fnode[0][1] = first_is_bad;
                    (*it)->node_ptr[0][1] = *jt;
                }
                jt = &((*jt)->next);
            }   

            if ((*it)->val[0][0] > inf / 1.1 && t != prob->T + 1) {
                delete_node(t, (*it)->i);
                delete_node_ptr(it);
            } else {
                it = &((*it)->next);
            }
        }
    }
    std::vector<int> sequence;
    Node* vno = tm[prob->T + 1].at;
    if (!vno || vno->val[0][0] > inf / 1.1) { [[unlikely]]
        std::cout << "finish for empty path (LR2 - fordward)" << std::endl;
        return {sequence, inf};
    }
    double lagrangian_cost = vno->val[0][0] + prob->su; 
    bool second_node = 0;
    Node* no;
    while (no = vno->node_ptr[0][second_node], no->i > 0) {  
        second_node = vno->fnode[0][second_node];
        sequence.emplace_back(no->i);
        vno = no;
    }

    std::reverse(sequence.begin(), sequence.end());

    return {sequence, lagrangian_cost};
}

std::pair<std::vector<int>, double> solve_LR2_backward(info* prob, T2map* t2m) {
    Tmap* tm = t2m->tm;

    for (int t = 0; t <= prob->T + 1; t++) {
        if (!check_time(t)) continue;
        for (Node* it = tm[t].at; it; it = it->next) {
            if (!check_node(t, it->i)) continue;
            it->val[1][0] = it->val[1][1] = inf;
            it->node[1][0] = it->node[1][1] = prob->n + 2;
        }
    }

    tm[prob->T+1].at->val[1][0] = 0;
    tm[prob->T+1].at->node[1][0] = prob->n + 1;
    tm[prob->T+1].at->fnode[1][0] = 0;
    tm[prob->T+1].at->node_ptr[1][0] = nullptr;
    for (int t = prob->T + 1; t > 0; t--) {
        if (!check_time(t)) continue;
        
        for (Node** it = &(tm[t].at); *it;) {
            if (!check_node(t, (*it)->i)) {
                delete_node_ptr(it);
                continue;
            }

            //if ((*it)->node[1][0] > prob->n + 1) {
            //    delete_node(t, (*it)->i, true);
            //    delete_node(it);
            //    continue;
            //} 

            if (t != prob->T + 1 && (*it)->val[1][(*it)->node[0][0] == (*it)->node[1][0]] + (*it)->val[0][0] + prob->su > prob->ub - 1 + prob->eps) {
                delete_node(t, (*it)->i);
                delete_node_ptr(it);
                continue;
            }
        
            int tj = t - prob->p[(*it)->i];
            for (Node** jt = &(tm[tj].at); *jt;) {
                if (!check_node(tj, (*jt)->i)) {   
                    delete_node_ptr(jt);
                    continue;
                }

                if (!check_edge(t, (*jt)->i, (*it)->i)) {
                    jt = &((*jt)->next);
                    continue;
                }

                if ((*it)->node[1][0] == (*jt)->i && (*it)->node[1][1] > prob->n + 1) {
                    delete_edge(t, (*jt)->i, (*it)->i);
                    jt = &((*jt)->next);
                    continue;
                }

                bool first_is_bad = (*it)->node[1][0] == (*jt)->i;
                double value_path = (*it)->val[1][first_is_bad] + tm[t].f[(*it)->i] - prob->u[(*it)->i];  
                if (prob->ub - 1 + prob->eps < value_path + (*jt)->val[0][(*jt)->node[0][0] == (*it)->i] + prob->su) {
                    delete_edge(t, (*jt)->i, (*it)->i);
                    jt = &((*jt)->next);
                    continue;
                }
                
                if (value_path < (*jt)->val[1][0]) {
                    (*jt)->val[1][1] = (*jt)->val[1][0];
                    (*jt)->node[1][1] = (*jt)->node[1][0];
                    (*jt)->fnode[1][1] = (*jt)->fnode[1][0];
                    (*jt)->node_ptr[1][1] = (*jt)->node_ptr[1][0];
                    
                    (*jt)->val[1][0] = value_path;
                    (*jt)->node[1][0] = (*it)->i;
                    (*jt)->fnode[1][0] = first_is_bad;
                    (*jt)->node_ptr[1][0] = *it;
                } else if (value_path < (*jt)->val[1][1]) {
                    (*jt)->val[1][1] = value_path;
                    (*jt)->node[1][1] = (*it)->i;
                    (*jt)->fnode[1][1] = first_is_bad;
                    (*jt)->node_ptr[1][1] = *it;
                }
                
                jt = &((*jt)->next);
            }

            it = &((*it)->next);
        }
    }
        // Reconstruir secuencia de trabajos
    std::vector<int> sequence;
    auto vno = tm[0].at;
    double lagrangian_cost = vno->val[1][0] + prob->su;
    // Si no hay solución factible
    if (lagrangian_cost > inf / 1.1) {
        std::cout << "finish for empty path (LR2 - backward)" << std::endl;
        return {sequence, inf};
    }

    if (!check_dominance()) {
        std::cout << "finish for infeasible path (LR2 - backward)" << std::endl;
        return {sequence, inf};
    }
    
    // Reconstruir la secuencia
    bool second_node = 0;
    Node* no;
    while (no = vno->node_ptr[1][second_node], no->i > 0 && no->i <= prob->n) {
        second_node = vno->fnode[1][second_node];
        sequence.push_back(no->i);
        vno = no;
    }

    
    return {sequence, lagrangian_cost};
}

bool check_func(int t, Node2m* cnode2, int i) {
    return true;
}

/** 
void modifiers_initialize_l2_forward() {
    add_node(subl_graph->node[0], 0);
    subl_graph->node[0]->j[0] = 0;
    subl_graph->node[0]->j[1] = prob->n + 2;
    subl_graph->node[0]->j[2] = prob->n + 2;
    subl_graph->node[0]->n[0] = subl_graph->node[0];
    for (int t = 0; t < prob->T; t++) {
        if (!check_time(t)) continue;
        if (subl_graph->node[t] == nullptr) {
            delete_time(t);
            continue;
        }
        for (int i = 0; i < prob->n; i++) {
            int ft = t + prob->p[i];
            if (ft > prob->T) continue;


            double g = -prob->u[i] + t2m->tm[t].f[i];
            double v1 = inf, v2 = inf;
            Node2m* pr1 = nullptr, *pr2 = nullptr;
            int ty1 = 0, ty2 = 0;
            Edge2m* e = nullptr;


            for (Node2m* cnode2 = subl_graph->node[ft]; cnode2 != nullptr; cnode2 = cnode2->next) {
                if (!check_edge(t, cnode2->j[0], i)) continue;
                if (!check_func(t, cnode2, i)) {
                    delete_edge(t, cnode2->j[0], i);
                    continue;
                }
                double f;
                int ty;
                if (cnode2->n[0]->j[0] == i) {
                    if (cnode2->n[1] == nullptr) continue;
                    f = cnode2->v[1] + g;
                    ty = 1;
                } else {
                    f = cnode2->v[0] + g;    
                    ty = 0;
                }
                if (f < v1) {       
                    v2 = v1;
                    v1 = f;
                    pr2 = pr1;
                    pr1 = cnode2;
                    ty2 = ty1;
                    ty1 = ty;
                } else if (f < v2) {
                    v2 = f;
                    pr2 = cnode2;
                    ty2 = ty;
                }

                auto e2 = new Edge2m();
                e2->n = cnode2;
                e2->next = e;
                e = e2;
                subl_graph->n_edges++;
            }

             if(e != NULL) {
                add_node(subl_graph->node[ft], i);
                subl_graph->occ[i]++;
                subl_graph->node[ft]->j[0] = (unsigned short) i;
                subl_graph->node[ft]->j[1] = (unsigned short) (prob->n + 2);
                subl_graph->node[ft]->j[2] = (unsigned short) (prob->n + 2);
                subl_graph->node[ft]->v[0] = v1;
                subl_graph->node[ft]->v[1] = v2;
                subl_graph->node[ft]->ty[0] = ty1;
                subl_graph->node[ft]->ty[1] = ty2;
                subl_graph->node[ft]->n[0] = pr1;
                subl_graph->node[ft]->n[1] = pr2;
                subl_graph->node[ft]->e = e;
            } else {
                delete_node(t, i);
            }
        }
    }

    add_node(subl_graph->node[prob->T + 1], prob->n + 1);
    subl_graph->node[prob->T + 1]->j[0] = (unsigned short) N_JOBS;
    subl_graph->node[prob->T + 1]->j[1] = (unsigned short) (N_JOBS + 1);
    subl_graph->node[prob->T + 1]->j[2] = (unsigned short) (N_JOBS + 1);
    subl_graph->node[prob->T + 1]->next = NULL;
    subl_graph->node[prob->T + 1]->e = NULL;
    double f = LARGE_REAL;
    Node2m* pr = nullptr;
    for(Node2m* cnode2 = subl_graph->node[prob->T]; cnode2 != nullptr; cnode2 = cnode2->next) {
        auto e = new Edge2m();
        subl_graph->n_edges++;
        e->n = cnode2;
        e->next = subl_graph->node[prob->T + 1]->e;
        subl_graph->node[prob->T + 1]->e = e;
        if(f > cnode2->v[0]) {
            f = cnode2->v[0];
            pr = cnode2;
        }
    }
    subl_graph->node[prob->T + 1]->v[0] = f;
    subl_graph->node[prob->T + 1]->v[1] = inf;
    subl_graph->node[prob->T + 1]->n[0] = pr;
    subl_graph->node[prob->T + 1]->n[1] = NULL;
    subl_graph->node[prob->T + 1]->ty[0] = 0;
    subl_graph->node[prob->T + 1]->ty[1] = 0;

    return check_dominance();
}**/
