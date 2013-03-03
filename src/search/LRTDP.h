/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.9 $  $Author: trey $  $Date: 2006-10-20 04:55:49 $
   
 @file    LRTDP.h
 @brief   Implementation of Bonet and Geffner's LRTDP algorithm.

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

#ifndef INCLRTDP_h
#define INCLRTDP_h

#include "RTDPCore.h"

namespace zmdp {

struct LRTDPExtraNodeData {
  bool isSolved;
};

struct LRTDP : public RTDPCore {
  LRTDP(void);

  void getNodeHandler(MDPNode& cn);
  static void staticGetNodeHandler(MDPNode& cn, void* handlerData);
  static bool& getIsSolved(const MDPNode& cn);
  void cacheQ(MDPNode& cn);
  double residual(MDPNode& cn);
  bool checkSolved(MDPNode& cn);

  void updateInternal(MDPNode& cn);
  bool trialRecurse(MDPNode& cn, int depth);
  bool doTrial(MDPNode& cn);
  void derivedClassInit(void);
};

}; // namespace zmdp

#endif /* INCLRTDP_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.8  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.7  2006/04/07 19:41:45  trey
 * removed initLowerBound, initUpperBound arguments to constructor
 *
 * Revision 1.6  2006/04/04 17:24:52  trey
 * modified to use IncrementalBounds methods
 *
 * Revision 1.5  2006/02/27 20:12:36  trey
 * cleaned up meta-information in header
 *
 * Revision 1.4  2006/02/19 18:35:09  trey
 * targetPrecision now stared as a field rather than passed around recursively
 *
 * Revision 1.3  2006/02/14 19:34:34  trey
 * now use targetPrecision properly
 *
 * Revision 1.2  2006/02/13 20:20:33  trey
 * refactored some common code from RTDP and LRTDP
 *
 * Revision 1.1  2006/02/13 19:09:24  trey
 * initial check-in
 *
 *
 ***************************************************************************/
