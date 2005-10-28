/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2005-10-28 03:05:27 $
   
 @file    testSpec.cc
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * This code and derivatives may not be incorporated into commercial
   products without specific prior written permission.
 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * The name of Trey Smith may not be used to endorse or promote products
   derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
