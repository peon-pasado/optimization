#pragma once

#include "models.h"
#include <algorithm>


inline int index(int t, int i, int j) {
    return t * N2 + j * N + i;
}

inline bool check_time(int t) {
    return tmap[t].n_nos > 0;
}

//finaliza (t, j)
inline void delete_edge(int t, int i, int j) {
    t2m->tb->set(index(t, i, j));
}

inline bool check_node(int t, int i) {
    return tmap[t].b->test(i) == 0;
}

inline void delete_node(int t, int i) {
    if (check_node(t, i)) {
        int tj = t - prob->p[i];
        if (tj >= 0) {
            t2m->tb->set_range(index(t, 0, i), index(t, prob->n + 1, i));
        }

        //t2m->tb->set_range(index(t, i, 0), index(t, i, prob->n + 1));
        for (int j = 0; j <= prob->n + 1; j++) {
            if (t + prob->p[j] <= prob->e[j]) {
                delete_edge(t + prob->p[j], i, j);
            }
        }   
        
        tmap[t].n_nos -= 1;
        tmap[t].b->set(i);
    }
}

inline void delete_time(int t) {
    tmap[t].n_nos = 0;
    for (int i=0; i<=prob->n + 1; ++i) {
        delete_node(t, i);
    }
}

inline bool check_edge(int t, int i, int j) {
    return t2m->tb->test(index(t, i, j)) == 0;
} 

inline void add_node(Node*& head, int i) {
    total_nodes++;
    if (head == nullptr) {
        head = new Node(i);
        return;
    }
    Node* new_node = new Node(i);
    new_node->next = head;
    head = new_node;
}

inline void delete_node_ptr(Node**& cur) {
    total_nodes--;
    Node* tmp = *cur;
    *cur = (*cur)->next;
    delete tmp;
}

/** 
inline void add_node(Node2m*& head, int i) {
    subl_graph->n_nodes++;
    if (head == nullptr) {
        head = new Node2m(i);
        return;
    }
    Node2m* new_node = new Node2m(i);
    new_node->next = head;
    head = new_node;
}**/

/** 
void initialize_subl_graph() {
    subl_graph->n_nodes = 0;
    subl_graph->n_edges = 0;
    subl_graph->node = new Node2m*[prob->T + 2];
    subl_graph->direction = 0;
    subl_graph->occ = new int[prob->n];
    for (int i = 0; i < prob->n; i++) {
        subl_graph->occ[i] = 0;
    }
    for (int t = 0; t <= prob->T + 1; t++) {
        subl_graph->node[t] = nullptr;
    }
}
**/