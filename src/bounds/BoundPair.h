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

#ifndef ZMDP_SRC_BOUNDS_BOUNDPAIR_H_
#define ZMDP_SRC_BOUNDS_BOUNDPAIR_H_

#include <iostream>
#include <string>
#include <vector>

#include "BoundPairCore.h"
#include "IncrementalLowerBound.h"
#include "IncrementalUpperBound.h"

using namespace sla;

namespace zmdp {

struct BoundPair : public BoundPairCore {
  MDP *problem;
  const ZMDPConfig *config;
  IncrementalLowerBound *lowerBound;
  IncrementalUpperBound *upperBound;
  bool maintainLowerBound;
  bool maintainUpperBound;
  bool useUpperBoundRunTimeActionSelection;
  bool dualPointBounds;
  double targetPrecision;

  BoundPair(bool _maintainLowerBound, bool _maintainUpperBound,
            bool _useUpperBoundRunTimeActionSelection, bool _dualPointBounds);

  void updateDualPointBounds(MDPNode &cn, int *maxUBActionP);

  void initialize(MDP *_problem, const ZMDPConfig *_config);

  MDPNode *getRootNode(void);
  MDPNode *getNode(const state_vector &s);
  MDPNode *getNodeOrNull(const state_vector &s) const;
  void expand(MDPNode &cn);
  void update(MDPNode &cn, int *maxUBActionP);
  int chooseAction(const state_vector &s) const;
  ValueInterval getValueAt(const state_vector &s) const;
  ValueInterval getQValue(const state_vector &s, int a) const;
  void writePolicy(const std::string &outFileName, bool canModifyBounds);
};

};  // namespace zmdp

#endif  // ZMDP_SRC_BOUNDS_BOUNDPAIR_H_
