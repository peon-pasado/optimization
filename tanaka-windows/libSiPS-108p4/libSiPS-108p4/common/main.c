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
 *  $Id: main.c,v 1.20 2013/06/10 07:25:45 tanaka Rel $
 *  $Revision: 1.20 $
 *  $Date: 2013/06/10 07:25:45 $
 *  $Author: tanaka $
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sips_common.h"

enum args { TypeI, TypeD, TypeT, TypeF };

typedef struct {
  char *name;
  int type;
  int opt;
} array_t;

/*
 * Option array
 */
static array_t opts[] = {
  { "sk1" , TypeT, SIPS_I_SKIPSTAGE1     },
  { "u"   , TypeD, SIPS_R_MULTIPLIER     },
  { "lk1" , TypeD, SIPS_R_INITSTEP1      },
  { "is1" , TypeD, SIPS_R_INITSTEP1      },
  { "lk2" , TypeD, SIPS_R_INITSTEP2      },
  { "is2" , TypeD, SIPS_R_INITSTEP2      },
  { "it1" , TypeD, SIPS_R_INITTERMITER1  },
  { "it2" , TypeD, SIPS_R_INITTERMITER2  },
  { "ms1" , TypeD, SIPS_R_MAXSTEP1       },
  { "ms2" , TypeD, SIPS_R_MAXSTEP2       },
  { "ti1" , TypeD, SIPS_R_TERMITER1      },
  { "si1" , TypeI, SIPS_I_SHRINKITER1    },
  { "ti2" , TypeD, SIPS_R_TERMITER2      },
  { "si2" , TypeI, SIPS_I_SHRINKITER2    },
  { "s1"  , TypeD, SIPS_R_SHRINK1        },
  { "s2"  , TypeD, SIPS_R_SHRINK2        },
  { "e1"  , TypeD, SIPS_R_EXPAND1        },
  { "e2"  , TypeD, SIPS_R_EXPAND2        },
  { "r1"  , TypeD, SIPS_R_TERMRATIO1     },
  { "r2"  , TypeD, SIPS_R_TERMRATIO2     },
  { "ub1" , TypeT, SIPS_I_UBUPDATE1      },
  { "nub1", TypeF, SIPS_I_UBUPDATE1      },
  { "ubi" , TypeI, SIPS_I_UBITERATION    },
  { "nup" , TypeI, SIPS_I_MINUPDATE      },
  { "ls"  , TypeI, SIPS_I_SEARCHTYPE     },
#if SIPSI
  { "dmy" , TypeI, SIPS_I_DUMMYJOBS      },
#endif /* SIPSI */
  { "tb"  , TypeI, SIPS_I_TIEBREAK       },
  { "mem" , TypeI, SIPS_I_MAXMEMORY      },
  { "ssw" , TypeI, SIPS_I_SECSWITCH      },
  { "sir" , TypeD, SIPS_R_SECINIRATIO    },
  { "sr"  , TypeD, SIPS_R_SECRATIO       },
  { "nbi" , TypeF, SIPS_I_BISECTION      },
  { "bi"  , TypeT, SIPS_I_BISECTION      },
  { "mod" , TypeI, SIPS_I_MAXMODIFIER    },
  { "msw" , TypeI, SIPS_I_MODSWITCH      },
  { "zmp" , TypeT, SIPS_I_ZEROMULTIPLIER },
  { "nzmp", TypeF, SIPS_I_ZEROMULTIPLIER },
  { "ub3" , TypeT, SIPS_I_UBUPDATE3      },
  { "nub3", TypeF, SIPS_I_UBUPDATE3      },
  { "tl"  , TypeD, SIPS_R_TIMELIMIT      },
  { "v"   , TypeI, SIPS_I_VERBOSE        },

  { ""    , TypeI, SIPS_NONE             },
};

static int parse_args(sips *, int, char **, char **);
static void _usage(char *);

/*
 * main(argc, argv)
 *   main function.
 *
 */
int main(int argc, char **argv)
{
  int ret;
  sips *prob;
  char *fname = NULL;

  prob = SiPS_create_problem();
  SiPS_set_int_param(prob, SIPS_I_VERBOSE, 2);

  if(parse_args(prob, argc, argv, &fname)) {
    _usage(argv[0]);
    SiPS_free_problem(prob);
    exit(1);
  }

  if(SiPS_read_problem(prob, fname) != SIPS_OK) {
    return(1);
  }

  ret = SiPS_solve(prob);
  SiPS_print_solution(prob);
  SiPS_free_problem(prob);

  return(ret);
}

int parse_args(sips *prob, int ac, char **av, char **f)
{
  int i;
  int opt;
  int ret;

  for(av++ ; (ac > 1) && **av == '-' ; ac--, av++) {
    if(strcmp(*av, "-help") == 0 || strcmp(*av, "-h") == 0) {
      return(1);
    } else {
      for(i = 0; (opt = opts[i].opt) != SIPS_NONE; i++) {
	if(strcmp(*av + 1, opts[i].name) == 0) {
	  if(opts[i].type != TypeT && opts[i].type != TypeF) {
	    if(--ac == 1) {
	      fprintf(stderr, "No argument is specified for %s\n", *av);
	      return(1);
	    }
	  }

	  switch (opts[i].type) {
	  default:
	  case TypeI:
	    ret = SiPS_set_int_param(prob, opt, atoi(*++av));
	    break;
	  case TypeD:
	    ret = SiPS_set_real_param(prob, opt,
				      strtod(*++av, (char **) NULL));
	    break;
	  case TypeT:
	    ret = SiPS_set_int_param(prob, opt, SIPS_TRUE);
	    break;
	  case TypeF:
	    ret = SiPS_set_int_param(prob, opt, SIPS_FALSE);
	    break;
	  }

	  if(ret != SIPS_OK) {
	    fprintf(stderr, "Failed to set %s for -%s\n",
		    *av, opts[i].name);
	    return(1);
	  }

	  break;
	}
      }
      if(opt == SIPS_NONE) {
	fprintf(stderr, "Illegal option %s\n", *av);
	return(1);
      }
    }
  }

  if(f != NULL) {
    if(ac > 1) {
      *f = *av;
    } else {
      *f = NULL;
    }
  }

  return(0);
}

/*
 * _usage(name)
 *   prints out the usage.
 *     name: command name
 *
 */
void _usage(char *name)
{
  fprintf(stdout, "Usage: %s [options] [output file]\n", name);

  fprintf(stdout, "Single-machine scheduling problem solver.\n\n");

  fprintf(stdout, "  -tl  I  time limit.\n");
  fprintf(stdout, "  -v   I  verbose level (0-2).\n");
  fprintf(stdout, "  -sk1    skip Stage 1.\n");
  fprintf(stdout, "  -u   R  initial Lagrangian multipliers.\n");
  fprintf(stdout, "  -is1 R  initial step size for (LR1).\n");
  fprintf(stdout, "  -is2 R  initial step size for (LR2adj).\n");
  fprintf(stdout, "  -it1 R  initial termination counter for (LR1) ((# job)/D).\n");
  fprintf(stdout, "  -it2 R  initial termination counter for (LR2adj) ((# job)/D).\n");
  fprintf(stdout, "  -ms1 R  maximum step size for (LR1).\n");
  fprintf(stdout, "  -ms2 R  maximum step size for (LR2adj).\n");
  fprintf(stdout, "  -ti1 R  termination counter for (LR1) ((# job)/D).\n");
  fprintf(stdout, "  -si1 I  step size shrink counter for (LR1).\n");
  fprintf(stdout, "  -ti2 R  termination counter for (LR2adj) ((# job)/D).\n");
  fprintf(stdout, "  -si2 I  step size shrink counter for (LR2adj).\n");
  fprintf(stdout, "  -s1  R  step size shrink factor for (LR1).\n");
  fprintf(stdout, "  -s2  R  step size shrink factor for (LR2adj).\n");
  fprintf(stdout, "  -e1  R  step size expand factor for (LR1).\n");
  fprintf(stdout, "  -e2  R  step size expand factor for (LR2adj).\n");
  fprintf(stdout, "  -r1  R  gap update ratio for termination for (LR1).\n");
  fprintf(stdout, "  -r2  R  gap update ratio for termination for (LR2adj).\n");
  fprintf(stdout, "  -ub1    upper bound is updated at the end of Stage 1.\n");
  fprintf(stdout, "  -nub1   upper bound is NOT updated at the end of Stage 1.\n");
  fprintf(stdout, "  -ubi I  upper bound is updated once in this number of iterations in Stage 2.\n");
  fprintf(stdout, "  -nup I  minimum number of LB updates before terminating Stage 2 is (n/nup).\n");
  fprintf(stdout, "  -ls  I  local search type.\n");
  fprintf(stdout, "            0: NONE\n");
  fprintf(stdout, "            1: dynasearch\n");
  fprintf(stdout, "            2: dynasearch/edynasearch (A)\n");
  fprintf(stdout, "            3: dynasearch/edynasearch (B)\n");
  fprintf(stdout, "            4: edynasearch\n");
  fprintf(stdout, "  -tb  I  tie breaking rule in adjacent constraints.\n");
  fprintf(stdout, "            0: initial heuristics\n");
  fprintf(stdout, "            1: initial heuristics (reverse order)\n");
  fprintf(stdout, "            2: EDD (+ SPT)\n");
  fprintf(stdout, "            3: SPT (+ EDD)\n");
  fprintf(stdout, "            4: LDD (+ LPT)\n");
  fprintf(stdout, "            5: LPT (+ LDD)\n");
  fprintf(stdout, "            6: WSPT (+ EDD)\n");
  fprintf(stdout, "            7: WLPT (+ LDD)\n");
#ifdef SIPSI
  fprintf(stdout, "            8: release date (+ EDD)\n");
#endif /* SIPSI */
  fprintf(stdout, "  -ssw I  memory size to enter the sectioning mode in Stage 3.\n");
  fprintf(stdout, "  -sir R  initial ratio to increase the tentative upper bound in Stage 3.\n");
  fprintf(stdout, "  -sr  R  ratio to increase the tentative upper bound in Stage 3.\n");
  fprintf(stdout, "  -bi     bisection mode in Stage 3.\n");
  fprintf(stdout, "  -mod I  maximum number of modifiers added in one iteration.\n");
  fprintf(stdout, "  -msw I  memory size for switching modifier strategy.\n");
  fprintf(stdout, "  -zmp    zero multipliers are used simultaneously in Stage 3.\n");
  fprintf(stdout, "  -nzmp   zero multipliers are not used in Stage 3.\n");
  fprintf(stdout, "  -ub3    upper bound is updated at every iteration of Stage 3.\n");
  fprintf(stdout, "  -nub3   upper bound is NOT updated at every iteration of Stage 3.\n");
  fprintf(stdout, "  -mem I  maximum memory size (MB) used for dynamic programming.\n");
#ifdef SIPSI
  fprintf(stdout, "  -dmy I  number of dummy jobs.\n");
#endif /* SIPSI */
  fprintf(stdout, "\n");
}
