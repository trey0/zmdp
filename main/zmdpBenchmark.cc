/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-04-27 20:19:22 $

 @file    zmdpBenchmark.cc
 @brief   No brief

 Copyright (c) 2002-2006, Trey Smith.  All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * The software may not be sold or incorporated into a commercial
   product without specific prior written permission.
 * The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 ***************************************************************************/

#include <assert.h>
#include <sys/time.h>
#include <getopt.h>

#include <iostream>
#include <fstream>

#include "Interleave.h"
#include "stdinInterface.h"

#include "solverUtils.h"

using namespace std;
using namespace MatrixUtils;
using namespace zmdp;

struct BenchmarkParams {
  int minOrder;
  int maxOrder;
  int numIterations;

  BenchmarkParams(void);
};

BenchmarkParams::BenchmarkParams(void) {
  minOrder = 0;
  maxOrder = 3;
  numIterations = 100;
}

void doBenchmark(const SolverParams& p, const BenchmarkParams& bp)
{
  init_matrix_utils();

  SolverObjects so;
  constructSolverObjects(so, p);

  Interleave x;
  x.batchTestIncremental(/* numIterations = */ bp.numIterations,
			 so.sim, *so.solver,
			 /* numSteps = */ 251,
			 /* targetPrecision = */ p.targetPrecision,
			 /* minOrder = */ bp.minOrder,
			 /* maxOrder = */ bp.maxOrder,
			 /* ticksPerOrder = */ 10,
			 /* outFileName = */ "inc.plot",
			 /* boundsFileName = */ "bounds.plot",
			 /* simFileName = */ "sim.plot");
  x.printRewards();
}

void usage(const char* cmdName) {
  cerr <<
    "usage: " << cmdName << " OPTIONS <model>\n"
    "  -h or --help           Print this help\n"
    "  --version              Print version information (CFLAGS used at compilation)\n"
    "\n"
    "Solver options:\n"
    "  -s or --search         Specifies search strategy. Valid choices:\n"
    "                           rtdp, lrtdp, hdp, hsvi, frtdp [default: frtdp]\n"
    "  -t or --type           Specifies problem type. Valid choices:\n"
    "                           racetrack, pomdp [default: infer from model filename]\n"
    "  -v or --value          Specifies value function representation. Valid choices\n"
    "                         depend on problem type. With -t pomdp, choices are:\n"
    "                           point, convex [default: convex]\n"
    "                         For other problem types, choices are:\n"
    "                           point [default: point]\n"
    "  -f or --fast           Use fast (but very picky) alternate POMDP parser\n"
    "  -p or --precision      Set target precision in solution quality; run ends when\n"
    "                           target is reached [default: 1e-3]\n"
    "  --weak-heuristic       Avoid spending time generating a good upper bound heuristic\n"
    "                           (only valid for some problems, interpretation depends on\n"
    "                            the problem; e.g. sets h_U = 0 for racetrack)\n"
    "  --lower-bound          Forces zmdpBenchmark to maintain a lower bound and use it for\n"
    "                           action selection, even if it is not used during search.\n"
    "  --upper-bound          Forces zmdpBenchmark to use the upper bound for action selection\n"
    "                           (normally the lower bound is used if it is available).\n"
    "\n"
    "Performance evaluation options:\n"
    "  -i or --iterations     Set number of simulation iterations at each policy\n"
    "                           evaluation epoch [default: 100]\n"
    "  --min-eval             If minEval=k, start logging policy evaluation epochs\n"
    "                           after 10^k seconds of policy improvement [default: 0]\n"
    "  --max-eval             If maxEval=k, run ends after at most 10^k seconds of policy\n"
    "                           improvement [default: 3]\n"
    "\n"
    "Examples:\n"
    "  " << cmdName << " RockSample_4_4.pomdp\n"
    "  " << cmdName << " large-b.racetrack\n"
    "  " << cmdName << " --min-eval -1 RockSample_4_4.pomdp\n"
    "  " << cmdName << " --search lrtdp --value point RockSample_4_4.pomdp\n"
    "  " << cmdName << " -f RockSample_5_7.pomdp\n"
    "\n"
;
  exit(-1);
}

int main(int argc, char **argv) {
  static char shortOptions[] = "hs:t:v:fi:";
  static struct option longOptions[]={
    {"help",          0,NULL,'h'},
    {"search",        1,NULL,'s'},
    {"type",          1,NULL,'t'},
    {"value",         1,NULL,'v'},
    {"version",       0,NULL,'V'},
    {"fast",          0,NULL,'f'},
    {"precision",     1,NULL,'p'},
    {"weak-heuristic",0,NULL,'W'},
    {"lower-bound",   0,NULL,'L'},
    {"upper-bound",   0,NULL,'U'},
    {"iterations",    1,NULL,'i'},
    {"min-eval",      1,NULL,'X'},
    {"max-eval",      1,NULL,'Y'},
    {NULL,0,0,0}
  };

  SolverParams p;
  BenchmarkParams bp;

  // save arguments for debug printout later
  ostringstream outs;
  for (int i=1; i < argc; i++) {
    outs << argv[i] << " ";
  }

  p.cmdName = argv[0];
  while (1) {
    char optchar = getopt_long(argc,argv,shortOptions,longOptions,NULL);
    if (optchar == -1) break;

    switch (optchar) {
    case 'h': // help
      usage(argv[0]);
      break;
    case 'V': // version
      cout << "CFLAGS = " << CFLAGS << endl;
      exit(EXIT_SUCCESS);
      break;
    case 's': // search
      p.setStrategy(optarg);
      break;
    case 't': // type
      p.setProbType(optarg);
      break;
    case 'v': // value
      p.setValueRepr(optarg);
      break;
    case 'f': // fast
      p.useFastParser = true;
      break;
    case 'p': // precision
      p.targetPrecision = atof(optarg);
      break;
    case 'W': // weak-heuristic
      p.useHeuristic = false;
      break;
    case 'L': // lower-bound
      p.forceLowerBound = true;
      break;
    case 'U': // upper-bound
      p.forceUpperBoundActionSelection = true;
      break;
    case 'i': // iterations
      bp.numIterations = atoi(optarg);
      break;
    case 'X': // min-eval
      bp.minOrder = atoi(optarg);
      break;
    case 'Y': // max-eval
      bp.maxOrder = atoi(optarg);
      break;

    case '?': // unknown option
    case ':': // option with missing parameter
      // getopt() prints an informative error message
      cerr << endl;
      usage(argv[0]);
      break;
    default:
      abort(); // never reach this point
    }
  }
  if (argc-optind != 1) {
    cerr << "ERROR: wrong number of arguments (should be 1)" << endl << endl;
    usage(argv[0]);
  }

  p.probName = argv[optind++];
  p.inferMissingValues();

  printf("CFLAGS = %s\n", CFLAGS);
  printf("ARGS = %s\n", outs.str().c_str());
  fflush(stdout);

  doBenchmark(p, bp);

  // signal we are done -- an external batch process that runs zmdpBenchmark
  // can check for completion by polling for existence of this file
  // (which may be easier that using fork()/wait(), depending on the
  // implementation)
  FILE *fp = fopen("/tmp/zmdpBenchmark_done", "w");
  fprintf(fp, "success\n");
  fclose(fp);

  return 0;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2006/04/12 19:23:22  trey
 * added wrtdp support and extra error checking for parameters
 *
 * Revision 1.3  2006/04/10 20:27:05  trey
 * added --lower-bound and --upper-bound args
 *
 * Revision 1.2  2006/04/07 20:15:40  trey
 * solveMDP now uses a strong heuristic by default; improved usage() help
 *
 * Revision 1.1  2006/04/07 19:40:41  trey
 * switched back to a unified binary for all algorithms, turns out it cuts down on code maintenance
 *
 *
 ***************************************************************************/
