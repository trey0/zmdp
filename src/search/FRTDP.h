/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.14 $  $Author: trey $  $Date: 2006-10-20 04:55:49 $
   
 @file    FRTDP.h
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

#ifndef INCFRTDP_h
#define INCFRTDP_h

#include "RTDPCore.h"

namespace zmdp {

struct FRTDPUpdateResult {
  int maxUBAction;
  double ubResidual;
  int maxPrioOutcome;
  double maxPrio;
};

struct FRTDPExtraNodeData {
  double prio;
};

struct FRTDP : public RTDPCore {
  double oldMaxDepth;
  double maxDepth;
  double oldQualitySum;
  int oldNumUpdates;
  double newQualitySum;
  int newNumUpdates;

  FRTDP(void);

  void getNodeHandler(MDPNode& cn);
  static void staticGetNodeHandler(MDPNode& cn, void* handlerData);
  static double& getPrio(const MDPNode& cn);
  void getMaxPrioOutcome(MDPNode& cn, int a, FRTDPUpdateResult& result) const;
  void update(MDPNode& cn, FRTDPUpdateResult& result);
  void trialRecurse(MDPNode& cn, double logOcc, int depth);
  bool doTrial(MDPNode& cn);
  void derivedClassInit(void);
};

}; // namespace zmdp

#endif /* INCRTDP_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.13  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.12  2006/04/26 18:44:00  trey
 * removed variable r.maxUBVal that is no longer used
 *
 * Revision 1.11  2006/04/07 19:41:17  trey
 * removed initLowerBound, initUpperBound arguments to constructor
 *
 * Revision 1.10  2006/04/06 04:14:50  trey
 * changed how bounds are initialized
 *
 * Revision 1.9  2006/04/04 17:23:34  trey
 * made getPrio() static
 *
 * Revision 1.8  2006/04/03 21:39:03  trey
 * updated to use IncrementalBounds
 *
 * Revision 1.7  2006/03/21 21:24:43  trey
 * removed code that was obsoleted by new termination condition
 *
 * Revision 1.6  2006/03/21 21:09:24  trey
 * added adaptive maxDepth termination criterion
 *
 * Revision 1.5  2006/02/20 02:04:55  trey
 * changed altPrio margin to be based on occupancy
 *
 * Revision 1.4  2006/02/19 18:34:35  trey
 * lots of changes, trying out different termination approaches
 *
 * Revision 1.3  2006/02/15 16:24:27  trey
 * switched to a better termination criterion
 *
 * Revision 1.2  2006/02/14 19:34:34  trey
 * now use targetPrecision properly
 *
 * Revision 1.1  2006/02/13 21:46:46  trey
 * initial check-in
 *
 *
 ***************************************************************************/
