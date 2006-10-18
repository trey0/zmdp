/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.8 $  $Author: trey $  $Date: 2006-10-18 18:30:13 $

 @file    zmdpEvaluate.cc
 @brief   Use to evaluate a POMDP policy in simulation.

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

#include "MatrixUtils.h"
#include "PomdpSim.h"
#include "MaxPlanesLowerBoundExec.h"
#include "LSPathAndReactExec.h"
#include "solverUtils.h"
#include "zmdpMainConfig.h"

using namespace std;
using namespace MatrixUtils;
using namespace zmdp;

#define ZE_DEFAULT_POLICY_TYPE ("maxplanes")

const char* policyTypeG = ZE_DEFAULT_POLICY_TYPE;
const char* policyFileNameG = NULL;
const char* sourceModelFileNameG = NULL;
const char* simModelFileNameG = NULL;
const char* plannerModelFileNameG = NULL;

void doit(const ZMDPConfig& config, SolverParams& p)
{
  // seeds random number generator
  init_matrix_utils();

  if (NULL == plannerModelFileNameG) {
    plannerModelFileNameG = simModelFileNameG;
  }

  // initialize exec
  PomdpExec* e;
  if (0 == strcmp(policyTypeG, "maxplanes")) {
    if (NULL == policyFileNameG) {
      fprintf(stderr, "ERROR: maxplanes policy type requires -p argument (-h for help)\n");
      exit(EXIT_FAILURE);
    }
    MaxPlanesLowerBoundExec* em = new MaxPlanesLowerBoundExec();
    em->init(plannerModelFileNameG, p.useFastPomdpParser, policyFileNameG, config);
    e = em;
  } else if (0 == strcmp(policyTypeG, "lspath")) {
    if (NULL == sourceModelFileNameG) {
      fprintf(stderr, "ERROR: lspath policy type requires -s argument (-h for help)\n");
      exit(EXIT_FAILURE);
    }
    LSPathAndReactExec* el = new LSPathAndReactExec();
    el->init(plannerModelFileNameG, p.useFastPomdpParser, sourceModelFileNameG);
    e = el;
  } else {
    fprintf(stderr, "ERROR: unknown policy type '%s' (-h for help)\n",
	    policyTypeG);
    exit(EXIT_FAILURE);
  }

  // initialize simulator
  Pomdp* simPomdp;
  if (plannerModelFileNameG == simModelFileNameG) {
    simPomdp = e->pomdp;
  } else {
    simPomdp = new Pomdp(simModelFileNameG, p.useFastPomdpParser);
  }
  PomdpSim* sim = new PomdpSim(simPomdp);

  const char* simulationTraceOutputFile =
    config.getString("simulationTraceOutputFile").c_str();
  
  ofstream simOutFile(simulationTraceOutputFile);
  if (!simOutFile) {
    fprintf(stderr, "ERROR: couldn't open %s for writing: %s\n",
	    simulationTraceOutputFile, strerror(errno));
    exit(EXIT_FAILURE);
  }
  sim->simOutFile = &simOutFile;

  int simulationTracesToLogPerEpoch = config.getInt("simulationTracesToLogPerEpoch");
  if (simulationTracesToLogPerEpoch < 0) {
    simulationTracesToLogPerEpoch = INT_MAX;
  }

  ofstream scoresOutFile("scores.plot");
  if (!scoresOutFile) {
    fprintf(stderr, "ERROR: couldn't open scores.plot for writing: %s\n",
	    strerror(errno));
    exit(EXIT_FAILURE);
  }

  // do evaluation
  std::vector<double> rewardValues;
  for (int i=0; i < p.evaluationTrialsPerEpoch; i++) {
    if (i >= simulationTracesToLogPerEpoch) {
      sim->simOutFile = NULL; // stop logging
    }

    sim->restart();
    e->setToInitialBelief();
    for (int j=0; (j < p.evaluationMaxStepsPerTrial) || (0 == p.evaluationMaxStepsPerTrial); j++) {
      int a = e->chooseAction();
      sim->performAction(a);
      e->advanceToNextBelief(a, sim->lastObservation);
      if (sim->terminated) break;
    }
    rewardValues.push_back(sim->rewardSoFar);
    scoresOutFile << sim->rewardSoFar << endl;
    
    if (i%10 == 9) {
      printf(".");
      fflush(stdout);
    }
  }
  printf("\n");

  double avg, stdev;
  calc_avg_stdev_collection(rewardValues.begin(), rewardValues.end(),
			    avg, stdev);
  double conf95 = 1.96 * stdev / sqrt((double)rewardValues.size());
  printf("REWARD_MEAN_MEANCONF95 %.3lf %.3lf\n", avg, conf95);

  simOutFile.close();
  scoresOutFile.close();
}

void usage(const char* cmdName) {
  cerr <<
    "usage: " << cmdName << " OPTIONS <evaluationModel>\n"
    "  -h or --help           Print this help\n"
    "  --version              Print version information (CFLAGS used at compilation)\n"
    "  -c or --config <file>  Specify a config file to read options from\n"
    "  --genConfig <file>     Generate an example config file and exit\n"
    "\n"
    "  -t or --policyType     Specifies policy type.  Choices are: lspath, maxplanes\n"
    "                           [default: " << ZE_DEFAULT_POLICY_TYPE << "]\n"
    "  -p or --policy         Specify input policy file (required with -t maxplanes)\n"
    "  -m or --model          Specify planner model (if different from evaluation model)\n"
    "  -s or --sourceModel    Specify source model file (required with -t lspath)\n"
    "\n"
    "  zmdpEvaluate evaluates a policy output by zmdpSolve or zmdpBenchmark.\n"
    "  The policy is evaluated by executing a number of trials in simulation;\n"
    "  the output is mean reward and a 95% confidence interval for the estimate\n"
    "  of the mean under the (not necessarily correct) assumption that the\n"
    "  per-trial reward is normally distributed.  Sorry, statisticians may wince...\n"
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
    "  -f = --useFastPomdpParser 1\n"
    "  -i = --evaluationTrialsPerEpoch\n"
    "\n"
    "Examples:\n"
    "  " << cmdName << " -p my.policy -f ltv1.pomdp\n"
    "  " << cmdName << " -t lspath -s ltv1.lifeSurvey -f ltv1.pomdp\n"
    "  " << cmdName << " -t lspath -s ltv1.lifeSurvey -f ltv1.pomdp -i 1000\n"
    "\n"
;
  exit(-1);
}

int main(int argc, char **argv) {
#if 0
  // save arguments for debug printout later
  ostringstream outs;
  for (int i=1; i < argc; i++) {
    outs << argv[i] << " ";
  }
#endif

  bool argsOnly = false;
  const char* configFileName = NULL;
  ZMDPConfig commandLineConfig;
  SolverParams p;

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
      } else if (args == "-t" || args == "--policyType") {
	if (++argi == argc) {
	  fprintf(stderr, "ERROR: found -t option without argument (use -h for help)\n");
	  exit(EXIT_FAILURE);
	}
	policyTypeG = argv[argi];
      } else if (args == "-p" || args == "--policy") {
	if (++argi == argc) {
	  fprintf(stderr, "ERROR: found -p option without argument (use -h for help)\n");
	  exit(EXIT_FAILURE);
	}
	policyFileNameG = argv[argi];
      } else if (args == "-m" || args == "--model") {
	if (++argi == argc) {
	  fprintf(stderr, "ERROR: found -m option without argument (use -h for help)\n");
	  exit(EXIT_FAILURE);
	}
	plannerModelFileNameG = argv[argi];
      } else if (args == "-s" || args == "--sourceModel") {
	if (++argi == argc) {
	  fprintf(stderr, "ERROR: found -s option without argument (use -h for help)\n");
	  exit(EXIT_FAILURE);
	}
	sourceModelFileNameG = argv[argi];
      } else {
	// replace abbreviations
	if (args == "-f") {
	  commandLineConfig.setBool("useFastPomdpParser", true);
	  continue;
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
      cout << "args = " << args << endl;
      if (NULL == simModelFileNameG) {
	simModelFileNameG = argv[argi];
      } else {
	fprintf(stderr, "ERROR: expected exactly 1 argument (use -h for help)\n");
	exit(EXIT_FAILURE);
      }
    }
  }
  if (NULL == simModelFileNameG) {
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

  p.setValues(config);

  doit(config, p);

  return EXIT_SUCCESS;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.7  2006/10/18 18:05:56  trey
 * now propagating config data structure to lower levels so config fields can be used to control more parts of the system
 *
 * Revision 1.6  2006/10/16 17:32:17  trey
 * switched zmdpEvaluate to use new config system
 *
 * Revision 1.5  2006/09/21 17:26:06  trey
 * renamed output field
 *
 * Revision 1.4  2006/09/19 01:49:02  trey
 * added --max-steps option
 *
 * Revision 1.3  2006/08/04 22:30:41  trey
 * fixed a serious error that caused incorrect evaluation when planning and evaluation models are different
 *
 * Revision 1.2  2006/07/10 19:34:35  trey
 * added ability to use different models for planning and simulator evaluation
 *
 * Revision 1.1  2006/07/10 03:35:49  trey
 * initial check-in
 *
 *
 ***************************************************************************/
