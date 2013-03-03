/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.8 $  $Author: trey $  $Date: 2006-10-20 04:55:49 $
   
 @file    HDP.h
 @brief   Implementation of Bonet and Geffner's HDP algorithm
          (as presented at IJCAI in 2003).

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

#ifndef INCHDP_h
#define INCHDP_h

#include "RTDPCore.h"

namespace zmdp {

struct HDPExtraNodeData {
  bool isSolved;
  int low, idx;
};

struct HDP : public RTDPCore {
  int index;
  NodeStack nodeStack;
  std::stack<MDPNode*> visited;

  HDP(void);

  void getNodeHandler(MDPNode& cn);
  static void staticGetNodeHandler(MDPNode& cn, void* handlerData);
  static bool& getIsSolved(const MDPNode& cn);
  static int& getLow(const MDPNode& cn);
  static int& getIdx(const MDPNode& cn);

  void cacheQ(MDPNode& cn);
  double residual(MDPNode& cn);

  void updateInternal(MDPNode& cn);
  bool trialRecurse(MDPNode& cn, int depth);
  bool doTrial(MDPNode& cn);
  void derivedClassInit(void);
};

}; // namespace zmdp

#endif /* INCHDP_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.7  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.6  2006/04/07 19:41:30  trey
 * removed initLowerBound, initUpperBound arguments to constructor
 *
 * Revision 1.5  2006/04/06 04:14:50  trey
 * changed how bounds are initialized
 *
 * Revision 1.4  2006/04/04 17:23:58  trey
 * modified to use IncrementalBounds methods
 *
 * Revision 1.3  2006/02/20 00:04:49  trey
 * added optional lower bound use
 *
 * Revision 1.2  2006/02/19 18:33:35  trey
 * targetPrecision now stared as a field rather than passed around recursively
 *
 * Revision 1.1  2006/02/17 18:20:55  trey
 * initial check-in
 *
 *
 ***************************************************************************/
