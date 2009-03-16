/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.26 $  $Author: trey $  $Date: 2007-04-19 22:18:46 $
   
 @file    RTDPCore.cc
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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

#include <iostream>
#include <fstream>
#include <queue>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "RTDPCore.h"
#include "BoundPairCore.h"
#include "StateLog.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

RTDPCore::RTDPCore(void) :
  boundsFile(NULL),
  initialized(false)
{}

void RTDPCore::setBounds(BoundPairCore* _bounds)
{
  bounds = _bounds;
}

void RTDPCore::init(void)
{
  bounds->initialize(problem, config);

  previousElapsedTime = secondsToTimeval(0.0);
  lastPrintTime = 0;

  numTrials = 0;

  if (NULL != boundsFile) {
    (*boundsFile) << "# wallclock time"
		  << ", lower bound"
		  << ", upper bound"
		  << ", # states touched"
		  << ", # states expanded"
		  << ", # trials"
		  << ", # backups"
		  << endl;
    boundsFile->flush();
  }

  derivedClassInit();

  initialized = true;
}

void RTDPCore::planInit(MDP* _problem,
			const ZMDPConfig* _config)
{
  config = _config;
  problem = _problem;
  initialized = false;
  targetPrecision = config->getDouble("terminateRegretBound");
  terminateNumBackups = config->getInt("terminateNumBackups");
  if (terminateNumBackups < 0) {
    terminateNumBackups = INT_MAX;
  }
  bool useTimeWithoutHeuristic = config->getBool("useTimeWithoutHeuristic");

  // backup logging setup
  useLogBackups = config->getBool("useLogBackups");
  stateIndexOutputFile = config->getString("stateIndexOutputFile");
  backupsOutputFile = config->getString("backupsOutputFile");
  boundValuesOutputFile = config->getString("boundValuesOutputFile");
  qValuesOutputFile = config->getString("qValuesOutputFile");

  if (useTimeWithoutHeuristic) {
    init();
  }
}

bool RTDPCore::planFixedTime(const state_vector& s,
			     double maxTimeSeconds,
			     double _targetPrecision)
{
  boundsStartTime = getTime() - previousElapsedTime;

  if (!initialized) {
    boundsStartTime = getTime();
    init();
  }

  // disable this termination check for now
  //if (root->ubVal - root->lbVal < targetPrecision) return true;
  bool done = doTrial(*bounds->getRootNode());
  done = done || (bounds->numBackups >= terminateNumBackups);

  previousElapsedTime = getTime() - boundsStartTime;

  if (NULL != boundsFile) {
    double elapsed = timevalToSeconds(getTime() - boundsStartTime);
    if (done || (0 == lastPrintTime) || elapsed / lastPrintTime >= (1+1e-4)) {
      (*boundsFile) << timevalToSeconds(getTime() - boundsStartTime)
		    << " " << bounds->getRootNode()->lbVal
		    << " " << bounds->getRootNode()->ubVal
		    << " " << bounds->numStatesTouched
		    << " " << bounds->numStatesExpanded
		    << " " << numTrials
		    << " " << bounds->numBackups
		    << endl;
      boundsFile->flush();
      lastPrintTime = elapsed;
    }
  }

  return done;
}

// this implementation is not very efficient, but it is guaranteed not
// to modify the algorithm state, so it can safely be used for
// simulation testing in the middle of a run.
int RTDPCore::chooseAction(const state_vector& s)
{
  return bounds->chooseAction(s);
}

void RTDPCore::setBoundsFile(std::ostream* _boundsFile)
{
  boundsFile = _boundsFile;
}

ValueInterval RTDPCore::getValueAt(const state_vector& s) const
{
  return bounds->getValueAt(s);
}

void RTDPCore::trackBackup(const MDPNode& backedUpNode)
{
  if (useLogBackups) {
    backedUpNodes.push_back(&backedUpNode);
  }
}

void RTDPCore::maybeLogBackups(void)
{
  if (!useLogBackups && qValuesOutputFile == "none") return;

  StateIndex index(problem->getNumStateDimensions());
  StateLog log(&index);
  FOR_EACH (node, backedUpNodes) {
    log.addState((*node)->s);
  }
  if (qValuesOutputFile != "none") {
    // make sure the index contains all *queried* states, not just all
    // backed up states.  (but the log will still have only backed up
    // states.)
    FOR_EACH (pr, *(bounds->lookup)) {
      index.getStateId(pr->second->s);
    }
  }

  index.writeToFile(stateIndexOutputFile);
  if (useLogBackups) {
    log.writeToFile(backupsOutputFile);
    index.writeBoundValuesToFile(boundValuesOutputFile, *bounds);
  }
  if (qValuesOutputFile != "none") {
    index.writeQValuesToFile(qValuesOutputFile, *bounds, problem->getNumActions());
  }
}

void RTDPCore::finishLogging(void)
{
  maybeLogBackups();
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.25  2007/04/03 06:05:53  trey
 * removed code that causes trials bounds values to not always be logged.  although sometimes useful, this could cause apparent differences in plots that should be absolutely identical.
 *
 * Revision 1.24  2007/03/24 22:45:13  trey
 * removed some const assertions for added implementation flexibility
 *
 * Revision 1.23  2006/11/07 20:07:12  trey
 * added support for qValuesOutputFile
 *
 * Revision 1.22  2006/10/27 18:24:29  trey
 * replaced logBackups() virtual function with finishLogging(), which provides a more general hook where other search strategies can do their cleanup actions
 *
 * Revision 1.21  2006/10/24 02:37:05  trey
 * updated for modified bounds interfaces
 *
 * Revision 1.20  2006/10/20 20:03:14  trey
 * added boundValuesOutputFile support
 *
 * Revision 1.19  2006/10/19 19:31:16  trey
 * added support for backup logging
 *
 * Revision 1.18  2006/10/18 18:06:26  trey
 * now propagating config data structure to lower levels so config fields can be used to control more parts of the system
 *
 * Revision 1.17  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.16  2006/04/07 19:43:26  trey
 * now initialize with a single IncrementalBounds object rather than a pair of AbstractBound objects for the upper and lower bounds
 *
 * Revision 1.15  2006/04/06 04:14:11  trey
 * changed how bounds are initialized
 *
 * Revision 1.14  2006/04/04 17:25:22  trey
 * moved chooseAction() implementation to IncrementalBounds
 *
 * Revision 1.13  2006/04/03 21:39:24  trey
 * updated to use IncrementalBounds
 *
 * Revision 1.12  2006/03/17 20:06:44  trey
 * added derivedClassInit() virtual function for more flexibility
 *
 * Revision 1.11  2006/02/27 20:12:37  trey
 * cleaned up meta-information in header
 *
 * Revision 1.10  2006/02/20 00:05:13  trey
 * added USE_RTDPCORE_UB_ACTION
 *
 * Revision 1.9  2006/02/19 18:34:52  trey
 * targetPrecision is now stored as a field; modified how prio is initialized in getNode(); lastPrintTime is now initialized properly
 *
 * Revision 1.8  2006/02/17 18:20:41  trey
 * renamed LStack -> NodeStack and moved it from LRTDP to RTDPCore so that it can also be used by HDP; added initialization of idx field of nodes in getNode()
 *
 * Revision 1.7  2006/02/15 16:26:15  trey
 * added USE_TIME_WITHOUT_HEURISTIC support, switched prio to be logarithmic, added tie-break condition for chooseAction()
 *
 * Revision 1.6  2006/02/14 19:34:34  trey
 * now use targetPrecision properly
 *
 * Revision 1.5  2006/02/13 21:47:27  trey
 * added initialization of prio field
 *
 * Revision 1.4  2006/02/13 20:20:33  trey
 * refactored some common code from RTDP and LRTDP
 *
 * Revision 1.3  2006/02/13 19:53:11  trey
 * reduced amount of debug output
 *
 * Revision 1.2  2006/02/13 19:08:49  trey
 * moved numBackups tracking code for better flexibility
 *
 * Revision 1.1  2006/02/11 22:38:10  trey
 * moved much of the RTDP implementation into RTDPCore, where it can be shared by many RTDP variants
 *
 *
 ***************************************************************************/
