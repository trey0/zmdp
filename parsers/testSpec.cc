/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.2 $  $Author: trey $  $Date: 2005-10-27 21:37:17 $
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

#include "pomdpCassandraWrapper.h"

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
  PomdpCassandraWrapper p;
  p.readFromFile(pomdpFileName);

  // print out stats
  cout << "numStates = " << p.numStates() << endl;
  cout << "numActions = " << p.numActions() << endl;
  cout << "numObservations = " << p.numObservations() << endl;
  cout << "discount = " << p.discount() << endl;
  cout << endl;

  int s, sp, a, o;

  cout << "R(s,a) matrix = " << endl;
  for (s=0; s < p.numStates(); s++) {
    for (a=0; a < p.numActions(); a++) {
      printf("%9.2f ", p.R(s,a));
    }
    cout << endl;
  }
  cout << endl;

  for (a=0; a < p.numActions(); a++) {
    cout << "T(s," << a << ",sp) matrix = " << endl;
    for (s=0; s < p.numStates(); s++) {
      for (sp=0; sp < p.numStates(); sp++) {
	printf("%5.3f ", p.T(s,a,sp));
      }
      cout << endl;
    }
    cout << endl;
  }

  for (a=0; a < p.numActions(); a++) {
    cout << "O(sp," << a << ",o) matrix = " << endl;
    for (sp=0; sp < p.numStates(); sp++) {
      for (o=0; o < p.numObservations(); o++) {
	printf("%5.3f ", p.O(sp,a,o));
      }
      cout << endl;
    }
    cout << endl;
  }
}


/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.1.1.1  2003/01/07 19:19:41  trey
 * Imported sources
 *
 *
 ***************************************************************************/
