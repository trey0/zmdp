/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1.1.1 $  $Author: trey $  $Date: 2004-11-09 16:18:56 $
 *  
 * PROJECT: FIRE Architecture Project
 *
 * @file    template.cc
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

#include "pomdp.h"
#include "mdp.h"

int Pomdp::getNumStates(void) const {
  return gNumStates;
}

int Pomdp::getNumActions(void) const {
  return gNumActions;
}

int Pomdp::getNumObservations(void) const {
  return gNumObservations;
}

ValueType Pomdp::getDiscount(void) const {
  return gDiscount;
}

ValueType Pomdp::getInitialBelief(StateType s) const {
  return gInitialBelief[s];
}

bool Pomdp::isTerminalState(StateType s) const {
  return gTerminalStates[s];
}

ValueType Pomdp::R(StateType s, ActionType a) const {
  return getEntryMatrix( Q, a, s );
}

ProbType Pomdp::T(StateType s, ActionType a, StateType sp) const {
  return getEntryMatrix( P[a], s, sp );
}

ProbType Pomdp::O(StateType sp, ActionType a, ObsType o) const {
  return getEntryMatrix( ::R[a], sp, o );
}

void Pomdp::readFromFile(const string& fileName) {
  if (! readMDP(const_cast<char *>(fileName.c_str())) ) {
    throw InputError();
  }
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
