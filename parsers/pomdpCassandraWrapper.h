/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2005-10-28 03:05:27 $
   
 @file    pomdpCassandraWrapper.h
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

#ifndef INCpomdpCassandraWrapper_h
#define INCpomdpCassandraWrapper_h

using namespace std;

#include <string>

// Exception class
class InputError { };

typedef int StateType;
typedef int ActionType;
typedef int ObsType;
typedef double ProbType;
typedef double ValueType;

struct PomdpCassandraWrapper {
  PomdpCassandraWrapper(void) {}

  int getNumStates(void) const;
  int getNumActions(void) const;
  int getNumObservations(void) const;
  ValueType getDiscount(void) const;
  ValueType getInitialBelief(StateType s) const;
  bool isTerminalState(StateType s) const;

  // rewards
  ValueType R(StateType s, ActionType a) const;

  // transition probabilities
  ProbType T(StateType s, ActionType a, StateType sp) const;

  // observation probabilities
  ProbType O(StateType sp, ActionType a, ObsType o) const;

  void readFromFile(const string& fileName);
};


#endif // INCpomdpCassandraWrapper_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
