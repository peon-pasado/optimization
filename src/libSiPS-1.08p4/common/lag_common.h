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
 *  $Id: lag_common.h,v 1.9 2013/05/28 13:27:15 tanaka Rel $
 *  $Revision: 1.9 $
 *  $Date: 2013/05/28 13:27:15 $
 *  $Author: tanaka $
 *
 */
#ifndef LAG_COMMON_H
#define LAG_COMMON_H
#include "define.h"
#include "sips_common.h"

#define ELIMINATED_NODE(i) ((i)->ty & 0x4)
#define ELIMINATE_NODE(i) ((i)->ty |= 0x4)
#define RCONV_START_NODE(i) ((i)->ty &= 0x3)
#define CONV_START_NODE(i) ((i)->ty |= 0x8)
#define CHECK_START_OR_ELIMINATED_NODE(i) ((i)->ty & 0xc)

#ifdef SIPSI
#define IS_REAL_JOB(i) ((i) < prob->n)
#define NOT_REAL_JOB(i) ((i) >= prob->n)
#define IS_DUMMY_JOB(i) ((i) >= prob->n && (i) < N_JOBS)
#else /* SIPSI */
#define IS_REAL_JOB(i) (1)
#define NOT_REAL_JOB(i) (0)
#define IS_DUMMY_JOB(i) (0)
#endif /* SIPSI */
#define NOT_JOB(i) ((i) > N_JOBS)
#define IS_SOURCE_OR_SINK(i) ((i) == N_JOBS)
#define IS_JOB(i) ((i) < N_JOBS)

#define CONV_TYPE(a, b)	((a) | ((b) << 1))
#define UNCONV_TYPE(n) ((int) ((ty & 1)?((n->ty & 0x2) >> 1):(n->ty & 0x1)))

#endif /* LAG_COMMON_H */
