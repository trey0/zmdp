/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-04-27 23:07:24 $

 @file    zmdpSolve.cc
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
#include "zmdpCommonTime.h"

#include "solverUtils.h"

using namespace std;
using namespace MatrixUtils;
using namespace zmdp;

struct OutputParams {
  const char* outFileName;
  double timeoutSeconds;

  OutputParams(void);
};

OutputParams::OutputParams(void) {
  outFileName = "out.policy";
  timeoutSeconds = -1;
}

bool userTerminatedG = false;

void doSolve(const SolverParams& p, const OutputParams& op)
{
  StopWatch run;
  init_matrix_utils();

  printf("%05d reading model file and allocating data structures\n",
	 (int) run.elapsedTime());
  SolverObjects so;
  constructSolverObjects(so, p);

  if (!so.bounds->getSupportsPolicyOutput()) {
    cerr << "ERROR: with selected options, policy output is not supported:" << endl;
    cerr << "  try adding '-v convex' and '--lower-bound' options" << endl;
    exit(EXIT_FAILURE);
  }


  // initialize the solver
  printf("%05d calculating initial heuristics\n",
	 (int) run.elapsedTime());
  so.solver->planInit(so.sim->getModel(), p.targetPrecision);
  printf("%05d finished initialization, beginning to improve policy\n",
	 (int) run.elapsedTime());
  
  double lastPrintTime = -1000;
  bool reachedTargetPrecision = false;
  bool reachedTimeout = false;
  int numSolverCalls = 0;
  while (!(reachedTargetPrecision || reachedTimeout || userTerminatedG)) {
    // make a call to the solver
    reachedTargetPrecision =
      so.solver->planFixedTime(so.sim->getModel()->getInitialState(),
			       /* maxTime = */ -1, p.targetPrecision);
    numSolverCalls++;

    // check timeout
    double elapsed = run.elapsedTime();
    if (op.timeoutSeconds > 0 && elapsed >= op.timeoutSeconds) {
      reachedTimeout = true;
    }

    // print a progress update every 10 seconds
    if ((elapsed - lastPrintTime > 10)
	|| reachedTargetPrecision || reachedTimeout || userTerminatedG) {
      ValueInterval intv = so.solver->getValueAt(so.sim->getModel()->getInitialState());
      printf("%05d %6d calls to solver, bounds [%8.4f .. %8.4f], precision %g\n",
	     (int) elapsed, numSolverCalls, intv.l, intv.u, (intv.u - intv.l));
      lastPrintTime = elapsed;
    }
  }

  // say why the run ended
  if (reachedTargetPrecision) {
    printf("%05d terminating run; reached target precision of %g\n",
	   (int) run.elapsedTime(), p.targetPrecision);
  } else if (reachedTimeout) {
    printf("%05d terminating run; passed specified timeout of %g seconds\n",
	   (int) run.elapsedTime(), op.timeoutSeconds);
  } else {
    printf("%05d terminating run; caught SIGINT from user\n",
	   (int) run.elapsedTime());
  }

  // write out a policy
  printf("%05d writing policy to '%s'\n", (int) run.elapsedTime(), op.outFileName);
  so.bounds->writePolicy(op.outFileName);

  printf("%05d done\n", (int) run.elapsedTime());
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
    "  --lower-bound          Forces zmdpSolve to maintain a lower bound and use it for\n"
    "                           action selection, even if it is not used during search.\n"
    "  --upper-bound          Forces zmdpSolve to use the upper bound for action selection\n"
    "                           (normally the lower bound is used if it is available).\n"
    "\n"
    "Policy output options:\n"
    "  -o or --output         Specifies name of policy output file [default: 'out.policy']\n"
    "  --timeout              Specifies a timeout in seconds.  If running time exceeds\n"
    "                           the specified value, zmdpSolve writes out a policy\n"
    "                           and terminates [default: no maximum]\n"
    "\n"
    "Examples:\n"
    "  " << cmdName << " RockSample_4_4.pomdp\n"
    "  " << cmdName << " large-b.racetrack\n"
    "  " << cmdName << " --timeout 60 --output my.policy RockSample_4_4.pomdp\n"
    "  " << cmdName << " --search lrtdp --value point RockSample_4_4.pomdp\n"
    "  " << cmdName << " -f RockSample_5_7.pomdp\n"
    "\n"
;
  exit(-1);
}

int main(int argc, char **argv) {
  static char shortOptions[] = "hs:t:v:fo:";
  static struct option longOptions[]={
    {"help",          0,NULL,'h'},
    {"version",       0,NULL,'V'},
    {"search",        1,NULL,'s'},
    {"type",          1,NULL,'t'},
    {"value",         1,NULL,'v'},
    {"fast",          0,NULL,'f'},
    {"precision",     1,NULL,'p'},
    {"weak-heuristic",0,NULL,'W'},
    {"lower-bound",   0,NULL,'L'},
    {"upper-bound",   0,NULL,'U'},
    {"output",        1,NULL,'o'},
    {"timeout",       1,NULL,'T'},
    {NULL,0,0,0}
  };

  SolverParams p;
  OutputParams op;

#if USE_DEBUG_PRINT
  // save arguments for debug printout later
  ostringstream outs;
  for (int i=1; i < argc; i++) {
    outs << argv[i] << " ";
  }
#endif

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
    case 'o': // output
      op.outFileName = optarg;
      break;
    case 'T': // timeout
      op.timeoutSeconds = atof(optarg);
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

#if USE_DEBUG_PRINT
  printf("CFLAGS = %s\n", CFLAGS);
  printf("ARGS = %s\n", outs.str().c_str());
  fflush(stdout);
#endif

  doSolve(p, op);

  return 0;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
