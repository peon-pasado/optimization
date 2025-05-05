#pragma once 
#include "models.hpp"
#include <cstring>
#include <algorithm>

const int my_char_bit = 8;

#define prec_set_adj_tij(t, i, j)					\
  *(ptable->adj + ((t)*ptable->adj_per_t				\
		   + ((i)*ptable->n + (j))/my_char_bit))		\
  |= 1<<(((i)*ptable->n + j)%my_char_bit)
#define prec_get_adj_tij0(t, i, j)					\
  (*(ptable->adj + ((t)*ptable->adj_per_t				\
		    + ((i)*ptable->n + (j))/my_char_bit))		\
   & 1<<(((i)*ptable->n + (j))%my_char_bit))
#define prec_get_adj_tij1(t, i, j)					\
  ((i) < ptable->n							\
   && (*(ptable->adj + ((t)*ptable->adj_per_t				\
			+ ((i)*ptable->n + (j))/my_char_bit))		\
       & 1<<(((i)*ptable->n + (j))%my_char_bit)))
#define prec_get_adj_tij2(t, i, j)					\
  ((j) < ptable->n							\
   && (*(ptable->adj + ((t)*ptable->adj_per_t				\
			+ ((i)*ptable->n + (j))/my_char_bit))		\
       & 1<<(((i)*ptable->n + (j))%my_char_bit)))
#define prec_get_adj_tij(t, i, j)					\
  ((i) < ptable->n && (j) < ptable->n					\
   && (*(ptable->adj + ((t)*ptable->adj_per_t				\
			+ ((i)*ptable->n + (j))/my_char_bit))		\
       & 1<<(((i)*ptable->n + (j))%my_char_bit)))
#define prec_set_dom_ti(t, i)						\
  *(ptable->dom + ((t)*ptable->dom_per_t + (i)/my_char_bit))		\
  |= 1<<((i)%my_char_bit)
#define prec_rev_dom_ti(t, i)						\
  *(ptable->dom + ((t)*ptable->dom_per_t + (i)/my_char_bit))		\
  ^= 1<<((i)%my_char_bit)
#define prec_get_dom_ti(t, i)						\
  (*(ptable->dom + ((t)*ptable->dom_per_t + (i)/my_char_bit))		\
   & 1<<((i)%my_char_bit))
#define prec_set_sd_t(t)						\
  *(ptable->sd + (t)/my_char_bit) |= 1<<((t)%my_char_bit)
#define prec_get_sd_t(t)						\
  (*(ptable->sd + (t)/my_char_bit) & 1<<((t)%my_char_bit))

#define _set_inc(x, y) {				\
    inc[(x)][(y)] = 1;					\
    inc[(y)][(x)] = -1;					\
    ptable->pre_psum[(y)] += prob->sjob[(x)]->p;	\
    ptable->suc_psum[(x)] += prob->sjob[(y)]->p;	\
  }  

  void _copy_ptable(_ptable_t* dest, const _ptable_t* src) {
    dest->dom_per_t = src->dom_per_t;
    dest->adj_per_t = src->adj_per_t;
    dest->sd_size = src->sd_size;
    dest->n = src->n;
    dest->T = src->T;
    dest->mem = src->mem;

    if (src->adj)
        std::copy_n(src->adj, (prob->T + 1) * src->adj_per_t, dest->adj);

    if (src->dom)
        std::copy_n(src->dom, (prob->T + 1) * src->dom_per_t, dest->dom);

    if (src->sd)
        std::memcpy(dest->sd, src->sd, src->sd_size); // por ser binario, mejor no usar copy_n

    if (src->inc)
        std::copy_n(src->inc[0], prob->n * prob->n, dest->inc[0]);

    if (src->pre_psum)
        std::copy_n(src->pre_psum, 2 * prob->n, dest->pre_psum);

    if (src->suc_psum)
        std::copy_n(src->suc_psum, 2 * prob->n, dest->suc_psum);

    if (src->occ)
        std::copy_n(src->occ, prob->n + 2, dest->occ);

    if (src->window)
        std::copy_n(src->window, prob->n, dest->window);
}

void ptable_initialize() {
  _ptable_t* ptable;
  prob->graph->ptable = ptable = new _ptable_t();
  ptable->n = prob->n;
  ptable->sd_size = prob->T/my_char_bit + 1;
  ptable->adj_per_t = 0;
  ptable->dom_per_t = (ptable->n - 1)/my_char_bit + 1;
  ptable->T = prob->T;
  ptable->mem
    = (double) ((prob->T + 1)*ptable->dom_per_t + ptable->sd_size)
    / (1<<20);
  ptable->adj = nullptr;
  ptable->dom = new char[(prob->T + 1)*ptable->dom_per_t]{};
  ptable->sd = new char[ptable->sd_size]{};
  for (int i = 0; i < prob->n; i++) {
    for (int t = 0; t < prob->sjob[i]->p; t++) {
      prec_set_dom_ti(t, i);
    }
  }
}

void _ptable_initialize_window(_ptable_t *ptable) {
    double estimated_memory = (double) (prob->n*sizeof(char *)
                    + prob->n*sizeof(_window_t)
                    + prob->n*prob->n
                    + (2*prob->n + prob->n + 2)*sizeof(int))
      / (1<<20);
  
    ptable->inc = new char*[prob->n]{};
    ptable->inc[0] = new char[prob->n * prob->n]{};
    for (int i = 1; i < prob->n; i++) {
      ptable->inc[i] = ptable->inc[i - 1] + prob->n;
    }
    ptable->pre_psum = new int[2*prob->n]{};
    ptable->suc_psum = ptable->pre_psum + prob->n;
    ptable->occ = (unsigned int *) new unsigned int[prob->n + 2]{};
    ptable->window = new _window_t[prob->n]{};
    for (int i = 0; i < prob->n; i++) {
      ptable->window[i].j = i;
      ptable->window[i].s = prob->graph->Tmin + prob->sjob[i]->p;
      ptable->window[i].e = prob->graph->Tmax;
      for (int t = prob->graph->Tmin + prob->sjob[i]->p; t <= prob->graph->Tmax; t++) {
          if(!prec_get_dom_ti(t, i)) {
              ptable->window[i].s = t;
              break;
          }
      }
      for (int t = prob->graph->Tmax; t > ptable->window[i].s; t--) {
          if(!prec_get_dom_ti(t, i)) {
              ptable->window[i].e = t;
              break;
          }
      }
    }
    ptable->mem += estimated_memory;
  }

void ptable_initialize_adj(_ptable_t *ptable) {
  int t;
  int i, j;
  int nn;
  int f1, f2;
  _solution_t *tmpsol;

  ptable->adj_per_t = (ptable->n * ptable->n - 1)/my_char_bit + 1;
  ptable->mem += (double) ((prob->T + 1) * ptable->adj_per_t) / (1<<20);
  _ptable_initialize_window(prob->graph->ptable);
  ptable->adj = new char[(prob->T + 1)*ptable->adj_per_t]{};
  for(i = 0; i < prob->n; i++) {
    for(t = prob->sjob[i]->p; t <= prob->T; t++) {
      prec_set_adj_tij(t, i, i);
    }
  }

  tmpsol = create_solution();


    for (i = 0; i < prob->n; i++) tmpsol->job[i] = prob->sjob[i];
    tmpsol->n = prob->n;
    sort(tmpsol->job, tmpsol->job + tmpsol->n, [](auto p, auto q) {
        if (p->d != q->d) return p->d < q->d;
        if (p->p != q->p) return p->p < q->p;
        if (p->w != q->w) return p->w < q->w;
        return p->no < q->no;
    });
    for(i = 0; i < prob->n; i++) {
        prob->sjob[tmpsol->job[i]->no]->tno = i;
    }
    free_solution(tmpsol);
    nn = prob->n;
    for(t = 0; t <= prob->T; t++) {
      for(i = 0; i < nn - 1; i++) {
	      for(j = i + 1; j < nn; j++) {
	        if(t < prob->sjob[i]->p + prob->sjob[j]->p) {
            prec_set_adj_tij(t, i, j);
            prec_set_adj_tij(t, j, i);
            continue;
          }

	  f1 = prob->sjob[i]->f[t] + prob->sjob[j]->f[t - prob->sjob[i]->p];
	  f2 = prob->sjob[i]->f[t - prob->sjob[j]->p] + prob->sjob[j]->f[t];
	  /* f1: j->i, f2: i->j */
	  if (f1 < f2) {
	    prec_set_adj_tij(t, i, j);
	  } else if (f1 > f2) {
	    prec_set_adj_tij(t, j, i);
	  } else if (prob->sjob[i]->tno < prob->sjob[j]->tno) {
	    prec_set_adj_tij(t, j, i);
	  } else {
	    prec_set_adj_tij(t, i, j);
	  }
	}
      }
    }
}

_ptable_t *duplicate_ptable(_ptable_t *src) {
  if (!src) return nullptr;

  _ptable_t* dest = new _ptable_t();

  if(src->adj) {
    dest->adj = new char[(prob->T + 1)*src->adj_per_t]{};
  }
  if (src->dom) {
    dest->dom = new char[(prob->T + 1)*src->dom_per_t]{};
  }
  if (src->sd) {
    dest->sd = new char[src->sd_size]{};
  }
  if (src->inc) {
    dest->inc = new char*[prob->n]{};
    dest->inc[0] = new char[prob->n * prob->n]{};
    for (int i = 1; i < prob->n; i++) {
      dest->inc[i] = dest->inc[i - 1] + prob->n;
    }
  }
  if (src->pre_psum) {
    dest->pre_psum = new int[2*prob->n]{};
    dest->suc_psum = dest->pre_psum + prob->n;
  }
  if (src->occ) {
    dest->occ = new unsigned int[prob->n + 2]{};
  }
  if(src->window) {
    dest->window = new _window_t[prob->n]{};
  }
  _copy_ptable(dest, src);
  return dest;
}

void ptable_free_adj(_ptable_t *ptable) {
  if (ptable) {
    delete[] ptable->adj;
    delete[] ptable->sd;
    ptable->adj = nullptr;
    ptable->sd = nullptr;
    ptable->mem
      -= (double) ((ptable->T + 1)*ptable->adj_per_t + ptable->sd_size)
      / (1<<20);
  }
}

void ptable_free(_ptable_t *ptable) {
  if (ptable) {
    delete[] ptable->window;
    delete[] ptable->occ;
    delete[] ptable->pre_psum;
    if (ptable->inc) delete[] ptable->inc[0];
    delete[] ptable->inc;
    delete[] ptable->adj;
    delete[] ptable->sd;
    delete[] ptable->dom;
    delete ptable;
  }
}

double prec_get_memory_in_MB() {
  return prob->graph->ptable?prob->graph->ptable->mem:0.0;
}