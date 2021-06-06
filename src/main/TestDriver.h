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

#ifndef INCTestDriver_h
#define INCTestDriver_h

#include <vector>

#include "solverUtils.h"

namespace zmdp {

class TestDriver {
public:
  MDPSim *sim;
  dvector rewardSamples;

  TestDriver(void) : sim(NULL) {}
  ~TestDriver(void) {
    if (NULL != sim) {
      delete sim;
      sim = NULL;
    }
  }

  double getReward(void) { return sim->rewardSoFar; }
  void batchTestIncremental(const ZMDPConfig &config, int numIterations,
                            SolverObjects &so, int numSteps,
                            double minPrecision, double minOrder,
                            double maxOrder, double ticksPerOrder,
                            const std::string &incPlotFileName,
                            const std::string &boundsFileName,
                            const std::string &simFileName,
                            const char *outPolicyFileName);
};

}; // namespace zmdp

#endif // INCTestDriver_h
