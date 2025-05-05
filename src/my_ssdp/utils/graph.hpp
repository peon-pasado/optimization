#pragma once
#include "../models.hpp"
#include "constants.hpp"
#include "../fixed.hpp"
#include "../ptable.hpp"
#include "benv.hpp"


void problem_free_graph(sips *prob) {
  if (prob && prob->graph) {
    delete prob->graph->copy->bnode;
    prob->graph->copy->bnode = nullptr;
    delete prob->graph->copy->bedge;
    prob->graph->copy->bedge = nullptr;
    ptable_free(prob->graph->copy->ptable);
    free_fixed(prob->graph->copy->fixed);
    delete[] prob->graph->copy->node2;
    delete prob->graph->copy;
    delete prob->graph->bnode;
    prob->graph->bnode = nullptr;
    delete prob->graph->bedge;
    prob->graph->bedge = nullptr;
    delete[] prob->graph->node1;
    delete[] prob->graph->node2;
    ptable_free(prob->graph->ptable);
    free_fixed(prob->graph->fixed);
    delete prob->graph;
    prob->graph = nullptr;
  }
}

void problem_create_graph() {
  problem_free_graph(prob);
  prob->graph = new _graph_t();
  prob->graph->hdir = SIPS_FORWARD;
  prob->graph->copy = new _cgraph_t();
}

double lag2_get_real_memory_in_MB() {
  double mem = (double) (prob->T + 2)*sizeof(_node2m_t *);
  if (!prob->graph->bnode) {
    mem += prob->graph->bnode->used_memory() 
        + prob->graph->bedge->used_memory();
  }
  mem /= (double) (1<<20);
  mem += prec_get_memory_in_MB();
  mem += prob->graph->copy->rmem;
  return mem;
}

double lag2_get_memory_in_MB() {
  double mem;
  mem = (prob->T + 2)*sizeof(_node2m_t *);
  mem += sizeof(_edge2m_t) * prob->graph->n_edges
    + sizeof(_node2m_t) * prob->graph->n_nodes;
  mem /= (1<<20);
  mem += prec_get_memory_in_MB();
  mem += prob->graph->copy->mem;
  return mem;
}

void _lag2_recover_nodes_forward() {
  int t, tt, tt2;
  int i;
  _node2m_t **snode, **node, ***ntable;
  _node2m_t *csnode, **pnode, *cnode;
  _edge2m_t *e, **pe;
  Benv *bnode, *bedge;

  snode = (_node2m_t **) prob->graph->copy->node2m;
  node = new _node2m_t*[prob->T + 2]{};
  bnode = new Benv(sizeof(_node2m_t));
  bedge = new Benv(sizeof(_edge2m_t));

  ntable = new _node2m_t **[prob->pmax + 1]{};
  ntable[0] = new _node2m_t*[(prob->pmax + 1)*(prob->n + 1)]{};

  for(i = 1; i <= prob->pmax; i++) {
    ntable[i] = ntable[i - 1] + prob->n + 1;
  }

  node[prob->graph->Tmin] = cnode = static_cast<_node2m_t*>(bnode->alloc_element());

  memset(cnode, 0, sizeof(_node2m_t));
  cnode->j[0] = snode[prob->graph->Tmin]->j[0];
  cnode->j[1] = snode[prob->graph->Tmin]->j[1];
  cnode->j[2] = snode[prob->graph->Tmin]->j[2];
  cnode->n[0] = cnode;
  memset(ntable[prob->graph->Tmin%(prob->pmax + 1)], 0,(prob->n + 1)*sizeof(_node2m_t *));
  ntable[prob->graph->Tmin%(prob->pmax + 1)][cnode->j[0]] = cnode;

  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax; t++) {
    pnode = node + t;
    tt = t % (prob->pmax + 1);
    memset(ntable[tt], 0, (prob->n + 1)*sizeof(_node2m_t *));

    for (csnode = snode[t]; csnode; csnode = csnode->next) {
      tt2 = (t - prob->sjob[csnode->j[0]]->p)%(prob->pmax + 1);

      *pnode = cnode = static_cast<_node2m_t*>(bnode->alloc_element());
      cnode->j[0] = csnode->j[0];
      cnode->j[1] = csnode->j[1];
      cnode->j[2] = csnode->j[2];
      cnode->ty = csnode->ty;
      cnode->v[0] = csnode->v[0];
      cnode->v[1] = csnode->v[1];

        if(csnode->n[0]) {
	        cnode->n[0] = ntable[tt2][csnode->n[0]->j[0]];
            if(csnode->n[1]) {
                cnode->n[1] = ntable[tt2][csnode->n[1]->j[0]];
            } else {
                cnode->n[1] = nullptr;
            }
        } else {
          cnode->n[1] = cnode->n[0] = nullptr;
        }

        for (pe = &(cnode->e), e = csnode->e; e; e = e->next, pe = &((*pe)->next)) {
            *pe = static_cast<_edge2m_t*>(bedge->alloc_element());
	          (*pe)->n = ntable[tt2][e->n->j[0]];
        }
        *pe = nullptr;
        ntable[tt][cnode->j[0]] = cnode;
        pnode = &(cnode->next);      
    }
    *pnode = nullptr;
  }

  tt2 = prob->graph->Tmax % (prob->pmax + 1);
  csnode = snode[prob->graph->Tmax + 1];
  node[prob->graph->Tmax + 1] = cnode = static_cast<_node2m_t*>(bnode->alloc_element());
  cnode->j[0] = csnode->j[0];
  cnode->j[1] = csnode->j[1];
  cnode->j[2] = csnode->j[2];
  cnode->ty = csnode->ty;
  cnode->v[0] = csnode->v[0];
  cnode->v[1] = csnode->v[1];
  cnode->n[0] = ntable[tt2][csnode->n[0]->j[0]];
  cnode->n[1] = nullptr;
  cnode->next = nullptr;
  for (pe = &(cnode->e), e = csnode->e; e; e = e->next, pe = &((*pe)->next)) {
    *pe = static_cast<_edge2m_t*>(bedge->alloc_element());
    (*pe)->n = ntable[tt2][e->n->j[0]];
  }
  *pe = nullptr;
  prob->graph->node2m = (_node2m_t*) node;
  prob->graph->bnode = bnode;
  prob->graph->bedge = bedge;
  delete[] ntable[0];
  delete[] ntable;
}

int lag2_recover_nodes() {
  if (!prob->graph->copy->node2m) return SIPS_FAIL;
  delete prob->graph->bnode;
  prob->graph->bnode = nullptr;
  delete prob->graph->bedge;
  prob->graph->bedge = nullptr;
  delete[] prob->graph->node2m;
  prob->graph->node2m = nullptr;
  ptable_free(prob->graph->ptable);
  free_fixed(prob->graph->fixed);
  prob->graph->n_nodes = prob->graph->copy->n_nodes;
  prob->graph->n_edges = prob->graph->copy->n_edges;
  prob->graph->Tmin = prob->graph->copy->Tmin;
  prob->graph->Tmax = prob->graph->copy->Tmax;
  prob->graph->direction = prob->graph->copy->direction;
  prob->graph->ptable = duplicate_ptable(prob->graph->copy->ptable);
  prob->graph->fixed = duplicate_fixed(prob->graph->copy->fixed);
  _lag2_recover_nodes_forward();
  return SIPS_OK;
}

void lag2_free() {
  if (prob->graph) {
    delete prob->graph->bedge;
    prob->graph->bedge = nullptr;
    delete prob->graph->bnode;
    prob->graph->bnode = nullptr;
    delete[] prob->graph->node2m;
    prob->graph->node2m = nullptr;
    delete[] prob->graph->node1;
    prob->graph->node1 = nullptr;
    ptable_free(prob->graph->ptable);
    free_fixed(prob->graph->fixed);
    prob->graph->bedge = nullptr;
    prob->graph->bnode = nullptr;
    prob->graph->node2m = nullptr;
    prob->graph->node1 = nullptr;
    prob->graph->ptable = nullptr;
    prob->graph->fixed = nullptr;
    prob->graph->n_nodes = 0;
    prob->graph->n_edges = 0;
  }
}

void lag2_free_copy() {
  if (prob->graph && prob->graph->copy->bnode) {
    delete prob->graph->copy->bedge;
    prob->graph->copy->bedge = nullptr;
    delete prob->graph->copy->bnode;
    prob->graph->copy->bnode = nullptr;
    delete[] prob->graph->copy->node2m;
    prob->graph->copy->node2m = nullptr;
    ptable_free(prob->graph->copy->ptable);
    free_fixed(prob->graph->copy->fixed);
    prob->graph->copy->bedge = nullptr;
    prob->graph->copy->bnode = nullptr;
    prob->graph->copy->node2m = nullptr;
    prob->graph->copy->ptable = nullptr;
    prob->graph->copy->fixed = nullptr;
    prob->graph->copy->n_nodes = 0;
    prob->graph->copy->n_edges = 0;
    prob->graph->copy->Tmin = 0;
    prob->graph->copy->Tmax = 0;
    prob->graph->copy->mem = 0.0;
    prob->graph->copy->rmem = 0.0;
  }
}

int lag2_push_nodes() {
    delete prob->graph->copy->bnode;
    prob->graph->copy->bnode = nullptr;
    delete prob->graph->copy->bedge;
    prob->graph->copy->bedge = nullptr;
    delete[] prob->graph->copy->node2m;
    prob->graph->copy->node2m = nullptr;
    ptable_free(prob->graph->copy->ptable);
    free_fixed(prob->graph->copy->fixed);
    prob->graph->copy->bnode = prob->graph->bnode;
    prob->graph->copy->bedge = prob->graph->bedge;
    prob->graph->copy->node2m = prob->graph->node2m;
    prob->graph->copy->ptable = prob->graph->ptable;
    prob->graph->copy->fixed = prob->graph->fixed;
    prob->graph->copy->direction = prob->graph->direction;
    prob->graph->copy->n_nodes = prob->graph->n_nodes;
    prob->graph->copy->n_edges = prob->graph->n_edges;
    prob->graph->copy->Tmin = prob->graph->Tmin;
    prob->graph->copy->Tmax = prob->graph->Tmax;
    prob->graph->copy->mem = lag2_get_memory_in_MB();
    prob->graph->copy->rmem = lag2_get_real_memory_in_MB();
    prob->graph->bnode = nullptr;
    prob->graph->bedge = nullptr;
    prob->graph->node2m = nullptr;
    prob->graph->ptable = nullptr;
    prob->graph->fixed = nullptr;
    prob->graph->Tmin = 0;
    prob->graph->Tmax = 0;
    prob->graph->n_nodes = 0;
    prob->graph->n_edges = 0;

    return SIPS_OK;
}

int lag2_pop_nodes() {
    delete prob->graph->bnode;
    prob->graph->bnode = nullptr;
    delete prob->graph->bedge;
    prob->graph->bedge = nullptr;
    delete[] prob->graph->node2m;
    prob->graph->node2m = nullptr;
  ptable_free(prob->graph->ptable);
  free_fixed(prob->graph->fixed);

  prob->graph->bnode = prob->graph->copy->bnode;
  prob->graph->bedge = prob->graph->copy->bedge;
  prob->graph->node2m = prob->graph->copy->node2m;
  prob->graph->ptable = prob->graph->copy->ptable;
  prob->graph->fixed = prob->graph->copy->fixed;

  prob->graph->direction = prob->graph->copy->direction;
  prob->graph->n_nodes = prob->graph->copy->n_nodes;
  prob->graph->n_edges = prob->graph->copy->n_edges;
  prob->graph->Tmin = prob->graph->copy->Tmin;
  prob->graph->Tmax = prob->graph->copy->Tmax;

  prob->graph->copy->bnode = nullptr;
  prob->graph->copy->bedge = nullptr;
  prob->graph->copy->node2m = nullptr;
  prob->graph->copy->ptable = nullptr;
  prob->graph->copy->fixed = nullptr;
  prob->graph->copy->Tmin = 0;
  prob->graph->copy->Tmax = 0;
  prob->graph->copy->n_nodes = 0;
  prob->graph->copy->n_edges = 0;
  prob->graph->copy->mem = 0.0;
  prob->graph->copy->rmem = 0.0;

  return SIPS_OK;
}


void _lag2_move_edges_head() {
  int t;
  _node2m_t **node;
  _node2m_t *cnode;
  _edge2m_t *e, *e2, *e3;
  node = (_node2m_t **) prob->graph->node2m;
  for(t = prob->graph->Tmax; t >= prob->graph->Tmin; t--) {
      for(cnode = node[t]; cnode; cnode = cnode->next) {
          for(e = cnode->e; e;) {
              e3 = e->next;
              e2 = e->n->e;
              e->n->e = e;
              e->n = cnode;
              e->next = e2;
              e = e3;
          }
    cnode->e = nullptr;
  }
}
}

void _lag2_move_edges_tail() {
  int t;
  _node2m_t **node;
  _node2m_t *cnode;
  _edge2m_t *e, *e2, *e3;

  node = (_node2m_t **) prob->graph->node2m;
  for(t = prob->graph->Tmin + 1; t <= prob->graph->Tmax + 1; t++) {
      for(cnode = node[t]; cnode; cnode = cnode->next) {
          for(e = cnode->e; e;) {
              e3 = e->next;
              e2 = e->n->e;
              e->n->e = e;
              e->n = cnode;
              e->next = e2;
              e = e3;
          }
          cnode->e = nullptr;
      }
  }
}
