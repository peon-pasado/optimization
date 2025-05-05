/*
 * Copyright 2006-2015 Shunji Tanaka.  All rights reserved.
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
 *  $Id: timer.c,v 1.15 2015/01/12 06:50:54 tanaka Rel $
 *  $Revision: 1.15 $
 *  $Date: 2015/01/12 06:50:54 $
 *  $Author: tanaka $
 *
 */
#include <stdio.h>

#include "define.h"
#include "sips_common.h"
#include "timer.h"

/*
 * _timer_start()
 *   starts the timer.
 *
 */
void _timer_start(sips *prob)
{
#ifdef USE_CLOCK
  prob->stime = ((_real) clock())/(_real) CLOCKS_PER_SEC;
#else /* USE_CLOCK */
  struct rusage ru;

  getrusage(RUSAGE_SELF, &ru);
  prob->stime = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.0;
#ifdef INCLUDE_SYSTEM_TIME
  prob->stime += ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1000000.0;
#endif /* INCLUDE_SYSTEM_TIME */
#endif /* USE_CLOCK */
  prob->time = 0.0;
}

/*
 * _get_time()
 *   returns the number of seconds elapsed since when timer_start()
 *   is called.
 *
 */
_real _get_time(sips *prob)
{
#ifdef USE_CLOCK
  prob->time = (_real) clock()/(_real) CLOCKS_PER_SEC;
#else /* USE_CLOCK */
  struct rusage ru;

  getrusage(RUSAGE_SELF, &ru);
  prob->time = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.0;
#ifdef INCLUDE_SYSTEM_TIME
  prob->time += ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1000000.0;
#endif /* INCLUDE_SYSTEM_TIME */
#endif /* USE_CLOCK */
  prob->time -= prob->stime;
  if(prob->time < 0.0) {
    prob->time = 0.0;
  }

  return((_real) prob->time);
}

/*
 * _print_time(prob, fp)
 *   print the number of seconds elapsed since when _timer_start()
 *   is called.
 *        fp: file pointer
 *
 */
void _print_time(sips *prob, FILE *fp)
{
  print_real(fp, 6.4, _get_time(prob));
  fprintf(fp, "\n");
}
