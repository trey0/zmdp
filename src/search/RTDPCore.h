/********** tell emacs we use -*- c++ -*- style comments *******************
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

#include "BoundPairCore.h"
#include "MatrixUtils.h"
#include "Solver.h"

#define RT_CLEAR_STD_STACK(x)                                                  \
  while (!(x).empty())                                                         \
    (x).pop();
#define RT_IDX_PLUS_INFINITY (INT_MAX)
#define RT_PRIO_MINUS_INFINITY (-99e+20)
#define RT_PRIO_IMPROVEMENT_CONSTANT (0.5)

namespace zmdp {

// data structure used by LRTDP and HDP: stack with O(1) element existence check
struct NodeStack {
  std::stack<MDPNode *> data;
  EXT_NAMESPACE::hash_map<MDPNode *, bool> lookup;

  void push(MDPNode *n) {
    data.push(n);
    lookup[n] = true;
  }
  MDPNode *pop(void) {
    MDPNode *n = data.top();
    data.pop();
    lookup.erase(n);
    return n;
  }
  void clear(void) {
    RT_CLEAR_STD_STACK(data);
    lookup.clear();
  }

  MDPNode *top(void) const { return data.top(); }
  bool empty(void) const { return data.empty(); }
  size_t size(void) const { return data.size(); }
  bool contains(MDPNode *n) const { return (lookup.end() != lookup.find(n)); }
};

struct RTDPCore : public Solver {
  MDP *problem;
  BoundPairCore *bounds;
  timeval boundsStartTime;
  timeval previousElapsedTime;
  int numTrials;
  double lastPrintTime;
  std::ostream *boundsFile;
  bool initialized;
  double targetPrecision;
  const ZMDPConfig *config;
  int terminateNumBackups;

  bool useLogBackups;
  std::string stateIndexOutputFile;
  std::string backupsOutputFile;
  std::string boundValuesOutputFile;
  std::string qValuesOutputFile;
  std::vector<const MDPNode *> backedUpNodes;

  RTDPCore(void);

  void setBounds(BoundPairCore *_bounds);
  void init(void);

  // different derived classes (RTDP variants) will implement these
  // in varying ways
  virtual bool doTrial(MDPNode &cn) = 0;
  virtual void derivedClassInit(void) {}

  // virtual functions from Solver that constitute the external api
  void planInit(MDP *problem, const ZMDPConfig *_config);
  bool planFixedTime(const state_vector &s, double maxTimeSeconds,
                     double _targetPrecision);
  int chooseAction(const state_vector &s);
  void setBoundsFile(std::ostream *boundsFile);
  ValueInterval getValueAt(const state_vector &s) const;
  void trackBackup(const MDPNode &backedUpNode);
  void maybeLogBackups(void);
  void finishLogging(void);
};

}; // namespace zmdp

#endif /* INCRTDPCore_h */
