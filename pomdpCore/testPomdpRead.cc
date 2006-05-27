/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-05-27 19:01:26 $
   
 @file    testPomdpRead.cc
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

#include "Pomdp.h"

using namespace std;
using namespace zmdp;

void usage(void) {
  cerr <<
    "usage: testPomdpRead OPTIONS <foo.POMDP>\n"
    "  -h or --help   Display this help\n";
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  int argi;
  char *pomdpFileName = 0;
  for (argi=1; argi < argc; argi++) {
    if (0 == strcmp("-h",argv[argi]) || 0 == strcmp("--help",argv[argi])) {
      usage();
    } else if (0 == pomdpFileName) {
      pomdpFileName = argv[argi];
    } else {
      cerr << "too many arguments" << endl;
      usage();
    }
  }
  if ( 0 == pomdpFileName ) {
    usage();
  }

  // read it in
  Pomdp p;
  p.readFromFile(pomdpFileName);

  // print out stats
  cout << "numStates = " << p.getBeliefSize() << endl;
  cout << "numActions = " << p.getNumActions() << endl;
  cout << "numObservations = " << p.getNumObservations() << endl;
  cout << "discount = " << p.getDiscount() << endl;
  cout << endl;

  int s, sp, a, o;

  printf("R(s,a) matrix (%d x %d) =\n", p.R.size1(), p.R.size2());
  for (s=0; s < p.getBeliefSize(); s++) {
    for (a=0; a < p.getNumActions(); a++) {
      printf("%9.2f ", p.R(s,a));
    }
    cout << endl;
  }
  cout << endl;

  for (a=0; a < p.getNumActions(); a++) {
    printf("T_%d(s,sp) matrix (%d x %d) =\n", a, p.T[a].size1(), p.T[a].size2());
    for (s=0; s < p.getBeliefSize(); s++) {
      for (sp=0; sp < p.getBeliefSize(); sp++) {
	printf("%5.3f ", p.T[a](s,sp));
      }
      cout << endl;
    }
    cout << endl;
  }

  for (a=0; a < p.getNumActions(); a++) {
    printf("Ttr_%d(sp,s) matrix (%d x %d) =\n", a, p.Ttr[a].size1(), p.Ttr[a].size2());
    for (sp=0; sp < p.getBeliefSize(); sp++) {
      for (s=0; s < p.getBeliefSize(); s++) {
	printf("%5.3f ", p.Ttr[a](sp,s));
      }
      cout << endl;
    }
    cout << endl;
  }

  for (a=0; a < p.getNumActions(); a++) {
    printf("O_%d(sp,o) matrix (%d x %d) =\n", a, p.O[a].size1(), p.O[a].size2());
    for (sp=0; sp < p.getBeliefSize(); sp++) {
      for (o=0; o < p.getNumObservations(); o++) {
	printf("%5.3f ", p.O[a](sp,o));
      }
      cout << endl;
    }
    cout << endl;
  }
}


/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
