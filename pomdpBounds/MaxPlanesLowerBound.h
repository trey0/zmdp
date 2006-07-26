/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.9 $  $Author: trey $  $Date: 2006-07-26 20:21:53 $
   
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
#if USE_MASKED_ALPHA
#  include "sla_mask.h"
#endif
#include "AbstractBound.h"
#include "Pomdp.h"

/**********************************************************************
 * CLASSES
 **********************************************************************/

namespace zmdp {

struct LBPlane {
  alpha_vector alpha;
  int action;
#if USE_MASKED_ALPHA
  sla::mvector mask;
#endif
  int numBackupsAtCreation;
#if USE_CONVEX_CACHE
  std::vector<LBPlane**> backPointers;
#endif

  LBPlane(void);
#if USE_MASKED_ALPHA
  LBPlane(const alpha_vector& _alpha, int _action, const sla::mvector& _mask);
#else
  LBPlane(const alpha_vector& _alpha, int _action);
#endif
  void write(std::ostream& out) const;
};

typedef std::list< LBPlane* > PlaneSet;

class MaxPlanesLowerBound : public AbstractBound {
public:
  const Pomdp* pomdp;
  PlaneSet planes;
  int lastPruneNumPlanes;
  int lastPruneNumBackups;
#if USE_CONVEX_SUPPORT_LIST
  std::vector<PlaneSet> supportList;
#endif
  
  MaxPlanesLowerBound(const MDP* _pomdp);
  ~MaxPlanesLowerBound(void);

  // performs any computation necessary to initialize the bound
  void initialize(double targetPrecision);

  // returns the bound value at state s
  double getValue(const belief_vector& b) const;

  LBPlane& getBestLBPlane(const belief_vector& b);
  const LBPlane& getBestLBPlaneConst(const belief_vector& b) const;
#if USE_CONVEX_CACHE
  LBPlane& getBestLBPlaneWithCache(const belief_vector& b, LBPlane* currPlane,
				   int lastSetPlaneNumBackups);
#endif
  void addLBPlane(LBPlane* av);
  void prunePlanes(int numBackups);
  void maybePrune(int numBackups);
  void deleteAndForward(LBPlane* victim, LBPlane* dominator);

  void writeToFile(const std::string& outFileName) const;
  void readFromFile(const std::string& inFileName);
};

}; // namespace zmdp

#endif // INCMaxPlanesLowerBound_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
