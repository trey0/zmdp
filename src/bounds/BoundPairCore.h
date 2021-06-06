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

#ifndef ZMDP_SRC_BOUNDS_BOUNDPAIRCORE_H_
#define ZMDP_SRC_BOUNDS_BOUNDPAIRCORE_H_

#include <iostream>
#include <string>
#include <vector>

#include "MDPCache.h"
#include "MDPModel.h"

#define BP_QVAL_UNDEFINED (-99e+20)

using namespace sla;

namespace zmdp {

typedef void (*GetNodeHandler)(MDPNode &cn, void *callbackData);

struct GetNodeHandlerStruct {
  GetNodeHandler h;
  void *hdata;
  GetNodeHandlerStruct(GetNodeHandler _h, void *_hdata)
      : h(_h), hdata(_hdata) {}
};

struct BoundPairCore {
  int numStatesTouched;
  int numStatesExpanded;
  int numBackups;
  std::vector<GetNodeHandlerStruct> getNodeHandlers;

  MDPNode *root;
  MDPHash *lookup;

  virtual ~BoundPairCore(void) {}

  virtual void initialize(MDP *_problem, const ZMDPConfig *_config) = 0;

  virtual MDPNode *getRootNode(void) = 0;
  virtual MDPNode *getNode(const state_vector &s) = 0;
  virtual void expand(MDPNode &cn) = 0;
  virtual void update(MDPNode &cn, int *maxUBActionP) = 0;
  virtual int chooseAction(const state_vector &s) const = 0;
  virtual ValueInterval getValueAt(const state_vector &s) const = 0;
  virtual ValueInterval getQValue(const state_vector &s, int a) const = 0;

  virtual void writePolicy(const std::string &outFileName,
                           bool canModifyBounds) {
    assert(0);
  }

  void addGetNodeHandler(GetNodeHandler getNodeHandler, void *handlerData);

  // relies on correct cached Q values!
  static int getMaxUBAction(MDPNode &cn);

  static int getSimulatedOutcome(MDPNode &cn, int a);
};

};  // namespace zmdp

#endif  // ZMDP_SRC_BOUNDS_BOUNDPAIRCORE_H_
