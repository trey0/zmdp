/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.11 $  $Author: trey $  $Date: 2007-01-14 00:54:10 $
   
 @file    SawtoothUpperBound.h
 @brief   No brief

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

#ifndef INCSawtoothUpperBound_h
#define INCSawtoothUpperBound_h

#include <list>

#include "zmdpConfig.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "IncrementalUpperBound.h"
#include "BoundPairCore.h"

#define PRUNE_EPS (1e-10)

namespace zmdp {

struct BVPair {
  belief_vector b;
  double v;
  double innerCornerCache;
  int numBackupsAtCreation;

  BVPair(void) {}
  BVPair(const belief_vector& _b, double _v) : b(_b), v(_v) {}
};

typedef std::list<BVPair*> BVList;

struct SawtoothUpperBound : public IncrementalUpperBound {
  const Pomdp* pomdp;
  const ZMDPConfig* config;
  BoundPairCore* core;
  int numStates;
  int lastPruneNumPts;
  int lastPruneNumBackups;
  BVList pts;
  sla::dvector cornerPts;
  std::vector<BVList> supportList;
  bool useSawtoothSupportList;

  SawtoothUpperBound(const MDP* _pomdp,
		     const ZMDPConfig* _config);
  ~SawtoothUpperBound(void);

  void initialize(double targetPrecision);
  double getValue(const belief_vector& b, const MDPNode* cn) const;
  void initNodeBound(MDPNode& cn);
  void update(MDPNode& cn, int* maxUBActionP);

  static double getBVValue(const belief_vector& b,
			   const BVPair* cPair,
			   double innerCornerPtsB,
			   double innerCornerPtsC);
  static bool dominates(const BVPair* xPair,
			const BVPair* yPair);
  
  void deleteAndForward(BVPair* victim,
			BVPair* dominator);
  void prune(int numBackups);
  void maybePrune(int numBackups);

  int whichCornerPoint(const belief_vector& b) const;
  void addPoint(const belief_vector& b, double val);
  void addPoint(BVPair* bv);
  void printToStream(std::ostream& out) const;

  double getNewUBValueQ(MDPNode& cn, int a);
  double getNewUBValueSimple(MDPNode& cn, int* maxUBActionP);
  double getNewUBValueUseCache(MDPNode& cn, int* maxUBActionP);
  double getNewUBValue(MDPNode& cn, int* maxUBActionP);
  void setUBForNode(MDPNode& cn, double newUB, bool addBV);
  double getUBForNode(MDPNode& cn);
  int getStorage(int whichMetric) const;
};

}; // namespace zmdp

#endif // INCBoundFunction_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.10  2006/10/24 19:12:32  trey
 * replaced useConvexSupportList with useSawtoothSupportList
 *
 * Revision 1.9  2006/10/24 02:13:04  trey
 * distributed update code from ConvexBounds to SawtoothUpperBound, allows more flexibility
 *
 * Revision 1.8  2006/10/18 18:07:13  trey
 * USE_TIME_WITHOUT_HEURISTIC is now a run-time config parameter
 *
 * Revision 1.7  2006/07/26 20:22:10  trey
 * new implementation of USE_CONVEX_CACHE; during pruning, now skip comparison of points if they were compared during last pruning cycle
 *
 * Revision 1.6  2006/07/24 17:08:02  trey
 * added USE_CONVEX_SUPPORT_LIST
 *
 * Revision 1.5  2006/07/14 15:09:48  trey
 * cleaned up pruning
 *
 * Revision 1.4  2006/07/12 19:45:55  trey
 * cleaned out copyFrom() cruft
 *
 * Revision 1.3  2006/04/28 18:53:23  trey
 * removed unused SawtoothWithQBound
 *
 * Revision 1.2  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.1  2006/04/05 21:43:20  trey
 * collected and renamed several classes into pomdpBounds
 *
 * Revision 1.16  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.15  2005/11/03 17:48:33  trey
 * removed MATRIX_NAMESPACE macro
 *
 * Revision 1.14  2005/10/28 03:52:15  trey
 * simplified license
 *
 * Revision 1.13  2005/10/28 02:55:36  trey
 * added copyright header
 *
 * Revision 1.12  2005/10/27 22:03:40  trey
 * cleaned out some cruft
 *
 * Revision 1.11  2005/10/21 20:20:09  trey
 * added namespace zmdp
 *
 * Revision 1.10  2005/03/28 18:14:35  trey
 * renamed updateAsSafety to useSafetyUpdate
 *
 * Revision 1.9  2005/03/25 21:43:26  trey
 * added updateAsSafety flag in BoundFunction and AlphaList, made some FocusedPomdp functions take a bound as an argument
 *
 * Revision 1.8  2005/03/11 19:24:35  trey
 * switched from hash_map to list representation
 *
 * Revision 1.7  2005/02/08 23:55:47  trey
 * updated to work when alpha_vector = cvector
 *
 * Revision 1.6  2005/02/06 16:41:30  trey
 * removed broken upperBoundInternal2 and added #if USE_CPLEX around upperBoundInternal1
 *
 * Revision 1.5  2005/02/04 21:03:40  trey
 * added upperBoundInternal3 and timing code for lp optimization
 *
 * Revision 1.4  2005/01/28 03:26:24  trey
 * removed spurious argument from resize() call
 *
 * Revision 1.3  2005/01/17 19:34:30  trey
 * added some test code for comparing upper bound mechanisms
 *
 * Revision 1.2  2004/11/24 20:51:32  trey
 * changed #include ValueFunction to #include AlphaList
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.8  2003/09/17 20:54:24  trey
 * seeing good performance on tagAvoid (but some mods since run started...)
 *
 * Revision 1.7  2003/09/16 00:57:01  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.6  2003/09/11 01:46:41  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.5  2003/07/18 16:07:59  trey
 * finished implementing Q caching
 *
 * Revision 1.4  2003/07/17 22:19:52  trey
 * fixed problems with timing early in the solution run, added upperBoundQ
 *
 * Revision 1.3  2003/07/17 18:50:40  trey
 * did prep for caching upper bound Q values
 *
 * Revision 1.2  2003/07/16 16:06:41  trey
 * alpha vectors now tagged with action, tweaked interface to LP solver
 *
 * Revision 1.1  2003/06/26 15:41:19  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/
