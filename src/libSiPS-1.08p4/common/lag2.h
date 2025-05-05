/*
 * Copyright 2006-2013 Shunji Tanaka.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  $Id: lag2.h,v 1.12 2013/05/28 13:27:14 tanaka Rel $
 *  $Revision: 1.12 $
 *  $Date: 2013/05/28 13:27:14 $
 *  $Author: tanaka $
 *
 */
#ifndef LAG2_H
#define LAG2_H
#include "define.h"
#include "sips_common.h"

struct _edge2m_t {
  struct _node2m_t *n;    /* connected node (tail in FW and head in BW) */
  struct _edge2m_t *next; /* next edge */
};

struct _node2m_t {
  unsigned short j[3];    /* job no */
  unsigned char m;        /* total modifier */
  
  char ty;                /* type (0: best, 1: second best) */
  _real v[2];             /* objective values */
  struct _node2m_t *n[2]; /* prev. nodes realizing the min and second min */

  struct _edge2m_t *e;    /* connected arcs */
  struct _node2m_t *next; /* next node with the same t */
};

typedef struct _edge2m_t _edge2m_t;
typedef struct _node2m_t _node2m_t;

typedef struct {
  int n;                  /* number of total modifiers */
  int an;                 /* number of modifiers to be newly added */
  unsigned char *fl;      /* 1 if job i is assigned a nonzero modifier */
  int *jobs;              /* list of jobs newly assinged a nonzero modifier */
  unsigned char **v;      /* v[j][m]: whether modifier value of job j is m */
  unsigned char *m;       /* modifier value */
} _mod_t;

int lag2_initialize(sips *, _real *, _real, _solution_t *, _real *,
		    unsigned int *);
void lag2_free(sips *);
void lag2_free_copy(sips *);
int lag2_push_nodes(sips *);
int lag2_pop_nodes(sips *);
int lag2_recover_nodes(sips *);

int lag2_add_modifiers_LR2m(sips *, _real *, _real, _solution_t *,
			    _real *, unsigned int *, _mod_t *);
int lag2_add_modifiers_LR2m_without_elimination(sips *, _real *, _real,
						_solution_t *, _real *,
						unsigned int *,	_mod_t *);

int lag2_assign_modifiers(sips *, unsigned char, int, _mod_t *);

_real lag2_get_memory_in_MB(sips *);
_real lag2_get_real_memory_in_MB(sips *);

int lag2_solve_LR2m(sips *, _real *, _real, _solution_t *, _real *,
		    unsigned int *);
int lag2_solve_LR2m_without_elimination(sips *, _real *, _real, _solution_t *,
					_real *, unsigned int *);
int lag2_solve_LR2m_reverse(sips *, _real *, _real);
int lag2_solve_LR2m_reverse_without_elimination(sips *, _real *, _real);
void lag2_reverse(sips *);

#endif /* LAG2_H */
