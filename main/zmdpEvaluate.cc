/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-09-19 01:49:02 $

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

using namespace std;
using namespace MatrixUtils;
using namespace zmdp;

#define ZE_DEFAULT_ITERATIONS (100)
#define ZE_DEFAULT_MAX_STEPS (251)
#define ZE_DEFAULT_POLICY_TYPE ("maxplanes")

const char* policyTypeG = ZE_DEFAULT_POLICY_TYPE;
bool useFastParserG = false;
const char* policyFileNameG = NULL;
const char* sourceModelFileNameG = NULL;
const char* simModelFileNameG = NULL;
const char* plannerModelFileNameG = NULL;
int iterationsG = ZE_DEFAULT_ITERATIONS;
int maxStepsG = ZE_DEFAULT_MAX_STEPS;

void doit(void)
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
    em->init(plannerModelFileNameG, useFastParserG, policyFileNameG);
    e = em;
  } else if (0 == strcmp(policyTypeG, "lspath")) {
    if (NULL == sourceModelFileNameG) {
      fprintf(stderr, "ERROR: lspath policy type requires -s argument (-h for help)\n");
      exit(EXIT_FAILURE);
    }
    LSPathAndReactExec* el = new LSPathAndReactExec();
    el->init(plannerModelFileNameG, useFastParserG, sourceModelFileNameG);
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
    simPomdp = new Pomdp(simModelFileNameG, useFastParserG);
  }
  PomdpSim* sim = new PomdpSim(simPomdp);

  ofstream simOutFile("sim.plot");
  if (!simOutFile) {
    fprintf(stderr, "ERROR: couldn't open sim.plot for writing: %s\n",
	    strerror(errno));
    exit(EXIT_FAILURE);
  }
  sim->simOutFile = &simOutFile;

  ofstream scoresOutFile("scores.plot");
  if (!scoresOutFile) {
    fprintf(stderr, "ERROR: couldn't open scores.plot for writing: %s\n",
	    strerror(errno));
    exit(EXIT_FAILURE);
  }

  // do evaluation
  std::vector<double> rewardValues;
  for (int i=0; i < iterationsG; i++) {
    sim->restart();
    e->setToInitialBelief();
    for (int j=0; (j < maxStepsG) || (0 == maxStepsG); j++) {
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
  printf("REWARD_AVG_STDEV %.3lf %.3lf\n", avg, conf95);

  simOutFile.close();
  scoresOutFile.close();
}

void usage(const char* cmdName) {
  cerr <<
    "usage: " << cmdName << " OPTIONS <model.pomdp>\n"
    "  -h or --help           Print this help\n"
    "  --version              Print version information (CFLAGS used at compilation)\n"
    "  -t or --type           Specifies policy type.  Choices are: lspath, maxplanes\n"
    "                           [default: " << ZE_DEFAULT_POLICY_TYPE << "]\n"
    "  -f or --fast           Use fast (but very picky) alternate POMDP parser\n"
    "  -p or --policy         Specify policy file (required with -t maxplanes)\n"
    "  -m or --model          Specify planner model (if different from evaluation model)\n"
    "  -s or --source-model   Specify source model file (required with -t lspath)\n"
    "  -i or --iterations     Set number of simulation runs [default: " << ZE_DEFAULT_ITERATIONS << "]\n"
    "  --max-steps            Set maximum number of steps in each simulator run; a\n"
    "                            value of 0 means only terminate according to the model\n"
    "                            [default: " << ZE_DEFAULT_MAX_STEPS << "]\n"
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
  static char shortOptions[] = "ht:fp:m:s:i:";
  static struct option longOptions[]={
    {"help",          0,NULL,'h'},
    {"version",       0,NULL,'V'},
    {"type",          1,NULL,'t'},
    {"fast",          0,NULL,'f'},
    {"policy",        1,NULL,'p'},
    {"model",         1,NULL,'m'},
    {"source-model",  1,NULL,'s'},
    {"iterations",    1,NULL,'i'},
    {"max-steps",     1,NULL,'M'},
    {NULL,0,0,0}
  };

  // save arguments for debug printout later
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
    case 'V': // version
      cout << "CFLAGS = " << CFLAGS << endl;
      exit(EXIT_SUCCESS);
      break;
    case 't': // type
      policyTypeG = optarg;
      break;
    case 'f': // fast
      useFastParserG = true;
      break;
    case 'p': // policy
      policyFileNameG = optarg;
      break;
    case 'm': // model
      plannerModelFileNameG = optarg;
      break;
    case 's': // source-model
      sourceModelFileNameG = optarg;
      break;
    case 'i': // iterations
      iterationsG = atoi(optarg);
      break;
    case 'M': // max-steps
      maxStepsG = atoi(optarg);
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

  simModelFileNameG = argv[optind++];

#if 0
  printf("CFLAGS = %s\n", CFLAGS);
  printf("ARGS = %s\n", outs.str().c_str());
#endif

  doit();

  return 0;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
