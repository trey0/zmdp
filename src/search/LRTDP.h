/********** tell emacs we use -*- c++ -*- style comments *******************
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

#ifndef ZMDP_SRC_SEARCH_LRTDP_H_
#define ZMDP_SRC_SEARCH_LRTDP_H_

#include "RTDPCore.h"

namespace zmdp {

struct LRTDPExtraNodeData {
  bool isSolved;
};

struct LRTDP : public RTDPCore {
  LRTDP(void);

  void getNodeHandler(MDPNode &cn);
  static void staticGetNodeHandler(MDPNode &cn, void *handlerData);
  static bool &getIsSolved(const MDPNode &cn);
  void cacheQ(MDPNode &cn);
  double residual(MDPNode &cn);
  bool checkSolved(MDPNode &cn);

  void updateInternal(MDPNode &cn);
  bool trialRecurse(MDPNode &cn, int depth);
  bool doTrial(MDPNode &cn);
  void derivedClassInit(void);
};

};  // namespace zmdp

#endif  // ZMDP_SRC_SEARCH_LRTDP_H_
