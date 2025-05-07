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
 *  $Id: bmemory.c,v 1.10 2013/05/30 03:59:02 tanaka Rel $
 *  $Revision: 1.10 $
 *  $Date: 2013/05/30 03:59:02 $
 *  $Author: tanaka $
 *
 */

/*
 * bmemory.c
 *   Memory block manager for the DP graph.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "define.h"
#include "sips_common.h"
#include "bmemory.h"
#include "memory.h"

static int _n_per_block(size_t);

/*
 * create_benv(n)
 *   creates a block memory environment.
 *       n: size of an element
 *
 */
_benv_t *create_benv(size_t n)
{
  _benv_t *benv;

  benv = (_benv_t *) xmalloc(sizeof(_benv_t));
  benv->n = benv->cb = benv->nb = 0;
  benv->size = n;
  benv->n_per_block = _n_per_block(n);
  benv->ptr = NULL;
  benv->bptr = NULL;
  benv->cbptr = NULL;

  return(benv);
}

/*
 * free_benv(benv)
 *   frees a block memory environment.
 *     benv: block memory environment
 *
 */
void free_benv(_benv_t *benv)
{
  if(benv != NULL) {
    benv->cb = 0;
    free_unused_bmemory(benv);

    xfree(benv);
  }
}

/*
 * _n_per_block(n)
 *   computes how many elements can be allocated in one memory block.
 *        n: size of an element
 *
 */
int _n_per_block(size_t n)
{
  return((DP_BLOCKSIZE - 2*(int) sizeof(void *))/(int) n);
}

/*
 * alloc_bmemory(benv)
 *   returns a pointer to a new or reused block.
 *     benv: block memory environment
 *
 */
void *alloc_bmemory(_benv_t *benv)
{
  void *bp, *p;

  if(benv->cbptr == NULL) {
    bp = xmalloc(DP_BLOCKSIZE);
    *((void **) bp) = benv->bptr;
    if(benv->bptr != NULL) {
      *((void **) bp + 1) = *((void **) benv->bptr + 1);
      *((void **) benv->bptr + 1) = bp;
    } else {
      *((void **) bp + 1) = bp;
    }
    benv->bptr = bp;
    benv->nb++;
    benv->cb++;
    p = (void *) ((void **) bp + 2);
  } else {
    p = (void *) ((void **) benv->cbptr + 2);
    benv->cb++;
    if(benv->cb == benv->nb) {
      benv->cbptr = NULL;
    } else {
      benv->cbptr = *((void **) benv->cbptr + 1);
    }
  }

  return(p);
}

/*
 * free_unused_bmemory(benv)
 *   frees unused memory blocks.
 *     benv: block memory environment
 *
 */
void free_unused_bmemory(_benv_t *benv)
{
  void *p, *p2;

  while(benv->cb < benv->nb) {
    p = *((void **) benv->bptr);
    p2 = *((void **) benv->bptr + 1);
    xfree(benv->bptr);
    benv->bptr = p;
    benv->nb--;
    benv->n -= benv->n_per_block;
    if(benv->nb > 0) {
      *((void **) benv->bptr + 1) = p2;
    }
  }

  benv->ptr = NULL;
}

/*
 * free_bmemory(benv)
 *   rewinds a memory block pointer so that allocated memory blocks
 *   can be reused.
 *     benv: block memory environment
 *
 */
void free_bmemory(_benv_t *benv)
{
  if(benv->nb > 0) {
    benv->cbptr = *((void **) benv->bptr + 1);
  } else {
    benv->cbptr = NULL;
  }

  benv->cb = 0;
  benv->n = 0;
  benv->ptr = NULL;
}

/*
 * alloc_memory(benv)
 *   returns a pointer to an element.
 *     benv: block memory environment
 *
 */
void *alloc_memory(_benv_t *benv)
{
  int i;
  void *p;
  char *q;

  if(benv->ptr == NULL) {
    p = alloc_bmemory(benv);
    q = (char *) p + benv->size;
    benv->ptr = (void *) q;
    for(i = 1; i < benv->n_per_block - 1; i++, q += benv->size) {
      *((void **) q) = (void *) (q + benv->size);
    }
    *((void **) q) = NULL;
  } else {
    p = benv->ptr;
    benv->ptr = *((void **) p);
  }

  return(p);
}

/*
 * free_memory(benv, ptr)
 *   releases the element pointed by ptr.
 *     benv: block memory environment
 *
 */
void free_memory(_benv_t *benv, void *ptr)
{
  void *a;

  a = benv->ptr;
  benv->ptr = ptr;
  *((void **) ptr) = a;
}
