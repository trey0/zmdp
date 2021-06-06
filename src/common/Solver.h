/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2002-2005, Trey Smith. All rights reserved.

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

#ifndef ZMDP_SRC_COMMON_SOLVER_H_
#define ZMDP_SRC_COMMON_SOLVER_H_

#include "MDPModel.h"
#include "zmdpConfig.h"

namespace zmdp {

class Solver {
 public:
  virtual ~Solver(void) {}

  // sets up the problem
  virtual void planInit(MDP *problem, const ZMDPConfig *config) = 0;

  // plan for a fixed amount of time.  if maxTimeSeconds < 0,
  //   the amount of time is chosen by the solver to optimize
  //   time performance.  returns true if targetPrecision has been
  //   reached.
  virtual bool planFixedTime(const state_vector &currentState,
                             double maxTimeSeconds, double targetPrecision) = 0;

  virtual int chooseAction(const state_vector &currentState) = 0;

  virtual void setBoundsFile(std::ostream *boundsFile) = 0;
  virtual ValueInterval getValueAt(const state_vector &currentState) const = 0;

  // sets the minimum safety value, for a solver that understands safety
  virtual void setMinSafety(double _minSafety) {}

  // wraps up logging actions when the run is finished
  virtual void finishLogging(void) {}
};

};  // namespace zmdp

#endif  // ZMDP_SRC_COMMON_SOLVER_H_
