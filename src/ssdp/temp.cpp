int subgradient_LR1(sips *prob, _real *lb)
{
  int i;
  int ititer, titer, iter;
  int updated, nupdated, nsiter, pos;
  int ret, ret2;
  unsigned int osq;
  char pflag;
  _real g;
  _real ksi, dnorm;
  _real tk, lk, cl, off, lmax, lmax2;
  unsigned int *o;
  _real *u, *ubest, *d;
  _real *gap_table;
  _solution_t *tmpsol, *csol;

  if(prob->param->titer1 > prob->param->eps) {
    if((_real) prob->n < prob->param->titer1) {
      titer = 1;
    } else {
      g = ((_real) prob->n)/prob->param->titer1;
      titer = (int) g;
      if(((_real) (titer + 1)) - g < prob->param->eps) {
	titer++;
      }
    }
  } else if(prob->param->titer1 < - 1.0 + prob->param->eps) {
    titer = (int) (- prob->param->titer1);
    if(prob->param->titer1 + (_real) (titer + 1) < prob->param->eps) {
      titer++;
    }
  } else {
    titer = 0;
  }

  if(prob->param->ititer1 > prob->param->eps) {
    if((_real) prob->n < prob->param->ititer1) {
      ititer = 1;
    } else {
      g = ((_real) prob->n)/prob->param->ititer1;
      ititer = (int) g;
      if(((_real) (ititer + 1)) - g < prob->param->eps) {
	ititer++;
      }
    }
  } else if(prob->param->ititer1 < - 1.0 + prob->param->eps) {
    ititer = (int) (- prob->param->titer1);
    if(prob->param->ititer1 + (_real) (ititer + 1) < prob->param->eps) {
      ititer++;
    }
  } else {
    ititer = 0;
  }

  if(ititer < titer) {
    ititer = titer;
  }

  o = (unsigned int *) xmalloc(prob->n*sizeof(unsigned int));
  u = (_real *) xcalloc(2*(N_JOBS + 1), sizeof(_real));
  ubest = u + (N_JOBS + 1);
  d = (_real *) xcalloc(prob->n, sizeof(_real));

  if(titer > 0) {
    gap_table = (_real *) xmalloc(titer*sizeof(_real));
  } else {
    gap_table = NULL;
  }

  csol = create_solution(prob);
  tmpsol = create_solution(prob);

  if(is_true(prob->param->warm)) {
    for(i = 0; i < prob->n; i++) {
      u[i] = ubest[i] = prob->sjob[i]->u;
    }
  } else {
    for(i = 0; i < prob->n; i++) {
      u[i] = ubest[i] = prob->param->u0;
    }
  }

  for(i = 0; i < titer; i++) {
    gap_table[i] = - LARGE_REAL;
  }

  iter = updated = nupdated = nsiter = 0;
  pflag = 0;
  lmax = *lb;
  lmax2 = - LARGE_REAL;
  lk = prob->param->initstep1;
  pos = 0;
  ret = SIPS_UNSOLVED;
  dnorm = 1.0;

  while(++iter) {
    pflag = 0;
    off = 0.0;
    for(i = 0; i < prob->n; i++) {
      off += u[i];
    }

    ret2 = lag_solve_LR1(prob, u, ctod(prob->sol->f) - off, tmpsol, &cl, o);
    if(ret2 == SIPS_FAIL || ret2 == SIPS_MEMLIMIT) {
      ret = ret2;
      break;
    }

    cl += off;
    if(cl - lmax > prob->param->eps) {
      updated = nsiter = 0;
      nupdated++;
      pflag = 1;
      lmax = cl;
      memcpy((void *) ubest, (void *) u, prob->n*sizeof(_real));

      if(iter > 1) {
	    lk *= prob->param->expand1;
	    if(lk > prob->param->maxstep1 && lk > prob->param->initstep1) {
	        lk = max(prob->param->maxstep1, prob->param->initstep1);
	    }
      }

    } else if(lmax2 >= cl - prob->param->lbeps) {
      updated++;
    }

    if(iter > 1) {
      lmax2 = max(lmax2, cl);
    }

    if(ctod(prob->sol->f) - lmax < prob->param->lbeps) {
      ret = SIPS_SOLVED;
      break;
    }

    if(ret2 != SIPS_NORMAL) {
      if(ret2 == SIPS_OPTIMAL) {
	    UPDATE_UB(stdout, tmpsol);
      }
      ret = SIPS_SOLVED;
      break;
    }

    if(!titer || is_true(prob->param->skip1)) {
      break;
    }

    if(nupdated <= 1) {
      if(iter > ititer) {
	    break;
      }
    } else if(prob->param->tratio1*(ctod(prob->sol->f) - gap_table[pos])
	      > lmax - gap_table[pos]
	      && prob->param->tratio1*lmax + prob->param->eps
	      > lmax - gap_table[pos]
	      && (nupdated*prob->param->minupdate >= prob->n
		  || nsiter >= MAXNSITER)) {
      break;
    }

    if(iter == 1) {
      lag_solve_LR1_backward(prob, u, ctod(prob->sol->f) - off);
    }

    if(updated >= prob->param->siter1) {
      nsiter += updated;
      updated = 0;
      lk *= prob->param->shrink1;
      if(lk < 1.0e-4) {
	    break;
      }
    }

    gap_table[pos++] = lmax;
    pos %= titer;

    osq = 0;
    for(i = 0; i < prob->n; i++) {
      osq += (1 - (int) o[i])*(1 - (int) o[i]);
    }

    ksi = sqrt((_real) osq/dnorm);
    for(i = 0; i < prob->n; i++) {
      d[i] = (_real) (1 - (int) o[i]) + ksi*d[i];
    }

    dnorm = 0.0;
    for(i = 0; i < prob->n; i++) {
      dnorm += d[i]*d[i];
    }

    if(dnorm < prob->param->eps) {
      dnorm = (_real) osq;
      for(i = 0; i < prob->n; i++) {
	    d[i] = (_real) (1 - (int) o[i]);
      }
    }

    tk = lk*(ctod(prob->sol->f) - cl)/dnorm;
    for(i = 0; i < prob->n; i++) {
      u[i] += tk*d[i];
    }
  }

  if(ret == SIPS_UNSOLVED) {
    if(is_false(prob->param->skip1)) {
      off = 0.0;
      for(i = 0; i < prob->n; i++) {
	    off += ubest[i];
      }
      lag_solve_LR1(prob, ubest, ctod(prob->sol->f) - off, tmpsol, &cl, o);
      cl += off;
    }


    if(ctod(prob->sol->f) - cl < prob->param->lbeps) {
      ret = SIPS_SOLVED;
    } else if(is_true(prob->param->ubupd1)) {
      copy_solution(prob, csol, tmpsol);
      partialdp(prob, u, csol);

      if(prob->param->ls == SIPS_LS_EDYNA) {
	    edynasearch(prob, csol);
      } else if(prob->param->ls == SIPS_LS_DYNA
		|| prob->param->ls == SIPS_LS_COMBINED_A
		|| prob->param->ls == SIPS_LS_COMBINED_B) {
	    dynasearch(prob, csol);
      }

      UPDATE_UB(stdout, csol);

      if(ctod(prob->sol->f) - cl < prob->param->lbeps) {
	    ret = SIPS_SOLVED;
      } else {
	    lag_solve_LR1_backward(prob, ubest, ctod(prob->sol->f) - off);
      }
    } else {
      lag_solve_LR1_backward(prob, ubest, ctod(prob->sol->f) - off);
    }

    lmax = cl;
  }

}