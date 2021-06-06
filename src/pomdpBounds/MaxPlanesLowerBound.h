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

#ifndef INCMaxPlanesLowerBound_h
#define INCMaxPlanesLowerBound_h

/**********************************************************************
 * INCLUDES
 **********************************************************************/

// this causes problems if it is included after the Lapack headers, so
//  pre-emptively include it here.  not sure exactly what the problem is.
#include <iostream>

#include <list>
#include <string>
#include <vector>

#include "BoundPairCore.h"
#include "IncrementalLowerBound.h"
#include "Pomdp.h"
#include "sla_mask.h"
#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"
#include "zmdpConfig.h"

/**********************************************************************
 * CLASSES
 **********************************************************************/

namespace zmdp {

struct LBPlane {
  alpha_vector alpha;
  int action;
  sla::mvector mask;
  int numBackupsAtCreation;
  std::list<LBPlane **> backPointers;

  LBPlane(void);
  LBPlane(const alpha_vector &_alpha, int _action, const sla::mvector &_mask);
  void write(std::ostream &out, bool useMaxPlanesMasking) const;
};

typedef std::list<LBPlane *> PlaneSet;

struct MaxPlanesLowerBound : public IncrementalLowerBound {
  const Pomdp *pomdp;
  const ZMDPConfig *config;
  BoundPairCore *core;
  PlaneSet planes;
  int lastPruneNumPlanes;
  int lastPruneNumBackups;
  std::vector<PlaneSet> supportList;
  bool useMaxPlanesMasking;
  bool useMaxPlanesSupportList;
  bool useMaxPlanesCache;
  bool useMaxPlanesExtraPruning;
  bool initialized;

  MaxPlanesLowerBound(const MDP *_pomdp, const ZMDPConfig *_config);
  ~MaxPlanesLowerBound(void);

  void initialize(double targetPrecision);
  double getValue(const belief_vector &b, const MDPNode *cn) const;
  void initNodeBound(MDPNode &cn);
  void update(MDPNode &cn);
  int chooseAction(const state_vector &b);

  void getNewLBPlaneQ(LBPlane &result, MDPNode &cn, int a);
  void getNewLBPlane(LBPlane &result, MDPNode &cn);
  void updateLowerBound(MDPNode &cn);
  void setPlaneForNode(MDPNode &cn, LBPlane *newPlane);
  const LBPlane &getPlaneForNode(MDPNode &cn);

  LBPlane &getBestLBPlane(const belief_vector &b);
  const LBPlane &getBestLBPlaneConst(const belief_vector &b) const;
  LBPlane &getBestLBPlaneWithCache(const belief_vector &b, LBPlane *currPlane,
                                   int lastSetPlaneNumBackups);
  void addLBPlane(LBPlane *av);
  void prunePlanes(int numBackups);
  void maybePrune(int numBackups);
  void deleteAndForward(LBPlane *victim, LBPlane *dominator);

  void writeToFile(const std::string &outFileName) const;
  void readFromFile(const std::string &inFileName);
  void readFromCassandraAlphaFile(const std::string &inFileName);
  int getStorage(int whichMetric) const;
};

}; // namespace zmdp

#endif // INCMaxPlanesLowerBound_h
