/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-06-15 16:05:12 $
   
 @file    testReadPolicy.cc
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

#include <iostream>

#include "MaxPlanesLowerBound.h"

using namespace std;
using namespace zmdp;

void usage(void) {
  cerr <<
    "usage: testReadPolicy OPTIONS <foo.policy> <copy.policy>\n"
    "  -h or --help   Display this help\n";
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  int argi;
  char *inFileName = 0;
  char *outFileName = 0;
  for (argi=1; argi < argc; argi++) {
    if (0 == strcmp("-h",argv[argi]) || 0 == strcmp("--help",argv[argi])) {
      usage();
    } else if (0 == inFileName) {
      inFileName = argv[argi];
    } else if (0 == outFileName) {
      outFileName = argv[argi];
    } else {
      cerr << "too many arguments" << endl;
      usage();
    }
  }
  if (0 == outFileName) {
    usage();
  }

  // create a bogus pomdp because the reading code needs to
  // know the number of states... the rest of the pomdp model
  // is not used
  Pomdp* p = new Pomdp();
  p->numStates = 500000; // random number of states

  MaxPlanesLowerBound m(/* mdp = */ p);
  m.readFromFile(inFileName);
  m.writeToFile(outFileName);
}


/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/06/14 18:17:07  trey
 * initial check-in
 *
 *
 ***************************************************************************/
