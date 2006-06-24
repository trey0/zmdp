/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-06-24 16:25:28 $
   
 @file    testPomdpExec.cc
 @brief   No brief

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
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>

#include <iostream>

#include "MatrixUtils.h"
#include "PomdpExec.h"

using namespace std;
using namespace zmdp;

void doit(const char* modelFileName,
	  bool useFastParser,
	  const char* policyFileName)
{
  // seeds random number generator
  MatrixUtils::init_matrix_utils();

  PomdpExec p;
  printf("initializing\n");
  p.init(modelFileName, useFastParser, policyFileName);

  for (int i=0; i < 3; i++) {
    printf("new simulation run\n");
    p.setToInitialBelief();
    printf("  reset to initial belief\n");
    while (1) {
      int a = p.chooseAction();
      printf("  chose action %d\n", a);
      int o = p.getRandomObservation(a);
      printf("  simulated seeing random observation %d\n", o);
      p.advanceToNextBelief(a,o);
      printf("  updated belief\n");
      if (p.getBeliefIsTerminal()) {
	printf("  belief is terminal, ending run\n");
	break;
      }
    }
  }
}

void usage(const char* binaryName)
{
  cerr <<
    "usage: " << binaryName << " OPTIONS <foo.pomdp> <out.policy>\n"
    "  -h or --help   Display this help\n"
    "  -f or --fast   Use fast (but very picky) alternate model parser\n";
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  static char shortOptions[] = "hf";
  static struct option longOptions[]={
    {"help",          0,NULL,'h'},
    {"fast",          0,NULL,'f'},
    {NULL,0,0,0}
  };

  bool useFastParser = false;
  while (1) {
    char optchar = getopt_long(argc,argv,shortOptions,longOptions,NULL);
    if (optchar == -1) break;

    switch (optchar) {
    case 'h': // help
      usage(argv[0]);
      break;

    case 'f': // help
      useFastParser = true;
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
  if (2 != argc-optind) {
    cerr << "ERROR: wrong number of arguments (should be 2)" << endl << endl;
    usage(argv[0]);
  }

  const char* modelFileName = argv[optind++];
  const char* policyFileName = argv[optind++];

  doit(modelFileName, useFastParser, policyFileName);
}


/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
