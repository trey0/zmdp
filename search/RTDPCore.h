/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.12 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
 @file    RTDPCore.h
 @brief   Common code used by multiple RTDP variants found in this
          directory.

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

#ifndef INCRTDPCore_h
#define INCRTDPCore_h

#include <stack>

#include "MatrixUtils.h"
#include "Solver.h"
#include "IncrementalBounds.h"

#define RT_CLEAR_STD_STACK(x) while (!(x).empty()) (x).pop();
#define RT_IDX_PLUS_INFINITY (INT_MAX)
#define RT_PRIO_MINUS_INFINITY (-99e+20)
#define RT_PRIO_IMPROVEMENT_CONSTANT (0.5)

namespace zmdp {


// data structure used by LRTDP and HDP: stack with O(1) element existence check
struct NodeStack {
  std::stack<MDPNode*> data;
  EXT_NAMESPACE::hash_map<MDPNode*, bool> lookup;
  
  void push(MDPNode* n) {
    data.push(n);
    lookup[n] = true;
  }
  MDPNode* pop(void) {
    MDPNode* n = data.top();
    data.pop();
    lookup.erase(n);
    return n;
  }
  void clear(void) {
    RT_CLEAR_STD_STACK(data);
    lookup.clear();
  }

  MDPNode* top(void) const {
    return data.top();
  }
  bool empty(void) const {
    return data.empty();
  }
  size_t size(void) const {
    return data.size();
  }
  bool contains(MDPNode* n) const {
    return (lookup.end() != lookup.find(n));
  }
};

struct RTDPCore : public Solver {
  const MDP* problem;
  IncrementalBounds* bounds;
  timeval boundsStartTime;
  timeval previousElapsedTime;
  int numTrials;
  double lastPrintTime;
  std::ostream* boundsFile;
  bool initialized;
  double targetPrecision;

  RTDPCore(void);

  void setBounds(IncrementalBounds* _bounds);
  void init(double _targetPrecision);

  // different derived classes (RTDP variants) will implement these
  // in varying ways
  virtual bool getUseLowerBound(void) const = 0;
  virtual bool doTrial(MDPNode& cn) = 0;
  virtual void derivedClassInit(void) {}

  // virtual functions from Solver that constitute the external api
  void planInit(const MDP* problem, double _targetPrecision);
  bool planFixedTime(const state_vector& s,
		     double maxTimeSeconds,
		     double _targetPrecision);
  int chooseAction(const state_vector& s);
  void setBoundsFile(std::ostream* boundsFile);
  ValueInterval getValueAt(const state_vector& s) const;
};

}; // namespace zmdp

#endif /* INCRTDPCore_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.11  2006/04/07 19:43:26  trey
 * now initialize with a single IncrementalBounds object rather than a pair of AbstractBound objects for the upper and lower bounds
 *
 * Revision 1.10  2006/04/06 04:14:11  trey
 * changed how bounds are initialized
 *
 * Revision 1.9  2006/04/03 21:39:24  trey
 * updated to use IncrementalBounds
 *
 * Revision 1.8  2006/03/17 20:06:44  trey
 * added derivedClassInit() virtual function for more flexibility
 *
 * Revision 1.7  2006/02/27 20:12:37  trey
 * cleaned up meta-information in header
 *
 * Revision 1.6  2006/02/19 18:33:06  trey
 * targetPrecision is now stored as a field; modified how prio is initialized in getNode(); lastPrintTime is now initialized properly
 *
 * Revision 1.5  2006/02/17 18:20:41  trey
 * renamed LStack -> NodeStack and moved it from LRTDP to RTDPCore so that it can also be used by HDP; added initialization of idx field of nodes in getNode()
 *
 * Revision 1.4  2006/02/15 16:26:15  trey
 * added USE_TIME_WITHOUT_HEURISTIC support, switched prio to be logarithmic, added tie-break condition for chooseAction()
 *
 * Revision 1.3  2006/02/14 19:34:33  trey
 * now use targetPrecision properly
 *
 * Revision 1.2  2006/02/13 20:20:32  trey
 * refactored some common code from RTDP and LRTDP
 *
 * Revision 1.1  2006/02/11 22:38:10  trey
 * moved much of the RTDP implementation into RTDPCore, where it can be shared by many RTDP variants
 *
 *
 ***************************************************************************/
