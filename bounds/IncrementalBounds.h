/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.6 $  $Author: trey $  $Date: 2006-10-17 19:15:22 $
   
 @file    IncrementalBounds.h
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

#ifndef INCIncrementalBounds_h
#define INCIncrementalBounds_h

#include <iostream>
#include <string>
#include <vector>

#include "MDPCache.h"

using namespace sla;

namespace zmdp {

typedef void (*GetNodeHandler)(MDPNode& cn, void* callbackData);

struct IncrementalBounds {
  int numStatesTouched;
  int numStatesExpanded;
  int numBackups;
  GetNodeHandler getNodeHandler;
  void* handlerData;

  MDPNode* root;
  MDPHash* lookup;

  virtual ~IncrementalBounds(void) {}

  virtual void initialize(const MDP* _problem,
			  double _targetPrecision) = 0;

  virtual MDPNode* getRootNode(void) = 0;
  virtual MDPNode* getNode(const state_vector& s) = 0;
  virtual void expand(MDPNode& cn) = 0;
  virtual void update(MDPNode& cn, int* maxUBActionP) = 0;
  virtual int chooseAction(const state_vector& s) const = 0;
  virtual ValueInterval getValueAt(const state_vector& s) const = 0;

  virtual bool getSupportsPolicyOutput(void) const { return false; }
  virtual void writePolicy(const std::string& outFileName) { assert(0); }

  void setGetNodeHandler(GetNodeHandler getNodeHandler, void* handlerData);

  // relies on correct cached Q values!
  static int getMaxUBAction(MDPNode& cn);

  static int getSimulatedOutcome(MDPNode& cn, int a);
};

}; // namespace zmdp

#endif // INCIncrementalBounds_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.4  2006/04/27 22:58:37  trey
 * added API calls for writing policies
 *
 * Revision 1.3  2006/04/06 20:33:51  trey
 * moved setGetNodeHandler() implementation from PointBounds -> IncrementalBounds
 *
 * Revision 1.2  2006/04/05 21:33:07  trey
 * made some functions static
 *
 * Revision 1.1  2006/04/04 17:22:51  trey
 * initial check-in
 *
 *
 ***************************************************************************/

