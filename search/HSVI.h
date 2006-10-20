/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-10-20 04:55:49 $
   
 @file    HSVI.h
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

#ifndef INCHSVI_h
#define INCHSVI_h

#include "RTDPCore.h"

namespace zmdp {

struct HSVIUpdateResult {
  int maxUBAction;
  double maxUBVal;
  double ubResidual;
  int maxExcessUncOutcome;
  double maxExcessUnc;
};

struct HSVI : public RTDPCore {
  double trialTargetPrecision;
#if USE_HSVI_ADAPTIVE_DEPTH
  double oldMaxDepth;
  double maxDepth;
  double oldQualitySum;
  int oldNumUpdates;
  double newQualitySum;
  int newNumUpdates;
#endif

  HSVI(void);

  void getMaxExcessUncOutcome(MDPNode& cn, int depth, HSVIUpdateResult& r) const;
  void update(MDPNode& cn, int depth, HSVIUpdateResult& result);
  void trialRecurse(MDPNode& cn, double logOcc, int depth);
  bool doTrial(MDPNode& cn);
};

}; // namespace zmdp

#endif /* INCHSVI_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.1  2006/04/27 20:18:08  trey
 * renamed WRTDP to HSVI, maybe less confusing this way
 *
 * Revision 1.1  2006/04/12 19:22:41  trey
 * initial check-in
 *
 *
 ***************************************************************************/
