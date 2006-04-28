/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.6 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
 @file    PointBounds.h
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

#ifndef INCPointBounds_h
#define INCPointBounds_h

#include <iostream>
#include <string>
#include <vector>

#include "IncrementalBounds.h"

using namespace sla;

namespace zmdp {

struct PointBounds : public IncrementalBounds {
  const MDP* problem;
  AbstractBound* initLowerBound;
  AbstractBound* initUpperBound;
  double targetPrecision;
  MDPNode* root;
  MDPHash* lookup;
  bool forceUpperBoundActionSelection;

  PointBounds(void);

  // helper functions
  void updateValuesUB(MDPNode& cn, int* maxUBActionP);
  void updateValuesBoth(MDPNode& cn, int* maxUBActionP);

  // must be called before initialize()
  void setBounds(AbstractBound* _initLowerBound,
		 AbstractBound* _initUpperBound,
		 bool _forceUpperBoundActionSelection);

  // implementations of virtual functions declared in IncrementalBounds
  void initialize(const MDP* _problem,
		  double _targetPrecision);

  MDPNode* getRootNode(void);
  MDPNode* getNode(const state_vector& s);
  void expand(MDPNode& cn);
  void update(MDPNode& cn, int* maxUBActionP);
  int chooseAction(const state_vector& s) const;
  ValueInterval getValueAt(const state_vector& s) const;
};

}; // namespace zmdp

#endif // INCPointBounds_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/04/10 20:25:08  trey
 * added forceUpperBoundActionSelection
 *
 * Revision 1.4  2006/04/07 19:38:49  trey
 * moved getNodeHandler, handlerData from IncrementalBounds derived classes into parent class
 *
 * Revision 1.3  2006/04/06 20:33:51  trey
 * moved setGetNodeHandler() implementation from PointBounds -> IncrementalBounds
 *
 * Revision 1.2  2006/04/05 21:34:40  trey
 * changed initialization to match new IncrementalBounds API
 *
 * Revision 1.1  2006/04/04 17:22:51  trey
 * initial check-in
 *
 *
 ***************************************************************************/

