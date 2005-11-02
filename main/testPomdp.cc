/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.16 $  $Author: trey $  $Date: 2005-11-02 21:03:59 $

 @file    testPomdp.cc
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith
 All rights reserved.

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

#include "Pomdp.h"
#include "HSVI.h"
#include "QMDP.h"
#include "Interleave.h"
#include "stdinInterface.h"
#if USE_GHSVI
#  include "GHSVI.h"
#endif

using namespace std;
using namespace MatrixUtils;
using namespace pomdp;

void usage(void) {
  cerr <<
    "usage: testPomdp OPTIONS <algorithm> <probname> [minOrder maxOrder]\n"
    "  -h or --help           Print this help\n"
    "  --version              Print version information (CFLAGS used at compilation)\n"
    "  -f or --fast           Use fast (but very picky) alternate problem parser\n"
    "  -i or --iterations     Set number of simulation iterations (default: 1000)\n"
    "  -n or --no-console     Do not poll stdin for user quit command (helps when running in background)\n"
    "\n"
    "Available algorithms: hsvi "
#if USE_GHSVI
    "ghsvi "
#endif
    "qmdp\n"
    "\n"
    "These options are experimental, you probably don't want to use them:\n"
    "  --interleave           Test planner in interleaved mode\n"
    "  -p or --min-precision  Set minimum precision (for interleaving)\n"
    "  -s or --min-safety     Set minimum safety (for interleaving)\n"
    "  -w or --min-wait       Set minimum planning time between actions (for interleaving)\n"
;
  exit(-1);
}

void testBatchIncremental(string algorithm,
			  string prob_name,
			  int min_order,
			  int max_order,
			  int num_iterations,
			  bool use_fast_parser,
			  double minSafety,
			  double minPrecision,
			  double minWait,
			  bool useInterleave)
{
  Pomdp problem;

  cout << "CFLAGS = " << CFLAGS << endl;

  problem.readFromFile( prob_name, use_fast_parser );

  Solver* solver;
  if (algorithm == "hsvi") {
    solver = new HSVI();
  }
#if USE_GHSVI
  else if (algorithm == "ghsvi") {
    solver = new GHSVI();
  }
#endif
  else if (algorithm == "qmdp") {
    solver = new QMDP();
  }
  else {
    cerr << "ERROR: unknown algorithm " << algorithm << endl << endl;
    usage();
    return;
  }

  solver->setMinSafety( minSafety );

  //string prefix = "/tmp/";
  string prefix = "";
  Interleave x;
  if (useInterleave) {
    x.interleave(/* numIterations = */ num_iterations,
		 &problem, *solver,
		 /* numSteps = */ 251,
		 /* minPrecision = */ minPrecision,
		 /* minWait = */ minWait,
		 /* outFileName = */ "scatter.plot",
		 /* boundsFileNameFmt = */ "plots/bounds%04d.plot",
		 /* simFileNameFmt = */ "plots/sim%04d.plot");
  } else {
    x.batchTestIncremental(/* numIterations = */ num_iterations,
			   &problem, *solver,
			   /* numSteps = */ 251,
			   /* minPrecision = */ minPrecision,
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

  char *algorithm = NULL;
  char *prob_name = NULL;
  int min_order = -999;
  int max_order = -999;
  bool use_fast_parser = false;
  int num_iterations = 1000;
  bool past_options = false;
  double minSafety = -99e+20;
  double minPrecision = 1e-10;
  double minWait = 0;
  bool useInterleave = false;

  for (int argi=1; argi < argc; argi++) {
    string args = argv[argi];
    if (args[0] == '-' && !past_options) {
      if (args == "-h" || args == "--help") {
	usage();
      } else if (args == "--version") {
	cout << "CFLAGS = " << CFLAGS << endl;
	exit(EXIT_SUCCESS);
      } else if (args == "-f" || args == "--fast") {
	use_fast_parser = true;
      } else if (args == "-i" || args == "--iterations") {
	if (++argi == argc) {
	  cerr << "ERROR: -i flag without argument" << endl << endl;
	  usage();
	}
	num_iterations = atoi(argv[argi]);
      } else if (args == "-n" || args == "--no-console") {
	setPollingEnabled(0);
      } else if (args == "--interleave") {
	useInterleave = true;
      } else if (args == "-p" || args == "--min-precision") {
	if (++argi == argc) {
	  cerr << "ERROR: -p flag without argument" << endl << endl;
	  usage();
	}
	minPrecision = atof(argv[argi]);
      } else if (args == "-s" || args == "--min-safety") {
	if (++argi == argc) {
	  cerr << "ERROR: -s flag without argument" << endl << endl;
	  usage();
	}
	minSafety = atof(argv[argi]);
      } else if (args == "-w" || args == "--min-wait") {
	if (++argi == argc) {
	  cerr << "ERROR: -w flag without argument" << endl << endl;
	  usage();
	}
	minWait = atof(argv[argi]);
      } else if (args == "--") {
	past_options = true;
      } else {
	cerr << "ERROR: unknown option " << args << endl << endl;
	usage();
      }
    } else if (algorithm == NULL) {
      algorithm = argv[argi];
    } else if (prob_name == NULL) {
      prob_name = argv[argi];
    } else if (-999 == min_order) {
      min_order = atoi(argv[argi]);
    } else if (-999 == max_order) {
      max_order = atoi(argv[argi]);
    } else {
      cerr << "ERROR: too many arguments" << endl << endl;
      usage();
    }
  }

  if (prob_name == NULL) {
    cerr << "ERROR: not enough arguments" << endl << endl;
    usage();
  }
  if (-999 == min_order) {
    min_order = 0; // default
  }
  if (-999 == max_order) {
    max_order = 6; // default
  }

  testBatchIncremental(algorithm, prob_name, min_order, max_order,
		       num_iterations, use_fast_parser, minSafety,
		       minPrecision, minWait, useInterleave);

  // signal we are done
  FILE *fp = fopen("/tmp/testPomdp_done", "w");
  fprintf(fp, "success\n");
  fclose(fp);

  return 0;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.15  2005/10/28 03:53:08  trey
 * simplified license
 *
 * Revision 1.14  2005/10/28 02:59:30  trey
 * added copyright header
 *
 *
 ***************************************************************************/
