/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-11-08 16:33:14 $
   
 @file    AbstractSim.h
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

#ifndef INCAbstractSim_h
#define INCAbstractSim_h

#include "MDPModel.h"

namespace zmdp {

class AbstractSim {
public:
  double rewardSoFar;
  bool terminated;
  std::ostream *simOutFile;
  
  virtual ~AbstractSim(void) {}

  virtual const MDP* getModel(void) const = 0;
  virtual void restart(void) = 0;
  virtual void performAction(int a) = 0;
  virtual const state_vector& getInformationState(void) const = 0;
};

}; // namespace zmdp

#endif // INCAbstractSim_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.2  2006/02/01 01:09:37  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.1  2006/01/28 03:01:39  trey
 * initial check-in
 *
 *
 ***************************************************************************/
