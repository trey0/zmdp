/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
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

#include "Solver.h"
#include "AbstractSim.h"

namespace zmdp {

class TestDriver {
public:
  AbstractSim* sim;
  std::vector<double> rewardRecord;
  
  TestDriver(void) : sim(NULL) {}
  ~TestDriver(void) {
    if (NULL != sim) {
      delete sim;
      sim = NULL;
    }
  }

  void interleave(int numIterations,
		  AbstractSim* _sim,
		  Solver& solver,
		  int numSteps,
		  double minPrecision,
		  double minWait,
		  const std::string& outFileName,
		  const std::string& boundsFileNameFmt,
		  const std::string& simFileNameFmt);
  double getReward(void) { return sim->rewardSoFar; }
  void batchTestIncremental(int numIterations,
			    AbstractSim* _sim,
			    Solver& solver, int numSteps,
			    double minPrecision,
			    double minOrder, double maxOrder, double ticksPerOrder,
			    const std::string& outFileName,
			    const std::string& boundsFileName,
			    const std::string& simFileName);
  void printRewards(void);

};

}; // namespace zmdp

#endif // INCTestDriver_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
