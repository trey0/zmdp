/********** tell emacs we use -*- c++ -*- style comments *******************
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

#ifndef ZMDP_SRC_SEARCH_FRTDP_H_
#define ZMDP_SRC_SEARCH_FRTDP_H_

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

  void getNodeHandler(MDPNode &cn);
  static void staticGetNodeHandler(MDPNode &cn, void *handlerData);
  static double &getPrio(const MDPNode &cn);
  void getMaxPrioOutcome(MDPNode &cn, int a, FRTDPUpdateResult &result) const;
  void update(MDPNode &cn, FRTDPUpdateResult &result);
  void trialRecurse(MDPNode &cn, double logOcc, int depth);
  bool doTrial(MDPNode &cn);
  void derivedClassInit(void);
};

};  // namespace zmdp

#endif  // ZMDP_SRC_SEARCH_FRTDP_H_
