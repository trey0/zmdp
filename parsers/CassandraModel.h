/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-11-09 20:47:46 $
   
 @file    CassandraModel.h
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

#ifndef INCCassandraModel_h
#define INCCassandraModel_h

#include <iostream>
#include <string>
#include <vector>

#include "sla.h"
#include "MDPModel.h"

using namespace sla;

namespace zmdp {

struct CassandraModel : public MDP {
  int numStates, numObservations;

  CassandraModel(void);

  // initialState -- for MDPs
  state_vector initialState;
  // initialBelief(s) -- for POMDPs
  cvector initialBelief;
  // R(s,a)
  cmatrix R;
  // T[a](s,s'), Ttr[a](s',s), O[a](s',o)
  std::vector<cmatrix> T, Ttr, O;
  // isTerminalState[s] -- true if s is an absorbing state with 0 reward for all actions
  std::vector<bool> isTerminalState;

  // maxHorizon: see main/zmdp.config for an explanation
  int maxHorizon;

  void checkForTerminalStates(void);
  void debugDensity(void);
};

}; // namespace zmdp

#endif // INCCassandraModel_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/11/08 16:40:50  trey
 * initial check-in
 *
 *
 ***************************************************************************/

