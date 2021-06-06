/********** tell emacs we use -*- c++ -*- style comments *******************
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

#ifndef ZMDP_SRC_BOUNDS_POINTUPPERBOUND_H_
#define ZMDP_SRC_BOUNDS_POINTUPPERBOUND_H_

/**********************************************************************
 * INCLUDES
 **********************************************************************/

#include <list>
#include <string>
#include <vector>

#include "BoundPairCore.h"
#include "IncrementalUpperBound.h"
#include "MDPModel.h"
#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"
#include "zmdpConfig.h"

/**********************************************************************
 * CLASSES
 **********************************************************************/

namespace zmdp {

struct PointUpperBound : public IncrementalUpperBound {
  const MDP *problem;
  const ZMDPConfig *config;
  BoundPairCore *core;
  AbstractBound *initBound;
  double targetPrecision;

  PointUpperBound(const MDP *_pomdp, const ZMDPConfig *_config,
                  BoundPairCore *_core);
  ~PointUpperBound(void);

  void initialize(double _targetPrecision);
  double getValue(const state_vector &s, const MDPNode *cn) const;
  double getNewUBValueQ(MDPNode &cn, int a);
  void initNodeBound(MDPNode &cn);
  void updateSimple(MDPNode &cn, int *maxUBActionP);
  void updateUseCache(MDPNode &cn, int *maxUBActionP);
  void update(MDPNode &cn, int *maxUBActionP);
  int getStorage(int whichMetric) const;
};

};  // namespace zmdp

#endif  // ZMDP_SRC_BOUNDS_POINTUPPERBOUND_H_
