/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.10 $  $Author: trey $  $Date: 2006-10-18 18:05:56 $

 @file    zmdpBenchmark.cc
 @brief   No brief

 Copyright (c) 2002-2006, Trey Smith.  All rights reserved.

 Licensed under the Apache License, Version 2.0 (the "License"); you may
 not use this file except in compliance with the License.  You may
 obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 implied.  See the License for the specific language governing
 permissions and limitations under the License.

 ***************************************************************************/

#include <assert.h>
#include <sys/time.h>
#include <getopt.h>

#include <iostream>
#include <fstream>

#include "solverUtils.h"
#include "TestDriver.h"
#include "zmdpMainConfig.h"

using namespace std;
using namespace MatrixUtils;
using namespace zmdp;

void doBenchmark(const ZMDPConfig& config, SolverParams& p)
{
  init_matrix_utils();

  printf("reading input files\n");
  SolverObjects so;
  constructSolverObjects(so, p, config);

  if (NULL != p.policyOutputFile) {
    if (!so.bounds->getSupportsPolicyOutput()) {
      cerr << "ERROR: -o specified, but with selected options, policy output is not supported:" << endl;
      cerr << "  in order to enable policy output, problem must be a POMDP; if" << endl;
      cout << "  it is, try adding the '-v convex' and '--lower-bound' options" << endl;
      exit(EXIT_FAILURE);
    }
  }

  TestDriver x;
  x.batchTestIncremental(config,
			 /* numIterations = */ p.evaluationTrialsPerEpoch,
			 so,
			 /* numSteps = */ p.evaluationMaxStepsPerTrial,
			 /* targetPrecision = */ p.terminateRegretBound,
			 /* minOrder = */ p.evaluationFirstEpochWallclockSeconds,
			 /* maxOrder = */ p.terminateWallclockSeconds,
			 /* ticksPerOrder = */ p.evaluationEpochsPerMagnitude,
			 /* incPlotFileName = */ p.evaluationOutputFile,
			 /* boundsFileName = */ p.boundsOutputFile,
			 /* simFileName = */ p.simulationTraceOutputFile,
			 /* policyOutputFile = */ p.policyOutputFile);

  MDPNodeHashLogger::logToFile("nodes.plot", so.bounds->lookup);

  x.printRewards();
}

void usage(const char* cmdName) {
  cerr <<
    "usage: " << cmdName << " OPTIONS <model>\n"
    "  -h or --help           Print this help\n"
    "  --version              Print version information (CFLAGS used at compilation)\n"
    "  -c or --config <file>  Specify a config file to read options from\n"
    "  --genConfig <file>     Generate an example config file and exit\n"
    "\n"
    "  zmdpBenchmark is the advanced front-end to the ZMDP library.  While running a\n"
    "  solution algorithm, it keeps a number of log files that can be used (along\n"
    "  with scripts in the src/tools directory) to plot performance.\n"
    "\n"
    "  ZMDP gets configuration information from three places: (1) Default values\n"
    "  are embedded in the binary at compile time.  (2) If you specify a config file\n"
    "  using the --config option, any fields set in that file override the defaults.\n"
    "  (3) You can override individual fields in the config file from the command line.\n"
    "  For instance, if the config file specifies 'searchStrategy frtdp' you can override\n"
    "  with '--searchStrategy hsvi' at the command line.\n"
    "\n"
    "  To generate an example config file (including default values and comments describing\n"
    "  all the parameters), use the '--genConfig <file>' option.\n"
    "\n"
    "For convenience, there are also some abbreviations:\n"
    "  -s = --searchStrategy\n"
    "  -t = --modelType\n"
    "  -v = --valueFunctionRepresentation\n"
    "  -o = --policyOutputFile\n"
    "  -f = --useFastPomdpParser 1\n"
    "  -p = --terminateRegretBound\n"
    "  -i = --evaluationTrialsPerEpoch\n"
    "\n"
    "Examples:\n"
    "  " << cmdName << " RockSample_4_4.pomdp\n"
    "  " << cmdName << " large-b.racetrack\n"
    "  " << cmdName << " -s lrtdp -v point RockSample_4_4.pomdp\n"
    "  " << cmdName << " -f RockSample_5_7.pomdp\n"
;
  exit(-1);
}

int main(int argc, char **argv) {
  SolverParams p;

  // save arguments for debug printout later
  ostringstream outs;
  for (int i=1; i < argc; i++) {
    outs << argv[i] << " ";
  }

  bool argsOnly = false;
  const char* configFileName = NULL;
  ZMDPConfig commandLineConfig;

  p.cmdName = argv[0];
  for (int argi=1; argi < argc; argi++) {
    std::string args = argv[argi];
    if (!argsOnly && '-' == args[0]) {
      if (args == "--") {
	argsOnly = true;
      } else if (args == "-h" || args == "--help") {
	usage(argv[0]);
      } else if (args == "--version") {
	cout << "CFLAGS = " << CFLAGS << endl;
	exit(EXIT_SUCCESS);
      } else if (args == "-c" || args == "--config") {
	if (++argi == argc) {
	  fprintf(stderr, "ERROR: found -c option without argument (use -h for help)\n");
	  exit(EXIT_FAILURE);
	}
	configFileName = argv[argi];
      } else if (args == "--genConfig") {
	if (++argi == argc) {
	  fprintf(stderr, "ERROR: found --genConfig option without argument (use -h for help)\n");
	  exit(EXIT_FAILURE);
	}
	embedWriteToFile(argv[argi], defaultConfig);
	printf("wrote config file with default settings to %s\n", argv[argi]);
	exit(EXIT_SUCCESS);
      } else {
	// replace abbreviations
	if (args == "-s") {
	  args = "--searchStrategy";
	} else if (args == "-t") {
	  args = "--modelType";
	} else if (args == "-v") {
	  args = "--valueFunctionRepresentation";
	} else if (args == "-f") {
	  commandLineConfig.setBool("useFastPomdpParser", true);
	  continue;
	} else if (args == "-p") {
	  args = "--terminateRegretBound";
	} else if (args == "-o") {
	  args = "--policyOutputFile";
	} else if (args == "-i") {
	  args = "--evaluationTrialsPerEpoch";
	}

	if (args.find("--") != 0) {
	  fprintf(stderr, "ERROR: found unknown option '%s' (use -h for help)\n",
		  args.c_str());
	  exit(EXIT_FAILURE);
	} else {
	  if (++argi == argc) {
	    fprintf(stderr, "ERROR: found %s option without argument (-h for help)\n",
		    args.c_str());
	    exit(EXIT_FAILURE);
	  }
	  commandLineConfig.setString(args.substr(2), argv[argi]);
	}
      }
    } else {
      if (NULL == p.probName) {
	p.probName = argv[argi];
      } else {
	fprintf(stderr, "ERROR: expected exactly 1 argument (use -h for help)\n");
	exit(EXIT_FAILURE);
      }
    }
  }
  if (NULL == p.probName) {
    fprintf(stderr, "ERROR: expected exactly 1 argument (use -h for help)\n");
    exit(EXIT_FAILURE);
  }

  // config step 1: read defaults embedded in binary
  ZMDPConfig config;
  config.readFromString("<defaultConfig>", defaultConfig.data);

  // config step 2: overwrite defaults with values specified in config file
  // (signal an error if any new unexpected fields are defined)
  config.setOverWriteOnlyMode(true);
  if (NULL != configFileName) {
    config.readFromFile(configFileName);
  }

  // config step 3: overwrite with values specified on command line
  config.readFromConfig(commandLineConfig);

  // default value of policyOutputFile depends on the front end used;
  // for zmdpBenchmark, it is "none"
  if (config.getString("policyOutputFile") == "-") {
    config.setString("policyOutputFile", "none");
  }

  printf("CFLAGS = %s\n", CFLAGS);
  printf("ARGS = %s\n", outs.str().c_str());
  fflush(stdout);

  p.setValues(config);
  doBenchmark(config, p);

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
 * Revision 1.9  2006/10/16 17:32:41  trey
 * bug fixes related to new config system
 *
 * Revision 1.8  2006/10/16 05:50:11  trey
 * switched zmdpBenchmark to use new config mechanism
 *
 * Revision 1.7  2006/10/03 03:17:26  trey
 * added --max-horizon parameter
 *
 * Revision 1.6  2006/07/26 20:52:57  trey
 * added debug output
 *
 * Revision 1.5  2006/06/15 16:10:57  trey
 * restructured so zmdpBenchmark can output policies
 *
 * Revision 1.4  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.3  2006/04/27 23:19:03  trey
 * renamed Interleave -> TestDriver
 *
 * Revision 1.2  2006/04/27 23:08:18  trey
 * reordered getopt data structure to match usage()
 *
 * Revision 1.1  2006/04/27 20:19:22  trey
 * refactored command-line interface code, renamed solveMDP to zmdpBenchmark
 *
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
