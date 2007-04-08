/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.17 $  $Author: trey $  $Date: 2007-04-08 22:54:09 $
   
 @file    MaxPlanesLowerBound.h
 @brief   No brief

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

#include <string>
#include <vector>
#include <list>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"
#include "zmdpConfig.h"
#include "sla_mask.h"
#include "IncrementalLowerBound.h"
#include "BoundPairCore.h"
#include "Pomdp.h"

/**********************************************************************
 * CLASSES
 **********************************************************************/

namespace zmdp {

struct LBPlane {
  alpha_vector alpha;
  int action;
  sla::mvector mask;
  int numBackupsAtCreation;
  std::list<LBPlane**> backPointers;

  LBPlane(void);
  LBPlane(const alpha_vector& _alpha, int _action, const sla::mvector& _mask);
  void write(std::ostream& out, bool useMaxPlanesMasking) const;
};

typedef std::list< LBPlane* > PlaneSet;

struct MaxPlanesLowerBound : public IncrementalLowerBound {
  const Pomdp* pomdp;
  const ZMDPConfig* config;
  BoundPairCore* core;
  PlaneSet planes;
  int lastPruneNumPlanes;
  int lastPruneNumBackups;
  std::vector<PlaneSet> supportList;
  bool useMaxPlanesMasking;
  bool useMaxPlanesSupportList;
  bool useMaxPlanesCache;
  bool useMaxPlanesExtraPruning;
  bool initialized;
  
  MaxPlanesLowerBound(const MDP* _pomdp,
		      const ZMDPConfig* _config);
  ~MaxPlanesLowerBound(void);

  void initialize(double targetPrecision);
  double getValue(const belief_vector& b, const MDPNode* cn) const;
  void initNodeBound(MDPNode& cn);
  void update(MDPNode& cn);
  int chooseAction(const state_vector& b);

  void getNewLBPlaneQ(LBPlane& result, MDPNode& cn, int a);
  void getNewLBPlane(LBPlane& result, MDPNode& cn);
  void updateLowerBound(MDPNode& cn);
  void setPlaneForNode(MDPNode& cn, LBPlane* newPlane);
  const LBPlane& getPlaneForNode(MDPNode& cn);

  LBPlane& getBestLBPlane(const belief_vector& b);
  const LBPlane& getBestLBPlaneConst(const belief_vector& b) const;
  LBPlane& getBestLBPlaneWithCache(const belief_vector& b, LBPlane* currPlane,
				   int lastSetPlaneNumBackups);
  void addLBPlane(LBPlane* av);
  void prunePlanes(int numBackups);
  void maybePrune(int numBackups);
  void deleteAndForward(LBPlane* victim, LBPlane* dominator);

  void writeToFile(const std::string& outFileName) const;
  void readFromFile(const std::string& inFileName);
  void readFromCassandraAlphaFile(const std::string& inFileName);
  int getStorage(int whichMetric) const;
};

}; // namespace zmdp

#endif // INCMaxPlanesLowerBound_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.16  2007/03/23 00:04:59  trey
 * added an implementation of chooseAction() that uses the direct policy rather than the lookahead-one policy; should speed policy evaluation for benchmarking significantly
 *
 * Revision 1.15  2007/01/14 00:54:10  trey
 * added hooks for logging storage space during a run
 *
 * Revision 1.14  2006/10/25 19:13:01  trey
 * migrated to more run-time config parameters
 *
 * Revision 1.13  2006/10/24 19:12:13  trey
 * replaced useConvexSupportList with useMaxPlanesSupportList
 *
 * Revision 1.12  2006/10/24 02:12:47  trey
 * distributed update code from ConvexBounds to MaxPlanesLowerBound, allows more flexibility
 *
 * Revision 1.11  2006/10/18 18:07:13  trey
 * USE_TIME_WITHOUT_HEURISTIC is now a run-time config parameter
 *
 * Revision 1.10  2006/08/08 21:17:20  trey
 * fixed a bug in LB backPointers code; added USE_REF_COUNT_PRUNE
 *
 * Revision 1.9  2006/07/26 20:21:53  trey
 * new implementation of USE_CONVEX_CACHE; during pruning, now skip comparison of planes if they were compared during last pruning cycle
 *
 * Revision 1.8  2006/07/25 19:40:49  trey
 * added USE_CONVEX_CACHE support
 *
 * Revision 1.7  2006/07/24 17:07:47  trey
 * added USE_CONVEX_SUPPORT_LIST
 *
 * Revision 1.6  2006/07/14 15:09:13  trey
 * cleaned up pruning; removed belief argument from addLBPlane()
 *
 * Revision 1.5  2006/07/12 19:41:34  trey
 * cleaned out some cruft
 *
 * Revision 1.4  2006/06/14 18:17:21  trey
 * added abliity to read in a policy
 *
 * Revision 1.3  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.2  2006/04/27 23:10:31  trey
 * added support for writing policies
 *
 * Revision 1.1  2006/04/05 21:43:20  trey
 * collected and renamed several classes into pomdpBounds
 *
 * Revision 1.13  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.12  2005/11/08 18:13:41  trey
 * moved hash_map setup code to common/zmdpCommonTypes.h
 *
 * Revision 1.11  2005/10/28 03:52:15  trey
 * simplified license
 *
 * Revision 1.10  2005/10/28 02:55:36  trey
 * added copyright header
 *
 * Revision 1.9  2005/10/27 22:03:40  trey
 * cleaned out some cruft
 *
 * Revision 1.8  2005/10/21 20:19:37  trey
 * added namespace zmdp
 *
 * Revision 1.7  2005/03/28 18:14:03  trey
 * renamed updateAsSafety to useSafetyUpdate
 *
 * Revision 1.6  2005/03/25 21:43:26  trey
 * added updateAsSafety flag in BoundFunction and AlphaList, made some FocusedPomdp functions take a bound as an argument
 *
 * Revision 1.5  2005/03/25 19:23:39  trey
 * made lowerBoundV and upperBoundV explicit in FocusedPomdp
 *
 * Revision 1.4  2005/03/11 19:24:12  trey
 * switched from hash_map to list representation
 *
 * Revision 1.3  2005/03/10 21:14:06  trey
 * added masked alpha support
 *
 * Revision 1.2  2005/01/28 03:24:31  trey
 * removed testAlphaList and renamed bvector -> belief_vector
 *
 * Revision 1.1  2004/11/24 20:13:07  trey
 * split AlphaList.h out of ValueFunction.h
 *
 * Revision 1.2  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.6  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.5  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.4  2003/09/06 21:51:53  trey
 * many changes
 *
 * Revision 1.3  2003/07/23 20:50:00  trey
 * renamed AlphaVector::init() to copyFrom() to match convention used with other structs
 *
 * Revision 1.2  2003/07/16 16:14:07  trey
 * implemented AlphaList read() function, added support for tagging alpha vectors with actions
 *
 * Revision 1.1  2003/06/26 15:41:22  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/
