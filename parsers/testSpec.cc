/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1.1.1 $  $Author: trey $  $Date: 2004-11-09 16:18:56 $
 *  
 * PROJECT: FIRE Architecture Project
 *
 * @file    testSpec.cc
 * @brief   No brief
 ***************************************************************************/

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <iostream>
using namespace std;

#include "spec.h"

void usage(void) {
  cerr <<
    "usage: testSpec OPTIONS <foo.POMDP>\n"
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
  Pomdp spec;
  spec.readFromFile(pomdpFileName);

  // print out stats
  cout << "numStates = " << spec.numStates() << endl;
  cout << "numActions = " << spec.numActions() << endl;
  cout << "numObservations = " << spec.numObservations() << endl;
  cout << "discount = " << spec.discount() << endl;
  cout << endl;

  int s, sp, a, o;

  cout << "R(s,a) matrix = " << endl;
  for (s=0; s < spec.numStates(); s++) {
    for (a=0; a < spec.numActions(); a++) {
      printf("%9.2f ", spec.R(s,a));
    }
    cout << endl;
  }
  cout << endl;

  for (a=0; a < spec.numActions(); a++) {
    cout << "T(s," << a << ",sp) matrix = " << endl;
    for (s=0; s < spec.numStates(); s++) {
      for (sp=0; sp < spec.numStates(); sp++) {
	printf("%5.3f ", spec.T(s,a,sp));
      }
      cout << endl;
    }
    cout << endl;
  }

  for (a=0; a < spec.numActions(); a++) {
    cout << "O(sp," << a << ",o) matrix = " << endl;
    for (sp=0; sp < spec.numStates(); sp++) {
      for (o=0; o < spec.numObservations(); o++) {
	printf("%5.3f ", spec.O(sp,a,o));
      }
      cout << endl;
    }
    cout << endl;
  }
}


/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2003/01/07 19:19:41  trey
 * Imported sources
 *
 *
 ***************************************************************************/
