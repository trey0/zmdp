/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2005-10-28 02:51:40 $
  
 @file    Pomdp.cc
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
#include <sys/time.h>

#include <iostream>
#include <fstream>

#include "pomdpCommonDefs.h"
#include "pomdpCassandraWrapper.h"
#include "Pomdp.h"
#include "MatrixUtils.h"

using namespace std;
using namespace MatrixUtils;

namespace pomdp {

/***************************************************************************
 * STATIC HELPER FUNCTIONS
 ***************************************************************************/

static void readVector(char *data, dvector& b, int numValues)
{
  int i;
  char *inp = data;
  char *tok;
  
  for (i=0; i < numValues; i++) {
    tok = strtok(inp," ");
    if (0 == tok) {
      cout << "ERROR: not enough entries in initial belief distribution"
	   << endl;
      exit(EXIT_FAILURE);
    }
    inp = 0;

    b(i) = atof(tok);
  }
}

static void trimTrailingWhiteSpace(char *s)
{
  int n = strlen(s);
  int i;
  for (i = n-1; i >= 0; i--) {
    if (!isspace(s[i])) break;
  }
  s[i+1] = '\0';
}

/***************************************************************************
 * POMDPM FUNCTIONS
 ***************************************************************************/

void Pomdp::readFromFile(const std::string& fileName,
			  bool useFastParser)
{
  if (useFastParser) {
    readFromFileFast(fileName);
  } else {
    readFromFileCassandra(fileName);
  }
}

void Pomdp::readFromFileCassandra(const string& fileName) {
  dvector initialBeliefx;
  std::vector<bool> isTerminalStatex;
  kmatrix Rx;
  std::vector<kmatrix> Tx, Ox;

  timeval startTime, endTime;

  cout << "reading problem from " << fileName << endl;
  gettimeofday(&startTime,0);
  PomdpCassandraWrapper p;
  p.readFromFile(fileName);
  
  numStates = p.getNumStates();
  numActions = p.getNumActions();
  numObservations = p.getNumObservations();
  discount = p.getDiscount();

  // pre-process
  initialBeliefx.resize(numStates);
  set_to_zero(initialBeliefx);
  isTerminalStatex.resize(numStates, /* initialValue = */ false);
  Rx.resize(numStates, numActions);
  Tx.resize(numActions);
  Ox.resize(numActions);
  FOR (a, numActions) {
    Tx[a].resize(numStates, numStates);
    Ox[a].resize(numStates, numObservations);
  }

  // copy
  FOR (s, numStates) {
    initialBeliefx(s) = p.getInitialBelief(s);
    isTerminalStatex[s] = p.isTerminalState(s);
    FOR (a, numActions) {
      kmatrix_set_entry( Rx, s, a, p.R(s,a) );
      FOR (sp, numStates) {
	kmatrix_set_entry( Tx[a], s, sp, p.T(s,a,sp) );
      }
      FOR (o, numObservations) {
	kmatrix_set_entry( Ox[a], s, o, p.O(s,a,o) );
      }
    }
  }

  // post-process
  copy( initialBelief, initialBeliefx );
  isTerminalState = isTerminalStatex;
  copy( R, Rx );
  Ttr.resize(numActions);
  O.resize(numActions);
  T.resize(numActions);
  FOR (a, numActions) {
    copy( T[a], Tx[a] );
    kmatrix_transpose_in_place( Tx[a] );
    copy( Ttr[a], Tx[a] );
    copy( O[a], Ox[a] );
  }

  gettimeofday(&endTime,0);

  double numSeconds = (endTime.tv_sec - startTime.tv_sec)
    + 1e-6 * (endTime.tv_usec - startTime.tv_usec);
  cout << "[file reading took " << numSeconds << " seconds]" << endl;

  debugDensity();
}

// this is functionally similar to readFromFile() but much faster.
// the POMDP file must obey a restricted syntax.
void Pomdp::readFromFileFast(const std::string& fileName)
{
  char buf[1<<20];
  int lineNumber;
  ifstream in;
  char sbuf[512];
  int numSizesSet = 0;
  bool inPreamble = true;
  char *data;
  timeval startTime, endTime;

  cout << "reading problem (in fast mode) from " << fileName << endl;
  gettimeofday(&startTime,0);

  in.open(fileName.c_str());
  if (!in) {
    cerr << "ERROR: couldn't open " << fileName << " for reading: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  dvector initialBeliefx;
  std::vector<bool> isTerminalStatex;
  kmatrix Rx;
  std::vector<kmatrix> Tx, Ox;

#define PM_PREFIX_MATCHES(X) \
  (0 == strncmp(buf,(X),strlen(X)))

  lineNumber = 1;
  while (!in.eof()) {
    in.getline(buf,sizeof(buf));
    if (in.fail() && !in.eof()) {
      cerr << "ERROR: readFromFileFast: line too long for buffer"
	   << " (max length " << sizeof(buf) << ")" << endl;
      exit(EXIT_FAILURE);
    }

    if ('#' == buf[0]) continue;
    trimTrailingWhiteSpace(buf);
    if ('\0' == buf[0]) continue;
    
    if (inPreamble) {
      if (PM_PREFIX_MATCHES("discount:")) {
	if (1 != sscanf(buf,"discount: %lf", &discount)) {
	  cerr << "ERROR: line " << lineNumber
	       << ": syntax error in discount statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
      } else if (PM_PREFIX_MATCHES("values:")) {
	if (1 != sscanf(buf,"values: %s", sbuf)) {
	  cerr << "ERROR: line " << lineNumber
	       << ": syntax error in values statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	if (0 != strcmp(sbuf,"reward")) {
	  cerr << "ERROR: line " << lineNumber
	       << ": can only handle values of type reward"
	       << endl;
	  exit(EXIT_FAILURE);
	}
      } else if (PM_PREFIX_MATCHES("actions:")) {
	if (1 != sscanf(buf,"actions: %d", &numActions)) {
	  cerr << "ERROR: line " << lineNumber
	       << ": syntax error in actions statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	numSizesSet++;
      } else if (PM_PREFIX_MATCHES("observations:")) {
	if (1 != sscanf(buf,"observations: %d", &numObservations)) {
	  cerr << "ERROR: line " << lineNumber
	       << ": syntax error in observations statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	numSizesSet++;
      } else if (PM_PREFIX_MATCHES("states:")) {
	if (1 != sscanf(buf,"states: %d", &numStates)) {
	  cerr << "ERROR: line " << lineNumber
	       << ": syntax error in states statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	numSizesSet++;
      } else {
	cerr << "ERROR: line " << lineNumber
	     << ": got unexpected statement type while parsing preamble"
	     << endl;
	exit(EXIT_FAILURE);
      }

      if (3 == numSizesSet) {
	// pre-process
	initialBeliefx.resize(numStates);
	set_to_zero(initialBeliefx);
	isTerminalStatex.resize(numStates, false);
	Rx.resize(numStates, numActions);
	Tx.resize(numActions);
	Ox.resize(numActions);
	FOR (a, numActions) {
	  Tx[a].resize(numStates, numStates);
	  Ox[a].resize(numStates, numObservations);
	}

	inPreamble = false;
      }

    } else {

      if (PM_PREFIX_MATCHES("start:")) {
	data = buf + strlen("start: ");
	readVector(data,initialBeliefx,numStates);
      } else if (PM_PREFIX_MATCHES("E:")) {
	int s;
	if (1 != sscanf(buf, "E: %d", &s)) {
	  cerr << "ERROR: line " << lineNumber
	       << ": syntax error in E statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	isTerminalStatex[s] = true;
      } else if (PM_PREFIX_MATCHES("R:")) {
	int s, a;
	double reward;
	if (3 != sscanf(buf,"R: %d : %d : * : * %lf", &a, &s, &reward)) {
	  cerr << "ERROR: line " << lineNumber
	       << ": syntax error in R statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	kmatrix_set_entry( Rx, s, a, reward );
      } else if (PM_PREFIX_MATCHES("T:")) {
	int s, a, sp;
	double prob;
	if (4 != sscanf(buf,"T: %d : %d : %d %lf", &a, &s, &sp, &prob)) {
	  cerr << "ERROR: line " << lineNumber
	       << ": syntax error in T statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	kmatrix_set_entry( Tx[a], s, sp, prob );
      } else if (PM_PREFIX_MATCHES("O:")) {
	int s, a, o;
	double prob;
	if (4 != sscanf(buf,"O: %d : %d : %d %lf", &a, &s, &o, &prob)) {
	  cerr << "ERROR: line " << lineNumber
	       << ": syntax error in O statement"
	       << endl;
	  exit(EXIT_FAILURE);
	}
	kmatrix_set_entry( Ox[a], s, o, prob );
      } else {
	cerr << "ERROR: line " << lineNumber
	     << ": got unexpected statement type while parsing body"
	     << endl;
	exit(EXIT_FAILURE);
      }
    }

    lineNumber++;
  }

  in.close();

  // post-process
  copy( initialBelief, initialBeliefx );
  isTerminalState = isTerminalStatex;
  copy( R, Rx );
  Ttr.resize(numActions);
  O.resize(numActions);
  T.resize(numActions);
  FOR (a, numActions) {
    copy( T[a], Tx[a] );
    kmatrix_transpose_in_place( Tx[a] );
    copy( Ttr[a], Tx[a] );
    copy( O[a], Ox[a] );
  }

  gettimeofday(&endTime,0);

  double numSeconds = (endTime.tv_sec - startTime.tv_sec)
    + 1e-6 * (endTime.tv_usec - startTime.tv_usec);
  cout << "[file reading took " << numSeconds << " seconds]" << endl;

  debugDensity();
}

void Pomdp::debugDensity(void) {
  int Ttr_size = 0;
  int Ttr_filled = 0;
  int O_size = 0;
  int O_filled = 0;
  FOR (a, numActions) {
    Ttr_size += Ttr[a].size1() * Ttr[a].size2();
    O_size += O[a].size1() * O[a].size2();
#if !NO_COMPRESSED_MATRICES
    Ttr_filled += Ttr[a].filled();
    O_filled += O[a].filled();
#endif
  }
#if !NO_COMPRESSED_MATRICES
  cout << "T density = " << (((double) Ttr_filled) / Ttr_size)
       << ", O density = " << (((double) O_filled) / O_size)
       << endl;
#endif
}

}; // namespace pomdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/10/27 21:38:16  trey
 * renamed PomdpM to Pomdp
 *
 * Revision 1.7  2005/10/21 20:08:28  trey
 * added namespace pomdp
 *
 * Revision 1.6  2005/03/10 22:53:32  trey
 * now initialize T matrix even when using sla
 *
 * Revision 1.5  2005/02/08 23:54:25  trey
 * updated to use less type-specific function names
 *
 * Revision 1.4  2005/01/27 05:31:55  trey
 * switched to use Ttr instead of T under sla
 *
 * Revision 1.3  2005/01/26 04:10:41  trey
 * modified problem reading to work with sla
 *
 * Revision 1.2  2005/01/21 15:21:19  trey
 * added readFromFileFast
 *
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.2  2004/11/09 21:31:59  trey
 * got pomdp source tree into a building state again
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.7  2003/09/22 21:42:28  trey
 * made some minor fixes so that algorithm variants to compile
 *
 * Revision 1.6  2003/09/20 02:26:10  trey
 * found a major problem in F_a_o_transpose, now fixed; added command-line options for experiments
 *
 * Revision 1.5  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.4  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.3  2003/09/07 02:28:07  trey
 * started to adapt for boost matrix library
 *
 * Revision 1.2  2003/07/16 16:07:37  trey
 * added isTerminalState
 *
 * Revision 1.1  2003/06/26 15:41:22  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/
