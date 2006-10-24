/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.15 $  $Author: trey $  $Date: 2006-10-24 19:10:28 $

 @file    zmdpSolve.cc
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
#include <signal.h>

#include <iostream>
#include <fstream>

#include "zmdpCommonTime.h"
#include "solverUtils.h"
#include "zmdpMainConfig.h"

using namespace std;
using namespace MatrixUtils;
using namespace zmdp;

bool userTerminatedG = false;

void sigIntHandler(int sig) {
  userTerminatedG = true;

  printf("*** received SIGINT, user pressed control-C ***\n"
	 "terminating run and writing output policy as soon as the solver returns control\n");
  fflush(stdout);
}

void setSignalHandler(int sig, void (*handler)(int)) {
  struct sigaction act;
  memset (&act, 0, sizeof(act));
  act.sa_handler = handler;
  act.sa_flags = SA_RESTART;
  if (-1 == sigaction (sig, &act, NULL)) {
    cerr << "ERROR: unable to set handler for signal "
         << sig << endl;
    exit(EXIT_FAILURE);
  }
}

void doSolve(const ZMDPConfig& config, SolverParams& p)
{
  init_matrix_utils();
  StopWatch run;

  printf("%05d reading model file and allocating data structures\n",
	 (int) run.elapsedTime());
  SolverObjects so;
  constructSolverObjects(so, p, config);

  // initialize the solver
  printf("%05d calculating initial heuristics\n",
	 (int) run.elapsedTime());
  so.solver->planInit(so.sim->getModel(), &config);
  printf("%05d finished initialization, beginning to improve policy\n",
	 (int) run.elapsedTime());
  
  setSignalHandler(SIGINT, &sigIntHandler);

  double lastPrintTime = -1000;
  bool reachedTargetPrecision = false;
  bool reachedTimeout = false;
  int numSolverCalls = 0;
  while (!(reachedTargetPrecision || reachedTimeout || userTerminatedG)) {
    // make a call to the solver
    reachedTargetPrecision =
      so.solver->planFixedTime(so.sim->getModel()->getInitialState(),
			       /* maxTime = */ -1, p.terminateRegretBound);
    numSolverCalls++;

    // check timeout
    double elapsed = run.elapsedTime();
    if (elapsed >= p.terminateWallclockSeconds) {
      reachedTimeout = true;
    }

    // print a progress update every 10 seconds
    if ((elapsed - lastPrintTime > 10)
	|| reachedTargetPrecision || reachedTimeout || userTerminatedG) {
      ValueInterval intv = so.solver->getValueAt(so.sim->getModel()->getInitialState());
      printf("%05d %6d calls to solver, bounds [%8.4f .. %8.4f], regret <= %g\n",
	     (int) elapsed, numSolverCalls, intv.l, intv.u, (intv.u - intv.l));
      lastPrintTime = elapsed;
    }
  }

  // say why the run ended
  if (reachedTargetPrecision) {
    printf("%05d terminating run; reached target regret bound of %g\n",
	   (int) run.elapsedTime(), p.terminateRegretBound);
  } else if (reachedTimeout) {
    printf("%05d terminating run; passed specified timeout of %g seconds\n",
	   (int) run.elapsedTime(), p.terminateWallclockSeconds);
  } else {
    printf("%05d terminating run; caught SIGINT from user\n",
	   (int) run.elapsedTime());
  }

  // write out a policy
  if (NULL == p.policyOutputFile) {
    printf("%05d (policy output was not requested)\n", (int) run.elapsedTime());
  } else {
    printf("%05d writing policy to '%s'\n", (int) run.elapsedTime(), p.policyOutputFile);
    so.bounds->writePolicy(p.policyOutputFile);
  }

  printf("%05d done\n", (int) run.elapsedTime());
}

void usage(const char* cmdName) {
  cerr <<
    "usage: " << cmdName << " OPTIONS <model>\n"
    "  -h or --help           Print this help\n"
    "  --version              Print version information (CFLAGS used at compilation)\n"
    "  -c or --config <file>  Specify a config file to read options from\n"
    "  --genConfig <file>     Generate an example config file and exit\n"
    "\n"
    "  zmdpSolve is the simplified front-end to the ZMDP library.  Give it a model,\n"
    "  it runs for a while and writes out a policy.  You can then use zmdpEvaluate to\n"
    "  read in and evaluate the policy or execute the policy with your own executive.\n"
    "  If you want to monitor performance as the solution algorithm is running, use\n"
    "  zmdpBenchmark instead of zmdpSolve.\n"
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
    "\n"
    "Examples:\n"
    "  " << cmdName << " RockSample_4_4.pomdp\n"
    "  " << cmdName << " large-b.racetrack\n"
    "  " << cmdName << " -t 60 -o my.policy RockSample_4_4.pomdp\n"
    "  " << cmdName << " -s lrtdp -v point RockSample_4_4.pomdp\n"
    "  " << cmdName << " -f RockSample_5_7.pomdp\n"
    "\n"
;
  exit(-1);
}

int main(int argc, char **argv) {
  SolverParams p;

#if USE_DEBUG_PRINT
  // save arguments for debug printout later
  ostringstream outs;
  for (int i=1; i < argc; i++) {
    outs << argv[i] << " ";
  }
#endif

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
      cout << "args = " << args << endl;
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
  // for zmdpSolve, it is "out.policy"
  if (config.getString("policyOutputFile") == "-") {
    config.setString("policyOutputFile", "out.policy");
  }

#if USE_DEBUG_PRINT
  printf("CFLAGS = %s\n", CFLAGS);
  printf("ARGS = %s\n", outs.str().c_str());
  fflush(stdout);
#endif

  p.setValues(config);
  doSolve(config, p);

  return 0;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.14  2006/10/24 02:11:20  trey
 * changes to match Solver API change
 *
 * Revision 1.13  2006/10/18 18:05:56  trey
 * now propagating config data structure to lower levels so config fields can be used to control more parts of the system
 *
 * Revision 1.12  2006/10/17 19:18:47  trey
 * centralized proper handling of negative terminateWallclockSeconds values
 *
 * Revision 1.11  2006/10/16 17:33:04  trey
 * removed obsolete OutputParams struct
 *
 * Revision 1.10  2006/10/16 05:49:37  trey
 * renamed outputPolicyFile -> policyOutputFile
 *
 * Revision 1.9  2006/10/15 23:46:02  trey
 * switched to new config mechanism
 *
 * Revision 1.8  2006/10/03 03:17:26  trey
 * added --max-horizon parameter
 *
 * Revision 1.7  2006/06/15 16:08:37  trey
 * restructured so zmdpBenchmark can output policies
 *
 * Revision 1.6  2006/06/01 16:49:54  trey
 * moved installation of the SIGINT handler to after initialization, so an interrupt during initialization causes the solver to exit immediately
 *
 * Revision 1.5  2006/04/28 20:33:54  trey
 * added handling of SIGINT
 *
 * Revision 1.4  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.3  2006/04/27 23:42:54  trey
 * improved error diagnostic for policy output
 *
 * Revision 1.2  2006/04/27 23:19:14  trey
 * removed unnecessary #include of Interleave.h
 *
 * Revision 1.1  2006/04/27 23:07:24  trey
 * initial check-in
 *
 *
 ***************************************************************************/
