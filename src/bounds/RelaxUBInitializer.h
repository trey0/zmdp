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

#ifndef ZMDP_SRC_BOUNDS_RELAXUBINITIALIZER_H_
#define ZMDP_SRC_BOUNDS_RELAXUBINITIALIZER_H_

#include "AbstractBound.h"
#include "MDPCache.h"
#include "MDPModel.h"
#include "MatrixUtils.h"

namespace zmdp {

struct RelaxUBInitializer : public AbstractBound {
  MDP *problem;
  MDPNode *root;
  MDPHash *lookup;
  AbstractBound *initLowerBound;
  AbstractBound *initUpperBound;
  const ZMDPConfig *config;

  RelaxUBInitializer(MDP *_problem, const ZMDPConfig *_config);
  virtual ~RelaxUBInitializer(void) {}

  MDPNode *getNode(const state_vector &s);
  void setup(double targetPrecision);
  void expand(MDPNode &cn);
  void updateInternal(MDPNode &cn);
  void update(MDPNode &cn);
  int getMaxUBAction(MDPNode &cn, double *maxUBValP,
                     double *secondBestUBValP) const;
  void trialRecurse(MDPNode &cn, double costSoFar, double altActionPrio,
                    int depth);
  void doTrial(MDPNode &cn, double pTarget);

  // implementation of AbstractBound interface
  void initialize(double targetPrecision);
  double getValue(const state_vector &s, const MDPNode *cn) const;
  int getStorage(int whichMetric) const;
};

};  // namespace zmdp

#endif  // ZMDP_SRC_BOUNDS_RELAXUBINITIALIZER_H_
