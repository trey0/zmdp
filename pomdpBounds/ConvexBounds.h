/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.6 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
 @file    ConvexBounds.h
 @brief   No brief

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

#ifndef INCConvexBounds_h
#define INCConvexBounds_h

#include <iostream>
#include <string>
#include <vector>

#include "IncrementalBounds.h"
#include "MaxPlanesLowerBound.h"
#include "SawtoothUpperBound.h"

using namespace sla;

namespace zmdp {

struct ConvexBounds : public IncrementalBounds {
  const Pomdp* pomdp;
  MaxPlanesLowerBound* lowerBound;
  SawtoothUpperBound* upperBound;
  bool keepLowerBound;
  double targetPrecision;
  MDPNode* root;
  MDPHash* lookup;
  bool forceUpperBoundActionSelection;

  ConvexBounds(bool _keepLowerBound,
	       bool _forceUpperBoundActionSelection);

  // helper functions
  void getNewLBPlaneQ(LBPlane& result, const MDPNode& cn, int a);
  void getNewLBPlane(LBPlane& result, MDPNode& cn);
  void updateLowerBound(MDPNode& cn);
  double getNewUBValueQ(MDPNode& cn, int a);
  double getNewUBValueSimple(MDPNode& cn, int* maxUBActionP);
  double getNewUBValueUseCache(MDPNode& cn, int* maxUBActionP);
  double getNewUBValue(MDPNode& cn, int* maxUBActionP);
  void updateUpperBound(MDPNode& cn, int* maxUBActionP);

  // implementations of virtual functions declared in IncrementalBounds
  void initialize(const MDP* _pomdp,
		  double _targetPrecision);
  MDPNode* getRootNode(void);
  MDPNode* getNode(const state_vector& s);
  void expand(MDPNode& cn);
  void update(MDPNode& cn, int* maxUBActionP);
  int chooseAction(const state_vector& s) const;
  ValueInterval getValueAt(const state_vector& s) const;

  bool getSupportsPolicyOutput(void) const;
  void writePolicy(const std::string& outFileName);
};

}; // namespace zmdp

#endif // INCConvexBounds_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/04/27 23:10:31  trey
 * added support for writing policies
 *
 * Revision 1.4  2006/04/10 20:26:38  trey
 * added forceUpperBoundActionSelection
 *
 * Revision 1.3  2006/04/08 22:21:25  trey
 * fixed some bugs and added getNewUBValueUseCache()
 *
 * Revision 1.2  2006/04/06 20:34:47  trey
 * filled out most of ConvexBounds implementation
 *
 * Revision 1.1  2006/04/05 21:43:20  trey
 * collected and renamed several classes into pomdpBounds
 *
 *
 ***************************************************************************/

