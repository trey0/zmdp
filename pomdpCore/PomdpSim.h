/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
 @file    PomdpSim.h
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

#ifndef INCPomdpSim_h
#define INCPomdpSim_h

#include "AbstractSim.h"
#include "Pomdp.h"

namespace zmdp {

class PomdpSim : public AbstractSim {
public:
  Pomdp* pomdp;
  int elapsedTime;
  int state;
  cvector currentBelief;
  int lastState;
  
  PomdpSim(Pomdp* _pomdp);

  const MDP* getModel(void) const { return pomdp; }
  void restart(void);
  void performAction(int a);
  const state_vector& getInformationState(void) const { return currentBelief; }
};

}; // namespace zmdp

#endif // INCPomdpSim_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.1  2006/01/31 18:31:51  trey
 * moved many files from common to pomdpCore
 *
 * Revision 1.8  2006/01/28 22:00:45  trey
 * removed shadowed fields rewardSoFar and simOutFile, conflicting with parent class AbstractSim
 *
 * Revision 1.7  2006/01/28 03:04:39  trey
 * PomdpSim now inherits from AbstractSim
 *
 * Revision 1.6  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.5  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.4  2005/10/27 22:09:21  trey
 * rename PomdpM to Pomdp
 *
 * Revision 1.3  2005/10/21 20:08:53  trey
 * added namespace zmdp
 *
 * Revision 1.2  2005/01/26 04:11:31  trey
 * replaced bvector with cvector
 *
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.4  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.3  2003/09/06 21:51:53  trey
 * many changes
 *
 * Revision 1.2  2003/07/23 20:49:17  trey
 * published/fixed sparseRep
 *
 * Revision 1.1  2003/07/16 16:09:35  trey
 * initial check-in
 *
 *
 ***************************************************************************/
