#include "models.h"

_fixed_t *create_fixed() {
  _fixed_t *fixed = new _fixed_t();
  fixed->nhead = fixed->ntail = 0;
  fixed->fhead = fixed->ftail = 0;
  fixed->c = new int[prob->n];
  fixed->job = new _job_t*[prob->n];
  fixed->state = new bit_set(prob->n + 1);
  return fixed;
}

_fixed_t *duplicate_fixed(_fixed_t *src) {
  _fixed_t *dest;

  if(src == nullptr) return nullptr;

  dest = create_fixed();
  copy_fixed(dest, src);

  return dest;
}

void copy_fixed(_fixed_t *dest, _fixed_t *src) {
  if(src == nullptr || dest == nullptr) return;
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

void free_fixed(_fixed_t *fixed) {
  if(fixed != nullptr) {
    delete fixed->state;
    delete[] fixed->c;
    delete[] fixed->job;
    delete fixed;
  }
}
