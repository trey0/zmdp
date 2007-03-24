/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.8 $  $Author: trey $  $Date: 2007-03-24 23:15:54 $
   
 @file    MDPSim.h
 @brief   No brief

 Copyright (c) 2002-2006, Trey Smith. All rights reserved.

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

#ifndef INCMDPSim_h
#define INCMDPSim_h

#include "MDPModel.h"

namespace zmdp {

struct MDPSim {
  MDP* model;
  cvector state;
  double rewardSoFar;
  bool terminated;
  std::ostream *simOutFile;
  int elapsedTime;
  int lastOutcomeIndex;
  
  MDPSim(MDP* _model);

  MDP* getModel(void) { return model; }
  void restart(void);
  void performAction(int a);
  const state_vector& getInformationState(void) const { return state; }
};

}; // namespace zmdp

#endif // INCMDPSim_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.7  2007/03/24 22:38:15  trey
 * removed dependence on AbstractSim; removed const-ness of getModel()
 *
 * Revision 1.6  2007/03/22 18:21:08  trey
 * added lastOutcomeIndex field
 *
 * Revision 1.5  2006/11/08 16:33:14  trey
 * renamed MDP.h to MDPModel.h to avoid confusion with mdp.h, stupid case insensitive Mac OS X
 *
 * Revision 1.4  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.3  2006/02/01 01:09:37  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.2  2006/01/28 22:00:36  trey
 * removed shadowed fields rewardSoFar and simOutFile, conflicting with parent class AbstractSim
 *
 * Revision 1.1  2006/01/28 03:01:05  trey
 * initial check-in
 *
 *
 ***************************************************************************/
