/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2005-10-28 03:54:39 $
  
 @file    pomdpCassandraWrapper.cc
 @brief   A wrapper that provides access to the pomdp read in by
          Tony Cassandra's file reading code, without requiring you
          to (1) directly reference global variables or (2) include
          all of the headers like sparse-matrix.h.  Also uses notation
          I am more familiar with.

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
    throw InputError();
  }
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
