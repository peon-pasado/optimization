#pragma once
#include "models.hpp"

#define is_fixed(i)						\
  (*(fixed->state + (i)/my_char_bit) & 1<<((i)%my_char_bit))
#define set_fixed(i)						\
  *(fixed->state + (i)/my_char_bit) |= 1<<((i)%my_char_bit)


_fixed_t *create_fixed() {
    _fixed_t *fixed = new _fixed_t();
    fixed->nhead = fixed->ntail = 0;
    fixed->fhead = fixed->ftail = 0;
    fixed->c = new int[prob->n];
    fixed->job = new _job_t*[prob->n]{};
    fixed->state = new char[prob->n / 8 + 1]{};
    return fixed;
  }

  void copy_fixed(_fixed_t *dest, _fixed_t *src) {
    if (!src || !dest) return;
    dest->nhead = src->nhead;
    dest->ntail = src->ntail;
    dest->fhead = src->fhead;
    dest->ftail = src->ftail;
    for (int i = 0; i < prob->n; i++) {
      dest->c[i] = src->c[i];
      dest->job[i] = src->job[i];
    }
    dest->state = src->state;
  }
  
  _fixed_t *duplicate_fixed(_fixed_t *src) {
    _fixed_t *dest;
    if (!src) return nullptr;  
    dest = create_fixed();
    copy_fixed(dest, src);
    return dest;
  }
  
  void free_fixed(_fixed_t *fixed) {
    if (fixed) {
      delete fixed->state;
      delete[] fixed->c;
      delete[] fixed->job;
      delete fixed;
    }
  }
  