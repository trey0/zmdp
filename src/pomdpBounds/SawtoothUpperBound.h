/********** tell emacs we use -*- c++ -*- style comments *******************
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

#ifndef ZMDP_SRC_POMDPBOUNDS_SAWTOOTHUPPERBOUND_H_
#define ZMDP_SRC_POMDPBOUNDS_SAWTOOTHUPPERBOUND_H_

#include <list>

#include "BoundPairCore.h"
#include "IncrementalUpperBound.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "zmdpConfig.h"

#define PRUNE_EPS (1e-10)

namespace zmdp {

struct BVPair {
  belief_vector b;
  double v;
  double innerCornerCache;
  int numBackupsAtCreation;

  BVPair(void) {}
  BVPair(const belief_vector &_b, double _v) : b(_b), v(_v) {}
};

typedef std::list<BVPair *> BVList;

struct SawtoothUpperBound : public IncrementalUpperBound {
  const Pomdp *pomdp;
  const ZMDPConfig *config;
  BoundPairCore *core;
  int numStates;
  int lastPruneNumPts;
  int lastPruneNumBackups;
  BVList pts;
  sla::dvector cornerPts;
  std::vector<BVList> supportList;
  bool useSawtoothSupportList;

  SawtoothUpperBound(const MDP *_pomdp, const ZMDPConfig *_config);
  ~SawtoothUpperBound(void);

  void initialize(double targetPrecision);
  double getValue(const belief_vector &b, const MDPNode *cn) const;
  void initNodeBound(MDPNode &cn);
  void update(MDPNode &cn, int *maxUBActionP);

  static double getBVValue(const belief_vector &b, const BVPair *cPair,
                           double innerCornerPtsB, double innerCornerPtsC);
  static bool dominates(const BVPair *xPair, const BVPair *yPair);

  void deleteAndForward(BVPair *victim, BVPair *dominator);
  void prune(int numBackups);
  void maybePrune(int numBackups);

  int whichCornerPoint(const belief_vector &b) const;
  void addPoint(const belief_vector &b, double val);
  void addPoint(BVPair *bv);
  void printToStream(std::ostream &out) const;

  double getNewUBValueQ(MDPNode &cn, int a);
  double getNewUBValueSimple(MDPNode &cn, int *maxUBActionP);
  double getNewUBValueUseCache(MDPNode &cn, int *maxUBActionP);
  double getNewUBValue(MDPNode &cn, int *maxUBActionP);
  void setUBForNode(MDPNode &cn, double newUB, bool addBV);
  double getUBForNode(MDPNode &cn);
  int getStorage(int whichMetric) const;
};

};  // namespace zmdp

#endif  // ZMDP_SRC_POMDPBOUNDS_SAWTOOTHUPPERBOUND_H_
