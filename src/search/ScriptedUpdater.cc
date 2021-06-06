/********** tell emacs we use -*- c++ -*- style comments *******************
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <queue>

#include "MatrixUtils.h"
#include "Pomdp.h"
#include "ScriptedUpdater.h"
#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

ScriptedUpdater::ScriptedUpdater(void)
    : filesRead(false), stateIndex(NULL), backupsLog(NULL) {}

void ScriptedUpdater::readFiles(void) {
  std::string backupScriptInputDir = config->getString("backupScriptInputDir");
  if (backupScriptInputDir == "none") {
    fprintf(stderr, "ERROR: when using searchStrategy='script' you must "
                    "specify backupScriptInputDir (-h for help)\n");
    exit(EXIT_FAILURE);
  }
  std::string stateIndexInputFile =
      backupScriptInputDir + "/" + config->getString("stateIndexOutputFile");
  std::string backupsInputFile =
      backupScriptInputDir + "/" + config->getString("backupsOutputFile");

  stateIndex = new StateIndex(problem->getNumStateDimensions());
  stateIndex->readFromFile(stateIndexInputFile);

  backupsLog = new StateLog(stateIndex);
  backupsLog->readFromFile(backupsInputFile);

  currentLogEntry = 0;

  boundValuesOutputFile = config->getString("boundValuesOutputFile");
}

bool ScriptedUpdater::doTrial(MDPNode &cn) {
  if (zmdpDebugLevelG >= 1) {
    printf("-*- doTrial: trial %d\n", (numTrials + 1));
  }

  if (!filesRead) {
    readFiles();
    filesRead = true;
  }

  bool firstInTrial = true;
  int dummy;
  while (currentLogEntry < (int)backupsLog->size()) {
    int id = backupsLog->getLogEntry(currentLogEntry++);
    const state_vector &s = *stateIndex->entries[id];

    MDPNode &cn = *bounds->getNode(s);

    bounds->update(cn, &dummy);
    trackBackup(cn);

    if (!firstInTrial && 0 == id) {
      // returned to root node, marks end of trial, but not finished with script
      // yet
      numTrials++;
      return false;
    }

    firstInTrial = false;
  }

  // ran out of backups in script, return true to indicate we are finished
  return true;
}

void ScriptedUpdater::finishLogging(void) {
  maybeLogBackups();
  stateIndex->writeBoundValuesToFile(boundValuesOutputFile, *bounds);
}

}; // namespace zmdp
