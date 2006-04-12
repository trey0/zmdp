/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-04-12 19:23:22 $

 @file    solveMDP.cc
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

// search strategies
#include "RTDP.h"
#include "LRTDP.h"
#include "HDP.h"
#include "FRTDP.h"
#include "WRTDP.h"

// problem types
#include "RaceTrack.h"
#include "Pomdp.h"
#include "MDPSim.h"
#include "PomdpSim.h"

// value function representations
#include "PointBounds.h"
#include "ConvexBounds.h"

// initialization code
#include "RelaxUBInitializer.h"

using namespace std;
using namespace MatrixUtils;
using namespace zmdp;

struct SolveMDPParams {
  int strategy;
  int probType;
  int valueRepr;
  char *probName;
  int minOrder;
  int maxOrder;
  bool useFastParser;
  int numIterations;
  bool past_options;
  double targetPrecision;
  double minWait;
  bool useInterleave;
  bool useHeuristic;
  bool forceLowerBound;
  bool forceUpperBoundActionSelection;
};

struct EnumEntry {
  const char* key;
  int val;
};

enum StrategiesEnum {
  S_RTDP,
  S_LRTDP,
  S_HDP,
  S_FRTDP,
  S_WRTDP
};

EnumEntry strategiesG[] = {
  {"rtdp",  S_RTDP},
  {"lrtdp", S_LRTDP},
  {"hdp",   S_HDP},
  {"frtdp", S_FRTDP},
  {"wrtdp", S_WRTDP},
  {NULL, -1}
};

enum ProbTypesEnum {
  T_RACETRACK,
  T_POMDP
};

EnumEntry probTypesG[] = {
  {"racetrack", T_RACETRACK},
  {"pomdp",     T_POMDP},
  {NULL, -1}
};

enum ValueReprsEnum {
  V_POINT,
  V_CONVEX
};

EnumEntry valueReprsG[] = {
  {"point",  V_POINT},
  {"convex", V_CONVEX},
  {NULL, -1}
};

void usage(const char* cmdName);

int getEnum(const char* key, EnumEntry* table, const char* cmdName, char opt)
{
  EnumEntry* i = table;
  for (; NULL != i->key; i++) {
    if (0 == strcmp(i->key, key)) {
      return i->val;
    }
  }
  fprintf(stderr, "ERROR: invalid parameter %s for option -%c\n\n", key, opt);
  usage(cmdName);
  exit(EXIT_FAILURE);
}

bool endsWith(const std::string& s,
	      const std::string& suffix)
{
  if (s.size() < suffix.size()) return false;
  return (s.substr(s.size() - suffix.size()) == suffix);
}

void testBatchIncremental(const SolveMDPParams& p)
{
  init_matrix_utils();

  MDP* problem;
  AbstractSim* sim;
  switch (p.probType) {
  case T_RACETRACK:
    problem = new RaceTrack(p.probName);
    sim = new MDPSim(problem);
    break;
  case T_POMDP:
    problem = new Pomdp(p.probName, p.useFastParser);
    sim = new PomdpSim((Pomdp*) problem);
    break;
  default:
    assert(0); // never reach this point
  }

  Solver* solver;
  switch (p.strategy) {
  case S_RTDP:
    solver = new RTDP();
    break;
  case S_LRTDP:
    solver = new LRTDP();
   break;
  case S_HDP:
    solver = new HDP();
    break;
  case S_FRTDP:
    solver = new FRTDP();
    break;
  case S_WRTDP:
    solver = new WRTDP();
    break;
  default:
    assert(0); // never reach this point
  };

  bool keepLowerBound = p.forceLowerBound
    || ((RTDPCore *) solver)->getUseLowerBound();
  IncrementalBounds* bounds;
  switch (p.valueRepr) {
  case V_POINT:
    PointBounds* pb;
    AbstractBound *initLowerBound, *initUpperBound;

    pb = new PointBounds();
    initLowerBound = keepLowerBound ? problem->newLowerBound() : NULL;
    if (p.useHeuristic && T_POMDP != p.probType) {
      initUpperBound = new RelaxUBInitializer(problem);
    } else {
      initUpperBound = problem->newUpperBound();
    }
    pb->setBounds(initLowerBound, initUpperBound, p.forceUpperBoundActionSelection);
    bounds = pb;
    break;
  case V_CONVEX:
    bounds = new ConvexBounds(keepLowerBound, p.forceUpperBoundActionSelection);
    break;
  default:
    assert(0); // never reach this point
  };
  ((RTDPCore*) solver)->setBounds(bounds);

  Interleave x;
  if (p.useInterleave) {
    x.interleave(/* numIterations = */ p.numIterations,
		 sim, *solver,
		 /* numSteps = */ 251,
		 /* targetPrecision = */ p.targetPrecision,
		 /* minWait = */ p.minWait,
		 /* outFileName = */ "scatter.plot",
		 /* boundsFileNameFmt = */ "plots/bounds%04d.plot",
		 /* simFileNameFmt = */ "plots/sim%04d.plot");
  } else {
    x.batchTestIncremental(/* numIterations = */ p.numIterations,
			   sim, *solver,
			   /* numSteps = */ 251,
			   /* targetPrecision = */ p.targetPrecision,
			   /* minOrder = */ p.minOrder,
			   /* maxOrder = */ p.maxOrder,
			   /* ticksPerOrder = */ 10,
			   /* outFileName = */ "inc.plot",
			   /* boundsFileName = */ "bounds.plot",
			   /* simFileName = */ "sim.plot");
  }

  x.printRewards();
}

void usage(const char* cmdName) {
  cerr <<
    "usage: " << cmdName << " OPTIONS <model>\n"
    "\n"
    "Main flags:\n"
    "  -s or --search         Specifies search strategy. Valid choices:\n"
    "                           rtdp, lrtdp, hdp, frtdp, wrtdp [default: frtdp]\n"
    "  -t or --type           Specifies problem type. Valid choices:\n"
    "                           racetrack, pomdp [default: infer from model filename]\n"
    "  -v or --value          Specifies value function representation. Valid choices\n"
    "                         depend on problem type. With -t pomdp, choices are:\n"
    "                           point, convex [default: convex]\n"
    "                         For other problem types, choices are:\n"
    "                           point [default: point]\n"
    "\n"
    "Other flags:\n"
    "  -h or --help           Print this help\n"
    "  --version              Print version information (CFLAGS used at compilation)\n"
    "  -f or --fast           Use fast (but very picky) alternate POMDP parser\n"
    "  -i or --iterations     Set number of simulation iterations at each policy\n"
    "                           evaluation epoch [default: 100]\n"
    "  -p or --precision      Set target precision in solution quality; run ends when\n"
    "                           target is reached [default: 1e-3]\n"
    "  --weak-heuristic       Avoid spending time generating a good upper bound heuristic\n"
    "                           (only valid for some problems, interpretation depends on\n"
    "                            the problem; e.g. sets h_U = 0 for racetrack)\n"
    "  --lower-bound          Forces solveMDP to maintain a lower bound and use it for\n"
    "                           action selection, even if it is not used during search.\n"
    "  --upper-bound          Forces solveMDP to use the upper bound for action selection\n"
    "                           (normally the lower bound is used if it is available).\n"
    "  --min-eval             If minEval=k, start logging policy evaluation epochs\n"
    "                           after 10^k seconds of policy improvement [default: 0]\n"
    "  --max-eval             If maxEval=k, run ends after at most 10^k seconds of policy\n"
    "                           improvement [default: 3]\n"
    "\n"
    "Experimental flags (you probably don't want to use these):\n"
    "  --interleave           Test planner in interleaved mode\n"
    "  -w or --min-wait       Set minimum planning time between actions (for interleaving)\n"
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
  static char shortOptions[] = "hs:t:v:fi:p:w:";
  static struct option longOptions[]={
    {"help",          0,NULL,'h'},
    {"search",        1,NULL,'s'},
    {"type",          1,NULL,'t'},
    {"value",         1,NULL,'v'},
    {"version",       0,NULL,'V'},
    {"fast",          0,NULL,'f'},
    {"iterations",    1,NULL,'i'},
    {"precision",     1,NULL,'p'},
    {"weak-heuristic",0,NULL,'W'},
    {"lower-bound",   0,NULL,'L'},
    {"upper-bound",   0,NULL,'U'},
    {"min-eval",      1,NULL,'X'},
    {"max-eval",      1,NULL,'Y'},
    {"interleave",    1,NULL,'I'},
    {"min-wait",      1,NULL,'w'},
    {NULL,0,0,0}
  };

  SolveMDPParams p;
  p.strategy = S_FRTDP;
  p.probType = -1;
  p.valueRepr = -1;
  p.probName = NULL;
  p.minOrder = -999;
  p.maxOrder = -999;
  p.useFastParser = false;
  p.numIterations = 100;
  p.targetPrecision = 1e-3;
  p.minWait = 0;
  p.useInterleave = false;
  p.useHeuristic = true;
  p.forceLowerBound = false;
  p.forceUpperBoundActionSelection = false;

  ostringstream outs;
  for (int i=1; i < argc; i++) {
    outs << argv[i] << " ";
  }

  while (1) {
    char optchar = getopt_long(argc,argv,shortOptions,longOptions,NULL);
    if (optchar == -1) break;

    switch (optchar) {
    case 'h': // help
      usage(argv[0]);
      break;
    case 's': // search
      p.strategy = getEnum(optarg, strategiesG, argv[0], optchar);
      break;
    case 't': // type
      p.probType = getEnum(optarg, probTypesG, argv[0], optchar);
      break;
    case 'v': // value
      p.valueRepr = getEnum(optarg, valueReprsG, argv[0], optchar);
      break;
    case 'V': // version
      cout << "CFLAGS = " << CFLAGS << endl;
      exit(EXIT_SUCCESS);
      break;
    case 'f': // fast
      p.useFastParser = true;
      break;
    case 'i': // iterations
      p.numIterations = atoi(optarg);
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
    case 'X': // min-eval
      p.minOrder = atoi(optarg);
      break;
    case 'Y': // max-eval
      p.maxOrder = atoi(optarg);
      break;
    case 'I': // interleave
      p.useInterleave = true;
      break;
    case 'w': // min-wait
      p.minWait = atof(optarg);
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

  // fill in default and inferred values
  if (-1 == p.probType) {
    if (endsWith(p.probName, ".racetrack")) {
      p.probType = T_RACETRACK;
    } else if (endsWith(p.probName, ".pomdp")) {
      p.probType = T_POMDP;
    } else {
      fprintf(stderr, "ERROR: couldn't infer problem type from problem filename %s (use -t option)\n\n",
	      p.probName);
      usage(argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  if (-1 == p.valueRepr) {
    if (T_POMDP == p.probType) {
      p.valueRepr = V_CONVEX;
    } else {
      p.valueRepr = V_POINT;
    }
  }
  if (-999 == p.minOrder) {
    p.minOrder = 0; // default
  }
  if (-999 == p.maxOrder) {
    p.maxOrder = 3; // default
  }

  // error check
  if (V_CONVEX == p.valueRepr && T_POMDP != p.probType) {
    fprintf(stderr, "ERROR: '-v convex' can only be used with '-t pomdp'\n\n");
    usage(argv[0]);
  }

  printf("CFLAGS = %s\n", CFLAGS);
  printf("ARGS = %s\n", outs.str().c_str());
  fflush(stdout);

  testBatchIncremental(p);

  // signal we are done -- an external batch process that runs solveMDP
  // can check for completion by polling for existence of this file
  // (which may be easier that using fork()/wait(), depending on the
  // implementation)
  FILE *fp = fopen("/tmp/solveMDP_done", "w");
  fprintf(fp, "success\n");
  fclose(fp);

  return 0;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
