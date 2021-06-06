/********** tell emacs we use -*- c++ -*- style comments *******************
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

#include "RTDPCore.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <queue>

#include "BoundPairCore.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "StateLog.h"
#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

RTDPCore::RTDPCore(void) : boundsFile(NULL), initialized(false) {}

void RTDPCore::setBounds(BoundPairCore *_bounds) { bounds = _bounds; }

void RTDPCore::init(void) {
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
                  << ", # backups" << endl;
    boundsFile->flush();
  }

  derivedClassInit();

  initialized = true;
}

void RTDPCore::planInit(MDP *_problem, const ZMDPConfig *_config) {
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

bool RTDPCore::planFixedTime(const state_vector &s, double maxTimeSeconds,
                             double _targetPrecision) {
  boundsStartTime = getTime() - previousElapsedTime;

  if (!initialized) {
    boundsStartTime = getTime();
    init();
  }

  // disable this termination check for now
  // if (root->ubVal - root->lbVal < targetPrecision) return true;
  bool done = doTrial(*bounds->getRootNode());
  done = done || (bounds->numBackups >= terminateNumBackups);

  previousElapsedTime = getTime() - boundsStartTime;

  if (NULL != boundsFile) {
    double elapsed = timevalToSeconds(getTime() - boundsStartTime);
    if (done || (0 == lastPrintTime) || elapsed / lastPrintTime >= (1 + 1e-4)) {
      (*boundsFile) << timevalToSeconds(getTime() - boundsStartTime) << " "
                    << bounds->getRootNode()->lbVal << " "
                    << bounds->getRootNode()->ubVal << " "
                    << bounds->numStatesTouched << " "
                    << bounds->numStatesExpanded << " " << numTrials << " "
                    << bounds->numBackups << endl;
      boundsFile->flush();
      lastPrintTime = elapsed;
    }
  }

  return done;
}

// this implementation is not very efficient, but it is guaranteed not
// to modify the algorithm state, so it can safely be used for
// simulation testing in the middle of a run.
int RTDPCore::chooseAction(const state_vector &s) {
  return bounds->chooseAction(s);
}

void RTDPCore::setBoundsFile(std::ostream *_boundsFile) {
  boundsFile = _boundsFile;
}

ValueInterval RTDPCore::getValueAt(const state_vector &s) const {
  return bounds->getValueAt(s);
}

void RTDPCore::trackBackup(const MDPNode &backedUpNode) {
  if (useLogBackups) {
    backedUpNodes.push_back(&backedUpNode);
  }
}

void RTDPCore::maybeLogBackups(void) {
  if (!useLogBackups && qValuesOutputFile == "none") return;

  StateIndex index(problem->getNumStateDimensions());
  StateLog log(&index);
  FOR_EACH(node, backedUpNodes) { log.addState((*node)->s); }
  if (qValuesOutputFile != "none") {
    // make sure the index contains all *queried* states, not just all
    // backed up states.  (but the log will still have only backed up
    // states.)
    FOR_EACH(pr, *(bounds->lookup)) { index.getStateId(pr->second->s); }
  }

  index.writeToFile(stateIndexOutputFile);
  if (useLogBackups) {
    log.writeToFile(backupsOutputFile);
    index.writeBoundValuesToFile(boundValuesOutputFile, *bounds);
  }
  if (qValuesOutputFile != "none") {
    index.writeQValuesToFile(qValuesOutputFile, *bounds,
                             problem->getNumActions());
  }
}

void RTDPCore::finishLogging(void) { maybeLogBackups(); }

};  // namespace zmdp
