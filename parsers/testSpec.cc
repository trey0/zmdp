/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2005-10-28 03:54:39 $
   
 @file    testSpec.cc
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * The software may not be sold or incorporated into a commercial
   product without specific prior written permission.
 * The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
 * Revision 1.3  2005/10/28 03:05:27  trey
 * added copyright header
 *
 * Revision 1.2  2005/10/27 21:37:17  trey
 * brought names up to date
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.1.1.1  2003/01/07 19:19:41  trey
 * Imported sources
 *
 *
 ***************************************************************************/
