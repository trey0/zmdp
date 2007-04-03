/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.6 $  $Author: trey $  $Date: 2007-04-03 06:07:55 $
   
 @file    BoundPairCore.h
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

#ifndef INCBoundPairCore_h
#define INCBoundPairCore_h

#include <iostream>
#include <string>
#include <vector>

#include "MDPCache.h"
#include "MDPModel.h"

#define BP_QVAL_UNDEFINED (-99e+20)

using namespace sla;

namespace zmdp {

typedef void (*GetNodeHandler)(MDPNode& cn, void* callbackData);

struct GetNodeHandlerStruct {
  GetNodeHandler h;
  void* hdata;
  GetNodeHandlerStruct(GetNodeHandler _h, void* _hdata) : h(_h), hdata(_hdata) {}
};

struct BoundPairCore {
  int numStatesTouched;
  int numStatesExpanded;
  int numBackups;
  std::vector<GetNodeHandlerStruct> getNodeHandlers;

  MDPNode* root;
  MDPHash* lookup;

  virtual ~BoundPairCore(void) {}

  virtual void initialize(MDP* _problem,
			  const ZMDPConfig* _config) = 0;

  virtual MDPNode* getRootNode(void) = 0;
  virtual MDPNode* getNode(const state_vector& s) = 0;
  virtual void expand(MDPNode& cn) = 0;
  virtual void update(MDPNode& cn, int* maxUBActionP) = 0;
  virtual int chooseAction(const state_vector& s) const = 0;
  virtual ValueInterval getValueAt(const state_vector& s) const = 0;
  virtual ValueInterval getQValue(const state_vector& s, int a) const = 0;

  virtual void writePolicy(const std::string& outFileName, bool canModifyBounds) { assert(0); }

  void addGetNodeHandler(GetNodeHandler getNodeHandler, void* handlerData);

  // relies on correct cached Q values!
  static int getMaxUBAction(MDPNode& cn);

  static int getSimulatedOutcome(MDPNode& cn, int a);
};

}; // namespace zmdp

#endif // INCBoundPairCore_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2007/03/24 22:39:57  trey
 * removed some const assertions
 *
 * Revision 1.4  2006/11/08 16:32:07  trey
 * renamed MDP.h to MDPModel.h to avoid confusion with mdp.h, stupid case insensitive Mac OS X
 *
 * Revision 1.3  2006/11/07 20:06:47  trey
 * added getQValue() function
 *
 * Revision 1.2  2006/10/24 19:11:38  trey
 * removed getSupportsPolicyOutput(), replaced by logic in solverUtils
 *
 * Revision 1.1  2006/10/24 02:05:48  trey
 * initial check-in
 *
 * Revision 1.8  2006/10/20 19:58:57  trey
 * added include of MDP.h
 *
 * Revision 1.7  2006/10/18 18:05:02  trey
 * now propagating config data structure to lower levels so config fields can be used to control more parts of the system
 *
 * Revision 1.6  2006/10/17 19:15:22  trey
 * moved root and lookup members from derived classes to IncrementalBounds
 *
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

