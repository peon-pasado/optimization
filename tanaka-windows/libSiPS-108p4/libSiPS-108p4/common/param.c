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
 *  $Id: param.c,v 1.26 2015/01/12 06:50:44 tanaka Rel $
 *  $Revision: 1.26 $
 *  $Date: 2015/01/12 06:50:44 $
 *  $Author: tanaka $
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "define.h"
#include "sips_common.h"
#include "default.h"
#include "param.h"

/*
 * param_set_default(prob)
 *   resets the parameters by its default values.
 *
 */
void param_set_default(sips *prob)
{
  _param_t *p = prob->param;

  p->ptype       = SIPS_PROB_NONE;
  p->eps         = DEFAULT_EPS;
#ifdef COST_REAL
#ifdef SIPSI
  p->funceps     = DEFAULT_FUNCEPS;
#endif /* SIPSI */
  p->ctype       = SIPS_COST_REAL;
#endif /* COST_REAL */
#ifdef LB_DEBUG
  p->lbeps       = - p->eps;
#else /* LB_DEBUG */
#ifdef COST_REAL
  p->lbeps       = p->eps;
#else /* COST_REAL */
  p->lbeps       = 1.0 - p->eps;
#endif /* COST_REAL */
#endif /* LB_DEBUG */
  p->verbose     = DEFAULT_VERBOSE;
  p->lb          = DEFAULT_LB;
  p->ub          = DEFAULT_UB;
  p->warm        = DEFAULT_WARMSTART;
  p->mem         = DEFAULT_MAXMEM;
  p->tlimit      = DEFAULT_TLIMIT;
#ifdef SIPSI
  p->dummy       = DEFAULT_DUMMY;
#endif /* SIPSI */
  p->dlevel      = DEFAULT_DOMLEVEL;
  p->tiebreak    = DEFAULT_TIEBREAK;
  p->ls          = DEFAULT_LS;
  p->dpsize      = DEFAULT_DPSIZE;
  p->skip1       = DEFAULT_SKIP1;
  p->u0          = DEFAULT_U0;
  p->initstep1   = DEFAULT_INITSTEP1;
  p->maxstep1    = DEFAULT_MAXSTEP1;
  p->ititer1     = DEFAULT_INITTERMITER1;
  p->ititer2     = DEFAULT_INITTERMITER2;
  p->titer1      = DEFAULT_TERMITER1;
  p->siter1      = DEFAULT_SHRINKITER1;
  p->shrink1     = DEFAULT_SHRINK1;
  p->expand1     = DEFAULT_EXPAND1;
  p->tratio1     = DEFAULT_TERMRATIO1;
  p->initstep2   = DEFAULT_INITSTEP2;
  p->maxstep2    = DEFAULT_MAXSTEP2;
  p->ititer1     = DEFAULT_INITTERMITER2;
  p->titer2      = DEFAULT_TERMITER2;
  p->siter2      = DEFAULT_SHRINKITER2;
  p->shrink2     = DEFAULT_SHRINK2;
  p->expand2     = DEFAULT_EXPAND2;
  p->tratio2     = DEFAULT_TERMRATIO2;
  p->ubiter      = DEFAULT_UBITERATION;
  p->ubupd1      = DEFAULT_UBUPDATE1;
  p->ubupd3      = DEFAULT_UBUPDATE3;
  p->minupdate   = DEFAULT_MINUPDATE;
  p->secsw       = DEFAULT_SECSW;
  p->bisec       = DEFAULT_BISECTION;
  p->seciniratio = DEFAULT_SECINIRATIO;
  p->secratio    = DEFAULT_SECRATIO;
  p->modsw       = DEFAULT_MODSWITCH;
  p->mod         = DEFAULT_MOD;
  p->zmplier     = DEFAULT_ZMPLIER;
}

/*
 * SiPS_set_int_param(prob, param, value)
 *   sets an integer parameter
 *    param: parameter
 *    value: value
 *
 */
int SiPS_set_int_param(sips *prob, int param, int value)
{
  _param_t *p;

  null_check(prob);

  p = prob->param;

  switch(param) {
  case SIPS_I_BISECTION:
    if(value < 0 || value > 1) {
      return(SIPS_FAIL);
    }
    p->bisec = (unsigned char) value;
    break;

  case SIPS_I_DOMLEVEL:
    if(value > 6 || value < 0) {
      return(SIPS_FAIL);
    }
    p->dlevel = (unsigned char) value;
    break;

  case SIPS_I_DPSIZE:
    if(value > MAX_DPSIZE || value < 0) {
      return(SIPS_FAIL);
    }
    p->dpsize = (unsigned char) value;
    break;

#ifdef SIPSI
  case SIPS_I_DUMMYJOBS:
    if(value < 0 || value > 1000) {
      return(SIPS_FAIL);
    }
    p->dummy = value;
    break;
#endif /* SIPSI */

  case SIPS_I_MAXMEMORY:
    if(value < 0) {
      return(SIPS_FAIL);
    }
    p->mem = value;
    break;

  case SIPS_I_MAXMODIFIER:
    if(value <= 0 || value > 8) {
      return(SIPS_FAIL);
    }
    p->mod = (unsigned char) value;
    break;

  case SIPS_I_MINUPDATE:
    if(value < 0) {
      return(SIPS_FAIL);
    }
    p->minupdate = value;
    break;

  case SIPS_I_MODSWITCH:
    p->modsw = value;
    break;

  case SIPS_I_NOSTAGE3:
    if(not_true(value) && not_false(value)) {
      return(SIPS_FAIL);
    }
    p->lb = (unsigned char) value;
    break;

  case SIPS_I_UBONLY:
    if(not_true(value) && not_false(value)) {
      return(SIPS_FAIL);
    }
    p->ub = (unsigned char) value;
    break;

  case SIPS_I_SEARCHTYPE:
    if(value < SIPS_LS_NONE || value > SIPS_LS_EDYNA) {
      return(SIPS_FAIL);
    }
    p->ls = (unsigned char) value;
    break;

  case SIPS_I_SECSWITCH:
    p->secsw = value;
    break;

  case SIPS_I_SHRINKITER1:
    p->siter1 = value;
    break;

  case SIPS_I_SHRINKITER2:
    p->siter2 = value;
    break;

  case SIPS_I_SKIPSTAGE1:
    if(not_true(value) && not_false(value)) {
      return(SIPS_FAIL);
    }
    p->skip1 = (unsigned char) value;
    break;

  case SIPS_I_TIEBREAK:
#ifdef SIPSI
    if(value < 0 || value > 8) {
      return(SIPS_FAIL);
    }
#else /* SIPSI */
    if(value < 0 || value > 7) {
      return(SIPS_FAIL);
    }
#endif /* SIPSI */
    p->tiebreak = (unsigned char) value;
    break;

  case SIPS_I_UBITERATION:
    if(value < 0) {
      return(SIPS_FAIL);
    }
    p->ubiter = value;
    break;

  case SIPS_I_UBUPDATE1:
    if(not_true(value) && not_false(value)) {
      return(SIPS_FAIL);
    }
    p->ubupd1 = (unsigned char) value;
    break;

  case SIPS_I_UBUPDATE3:
    if(not_true(value) && not_false(value)) {
      return(SIPS_FAIL);
    }
    p->ubupd3 = (unsigned char) value;
    break;

  case SIPS_I_VERBOSE:
    if(value < 0 || value > 2) {
      return(SIPS_FAIL);
    }
    p->verbose = (unsigned char) value;
    break;

  case SIPS_I_WARMSTART:
    if(not_true(value) && not_false(value)) {
      return(SIPS_FAIL);
    }
    p->warm = (unsigned char) value;
    break;

  case SIPS_I_ZEROMULTIPLIER:
    if(not_true(value) && not_false(value)) {
      return(SIPS_FAIL);
    }
    p->zmplier = (unsigned char) value;
    break;

  default:
    return(SIPS_FAIL);
  }

  return(SIPS_OK);
}

/*
 * SiPS_set_real_param(prob, param, value)
 *   sets an real-valued parameter
 *    param: parameter
 *    value: value
 *
 */
int SiPS_set_real_param(sips *prob, int param, double value)
{
  _param_t *p;

  null_check(prob);

  p = prob->param;

  switch(param) {
  case SIPS_R_EPS:
    if(value <= 0.0) {
      return(SIPS_FAIL);
    }
    p->eps = value;
#ifdef LB_DEBUG
    p->lbeps    = - p->eps;
#else /* LB_DEBUG */
#ifdef COST_REAL
    if(p->ctype == SIPS_COST_INTEGER) {
      p->lbeps    = 1.0 - p->eps;
    } else {
      p->lbeps    = p->eps;
    }
#endif /* COST_REAL */
#endif /* LB_DEBUG */
    break;

#ifdef COST_REAL
#ifdef SIPSI
  case SIPS_R_FUNCEPS:
    if(value <= 0.0) {
      return(SIPS_FAIL);
    }
    p->funceps = value;
    break;
#endif /* SIPSI */
#endif /* COST_REAL */

  case SIPS_R_EXPAND1:
    if(value <= 0.0) {
      return(SIPS_FAIL);
    }
    p->expand1 = value;
    break;

  case SIPS_R_EXPAND2:
    if(value <= 0.0) {
      return(SIPS_FAIL);
    }
    p->expand2 = value;
    break;

  case SIPS_R_INITSTEP1:
    if(value <= 0.0) {
      return(SIPS_FAIL);
    }
    p->initstep1 = value;
    break;

  case SIPS_R_INITSTEP2:
    if(value <= 0.0) {
      return(SIPS_FAIL);
    }
    p->initstep2 = value;
    break;

  case SIPS_R_INITTERMITER1:
    p->ititer1 = value;
    break;

  case SIPS_R_INITTERMITER2:
    p->ititer2 = value;
    break;

  case SIPS_R_MAXSTEP1:
    if(value <= 0.0) {
      return(SIPS_FAIL);
    }
    p->maxstep1 = value;
    break;

  case SIPS_R_MAXSTEP2:
    if(value <= 0.0) {
      return(SIPS_FAIL);
    }
    p->maxstep2 = value;
    break;

  case SIPS_R_MULTIPLIER:
    p->u0 = value;
    break;

  case SIPS_R_SECINIRATIO:
    if(value >= 1.0) {
      return(SIPS_FAIL);
    }
    p->seciniratio = value;
    break;

  case SIPS_R_SECRATIO:
    if(value >= 1.0) {
      return(SIPS_FAIL);
    }
    p->secratio = value;
    break;

  case SIPS_R_SHRINK1:
    if(value <= 0.0 || value > 1.0) {
      return(SIPS_FAIL);
    }
    p->shrink1 = value;
    break;

  case SIPS_R_SHRINK2:
    if(value <= 0.0 || value > 1.0) {
      return(SIPS_FAIL);
    }
    p->shrink2 = value;
    break;

  case SIPS_R_TERMITER1:
    p->titer1 = value;
    break;

  case SIPS_R_TERMITER2:
    p->titer2 = value;
    break;

  case SIPS_R_TERMRATIO1:
    if(value <= 0.0 || value >= 1.0) {
      return(SIPS_FAIL);
    }
    p->tratio1 = value;
    break;

  case SIPS_R_TERMRATIO2:
    if(value <= 0.0 || value >= 1.0) {
      return(SIPS_FAIL);
    }
    p->tratio2 = value;
    break;

  case SIPS_R_TIMELIMIT:
    p->tlimit = value;
    break;

  default:
    return(SIPS_FAIL);
  }

  return(SIPS_OK);
}
