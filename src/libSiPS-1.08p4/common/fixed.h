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
 *  $Id: fixed.h,v 1.7 2013/05/28 13:27:08 tanaka Rel $
 *  $Revision: 1.7 $
 *  $Date: 2013/05/28 13:27:08 $
 *  $Author: tanaka $
 *
 */
#ifndef FIXED_H
#define FIXED_H
#include "sips_common.h"

_fixed_t *create_fixed(sips *);
_fixed_t *duplicate_fixed(sips *, _fixed_t *);
void copy_fixed(sips *, _fixed_t *, _fixed_t *);
void free_fixed(_fixed_t *);

#define is_fixed(i)						\
  (*(fixed->state + (i)/my_char_bit) & 1<<((i)%my_char_bit))
#define set_fixed(i)						\
  *(fixed->state + (i)/my_char_bit) |= 1<<((i)%my_char_bit)


#endif /* FIXED_H */
