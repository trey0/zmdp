/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2007-04-08 22:46:57 $
   
 @file    BoundPairExec.h
 @brief   No brief

 Copyright (c) 2006, Trey Smith. All rights reserved.

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

#ifndef INCBoundPairExec_h
#define INCBoundPairExec_h

/**********************************************************************
 * INCLUDES
 **********************************************************************/

#include <iostream>

#include <string>
#include <vector>

#include "MDPExec.h"
#include "Pomdp.h"
#include "BoundPair.h"

/**********************************************************************
 * CLASSES
 **********************************************************************/

namespace zmdp {

struct BoundPairExec : public MDPExec {
  BoundPair* bounds;

  BoundPairExec(void);

  // initializer to use if you already have data structures for the model
  // and the bounds
  void init(MDP* _mdp, BoundPair* _bounds);

  // alternate initializer that reads the model and a policy from files
  void initReadFiles(const std::string& modelFileName,
		     const std::string& policyFileName,
		     const ZMDPConfig& config);

  // implement MDPExec virtual methods
  void setToInitialState(void);
  int chooseAction(void);
  void advanceToNextState(int a, int o);

  // can use for finer control
  void setBelief(const belief_vector& b);
};

}; // namespace zmdp

#endif // INCBoundPairExec_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2007/03/23 02:12:04  trey
 * fixed init() to take MDP* argument instead of Pomdp*
 *
 * Revision 1.1  2007/03/23 00:26:11  trey
 * renamed MaxPlanesLowerBoundExec to BoundPairExec
 *
 * Revision 1.4  2007/03/23 00:01:04  trey
 * fixed to reflect migration from PomdpExec to MDPExec base class
 *
 * Revision 1.3  2006/11/08 16:40:15  trey
 * renamed useFastParser to useFastModelParser
 *
 * Revision 1.2  2006/10/18 18:06:16  trey
 * now propagating config data structure to lower levels so config fields can be used to control more parts of the system
 *
 * Revision 1.1  2006/06/27 18:19:26  trey
 * initial check-in
 *
 *
 ***************************************************************************/
