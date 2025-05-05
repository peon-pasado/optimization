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
 *  $Id: fixed.c,v 1.8 2013/05/28 13:27:07 tanaka Rel $
 *  $Revision: 1.8 $
 *  $Date: 2013/05/28 13:27:07 $
 *  $Author: tanaka $
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "define.h"
#include "sips_common.h"
#include "fixed.h"
#include "memory.h"

/*
 * create_fixed(prob)
 *   allocates memory for a list of fixed jobs.
 *
 */
_fixed_t *create_fixed(sips *prob)
{
  _fixed_t *fixed;

  fixed = (_fixed_t *) xmalloc(sizeof(_fixed_t));
  fixed->nhead = fixed->ntail = 0;
  fixed->fhead = fixed->ftail = ZERO_COST;
  fixed->c = (int *) xcalloc(prob->n, sizeof(int));
  fixed->job = (_job_t **) xcalloc(prob->n, sizeof(_job_t *));
  fixed->state = (char *) xcalloc((prob->n/my_char_bit + 1), 1);

  return(fixed);
}

/*
 * duplicate_fixed(prob, src)
 *   duplicates the list of fixed jobs.
 *      src: source
 *
 */
_fixed_t *duplicate_fixed(sips *prob, _fixed_t *src)
{
  _fixed_t *dest;

  if(src == NULL) {
    return(NULL);
  }

  dest = create_fixed(prob);
  copy_fixed(prob, dest, src);

  return(dest);
}

/*
 * copy_fixed(prob, dest, src)
 *   copies the list of fixed jobs.
 *     dest: destination
 *      src: source
 *
 */
void copy_fixed(sips *prob, _fixed_t *dest, _fixed_t *src)
{
  if(src == NULL || dest == NULL) {
    return;
  }

  dest->nhead = src->nhead;
  dest->ntail = src->ntail;
  dest->fhead = src->fhead;
  dest->ftail = src->ftail;
  memcpy((void *) dest->c, (void *) src->c, prob->n*sizeof(int));
  memcpy((void *) dest->job, (void *) src->job, prob->n*sizeof(_job_t *));
  memcpy((void *) dest->state, (void *) src->state, prob->n/my_char_bit + 1);
}

/*
 * free_fixed(fixed)
 *   frees the allocated memory for the list of fixed jobs.
 *      fixed: list of fixed jobs
 *
 */
void free_fixed(_fixed_t *fixed)
{
  if(fixed != NULL) {
    xfree(fixed->state);
    xfree(fixed->c);
    xfree(fixed->job);
    xfree(fixed);
  }
}
