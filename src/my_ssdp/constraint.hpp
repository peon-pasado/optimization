#pragma once
#include "models.hpp"
#include "utils/constants.hpp"
#include "ptable.hpp"
using namespace std;

int constraint_propagation_simple(_window_t *w) {
  _ptable_t *ptable = prob->graph->ptable;
  for (int i = 0; i < prob->n - 1; i++) {
    for (int j = i + 1; j < prob->n; j++) {
      for (int t = max(w[j].s, w[i].e + 1 - prob->sjob[i]->p); t < w[i].s + prob->sjob[j]->p && t <= w[j].e;	t++) {
        prec_set_dom_ti(t, j);						
      }
      for (int t = max(w[i].s, w[j].e + 1 - prob->sjob[j]->p); t < w[j].s + prob->sjob[i]->p && t <= w[i].e;	t++) {
        prec_set_dom_ti(t, i);						
      }	
    }
  }
  return SIPS_NORMAL;
}

int _update_dominance(_window_t *w) {
  int i, j, k;
  char **inc;
  _ptable_t *ptable;
  _list2_t *list, *list2;

  ptable = prob->graph->ptable;
  inc = ptable->inc;

  list = new _list2_t[2 * prob->n]{};
  list2 = list + prob->n;

  for(i = 0; i < prob->n; i++) {
    list[i].j = list2[i].j = i;
    list[i].key = - w[i].s;
    list2[i].key = - w[i].e + prob->sjob[i]->p;
  }
  sort(list, list + prob->n, [](auto x, auto y) {
    if (x.key != y.key) return x.key < y.key;
    return x.j < y.j;
  });
  sort(list2, list2 + prob->n, [](auto x, auto y) {
    if (x.key != y.key) return x.key < y.key;
    return x.j < y.j;
  });

  j = 0;
  for(i = 0; i < prob->n; i++) {
    for(; j < prob->n && list[i].key >= list2[j].key; j++);
    if(j < prob->n) {
      for(k = j; k < prob->n; k++) {
        if(list2[k].j != list[i].j) {
          if(inc[list2[k].j][list[i].j] == -1) {
            delete[] list;
            return SIPS_INFEASIBLE;
          } else if(inc[list2[k].j][list[i].j] != 1) {
            _set_inc(list2[k].j, list[i].j);
          }
        }
      }
    }
  }

  delete[] list;
  return SIPS_NORMAL;
}

int _forbidden_time_window(_window_t *w) {
  int t;
  int i, j;
  int ns, ne;
  char **inc;
  _ptable_t *ptable;

  ptable = prob->graph->ptable;
  inc = ptable->inc;

  for(i = 0; i < prob->n - 1; i++) {
    for(j = i + 1; j < prob->n; j++) {
      if(inc[i][j] == 0) {
        for (t = max(w[j].s, w[i].e + 1 - prob->sjob[i]->p);t < w[i].s + prob->sjob[j]->p && t <= w[j].e;	t++) {
          prec_set_dom_ti(t, j);						
        }
        for(t = max(w[i].s, w[j].e + 1 - prob->sjob[j]->p);	t < w[j].s + prob->sjob[i]->p && t <= w[i].e;	t++) {
          prec_set_dom_ti(t, i);						
        }	
        continue;
      }

      if(inc[i][j] == 1) {
        ns = w[i].s + prob->sjob[j]->p;
        if(w[j].s < ns) {
            w[j].s = ns;
            if(w[j].s > w[j].e) {
                return SIPS_INFEASIBLE;
            }
        }
        ne = w[j].e - prob->sjob[j]->p;
        if(w[i].e > ne) {
            w[i].e = ne;
            if(w[i].s > w[i].e) {
                return SIPS_INFEASIBLE;
            }
	    }
      } else if(inc[j][i] == 1) {
        ns = w[j].s + prob->sjob[i]->p;
        if(w[i].s < ns) {
            w[i].s = ns;
            if(w[i].s > w[i].e) {
                return SIPS_INFEASIBLE;
            }
        }
        ne = w[i].e - prob->sjob[i]->p;
        if(w[j].e > ne) {
            w[j].e = ne;
            if(w[j].s > w[j].e) {
                return SIPS_INFEASIBLE;
            }
        }
      }
    }
  }
  return SIPS_NORMAL;
}

int _adjust_by_psum(_window_t *w, _list_t **r, _list_t **d) {
  int i, j;
  int p, rmax;
  char updated;
  _list_t *list;
  char **inc;
  _ptable_t *ptable;

  ptable = prob->graph->ptable;
  inc = ptable->inc;

  if (!(*r)) {
    *r = list = new _list_t[prob->n]{};
    for(i = 0; i < prob->n; i++) {
      list[i].j = i;
      list[i].s = w[i].s - prob->sjob[i]->p;
      list[i].e = w[i].e;
    }
    sort(list, list+prob->n, [](auto x, auto y) {
        if (x.s != y.s) return x.s < y.s;
        if (x.e != y.e) return x.e < y.e;
        return x.j < y.j;
    });
  } else {
    list = *r;
  }

  updated = false;
  for(i = 0; i < prob->n; i++) {
    p = ptable->pre_psum[list[i].j];
    rmax = list[i].s;
    for(j = 0; j < i; j++) {
      if(inc[list[j].j][list[i].j] == 1) {
	rmax = max(rmax, list[j].s + p);
	p -= prob->sjob[list[j].j]->p;
      }
    }

    rmax += prob->sjob[list[i].j]->p;
    if(w[list[i].j].s < rmax) {
      updated = true;
      w[list[i].j].s = rmax;
      if(w[list[i].j].s > w[list[i].j].e) {
	        return SIPS_INFEASIBLE;
      }
    }
  }

  if (updated) {
    delete[] *r;
    *r = nullptr;
  }

  if (!*d) {
    *d = list = new _list_t[prob->n]{};
    for(i = 0; i < prob->n; i++) {
      list[i].j = i;
      list[i].s = w[i].s - prob->sjob[i]->p;
      list[i].e = w[i].e;
    }
    sort(list, list+prob->n, [](auto x, auto y) {
        if (x.e != y.e) return x.e < y.e;
        if (x.s != y.s) return x.s > y.s;
        return x.j > y.j;
    });
  } else {
    list = *d;
  }

  updated = false;
  for(i = prob->n - 1; i >= 0; i--) {
    p = ptable->suc_psum[list[i].j];
    rmax = list[i].e;
    for(j = prob->n - 1; j > i; j--) {
      if(inc[list[i].j][list[j].j] == 1) {
	rmax = min(rmax, list[j].e - p);
	p -= prob->sjob[list[j].j]->p;
      }
    }

    if(w[list[i].j].e > rmax) {
      updated = true;
      w[list[i].j].e = rmax;
      if(w[list[i].j].s > w[list[i].j].e) {
	return SIPS_INFEASIBLE;
      }
    }
  }

  if (updated) {
    delete[] *d;
    *d = nullptr;
  }

  return SIPS_NORMAL;
}

int _not_first(_window_t *w, _list_t **d) {
    int i, j;
    int Sji;
    int rpi, rpj;
    char updated;
    int *Deltaj;
    _list_t *list;
  
    if(*d == NULL) {
      *d = list = new _list_t[prob->n]{};
      for(i = 0; i < prob->n; i++) {
        list[i].j = i;
        list[i].s = w[i].s - prob->sjob[i]->p;
        list[i].e = w[i].e;
      }
      sort(list, list+prob->n, [](auto x, auto y) {
          if (x.e != y.e) return x.e < y.e;
          if (x.s != y.s) return x.s > y.s;
          return x.j > y.j;
      });  
    } else {
      list = *d;
    }
  
    Deltaj = new int[prob->n + 1];
  
    updated = false;
    for(j = 0; j < prob->n; j++) {
      memset(Deltaj, 0, (prob->n + 1)*sizeof(int));
      Sji = 0;
      Deltaj[0] = inf;
      for(i = 0; i < prob->n; i++) {
        if(list[j].s + prob->sjob[list[j].j]->p
       <= list[i].s + prob->sjob[list[i].j]->p) {
      Sji += prob->sjob[list[i].j]->p;
      Deltaj[i + 1] = min(Deltaj[i], list[i].e - Sji);
        } else {
      Deltaj[i + 1] = Deltaj[i];
        }
      }
  
      for(i = 0; i < prob->n; i++) {
        rpi = list[i].s + prob->sjob[list[i].j]->p;
        rpj = list[j].s + prob->sjob[list[j].j]->p;
        if(rpi < rpj) {
      if(rpi > Deltaj[prob->n]
         && w[list[i].j].s < rpj + prob->sjob[list[i].j]->p) {
        updated = true;
        w[list[i].j].s = rpj + prob->sjob[list[i].j]->p;
        if(w[list[i].j].s > w[list[i].j].e) {
          delete[] Deltaj;
          return SIPS_INFEASIBLE;
        }
      }
        } else if((rpi > Deltaj[i] || list[i].s > Deltaj[prob->n])
          && w[list[i].j].s < rpj + prob->sjob[list[i].j]->p) {
      w[list[i].j].s = rpj + prob->sjob[list[i].j]->p;
      if(w[list[i].j].s > w[list[i].j].e) {
        updated = true;
        delete[] Deltaj;
        return SIPS_INFEASIBLE;
      }
        }
      }
    }
  
    if (updated) {
      delete[] *d;
      *d = nullptr;
    }
    delete[] Deltaj;
    return SIPS_NORMAL;
  }
  
  int _not_last(_window_t *w, _list_t **r)
  {
    int i, j;
    int Sji;
    int rpi, rpj;
    char updated;
    int *Deltaj;
    _list_t *list;
  
    if (!*r) {
      *r = list = new _list_t[prob->n]{};
      for(i = 0; i < prob->n; i++) {
        list[i].j = i;
        list[i].s = w[i].s - prob->sjob[i]->p;
        list[i].e = w[i].e;
      }
      sort(list, list+prob->n, [](auto x, auto y) {
          if (x.s != y.s) return x.s < y.s;
          if (x.e != y.e) return x.e < y.e;
          return x.j < y.j;
      });
    } else {
      list = *r;
    }
    Deltaj = new int[prob->n + 1];
    updated = false;
    for(j = prob->n - 1; j >= 0; j--) {
      memset(Deltaj, 0, (prob->n + 1) * sizeof(int));
      Sji = 0;
      Deltaj[prob->n] = -inf;
      for(i = prob->n - 1; i >= 0; i--) {
        if(list[j].e + prob->sjob[list[i].j]->p
       >= list[i].e + prob->sjob[list[j].j]->p) {
      Sji += prob->sjob[list[i].j]->p;
      Deltaj[i] = max(Deltaj[i + 1], list[i].s + Sji);
        } else {
      Deltaj[i] = Deltaj[i + 1];
        }
      }
  
      for(i = prob->n - 1; i >= 0; i--) {
        rpi = list[i].e - prob->sjob[list[i].j]->p;
        rpj = list[j].e - prob->sjob[list[j].j]->p;
        if(rpi > rpj) {
      if(rpi < Deltaj[0] && w[list[i].j].e > rpj) {
        updated = true;
        w[list[i].j].e = rpj;
        if(w[list[i].j].s > w[list[i].j].e) {
          delete[] Deltaj;
          return SIPS_INFEASIBLE;
        }
      }
        } else if((rpi < Deltaj[i + 1] || list[i].e < Deltaj[0])
          && w[list[i].j].e > rpj) {
      updated = true;
      w[list[i].j].e = rpj;
      if(w[list[i].j].s > w[list[i].j].e) {
        delete[] Deltaj;
        return SIPS_INFEASIBLE;
      }
        }
      }
    }
  
    if (updated) {
      delete[] *r;
      *r = nullptr;
    }
    delete[] Deltaj;
    return SIPS_NORMAL;
  }
  

int _adjust_by_not_first_not_last(_window_t *w,
				  _list_t **r, _list_t **d)
{
  if(_not_first(w, d) == SIPS_INFEASIBLE) {
    return(SIPS_INFEASIBLE);
  }

  if(_not_last(w, r) == SIPS_INFEASIBLE) {
    return(SIPS_INFEASIBLE);
  }

  return SIPS_NORMAL;
}

int _edge_finding_head(_window_t *w, _list_t **r)
{
  int i, k;
  int P, C, H;
  int rnew;
  char updated;
  int *Ci;
  _list_t *list;

  if (!*r) {
    *r = list = new _list_t[prob->n];
    for(i = 0; i < prob->n; i++) {
      list[i].j = i;
      list[i].s = w[i].s - prob->sjob[i]->p;
      list[i].e = w[i].e;
    }
    sort(list, list+prob->n, [](auto x, auto y) {
        if (x.s != y.s) return x.s < y.s;
        if (x.e != y.e) return x.e < y.e;
        return x.j < y.j;
    });
  } else {
    list = *r;
  }

  Ci = new int[prob->n];

  updated = false;
  for(k = 0; k < prob->n; k++) {
    P = 0;
    C = -inf;
    for(i = prob->n - 1; i >= 0; i--) {
      if(list[i].e <= list[k].e) {
        P += prob->sjob[list[i].j]->p;
        C = max(C, list[i].s + P);
        if(C > list[k].e) {
            delete[] Ci;
            return SIPS_INFEASIBLE;
        }
      }
      Ci[i] = C;
    }

    H = -inf;
    for(i = 0; i < prob->n; i++) {
      if(list[i].e <= list[k].e) {
	H = max(H, list[i].s + P);
	P -= prob->sjob[list[i].j]->p;
      } else {
	rnew = w[list[i].j].s - prob->sjob[list[i].j]->p;
	if(list[i].s + P + prob->sjob[list[i].j]->p > list[k].e) {
	  rnew = max(rnew, Ci[i]);
	}
	if(H + prob->sjob[list[i].j]->p > list[k].e) {
	  rnew = max(rnew, C);
	}
	rnew += prob->sjob[list[i].j]->p;
	if(rnew > w[list[i].j].s) {     
	  updated = true;
	  w[list[i].j].s = rnew;
	  if(w[list[i].j].s > w[list[i].j].e) {
        delete[] Ci;
	    return SIPS_INFEASIBLE;
	  }
	}
      }
    }
  }
  if (updated) {
    delete[] *r;
    *r = nullptr;
  }
  delete[] Ci;
  return SIPS_NORMAL;
}

int _edge_finding_tail(_window_t *w, _list_t **d) {
  int i, k;
  int P, C, H;
  int dnew;
  char updated;
  int *Ci;
  _list_t *list;

  if (!*d) {
    *d = list = new _list_t[prob->n]{};
    for(i = 0; i < prob->n; i++) {
      list[i].j = i;
      list[i].s = w[i].s - prob->sjob[i]->p;
      list[i].e = w[i].e;
    }
    sort(list, list+prob->n, [](auto x, auto y) {
        if (x.e != y.e) return x.e < y.e;
        if (x.s != y.s) return x.s > y.s;
        return x.j > y.j;
    });  
  } else {
    list = *d;
  }

  Ci = new int[prob->n]{};

  updated = false;
  for(k = prob->n - 1; k >= 0; k--) {
    P = 0;
    C = inf;
    for(i = 0; i < prob->n; i++) {
      if(list[i].s >= list[k].s) {
	P += prob->sjob[list[i].j]->p;
	C = min(C, list[i].e - P);
	if(C < list[k].s) {
        delete[] Ci;
	  return SIPS_INFEASIBLE;
	}
      }
      Ci[i] = C;
    }

    H = inf;
    for(i = prob->n - 1; i >= 0; i--) {
      if(list[i].s >= list[k].s) {
	H = min(H, list[i].e - P);
	P -= prob->sjob[list[i].j]->p;
      } else {
	dnew = w[list[i].j].e;
	if(list[i].e - P - prob->sjob[list[i].j]->p < list[k].s) {
	  dnew = min(dnew, Ci[i]);
	}
	if(H - prob->sjob[list[i].j]->p < list[k].s) {
	  dnew = min(dnew, C);
	}
	if(dnew < w[list[i].j].e) {
	  updated = true;
	  w[list[i].j].e = dnew;
	  if(w[list[i].j].s > w[list[i].j].e) {
        delete[] Ci;
	    return SIPS_INFEASIBLE;
	  }
	}
      }
    }
  }
  if (updated) {
    delete[] *d;
    *d = nullptr;
  }
  delete[] Ci;
  return SIPS_NORMAL;
}

int _adjust_by_edge_finding(_window_t *w, _list_t **r, _list_t **d) {
  if(_edge_finding_head(w, r) == SIPS_INFEASIBLE) {
    return(SIPS_INFEASIBLE);
  }

  if(_edge_finding_tail(w, d) == SIPS_INFEASIBLE) {
    return(SIPS_INFEASIBLE);
  }

  return(SIPS_NORMAL);
}

int constraint_propagation(_window_t *w) {
    int ret;
    _list_t *r, *d;
    r = d = nullptr;
    ret = SIPS_NORMAL;
    do {
      if((ret = _update_dominance(w)) == SIPS_INFEASIBLE) {
        break;
      }
  
      if((ret = _forbidden_time_window(w)) == SIPS_INFEASIBLE) {
        break;
      }
  
      if((ret = _adjust_by_psum(w, &r, &d)) == SIPS_INFEASIBLE) {
        break;
      }
  
      if((ret = _adjust_by_edge_finding(w, &r, &d)) == SIPS_INFEASIBLE) {
        break;
      }
  
      if((ret = _adjust_by_not_first_not_last(w, &r, &d)) == SIPS_INFEASIBLE) {
        break;
      }
    } while(0);
  
    delete[] r;
    delete[] d;
    return ret;
  }

  int _lag2_check_time_window() {
    int t;
    int i;
    _ptable_t *ptable;
    _fixed_t *fixed;
    _window_t *w;
  
    ptable = prob->graph->ptable;
    fixed = prob->graph->fixed;
  
    for(i = 0; i < prob->n; i++) {
      if(!is_fixed(i) && (ptable->window[i].s > prob->graph->Tmax
        || ptable->window[i].e < prob->graph->Tmin)) {
        return(SIPS_INFEASIBLE);
      }
    }
  
    w = new _window_t[prob->n]{};
    memcpy(w, ptable->window, prob->n*sizeof(_window_t));
  
    if (constraint_propagation(w) == SIPS_INFEASIBLE) {
      delete[] w;
      return SIPS_INFEASIBLE;
    }
  
    for(i = 0; i < prob->n; i++) {
      for(t = ptable->window[i].s; t < w[i].s; t++) {
        prec_set_dom_ti(t, i);
      }
      for(t = ptable->window[i].e; t > w[i].e; t--) {
        prec_set_dom_ti(t, i);
      }
    }
  
    memcpy(ptable->window, w, prob->n*sizeof(_window_t));
    delete[] w;
    return SIPS_NORMAL;
  }