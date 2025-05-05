#pragma once

#include "models.h"
#include "utils_graph.h"
#include <algorithm>
#include <vector>
#include <cstring>
#include <iostream>
#include <limits>

struct ptable_t {
    std::vector<std::vector<short>> inc;
    std::vector<int> pre_psum;
    std::vector<int> suc_psum;
    std::vector<int> occ;    

    ptable_t(int n) {
        inc.resize(n+1, std::vector<short>(n+1, 0));
        pre_psum.resize(n+1, 0);
        suc_psum.resize(n+1, 0);
        occ.resize(n+1, 0);
    }
};

ptable_t* ptable = nullptr;
ptable_t* ptable_copy = nullptr;

inline void set_inc(int x, int y) {
    ptable->inc[x][y] = 1;
    ptable->inc[y][x] = -1;
    ptable->pre_psum[y] += prob->p[x];
    ptable->suc_psum[x] += prob->p[y];
}

struct DomList {
    int j;
    int key;
};

struct TimeList {
    int j;
    int s;
    int e;
};

extern struct info* prob;

void initialize_ptable() {
    if (ptable == nullptr) {
        ptable = new ptable_t(prob->n);
    }
    if (prob->s_before == nullptr) {
        prob->s_before = new int[prob->n + 1];
    }
    if (prob->e_before == nullptr) {
        prob->e_before = new int[prob->n + 1];
    }
    for (int j = 1; j <= prob->n; j++) {
        while (!check_node(prob->s[j], j)) {
            if (prob->s[j] > prob->e[j]) {
                return;
            }
            prob->s[j]++;
        }
        while (!check_node(prob->e[j], j)) prob->e[j]--;
        prob->s_before[j] = prob->s[j];
        prob->e_before[j] = prob->e[j];
    }
}

void copy_ptable(_ptable_t *dest, _ptable_t *src) {
/**     
  if(src == NULL || dest == NULL) {
    return;
  }

  dest->dom_per_t = src->dom_per_t;
  dest->adj_per_t = src->adj_per_t;
  dest->sd_size = src->sd_size;
  dest->n = src->n;
  dest->T = src->T;
  dest->mem = src->mem;

  if(src->adj != NULL) {
    memcpy((void *) dest->adj, (void *) src->adj,
	   (prob->T + 1)*src->adj_per_t);
  }
  if(src->dom != NULL) {
    memcpy((void *) dest->dom, (void *) src->dom,
	   (prob->T + 1)*src->dom_per_t);
  }
  if(src->sd != NULL) {
    memcpy((void *) dest->sd, (void *) src->sd, src->sd_size);
  }
  if(src->inc != NULL) {
    memcpy((void *) dest->inc[0], (void *) src->inc[0], prob->n*prob->n);
  }
  if(src->pre_psum != NULL) {
    memcpy((void *) dest->pre_psum, (void *) src->pre_psum,
	   2*prob->n*sizeof(int));
  }
  if(src->occ != NULL) {
    memcpy((void *) dest->occ, (void *) src->occ,
	   (N_JOBS + 2)*sizeof(unsigned int));
  }
  if(src->window != NULL) {
    memcpy((void *) dest->window, (void *) src->window,
	   N_JOBS*sizeof(_window_t));
  }
  **/
}

void duplicate_ptable() {
    /** 
    int i;
  _ptable_t *dest;

  if(src == nullptr) return nullptr;

  dest = (_ptable_t *) xcalloc(sizeof(_ptable_t), 1);

  if(src->adj != NULL) {
    dest->adj = (char *) xmalloc((prob->T + 1)*src->adj_per_t);
  }
  if(src->dom != NULL) {
    dest->dom = (char *) xmalloc((prob->T + 1)*src->dom_per_t);
  }
  if(src->sd != NULL) {
    dest->sd = (char *) xmalloc(src->sd_size);
  }
  if(src->inc != NULL) {
    dest->inc = (char **) xmalloc(prob->n*sizeof(char *));
    dest->inc[0] = (char *) xmalloc(prob->n*prob->n);
    for(i = 1; i < prob->n; i++) {
      dest->inc[i] = dest->inc[i - 1] + prob->n;
    }
  }
  if(src->pre_psum != NULL) {
    dest->pre_psum = (int *) xmalloc(2*prob->n*sizeof(int));
    dest->suc_psum = dest->pre_psum + prob->n;
  }
  if(src->occ != NULL) {
    dest->occ = (unsigned int *) xmalloc((N_JOBS + 2)*sizeof(unsigned int));
  }
  if(src->window != NULL) {
    dest->window = (_window_t *) xmalloc(N_JOBS*sizeof(_window_t));
  }

  _copy_ptable(prob, dest, src);

  return dest;
    **/
}

bool _update_dominance() {    
    struct JobKey {
        int j;
        int key;
    };
    
    std::vector<JobKey> list(prob->n);
    std::vector<JobKey> list2(prob->n);
    
    for (int i = 0; i < prob->n; i++) {
        int j = i + 1;
        list[i].j = list2[i].j = j;
        list[i].key = -prob->s[j];
        list2[i].key = -(prob->e[j] - prob->p[j]);
    }
    
    std::sort(list.begin(), list.end(), [](const JobKey& a, const JobKey& b) {
        if (a.key != b.key) return a.key < b.key;
        return a.j < b.j;
    });
    
    std::sort(list2.begin(), list2.end(), [](const JobKey& a, const JobKey& b) {
        if (a.key != b.key) return a.key < b.key;
        return a.j < b.j;
    });
    
    int j = 0;
    for (int i = 0; i < prob->n; i++) {
        while (j < prob->n && list[i].key >= list2[j].key) ++j;
        if (j < prob->n) {
            for (int k = j; k < prob->n; k++) {
                if (list2[k].j != list[i].j) {
                    int jk = list2[k].j, ji = list[i].j;
                    if (ptable->inc[jk][ji] == -1) return false;
                    else if (ptable->inc[jk][ji] != 1) set_inc(jk, ji);
                }
            }
        } else {
            break;
        }
    }
    
    return true;
}

static std::vector<TimeList> r_list;
static std::vector<TimeList> d_list;

void update_r_list() {
    if (r_list.empty()) r_list.resize(prob->n);
    for (int i = 0; i < prob->n; i++) {
        int j = i + 1;
        r_list[i].j = j;
        r_list[i].s = prob->s[j] - prob->p[j];
        r_list[i].e = prob->e[j];
    }
    std::sort(r_list.begin(), r_list.end(), [](const TimeList& a, const TimeList& b) {
        if (a.s != b.s) return a.s < b.s;
        if (a.e != b.e) return a.e < b.e;
        return a.j < b.j;
    });
}

void update_d_list() {
    if (d_list.empty()) d_list.resize(prob->n);
    for (int i = 0; i < prob->n; i++) {
        int j = i + 1;
        d_list[i].j = j;
        d_list[i].s = prob->s[j] - prob->p[j];
        d_list[i].e = prob->e[j];
    }
    std::sort(d_list.begin(), d_list.end(), [](const TimeList& a, const TimeList& b) {
        if (a.e != b.e) return a.e < b.e;
        if (a.s != b.s) return a.s > b.s;
        return a.j > b.j;
    });
}

void update_sorted_lists() {
    update_r_list();
    update_d_list();
}

bool _forbidden_time_window() {
    for (int i = 1; i <= prob->n; i++) {
        for (int j = i + 1; j <= prob->n; j++) {      
            if (ptable->inc[i][j] == 0) {
                int tmin_ji = std::max(prob->s[j], prob->e[i] + 1 - prob->p[i]);
                int tmax_ji = std::min(prob->s[i] + prob->p[j], prob->e[j]);
                
                for (int t = tmin_ji; t < tmax_ji; t++) {
                    delete_node(t, j);
                }

                int tmin_ij = std::max(prob->s[i], prob->e[j] + 1 - prob->p[j]);
                int tmax_ij = std::min(prob->s[j] + prob->p[i], prob->e[i]);
                
                for (int t = tmin_ij; t < tmax_ij; t++) {
                    delete_node(t, i);
                }
                continue;
            }
            if (ptable->inc[i][j] == 1) {
                int ns = prob->s[i] + prob->p[j];
                if(prob->s[j] < ns) {
                    prob->s[j] = ns;
                    if(prob->s[j] > prob->e[j]) return false;
                }
                int ne = prob->e[j] - prob->p[j];
                if(prob->e[i] > ne) {
                    prob->e[i] = ne;
                    if(prob->s[i] > prob->e[i]) return false;
                }
            } else if (ptable->inc[j][i] == 1) {
                int ns = prob->s[j] + prob->p[i];
                if (prob->s[i] < ns) {          
                    prob->s[i] = ns;
                    if (prob->s[i] > prob->e[i]) return false;
                }
                int ne = prob->e[i] - prob->p[i];
                if (prob->e[j] > ne) {
                    prob->e[j] = ne;
                    if (prob->s[j] > prob->e[j]) return false;
                }
            }
        }
    }
    
    return true;
}

bool _adjust_by_psum() {
        
    update_r_list();
    
    bool updated = false;
    
    for (int i = 0; i < prob->n; i++) {
        int j = r_list[i].j;
        int p = ptable->pre_psum[j];
        int rmax = r_list[i].s;
        for (int k = 0; k < i; k++) {
            int kj = r_list[k].j;
            if (ptable->inc[kj][j] == 1) {
                rmax = std::max(rmax, r_list[k].s + p);
                p -= prob->p[kj];
            }
        }
        rmax += prob->p[j];
        if (prob->s[j] < rmax) {
            prob->s[j] = rmax;
            updated = true;
            if (prob->s[j] > prob->e[j]) return false;
        }
    }

    if (updated) r_list.clear();

    update_d_list();
    
    updated = false;
    for (int i = prob->n - 1; i >= 0; i--) {
        int j = d_list[i].j;
        int p = ptable->suc_psum[j];
        int rmax = d_list[i].e;
        for (int k = prob->n - 1; k > i; k--) {
            int kj = d_list[k].j;
            if (ptable->inc[j][kj] == 1) {
                rmax = std::min(rmax, d_list[k].e - p);
                p -= prob->p[kj];
            }
        }
        if (prob->e[j] > rmax) {
            prob->e[j] = rmax;
            updated = true;
            if (prob->s[j] > prob->e[j]) return false;
        }
    }
    
    if (updated) d_list.clear();

    return true;
}

bool _not_first() {
    update_d_list();
    std::vector<int> Deltaj(prob->n + 1, 0);
    bool updated = false;
    for (int j = 0; j < prob->n; j++) {
        std::fill(Deltaj.begin(), Deltaj.end(), 0);
        int Sji = 0;
        Deltaj[0] = std::numeric_limits<int>::max();
        for (int i = 0; i < prob->n; i++) {
            if (d_list[j].s + prob->p[d_list[j].j] <= d_list[i].s + prob->p[d_list[i].j]) {
                Sji += prob->p[d_list[i].j];
                Deltaj[i + 1] = std::min(Deltaj[i], d_list[i].e - Sji);
            } else {
                Deltaj[i + 1] = Deltaj[i];
            }
        }
        for (int i = 0; i < prob->n; i++) {
            int rpi = d_list[i].s + prob->p[d_list[i].j];
            int rpj = d_list[j].s + prob->p[d_list[j].j];
            if (rpi < rpj) {
                if (rpi > Deltaj[prob->n] && prob->s[d_list[i].j] < rpj + prob->p[d_list[i].j]) {
                    prob->s[d_list[i].j] = rpj + prob->p[d_list[i].j];
                    updated = true;
                    if (prob->s[d_list[i].j] > prob->e[d_list[i].j]) return false;
                }
            } else if ((rpi > Deltaj[i] || d_list[i].s > Deltaj[prob->n]) 
                      && prob->s[d_list[i].j] < rpj + prob->p[d_list[i].j]) {
                prob->s[d_list[i].j] = rpj + prob->p[d_list[i].j];
                updated = true;
                if (prob->s[d_list[i].j] > prob->e[d_list[i].j]) return false;
            }
        }
    }
    if (updated) d_list.clear();
    return true;
}

bool _not_last() {   
    update_r_list();
    std::vector<int> Deltaj(prob->n + 1, 0);
    bool updated = false;
    for (int j = prob->n - 1; j >= 0; j--) {
        std::fill(Deltaj.begin(), Deltaj.end(), 0);
        int Sji = 0;
        Deltaj[prob->n] = -std::numeric_limits<int>::max();
        for (int i = prob->n - 1; i >= 0; i--) {
            if (r_list[j].e + prob->p[r_list[i].j] >= r_list[i].e + prob->p[r_list[j].j]) {
                Sji += prob->p[r_list[i].j];
            } else {
                Deltaj[i] = Deltaj[i + 1];
            }
        }   
        for (int i = prob->n - 1; i >= 0; i--) {
            int rpi = r_list[i].e - prob->p[r_list[i].j];
            int rpj = r_list[j].e - prob->p[r_list[j].j]; 
            if (rpi > rpj) {
                if (rpi < Deltaj[0] && prob->e[r_list[i].j] > rpj) {         
                    prob->e[r_list[i].j] = rpj;
                    updated = true;
                    if (prob->s[r_list[i].j] > prob->e[r_list[i].j]) return false;
                }
            } else if ((rpi < Deltaj[i + 1] || r_list[i].e < Deltaj[0]) 
                      && prob->e[r_list[i].j] > rpj) {
                prob->e[r_list[i].j] = rpj;
                updated = true;
                if (prob->s[r_list[i].j] > prob->e[r_list[i].j]) return false;
            }
        }
    }
    if (updated) r_list.clear();
    return true;
}

bool _adjust_by_not_first_not_last() {    
    if (!_not_first()) return false;
    if (!_not_last()) return false;    
    return true;
}

bool _edge_finding_head() {
    update_r_list();
    
    bool updated = false;
    std::vector<int> Ci(prob->n, 0);
    
    for (int k = 0; k < prob->n; k++) {
        int P = 0;
        int C = std::numeric_limits<int>::min();
        for (int i = prob->n - 1; i >= 0; i--) {
            if (r_list[i].e <= r_list[k].e) {
                P += prob->p[r_list[i].j];
                C = std::max(C, r_list[i].s + P);
                if (C > r_list[k].e) return false;
            }
            Ci[i] = C;
        }
        
        int H = std::numeric_limits<int>::min();    
        for (int i = 0; i < prob->n; i++) {
            if (r_list[i].e <= r_list[k].e) {
                H = std::max(H, r_list[i].s + P);
                P -= prob->p[r_list[i].j];
            } else {
                int rnew = r_list[i].s - prob->p[r_list[i].j];
                if (r_list[i].s + P + prob->p[r_list[i].j] > r_list[k].e) {
                    rnew = std::max(rnew, Ci[i]);
                }
                if (H + prob->p[r_list[i].j] > r_list[k].e) {
                    rnew = std::max(rnew, C);
                }
                rnew += prob->p[r_list[i].j];
                if (rnew > prob->s[r_list[i].j]) {
                    prob->s[r_list[i].j] = rnew;
                    updated = true;
                    if (prob->s[r_list[i].j] > prob->e[r_list[i].j]) return false;
                }
            }
        }
    }
    if (updated) r_list.clear();
    return true;
}

bool _edge_finding_tail() {
    update_d_list();
    bool updated = false;
    std::vector<int> Ci(prob->n, 0);
    for (int k = prob->n - 1; k >= 0; k--) {
        int P = 0;
        int C = std::numeric_limits<int>::max();
        for (int i = 0; i < prob->n; i++) {
            if (d_list[i].s >= d_list[k].s) {
                P += prob->p[d_list[i].j];
                C = std::min(C, d_list[i].e - P);
                if (C < d_list[k].s) return false;
            }
            Ci[i] = C;
        }
        int H = std::numeric_limits<int>::max();        
        for (int i = prob->n - 1; i >= 0; i--) {
            if (d_list[i].s >= d_list[k].s) {
                H = std::min(H, d_list[i].e - P);
                P -= prob->p[d_list[i].j];
            } else {
                int dnew = d_list[i].e;
                if (d_list[i].e - P - prob->p[d_list[i].j] < d_list[k].s) {
                    dnew = std::min(dnew, Ci[i]);
                }
                if (H - prob->p[d_list[i].j] < d_list[k].s) {
                    dnew = std::min(dnew, C);
                }
                if (dnew < prob->e[d_list[i].j]) {
                    prob->e[d_list[i].j] = dnew;
                    updated = true;
                    if (prob->s[d_list[i].j] > prob->e[d_list[i].j]) return false;
                }
            }
        }
    }
    if (updated) d_list.clear();
    return true;
}

bool _adjust_by_edge_finding() {
    if (!_edge_finding_head()) return false;
    if (!_edge_finding_tail()) return false;    
    return true;
}

bool check_dominance() {
    initialize_ptable();
    if (!_update_dominance()) return false;
    if (!_forbidden_time_window()) return false;
    if (!_adjust_by_psum()) return false;
    if (!_adjust_by_edge_finding()) return false;
    if (!_adjust_by_not_first_not_last()) return false;    
    for (int j = 1; j <= prob->n; j++) {
        if (prob->s[j] > prob->e[j]) return false;
        for (int t = prob->s_before[j]; t < prob->s[j]; t++) delete_node(t, j); 
        for (int t = prob->e[j] + 1; t <= prob->e_before[j]; t++) delete_node(t, j);
    }
    return true;
}