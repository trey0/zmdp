/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.15 $  $Author: trey $  $Date: 2006-10-30 20:00:15 $
   
 @file    RTDP.cc
 @brief   Implementation of Barto, Bradke, and Singh's RTDP algorithm.

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

/**********************************************************************
  This is my implementation of the RTDP algorithm, based on the paper

    "Learning to Act Using Real-Time Dynamic Programming."
    A. Barto, S. Bradke, and S. Singh.
    Artificial Intelligence 72(1-2): 81-138. 1995.

  Inevitably they could not include all the details of the algorithm in
  their paper, so it is possible that my implementation differs from
  theirs in important ways.  They have not signed off on this
  implementation: use at your own risk.  (And please inform me if you
  find any errors!)

  -Trey Smith, Feb. 2006
 **********************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <queue>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "RTDP.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

RTDP::RTDP(void)
{}

void RTDP::trialRecurse(MDPNode& cn, int depth)
{
  // check for termination
  if (cn.isTerminal) {
    if (zmdpDebugLevelG >= 1) {
      printf("trialRecurse: depth=%d ubVal=%g terminal node (terminating)\n",
	     depth, cn.ubVal);
    }
    return;
  }

  // cached Q values must be up to date for subsequent calls
  int maxUBAction;
  bounds->update(cn, &maxUBAction);
  trackBackup(cn);

  int simulatedOutcome = bounds->getSimulatedOutcome(cn, maxUBAction);

  if (zmdpDebugLevelG >= 1) {
    printf("  trialRecurse: depth=%d a=%d o=%d ubVal=%g\n",
	   depth, maxUBAction, simulatedOutcome, cn.ubVal);
    printf("  trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
  }

  // recurse to successor
  trialRecurse(cn.getNextState(maxUBAction, simulatedOutcome), depth+1);

  bounds->update(cn, NULL);
  trackBackup(cn);
}

bool RTDP::doTrial(MDPNode& cn)
{
  if (zmdpDebugLevelG >= 1) {
    printf("-*- doTrial: trial %d\n", (numTrials+1));
  }

  trialRecurse(cn, 0);
  numTrials++;

  return false;
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.14  2006/10/19 19:31:26  trey
 * added support for backup logging
 *
 * Revision 1.13  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.12  2006/04/07 19:42:42  trey
 * removed initUpperBound argument to constructor
 *
 * Revision 1.11  2006/04/06 04:14:11  trey
 * changed how bounds are initialized
 *
 * Revision 1.10  2006/04/03 21:39:24  trey
 * updated to use IncrementalBounds
 *
 * Revision 1.9  2006/02/27 20:12:37  trey
 * cleaned up meta-information in header
 *
 * Revision 1.8  2006/02/19 18:33:47  trey
 * targetPrecision now stared as a field rather than passed around recursively
 *
 * Revision 1.7  2006/02/14 19:34:43  trey
 * now use targetPrecision properly
 *
 * Revision 1.6  2006/02/13 20:20:33  trey
 * refactored some common code from RTDP and LRTDP
 *
 * Revision 1.5  2006/02/13 19:08:49  trey
 * moved numBackups tracking code for better flexibility
 *
 * Revision 1.4  2006/02/11 22:38:10  trey
 * moved much of the RTDP implementation into RTDPCore, where it can be shared by many RTDP variants
 *
 * Revision 1.3  2006/02/10 20:14:33  trey
 * standardized fields in bounds.plot
 *
 * Revision 1.2  2006/02/10 19:33:32  trey
 * chooseAction() now relies on upper bound as it should (since the lower bound is not even calculated in vanilla RTDP!
 *
 * Revision 1.1  2006/02/09 21:59:04  trey
 * initial check-in
 *
 *
 ***************************************************************************/
