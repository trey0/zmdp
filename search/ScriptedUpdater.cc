/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-10-20 04:58:08 $
   
 @file    ScriptedUpdater.cc
 @brief   No brief

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

#include <iostream>
#include <fstream>
#include <queue>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "ScriptedUpdater.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

ScriptedUpdater::ScriptedUpdater(void) :
  stateIndex(problem->getNumStateDimensions()),
  backupsLog(&stateIndex)
{}

void ScriptedUpdater::init(void)
{
  std::string backupScriptInputDir = config->getString("backupScriptInputDir");
  if (backupScriptInputDir == "undefined") {
    fprintf(stderr, "ERROR: when using searchStrategy='script' you must specify backupScriptInputDir (-h for help)\n");
    exit(EXIT_FAILURE);
  }
  std::string stateIndexInputFile =
    backupScriptInputDir + "/" + config->getString("stateIndexOutputFile");
  std::string backupsInputFile =
    backupScriptInputDir + "/" + config->getString("backupsOutputFile");

  stateIndex.readFromFile(stateIndexInputFile);
  backupsLog.readFromFile(backupsInputFile);
  currentLogEntry = 0;
}

bool ScriptedUpdater::doTrial(MDPNode& cn)
{
#if USE_DEBUG_PRINT
  printf("-*- doTrial: trial %d\n", (numTrials+1));
#endif

  if (!initialized) {
    init();
  }

  bool firstInTrial = true;
  int dummy;
  while (currentLogEntry < (int)backupsLog.size()) {
    int id = backupsLog.getLogEntry(currentLogEntry++);
    const state_vector& s = *stateIndex.entries[id];

    MDPNode& cn = *bounds->getNode(s);

    bounds->update(cn, &dummy);
    trackBackup(cn);

    if (!firstInTrial && 0 == id) {
      // returned to root node, marks end of trial, but not finished with script yet
      numTrials++;
      return false;
    }

    firstInTrial = false;
  }

  // ran out of backups in script, return and note we are finished
  numTrials++;
  return true;
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
