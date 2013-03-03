/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2007-04-08 22:48:04 $
  
 @file    CassandraParser.cc
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

#include "sparse-matrix.h"
#include "zmdpCommonDefs.h"
#include "MatrixUtils.h"
#include "slaMatrixUtils.h"
#include "sla_cassandra.h"
#include "CassandraParser.h"

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

void CassandraParser::readGenericDiscreteMDPFromFile(CassandraModel& mdp,
						     const std::string& _fileName)
{
  mdp.fileName = _fileName;
  readModelFromFile(mdp, /* expectPomdp = */ false);
}

void CassandraParser::readPomdpFromFile(CassandraModel& pomdp, const std::string& _fileName)
{
  pomdp.fileName = _fileName;
  readModelFromFile(pomdp, /* expectPomdp = */ true);
}

// This macro is a pure pass-through. I just use it to make clear
// that the argument is a global variable declared in Tony Cassandra's
// code base.
#define CASSANDRA_GLOBAL(X) (X)

void CassandraParser::readModelFromFile(CassandraModel& p,
					bool expectPomdp)
{
  timeval startTime, endTime;
  if (zmdpDebugLevelG >= 1) {
    cout << "reading problem from " << p.fileName << endl;
    gettimeofday(&startTime,0);
  }

  // this is the main call to Tony Cassandra's parsing code
  if (! readMDP(const_cast<char *>(p.fileName.c_str())) ) {
    // error messages should already have been printed
    exit(EXIT_FAILURE);
  }

  // from here forward, we're converting the model from the data structures in
  // Cassandra's library to sla data structures (with the names I'm used to)
  p.discount = CASSANDRA_GLOBAL(gDiscount);
  p.numStates = CASSANDRA_GLOBAL(gNumStates);
  p.numActions = CASSANDRA_GLOBAL(gNumActions);
  if (expectPomdp) {
    p.numObservations = CASSANDRA_GLOBAL(gNumObservations);
  } else {
    p.numObservations = -1;
  }

  // convert R to sla format
  kmatrix Rk;
  copy(Rk, CASSANDRA_GLOBAL(Q), p.numStates);
  kmatrix_transpose_in_place(Rk);
  copy(p.R, Rk);

  // convert T, Tr, and O to sla format
  kmatrix Tk;
  p.T.resize(p.numActions);
  p.Ttr.resize(p.numActions);
  if (expectPomdp) {
    p.O.resize(p.numActions);
  }
  FOR (a, p.numActions) {
    copy(Tk, CASSANDRA_GLOBAL(P[a]), p.numStates);
    copy(p.T[a], Tk);
    kmatrix_transpose_in_place(Tk);
    copy(p.Ttr[a], Tk);
    if (expectPomdp) {
      copy(p.O[a], CASSANDRA_GLOBAL(::R[a]), p.numObservations);
    }
  }

  if (expectPomdp) {
    // convert initialBelief to sla format
    dvector initialBeliefD;
    initialBeliefD.resize(p.numStates);
    FOR (s, p.numStates) {
      initialBeliefD(s) = CASSANDRA_GLOBAL(gInitialBelief[s]);
    }
    copy(p.initialBelief, initialBeliefD);
  } else {
    // convert initialState to sla format
    p.initialState.resize(1);
    p.initialState.push_back(0, CASSANDRA_GLOBAL(gInitialState));
  }

  p.checkForTerminalStates();

  if (zmdpDebugLevelG >= 1) {
    gettimeofday(&endTime,0);
    double numSeconds = (endTime.tv_sec - startTime.tv_sec)
      + 1e-6 * (endTime.tv_usec - startTime.tv_usec);
    cout << "[file reading took " << numSeconds << " seconds]" << endl;
    
    p.debugDensity();
  }

  // destroy intermediate data structures in Tony Cassandra's library
  deallocateMDP();
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/11/12 20:51:48  trey
 * added call to deallocateMDP() back in; it was accidentally left out when I migrated the translation code from pomdpCassandraWrapper to CassandraParser
 *
 * Revision 1.2  2006/11/09 20:48:40  trey
 * fixed some MDP vs. POMDP issues
 *
 * Revision 1.1  2006/11/08 16:40:50  trey
 * initial check-in
 *
 *
 ***************************************************************************/
