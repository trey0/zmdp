/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.7 $  $Author: trey $  $Date: 2007-03-25 21:39:06 $
   
 @file    TestDriver.h
 @brief   No brief

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
  MDPSim* sim;
  dvector rewardSamples;
  
  TestDriver(void) : sim(NULL) {}
  ~TestDriver(void) {
    if (NULL != sim) {
      delete sim;
      sim = NULL;
    }
  }

  double getReward(void) { return sim->rewardSoFar; }
  void batchTestIncremental(const ZMDPConfig& config,
			    int numIterations,
			    SolverObjects& so,
			    int numSteps,
			    double minPrecision,
			    double minOrder, double maxOrder, double ticksPerOrder,
			    const std::string& incPlotFileName,
			    const std::string& boundsFileName,
			    const std::string& simFileName,
			    const char* outPolicyFileName);

};

}; // namespace zmdp

#endif // INCTestDriver_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2007/03/24 22:42:52  trey
 * now use MDPSim rather than abstract parent class AbstractSim; fixed to conform to new PolicyEvaluator constructor interface
 *
 * Revision 1.5  2007/03/23 02:20:17  trey
 * removed interleave() function, unused for a long time; policy evaluation now uses PolicyEvaluator class
 *
 * Revision 1.4  2006/10/18 18:05:56  trey
 * now propagating config data structure to lower levels so config fields can be used to control more parts of the system
 *
 * Revision 1.3  2006/06/15 16:09:47  trey
 * restructured so zmdpBenchmark can output policies
 *
 * Revision 1.2  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.1  2006/04/27 23:16:45  trey
 * renamed common/Interleave to main/TestDriver
 *
 * Revision 1.8  2006/02/01 01:09:37  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.7  2006/01/28 03:02:14  trey
 * increased flexibiilty for use with mdps
 *
 * Revision 1.6  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.5  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.4  2005/10/27 22:28:54  trey
 * removed dependence on SmartRef header
 *
 * Revision 1.3  2005/10/27 22:11:43  trey
 * cleaned out cruft
 *
 * Revision 1.2  2005/10/21 20:07:00  trey
 * added namespace zmdp
 *
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.4  2003/09/20 02:26:09  trey
 * found a major problem in F_a_o_transpose, now fixed; added command-line options for experiments
 *
 * Revision 1.3  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.2  2003/09/06 21:51:53  trey
 * many changes
 *
 * Revision 1.1  2003/07/16 16:09:36  trey
 * initial check-in
 *
 *
 ***************************************************************************/
