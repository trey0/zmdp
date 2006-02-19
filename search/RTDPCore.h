/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.6 $  $Author: trey $  $Date: 2006-02-19 18:33:06 $
   
 @file    RTDPCore.h
 @brief   No brief

 Copyright (c) 2006, Trey Smith. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * The software may not be sold or incorporated into a commercial
   product without specific prior written permission.
 * The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 ***************************************************************************/

#ifndef INCRTDPCore_h
#define INCRTDPCore_h

#include <stack>

#include "MatrixUtils.h"
#include "Solver.h"
#include "AbstractBound.h"
#include "MDPCache.h"

#define OBS_IS_ZERO_EPS (1e-10)
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
  MDPNode* root;
  MDPHash* lookup;
  AbstractBound* initUpperBound;
  AbstractBound* initLowerBound;
  timeval boundsStartTime;
  timeval previousElapsedTime;
  int numStatesTouched;
  int numStatesExpanded;
  int numTrials;
  int numBackups;
  double lastPrintTime;
  std::ostream* boundsFile;
  bool initialized;
  double targetPrecision;

  RTDPCore(AbstractBound* _initUpperBound);

  void init(double _targetPrecision);
  MDPNode* getNode(const state_vector& s);
  void expand(MDPNode& cn);
  int getMaxUBAction(MDPNode& cn) const;
  int getSimulatedOutcome(MDPNode& cn, int a) const;
  void update(MDPNode& cn);

  // different derived classes (RTDP variants) will implement these
  // in varying ways
  virtual bool getUseLowerBound(void) const = 0;
  virtual void updateInternal(MDPNode& cn) = 0;
  virtual bool doTrial(MDPNode& cn) = 0;

  // virtual functions from Solver that constitute the external api
  void planInit(const MDP* pomdp, double _targetPrecision);
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
