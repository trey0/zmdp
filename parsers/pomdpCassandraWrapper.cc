/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.6 $  $Author: trey $  $Date: 2006-05-26 00:59:30 $
  
 @file    pomdpCassandraWrapper.cc
 @brief   A wrapper that provides access to the pomdp read in by
          Tony Cassandra's file reading code, without requiring you
          to (1) directly reference global variables or (2) include
          all of the headers like sparse-matrix.h.  Also uses notation
          I am more familiar with.

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

#include <iostream>
using namespace std;

#include "pomdpCassandraWrapper.h"
#include "mdp.h"

int PomdpCassandraWrapper::getNumStates(void) const {
  return gNumStates;
}

int PomdpCassandraWrapper::getNumActions(void) const {
  return gNumActions;
}

int PomdpCassandraWrapper::getNumObservations(void) const {
  return gNumObservations;
}

ValueType PomdpCassandraWrapper::getDiscount(void) const {
  return gDiscount;
}

ValueType PomdpCassandraWrapper::getInitialBelief(StateType s) const {
  return gInitialBelief[s];
}

bool PomdpCassandraWrapper::isTerminalState(StateType s) const {
  return gTerminalStates[s];
}

ValueType PomdpCassandraWrapper::R(StateType s, ActionType a) const {
  return getEntryMatrix( Q, a, s );
}

ProbType PomdpCassandraWrapper::T(StateType s, ActionType a, StateType sp) const {
  return getEntryMatrix( P[a], s, sp );
}

ProbType PomdpCassandraWrapper::O(StateType sp, ActionType a, ObsType o) const {
  return getEntryMatrix( ::R[a], sp, o );
}

void PomdpCassandraWrapper::readFromFile(const string& fileName) {
  if (! readMDP(const_cast<char *>(fileName.c_str())) ) {
    //throw InputError();
    exit(EXIT_FAILURE);
  }
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.4  2005/10/28 03:54:39  trey
 * simplified license
 *
 * Revision 1.3  2005/10/28 03:05:27  trey
 * added copyright header
 *
 * Revision 1.2  2005/10/27 21:34:20  trey
 * continued renaming process
 *
 * Revision 1.1  2005/10/27 21:27:10  trey
 * renamed pomdp to pomdpCassandraWrapper
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.3  2003/07/16 16:12:44  trey
 * added support for terminal states
 *
 * Revision 1.2  2003/04/02 17:13:56  trey
 * minor changes to enable compilation under gcc 3.0.1
 *
 * Revision 1.1.1.1  2003/01/07 19:19:41  trey
 * Imported sources
 *
 *
 ***************************************************************************/
