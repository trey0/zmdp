/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.15 $  $Author: trey $  $Date: 2006-10-03 03:18:59 $
  
 @file    Pomdp.cc
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith. All rights reserved.

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
#include <sys/time.h>

#include <iostream>
#include <fstream>

#include "zmdpCommonDefs.h"
#include "pomdpCassandraWrapper.h"
#include "Pomdp.h"
#include "MatrixUtils.h"
#include "slaMatrixUtils.h"
#include "sla_cassandra.h"
#include "MaxPlanesLowerBound.h"
#include "SawtoothUpperBound.h"

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

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
 * POMDP FUNCTIONS
 ***************************************************************************/

Pomdp::Pomdp(const std::string& fileName, bool useFastParser, int _maxHorizon)
{
  readFromFile(fileName, useFastParser);
  maxHorizon = _maxHorizon;
}

void Pomdp::readFromFile(const std::string& fileName,
			 bool useFastParser)
{
  if (useFastParser) {
    readFromFileFast(fileName);
  } else {
    readFromFileCassandra(fileName);
  }

  // post-process: calculate isPomdpTerminalState
#if USE_DEBUG_PRINT
  printf("Pomdp::readFromFile: marking zero-reward absorbing states as terminal\n");
#endif
  isPomdpTerminalState.resize(numStates, /* initialValue = */ true);
  FOR (s, numStates) {
    FOR (a, numActions) {
      if ((fabs(1.0 - T[a](s,s)) > OBS_IS_ZERO_EPS) || R(s,a) != 0.0) {
	isPomdpTerminalState[s] = false;
	break;
      }
    }
  }
}

const belief_vector& Pomdp::getInitialBelief(void) const
{
  return initialBelief;
}

obs_prob_vector& Pomdp::getObsProbVector(obs_prob_vector& result,
					 const belief_vector& b,
					 int a) const
{
  dvector tmp;
  // --- overall: result = O_a' * T_a' * b
  // tmp = T_a' * b
  mult( tmp, Ttr[a], b );
  // result = O_a' * tmp
  mult( result, tmp, O[a] );
  
  return result;
}

belief_vector& Pomdp::getNextBelief(belief_vector& result,
				    const belief_vector& b,
				    int a, int o) const
{
  belief_vector tmp;

  // result = O_a(:,o) .* (T_a * b)
  mult( tmp, Ttr[a], b );
  emult_column( result, O[a], o, tmp );

  // renormalize
  result *= (1.0/norm_1(result));

  return result;
}

double Pomdp::getReward(const belief_vector& b, int a) const
{
  return inner_prod_column( R, a, b );
}

AbstractBound* Pomdp::newLowerBound(void) const
{
  return new MaxPlanesLowerBound(this);
}

AbstractBound* Pomdp::newUpperBound(void) const
{
  return new SawtoothUpperBound(this);
}

bool Pomdp::getIsTerminalState(const state_vector& s) const
{
  double nonTerminalSum = 0.0;
  FOR_CV (s) {
    if (!isPomdpTerminalState[CV_INDEX(s)]) {
      nonTerminalSum += CV_VAL(s);
    }
  }
  return (nonTerminalSum < 1e-10);
}

void Pomdp::readFromFileCassandra(const string& fileName) {
#if USE_DEBUG_PRINT
  timeval startTime, endTime;
  cout << "reading problem from " << fileName << endl;
  gettimeofday(&startTime,0);
#endif

  PomdpCassandraWrapper p;
  p.readFromFile(fileName);
  
  numStates = p.getNumStates();
  setBeliefSize(numStates);
  numActions = p.getNumActions();
  numObservations = p.getNumObservations();
  discount = p.getDiscount();

  // convert R to sla format
  kmatrix Rk;
  copy(Rk, p.getRTranspose(), numStates);
  kmatrix_transpose_in_place(Rk);
  copy(R, Rk);

  // convert T, Tr, and O to sla format
  kmatrix Tk;
  T.resize(numActions);
  Ttr.resize(numActions);
  O.resize(numActions);
  FOR (a, numActions) {
    copy(Tk, p.getT(a), numStates);
    copy(T[a], Tk);
    kmatrix_transpose_in_place(Tk);
    copy(Ttr[a], Tk);
    copy(O[a], p.getO(a), numObservations);
  }

  // convert initialBelief to sla format
  dvector initialBeliefD;
  initialBeliefD.resize(numStates);
  FOR (s, numStates) {
    initialBeliefD(s) = p.getInitialBelief(s);
  }
  copy(initialBelief, initialBeliefD);

#if USE_DEBUG_PRINT
  gettimeofday(&endTime,0);
  double numSeconds = (endTime.tv_sec - startTime.tv_sec)
    + 1e-6 * (endTime.tv_usec - startTime.tv_usec);
  cout << "[file reading took " << numSeconds << " seconds]" << endl;

  debugDensity();
#endif
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

#if USE_DEBUG_PRINT
  timeval startTime, endTime;
  cout << "reading problem (in fast mode) from " << fileName << endl;
  gettimeofday(&startTime,0);
#endif

  in.open(fileName.c_str());
  if (!in) {
    cerr << "ERROR: couldn't open " << fileName << " for reading: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  dvector initialBeliefx;
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
	setBeliefSize(numStates);
	initialBeliefx.resize(numStates);
	set_to_zero(initialBeliefx);
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

#if USE_DEBUG_PRINT
  gettimeofday(&endTime,0);
  double numSeconds = (endTime.tv_sec - startTime.tv_sec)
    + 1e-6 * (endTime.tv_usec - startTime.tv_usec);
  cout << "[file reading took " << numSeconds << " seconds]" << endl;
#endif

  debugDensity();
}

void Pomdp::debugDensity(void) {
  // use doubles to avoid int overflow (e.g. T_size is sometimes larger than MAX_INT)
  double T_size = ((double) T[0].size1()) * T[0].size2() * numActions;
  double O_size = ((double) O[0].size1()) * O[0].size2() * numActions;
  double T_filled = 0;
  double O_filled = 0;
  FOR (a, numActions) {
    T_filled += T[a].filled();
    O_filled += O[a].filled();
  }

  cout << "T density = " << (T_filled / T_size)
       << ", O density = " << (O_filled / O_size)
       << endl;
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.14  2006/07/25 19:42:20  trey
 * deleted some old unused code; modified density calculation to avoid int overflow
 *
 * Revision 1.13  2006/06/05 20:10:46  trey
 * filled in reasonable defaults for pomdp bounds
 *
 * Revision 1.12  2006/05/29 06:05:43  trey
 * now mark zero-reward absorbing states as terminal, without an explicit list in the pomdp model file
 *
 * Revision 1.11  2006/05/29 05:05:19  trey
 * updated handling of isTerminalState, no longer explicitly specified in the model file
 *
 * Revision 1.10  2006/05/27 19:02:18  trey
 * cleaned up CassandraMatrix -> sla::cmatrix conversion
 *
 * Revision 1.9  2006/04/28 18:53:57  trey
 * removed obsolete #if for NO_COMPRESSED_MATRICES
 *
 * Revision 1.8  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.7  2006/04/27 23:10:48  trey
 * put some output in USE_DEBUG_PRINT
 *
 * Revision 1.6  2006/04/06 04:12:54  trey
 * removed default bounds (newLowerBound() and newUpperBound())
 *
 * Revision 1.5  2006/02/17 18:36:35  trey
 * fixed getIsTerminalState() function so RTDP can be used
 *
 * Revision 1.4  2006/02/06 19:26:09  trey
 * removed numOutcomes from MDP class because some MDPs have a varying number of outcomes depending on state; replaced with numObservations in Pomdp class
 *
 * Revision 1.3  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.2  2006/01/31 20:12:44  trey
 * added newXXXBound() functions
 *
 * Revision 1.1  2006/01/31 18:31:50  trey
 * moved many files from common to pomdpCore
 *
 * Revision 1.6  2006/01/29 00:18:36  trey
 * added Pomdp() constructor that calls readFromFile()
 *
 * Revision 1.5  2006/01/28 03:03:23  trey
 * replaced BeliefMDP -> MDP, corresponding changes in API
 *
 * Revision 1.4  2005/11/03 17:45:30  trey
 * moved transition dynamics from HSVI implementation to Pomdp
 *
 * Revision 1.3  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.2  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.1  2005/10/27 21:38:16  trey
 * renamed PomdpM to Pomdp
 *
 * Revision 1.7  2005/10/21 20:08:28  trey
 * added namespace zmdp
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
