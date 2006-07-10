/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-07-10 03:35:49 $

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
#define ZE_DEFAULT_POLICY_TYPE ("maxplanes")

const char* policyTypeG = ZE_DEFAULT_POLICY_TYPE;
bool useFastParserG = true;
const char* policyFileNameG = NULL;
const char* sourceModelFileNameG = NULL;
const char* modelFileNameG = NULL;
int iterationsG = ZE_DEFAULT_ITERATIONS;

void doit(void)
{
  // seeds random number generator
  init_matrix_utils();

  // initialize exec
  PomdpExec* e;
  if (0 == strcmp(policyTypeG, "maxplanes")) {
    if (NULL == policyFileNameG) {
      fprintf(stderr, "ERROR: maxplanes policy type requires -p argument (-h for help)\n");
      exit(EXIT_FAILURE);
    }
    MaxPlanesLowerBoundExec* em = new MaxPlanesLowerBoundExec();
    em->init(modelFileNameG, useFastParserG, policyFileNameG);
    e = em;
  } else if (0 == strcmp(policyTypeG, "lspath")) {
    if (NULL == sourceModelFileNameG) {
      fprintf(stderr, "ERROR: lspath policy type requires -s argument (-h for help)\n");
      exit(EXIT_FAILURE);
    }
    LSPathAndReactExec* el = new LSPathAndReactExec();
    el->init(modelFileNameG, useFastParserG, sourceModelFileNameG);
    e = el;
  } else {
    fprintf(stderr, "ERROR: unknown policy type '%s' (-h for help)\n",
	    policyTypeG);
    exit(EXIT_FAILURE);
  }

  // initialize simulator
  ofstream simOutFile("sim.plot");
  if (!simOutFile) {
    fprintf(stderr, "ERROR: couldn't open sim.plot for writing: %s\n",
	    strerror(errno));
    exit(EXIT_FAILURE);
  }
  PomdpSim* sim = new PomdpSim(e->pomdp);
  sim->simOutFile = &simOutFile;

  std::vector<double> rewardValues;
  for (int i=0; i < iterationsG; i++) {
    sim->restart();
    e->setToInitialBelief();
    while (1) {
      int a = e->chooseAction();
      sim->performAction(a);
      e->advanceToNextBelief(a, sim->lastObservation);
      if (sim->terminated) break;
    }
    rewardValues.push_back(sim->rewardSoFar);
    
    if (i%10 == 9) {
      printf(".");
      fflush(stdout);
    }
  }
  
  double avg, stdev;
  calc_avg_stdev_collection(rewardValues.begin(), rewardValues.end(),
			    avg, stdev);
  double conf95 = 1.96 * stdev / sqrt((double)rewardValues.size());
  printf("REWARD_AVG_STDEV %.3lf %.3lf\n", avg, conf95);
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
    "  -s or --source-model   Specify source model file (required with -t lspath)\n"
    "  -i or --iterations     Set number of simulation iterations [default: " << ZE_DEFAULT_ITERATIONS << "]\n"
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
  static char shortOptions[] = "ht:fp:s:i:";
  static struct option longOptions[]={
    {"help",          0,NULL,'h'},
    {"version",       0,NULL,'V'},
    {"type",          1,NULL,'t'},
    {"fast",          0,NULL,'f'},
    {"policy",        1,NULL,'p'},
    {"source-model",  1,NULL,'s'},
    {"iterations",    1,NULL,'i'},
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
    case 's': // source-model
      sourceModelFileNameG = optarg;
      break;
    case 'i': // iterations
      iterationsG = atoi(optarg);
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

  modelFileNameG = argv[optind++];

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
 *
 ***************************************************************************/
