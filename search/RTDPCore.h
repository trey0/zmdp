/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.20 $  $Author: trey $  $Date: 2007-03-24 22:45:13 $
   
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
#include "BoundPairCore.h"

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
  MDP* problem;
  BoundPairCore* bounds;
  timeval boundsStartTime;
  timeval previousElapsedTime;
  int numTrials;
  double lastPrintTime;
  std::ostream* boundsFile;
  bool initialized;
  double targetPrecision;
  const ZMDPConfig* config;
  int terminateNumBackups;

  bool useLogBackups;
  std::string stateIndexOutputFile;
  std::string backupsOutputFile;
  std::string boundValuesOutputFile;
  std::string qValuesOutputFile;
  std::vector<const MDPNode*> backedUpNodes;

  RTDPCore(void);

  void setBounds(BoundPairCore* _bounds);
  void init(void);

  // different derived classes (RTDP variants) will implement these
  // in varying ways
  virtual bool doTrial(MDPNode& cn) = 0;
  virtual void derivedClassInit(void) {}

  // virtual functions from Solver that constitute the external api
  void planInit(MDP* problem, const ZMDPConfig* _config);
  bool planFixedTime(const state_vector& s,
		     double maxTimeSeconds,
		     double _targetPrecision);
  int chooseAction(const state_vector& s);
  void setBoundsFile(std::ostream* boundsFile);
  ValueInterval getValueAt(const state_vector& s) const;
  void trackBackup(const MDPNode& backedUpNode);
  void maybeLogBackups(void);
  void finishLogging(void);
};

}; // namespace zmdp

#endif /* INCRTDPCore_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.19  2006/11/07 20:07:12  trey
 * added support for qValuesOutputFile
 *
 * Revision 1.18  2006/10/27 18:24:29  trey
 * replaced logBackups() virtual function with finishLogging(), which provides a more general hook where other search strategies can do their cleanup actions
 *
 * Revision 1.17  2006/10/24 02:37:05  trey
 * updated for modified bounds interfaces
 *
 * Revision 1.16  2006/10/20 20:03:14  trey
 * added boundValuesOutputFile support
 *
 * Revision 1.15  2006/10/20 04:56:07  trey
 * removed obsolete getUseLowerBound() function
 *
 * Revision 1.14  2006/10/19 19:31:16  trey
 * added support for backup logging
 *
 * Revision 1.13  2006/10/18 18:06:26  trey
 * now propagating config data structure to lower levels so config fields can be used to control more parts of the system
 *
 * Revision 1.12  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
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
