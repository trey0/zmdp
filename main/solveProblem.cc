/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.5 $  $Author: trey $  $Date: 2006-02-14 19:33:07 $

 @file    solveProblem.cc
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

#include <iostream>
#include <fstream>

#include "Interleave.h"
#include "stdinInterface.h"

using namespace std;
using namespace MatrixUtils;
using namespace zmdp;

void usage(const char* cmdName) {
  cerr <<
    "usage: " << cmdName << " OPTIONS " SP_EXTRA_ARGS " [minOrder maxOrder]\n"
    "  -h or --help           Print this help\n"
    "  --version              Print version information (CFLAGS used at compilation)\n"
    "  -f or --fast           Use fast (but very picky) alternate problem parser\n"
    "  -i or --iterations     Set number of simulation iterations (default: 1000)\n"
    "  -p or --precision      Set target precision\n"
    "  --heuristic            Use a non-trivial upper bound heuristic (interpretation\n"
    "                           depends on the problem)\n"
//    "  -n or --no-console     Do not poll stdin for user quit command (for running in background)\n"
    "\n"
    "These options are experimental, you probably don't want to use them:\n"
    "  --interleave           Test planner in interleaved mode\n"
    "  -s or --min-safety     Set minimum safety (for interleaving)\n"
    "  -w or --min-wait       Set minimum planning time between actions (for interleaving)\n"
;
  exit(-1);
}

void testBatchIncremental(string prob_name,
			  int min_order,
			  int max_order,
			  int num_iterations,
			  bool use_fast_parser,
			  double minSafety,
			  double targetPrecision,
			  double minWait,
			  bool useInterleave,
			  bool useHeuristic)
{

  cout << "CFLAGS = " << CFLAGS << endl;

  MDP* problem     = new SP_GENERATE_PROBLEM;
  Solver* solver   = new SP_GENERATE_SOLVER;
  AbstractSim* sim = new SP_GENERATE_SIM;

  solver->setMinSafety( minSafety );

  Interleave x;
  if (useInterleave) {
    x.interleave(/* numIterations = */ num_iterations,
		 sim, *solver,
		 /* numSteps = */ 251,
		 /* targetPrecision = */ targetPrecision,
		 /* minWait = */ minWait,
		 /* outFileName = */ "scatter.plot",
		 /* boundsFileNameFmt = */ "plots/bounds%04d.plot",
		 /* simFileNameFmt = */ "plots/sim%04d.plot");
  } else {
    x.batchTestIncremental(/* numIterations = */ num_iterations,
			   sim, *solver,
			   /* numSteps = */ 251,
			   /* targetPrecision = */ targetPrecision,
			   /* minOrder = */ min_order,
			   /* maxOrder = */ max_order,
			   /* ticksPerOrder = */ 3,
			   /* outFileName = */ "inc.plot",
			   /* boundsFileName = */ "bounds.plot",
			   /* simFileName = */ "sim.plot");
  }

  x.printRewards();
}

int main(int argc, char **argv) {
  init_matrix_utils();

  char *prob_name = NULL;
  int min_order = -999;
  int max_order = -999;
  bool use_fast_parser = false;
  int num_iterations = 1000;
  bool past_options = false;
  double minSafety = -99e+20;
  double targetPrecision = 1e-3;
  double minWait = 0;
  bool useInterleave = false;
  bool useHeuristic = false;

  for (int argi=1; argi < argc; argi++) {
    string args = argv[argi];
    if (args[0] == '-' && !past_options) {
      if (args == "-h" || args == "--help") {
	usage(argv[0]);
      } else if (args == "--version") {
	cout << "CFLAGS = " << CFLAGS << endl;
	exit(EXIT_SUCCESS);
      } else if (args == "-f" || args == "--fast") {
	use_fast_parser = true;
      } else if (args == "-i" || args == "--iterations") {
	if (++argi == argc) {
	  cerr << "ERROR: -i flag without argument" << endl << endl;
	  usage(argv[0]);
	}
	num_iterations = atoi(argv[argi]);
      } else if (args == "--heuristic") {
	useHeuristic = true;
#if 0
      } else if (args == "-n" || args == "--no-console") {
	setPollingEnabled(0);
#endif
      } else if (args == "--interleave") {
	useInterleave = true;
      } else if (args == "-p" || args == "--precision") {
	if (++argi == argc) {
	  cerr << "ERROR: -p flag without argument" << endl << endl;
	  usage(argv[0]);
	}
	targetPrecision = atof(argv[argi]);
      } else if (args == "-s" || args == "--min-safety") {
	if (++argi == argc) {
	  cerr << "ERROR: -s flag without argument" << endl << endl;
	  usage(argv[0]);
	}
	minSafety = atof(argv[argi]);
      } else if (args == "-w" || args == "--min-wait") {
	if (++argi == argc) {
	  cerr << "ERROR: -w flag without argument" << endl << endl;
	  usage(argv[0]);
	}
	minWait = atof(argv[argi]);
      } else if (args == "--") {
	past_options = true;
      } else {
	cerr << "ERROR: unknown option " << args << endl << endl;
	usage(argv[0]);
      }
    } else if (SP_REQUIRE_PROB_NAME && prob_name == NULL) {
      prob_name = argv[argi];
    } else if (-999 == min_order) {
      min_order = atoi(argv[argi]);
    } else if (-999 == max_order) {
      max_order = atoi(argv[argi]);
    } else {
      cerr << "ERROR: too many arguments" << endl << endl;
      usage(argv[0]);
    }
  }

  cerr << "done arg parsing" << endl;

  if (SP_REQUIRE_PROB_NAME && (prob_name == NULL)) {
    cerr << "ERROR: not enough arguments" << endl << endl;
    usage(argv[0]);
  }
  if (-999 == min_order) {
    min_order = 0; // default
  }
  if (-999 == max_order) {
    max_order = 6; // default
  }

  testBatchIncremental((prob_name ? prob_name : ""),
		       min_order, max_order,
		       num_iterations, use_fast_parser, minSafety,
		       targetPrecision, minWait, useInterleave,
		       useHeuristic);

  // signal we are done
  FILE *fp = fopen("/tmp/solveProblem_done", "w");
  fprintf(fp, "success\n");
  fclose(fp);

  return 0;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2006/02/08 20:28:35  trey
 * added --heuristic command line flag
 *
 * Revision 1.3  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.2  2006/01/31 18:13:42  trey
 * removed stdin polling
 *
 * Revision 1.1  2006/01/30 19:27:01  trey
 * initial check-in
 *
 * Revision 1.20  2006/01/28 22:09:21  trey
 * updated to reflect changes in Interleave
 *
 * Revision 1.19  2005/12/06 20:31:07  trey
 * added paostar algorithm
 *
 * Revision 1.18  2005/11/16 21:04:52  trey
 * added epsgreedy support
 *
 * Revision 1.17  2005/11/10 22:10:01  trey
 * reordered initialization steps in order to get better error messages
 *
 * Revision 1.16  2005/11/02 21:03:59  trey
 * added GHSVI
 *
 * Revision 1.15  2005/10/28 03:53:08  trey
 * simplified license
 *
 * Revision 1.14  2005/10/28 02:59:30  trey
 * added copyright header
 *
 *
 ***************************************************************************/
