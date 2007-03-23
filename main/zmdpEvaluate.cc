/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.16 $  $Author: trey $  $Date: 2007-03-23 00:30:28 $

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
#include "MDPSim.h"
#include "BoundPairExec.h"
#include "LSPathAndReactExec.h"
#include "solverUtils.h"
#include "zmdpMainConfig.h"

#include "zmdpMainConfig.cc" // embed default config file

using namespace std;
using namespace MatrixUtils;
using namespace zmdp;

#define ZE_DEFAULT_POLICY_TYPE ("maxPlanes")

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
  MDPExec* e;
  if (0 == strcmp(policyTypeG, "maxPlanes")) {
    if (NULL == policyFileNameG) {
      fprintf(stderr, "ERROR: maxPlanes policy type requires -p argument (-h for help)\n");
      exit(EXIT_FAILURE);
    }
    BoundPairExec* em = new BoundPairExec();
    em->initMaxPlanes(plannerModelFileNameG, p.useFastModelParser, policyFileNameG, config);
    e = em;
  } else if (0 == strcmp(policyTypeG, "lspath")) {
    if (NULL == sourceModelFileNameG) {
      fprintf(stderr, "ERROR: lspath policy type requires -s argument (-h for help)\n");
      exit(EXIT_FAILURE);
    }
    LSPathAndReactExec* el = new LSPathAndReactExec();
    el->init(plannerModelFileNameG, sourceModelFileNameG, &config);
    e = el;
  } else {
    fprintf(stderr, "ERROR: unknown policy type '%s' (-h for help)\n",
	    policyTypeG);
    exit(EXIT_FAILURE);
  }

  // initialize simulator
  Pomdp* simPomdp;
  if (plannerModelFileNameG == simModelFileNameG) {
    simPomdp = (Pomdp*) e->mdp;
  } else {
    Pomdp* plannerPomdp = (Pomdp*) e->mdp;
    simPomdp = new Pomdp(simModelFileNameG, &config);

    if (! ((plannerPomdp->getNumActions() == simPomdp->getNumActions())
	   && (plannerPomdp->getNumObservations() == simPomdp->getNumObservations()))) {
      printf("ERROR: planner model %s and evaluation model %s must have the same number of actions and observations\n",
	     plannerModelFileNameG, simModelFileNameG);
      exit(EXIT_FAILURE);
    }
  }
  MDPSim* sim = new MDPSim(simPomdp);

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

  std::string scoresOutputFile = config.getString("scoresOutputFile");

  ofstream scoresOut(scoresOutputFile.c_str());
  if (!scoresOut) {
    fprintf(stderr, "ERROR: couldn't open %s for writing: %s\n",
	    scoresOutputFile.c_str(), strerror(errno));
    exit(EXIT_FAILURE);
  }

  // do evaluation
  dvector rewardValues(p.evaluationTrialsPerEpoch);
  for (int i=0; i < p.evaluationTrialsPerEpoch; i++) {
    if (i >= simulationTracesToLogPerEpoch) {
      sim->simOutFile = NULL; // stop logging
    }

    sim->restart();
    e->setToInitialState();
    for (int j=0; (j < p.evaluationMaxStepsPerTrial) || (0 == p.evaluationMaxStepsPerTrial); j++) {
      int a = e->chooseAction();
      sim->performAction(a);
      e->advanceToNextState(a, sim->lastOutcomeIndex);
      if (sim->terminated) break;
    }
    rewardValues(i) = sim->rewardSoFar;
    scoresOut << sim->rewardSoFar << endl;
    
    if (i%10 == 9) {
      printf(".");
      fflush(stdout);
    }
  }
  printf("\n");

  // new calculation using bootstrap method
  double mean, quantile1, quantile2;
  dvector weights;
  set_to_one(weights, p.evaluationTrialsPerEpoch);
  calc_bootstrap_mean_quantile(weights, rewardValues,
			       0.05, // 95% confidence interval
			       mean, quantile1, quantile2);
  printf("REWARD_MEAN_CONF95MIN_CONF95MAX %.3lf %.3lf %.3lf\n", mean, quantile1, quantile2);

  simOutFile.close();
  scoresOut.close();
}

void usage(const char* cmdName) {
  cerr <<
    "usage: " << cmdName << " OPTIONS <evaluationModel>\n"
    "  -h or --help           Print this help\n"
    "  --version              Print version information (CFLAGS used at compilation)\n"
    "  -c or --config <file>  Specify a config file to read options from\n"
    "  --genConfig <file>     Generate an example config file and exit\n"
    "\n"
    "  -t or --policyType     Specifies policy type.  Choices are: lspath, maxPlanes\n"
    "                           [default: " << ZE_DEFAULT_POLICY_TYPE << "]\n"
    "  -p or --policy         Specify input policy file (required with -t maxPlanes)\n"
    "  -m or --model          Specify planner model (if different from evaluation model)\n"
    "  -s or --sourceModel    Specify source model file (required with -t lspath)\n"
    "\n"
    "  zmdpEvaluate evaluates a policy output by zmdpSolve or zmdpBenchmark.\n"
    "  The policy is evaluated by executing a number of trials in simulation;\n"
    "  the output is mean reward and the 95% confidence interval for the\n"
    "  estimate of the mean, calculated using the bootstrap method.\n"
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
    "  -f = --useFastModelParser 1\n"
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
	  commandLineConfig.setBool("useFastModelParser", true);
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
 * Revision 1.15  2007/03/23 00:05:19  trey
 * fixed to reflect migration from PomdpExec to MDPExec base class
 *
 * Revision 1.14  2007/03/22 18:24:54  trey
 * now use MDPSim instead of PomdpSim; switched to use bootstrap calculation of confidence intervals
 *
 * Revision 1.13  2007/03/05 08:57:08  trey
 * modified how zmdpMainConfig is used so binaries built in other directories can embed the default config
 *
 * Revision 1.12  2006/11/12 21:22:15  trey
 * added better error checking for incompatible planner and evaluation models
 *
 * Revision 1.11  2006/11/08 16:36:16  trey
 * renamed useFastPomdpParser to useFastModelParser
 *
 * Revision 1.10  2006/10/25 02:07:24  trey
 * removed old debug statement
 *
 * Revision 1.9  2006/10/24 19:10:14  trey
 * changed maxplanes -> maxPlanes for consistency
 *
 * Revision 1.8  2006/10/18 18:30:13  trey
 * NUM_SIM_ITERATIONS_TO_LOG changed to a run-time parameter simulationTracesToLogPerEpoch
 *
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
