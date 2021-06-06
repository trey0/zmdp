/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2006, Trey Smith. All rights reserved.

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

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>

#include "BoundPairExec.h"
#include "MatrixUtils.h"
#include "zmdpMainConfig.cc"  // embed default config file
#include "zmdpMainConfig.h"

using namespace std;
using namespace zmdp;

#define NUM_TRIALS (3)
#define NUM_STEPS_PER_TRIAL (100)

void doit(const char *modelFileName, bool useFastModelParser,
          const char *policyFileName) {
  // seeds random number generator
  MatrixUtils::init_matrix_utils();

  ZMDPConfig *config = new ZMDPConfig();
  config->readFromString("<defaultConfig>", defaultConfig.data);

  BoundPairExec *em = new BoundPairExec();
  printf("initializing\n");
  em->initReadFiles(modelFileName, policyFileName, *config);

  MDPExec *e = em;

  for (int i = 0; i < NUM_TRIALS; i++) {
    printf("new simulation run\n");
    e->setToInitialState();
    printf("  reset to initial belief\n");
    for (int j = 0; j < NUM_STEPS_PER_TRIAL; j++) {
      printf("  step %d\n", j);
      int a = e->chooseAction();
      printf("    chose action %d\n", a);
      int o = e->getRandomOutcome(a);
      printf("    simulated seeing random observation %d\n", o);
      e->advanceToNextState(a, o);
      printf("    updated belief\n");
      if (e->getStateIsTerminal()) {
        printf("  [belief is terminal, ending trial]\n");
        break;
      } else if (j == NUM_STEPS_PER_TRIAL - 1) {
        printf("  [reached trial length limit of %d steps, ending trial]\n",
               NUM_STEPS_PER_TRIAL);
      }
    }
  }
}

void usage(const char *binaryName) {
  cerr << "usage: " << binaryName
       << " OPTIONS <foo.pomdp> <out.policy>\n"
          "  -h or --help   Display this help\n"
          "  -f or --fast   Use fast (but very picky) alternate model parser\n";
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  static char shortOptions[] = "hf";
  static struct option longOptions[] = {
      {"help", 0, NULL, 'h'}, {"fast", 0, NULL, 'f'}, {NULL, 0, 0, 0}};

  bool useFastModelParser = false;
  while (1) {
    char optchar = getopt_long(argc, argv, shortOptions, longOptions, NULL);
    if (optchar == -1) break;

    switch (optchar) {
      case 'h':  // help
        usage(argv[0]);
        break;

      case 'f':  // help
        useFastModelParser = true;
        break;

      case '?':  // unknown option
      case ':':  // option with missing parameter
        // getopt() prints an informative error message
        cerr << endl;
        usage(argv[0]);
        break;
      default:
        abort();  // never reach this point
    }
  }
  if (2 != argc - optind) {
    cerr << "ERROR: wrong number of arguments (should be 2)" << endl << endl;
    usage(argv[0]);
  }

  const char *modelFileName = argv[optind++];
  const char *policyFileName = argv[optind++];

  doit(modelFileName, useFastModelParser, policyFileName);
}
