/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2002-2006, Trey Smith. All rights reserved.

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

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include "TestDriver.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>

#include "BoundPairExec.h"
#include "MatrixUtils.h"
#include "PolicyEvaluator.h"
#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

void TestDriver::batchTestIncremental(
    const ZMDPConfig &config, int numIterations, SolverObjects &so,
    int numSteps, double minPrecision, double firstEpochWallclockSeconds,
    double terminateWallclockSeconds, double ticksPerOrder,
    const string &incPlotFileName, const string &boundsFileName,
    const string &simFileName, const char *outPolicyFileName) {
  belief_vector last_belief, diff;

  sim = so.sim;

  ofstream incPlotFile(incPlotFileName.c_str());
  if (!incPlotFile) {
    cerr << "ERROR: couldn't open " << incPlotFileName
         << " for writing: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  ofstream boundsFile(boundsFileName.c_str());
  if (!boundsFile) {
    cerr << "ERROR: couldn't open " << boundsFileName
         << " for writing: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  so.solver->setBoundsFile(&boundsFile);

  ofstream simOutFile(simFileName.c_str());
  if (!simOutFile) {
    cerr << "ERROR: couldn't open " << simFileName
         << " for writing: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  int simulationTracesToLogPerEpoch =
      config.getInt("simulationTracesToLogPerEpoch");
  if (simulationTracesToLogPerEpoch < 0) {
    simulationTracesToLogPerEpoch = INT_MAX;
  }

  ofstream *storageOutputFile = NULL;
  string storageOutputFileName = config.getString("storageOutputFile");
  if (storageOutputFileName != "none") {
    storageOutputFile = new ofstream(storageOutputFileName.c_str());
    if (!storageOutputFile) {
      cerr << "ERROR: couldn't open " << storageOutputFile
           << " for writing: " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
  }

  double terminateLowerBoundValue =
      config.getDouble("terminateLowerBoundValue");
  double terminateUpperBoundValue =
      config.getDouble("terminateUpperBoundValue");

  printf("initializing solver (includes calculating initial bounds)\n");
  so.solver->planInit(sim->getModel(), &config);

  MDPNode *root = NULL;

  printf("entering solver main loop\n");
  double timeSoFar = 1e-20;
  double logLastSimTime = -99;
  bool solverFinished = false;
  while (!solverFinished && timeSoFar < terminateWallclockSeconds) {
    timeval plan_start = getTime();
    solverFinished =
        so.solver->planFixedTime(sim->getModel()->getInitialState(),
                                 /* maxTime = */ -1, minPrecision);
    double deltaTime = timevalToSeconds(getTime() - plan_start);
    timeSoFar += deltaTime;

    if (NULL == root) {
      root = so.bounds->getRootNode();
    }
    if (so.bounds->maintainLowerBound && so.bounds->maintainUpperBound &&
        minPrecision > 0) {
      if ((root->ubVal - root->lbVal) <= minPrecision) {
        solverFinished = true;
      }
    }
    if (so.bounds->maintainLowerBound && terminateLowerBoundValue != -999) {
      if (root->lbVal >= terminateLowerBoundValue) {
        solverFinished = true;
      }
    }
    if (so.bounds->maintainUpperBound && terminateUpperBoundValue != -999) {
      if (root->ubVal <= terminateUpperBoundValue) {
        solverFinished = true;
      }
    }

    sim->simOutFile = &simOutFile;

    BoundPairExec exec;
    exec.init(so.problem, so.bounds);
    PolicyEvaluator eval(so.problem, &exec, &config,
                         /* assumeIdenticalModels = */ true);

    if ((timeSoFar > firstEpochWallclockSeconds &&
         log(timeSoFar) - logLastSimTime > ::log(10) / ticksPerOrder)
        // ensure we do a simulation after the last iteration
        || timeSoFar >= terminateWallclockSeconds
        // or when the run ends because the solver signaled it is done
        || solverFinished) {
      logLastSimTime = ::log(timeSoFar);

      // write output policy at each evaluation epoch if that was requested
      if (NULL != outPolicyFileName) {
        // create a backup of the policy from the last epoch in case policy
        // writing is interrupted
        string cmd = string("mv ") + outPolicyFileName + " " +
                     outPolicyFileName + ".bak >& /dev/null";
        if (0 != system(cmd.c_str())) {
          fprintf(stderr, "failed to make policy backup");
        }

        so.bounds->writePolicy(outPolicyFileName,
                               /* canModifyBounds = */ false);

        // delete the backup
        cmd = string("rm -f ") + outPolicyFileName + ".bak >& /dev/null";
        if (0 != system(cmd.c_str())) {
          fprintf(stderr, "failed to delete policy backup");
        }
      }

      // simulate running the policy many times and collect the per-run total
      // reward values
      dvector rewardSamples;
      double successRate;
      eval.getRewardSamples(rewardSamples, successRate, /* verbose = */ true);

#if 0
      // collect policy evaluation statistics and write a line to the log file
      double avg, stdev;
      calc_avg_stdev_collection(rewardSamples.data.begin(),
                                rewardSamples.data.end(),
                                avg, stdev);

      incPlotFile << timeSoFar
                  << " " << avg
                  << " " << (stdev/::sqrt(numIterations)*1.96)
                  << " " << successRate << endl;
#endif

      // calculate summary statistics, mean and 95% confidence interval for the
      // mean
      double mean, quantile1, quantile2;
      calc_bootstrap_mean_quantile(rewardSamples,
                                   0.05,  // 95% confidence interval
                                   mean, quantile1, quantile2);

      incPlotFile << timeSoFar << " " << mean << " " << quantile1 << " "
                  << quantile2 << " " << successRate << endl;

      incPlotFile.flush();

      // record storage space used by the bounds representation
      if (storageOutputFile) {
        char sbuf[1024];

        AbstractBound *lb = so.bounds->lowerBound;
        int lbNumElts1 = 0, lbNumEntries1 = 0, lbNumElts2 = 0,
            lbNumEntries2 = 0;
        if (lb) {
          lbNumElts1 = lb->getStorage(ZMDP_S_NUM_ELTS);
          lbNumEntries1 = lb->getStorage(ZMDP_S_NUM_ENTRIES);
          lbNumElts2 = lb->getStorage(ZMDP_S_NUM_ELTS_TABULAR);
          lbNumEntries2 = lb->getStorage(ZMDP_S_NUM_ENTRIES_TABULAR);
        }

        AbstractBound *ub = so.bounds->upperBound;
        int ubNumElts1 = 0, ubNumEntries1 = 0, ubNumElts2 = 0,
            ubNumEntries2 = 0;
        if (ub) {
          ubNumElts1 = ub->getStorage(ZMDP_S_NUM_ELTS);
          ubNumEntries1 = ub->getStorage(ZMDP_S_NUM_ENTRIES);
          ubNumElts2 = ub->getStorage(ZMDP_S_NUM_ELTS_TABULAR);
          ubNumEntries2 = ub->getStorage(ZMDP_S_NUM_ENTRIES_TABULAR);
        }

        int totalEntries =
            lbNumEntries1 + lbNumEntries2 + ubNumEntries1 + ubNumEntries2;

        snprintf(sbuf, sizeof(sbuf),
                 "%10lf %10d %10d %10d %10d %10d %10d %10d %10d %10d",
                 timeSoFar, totalEntries, lbNumElts1, lbNumEntries1, lbNumElts2,
                 lbNumEntries2, ubNumElts1, ubNumEntries1, ubNumElts2,
                 ubNumEntries2);

        (*storageOutputFile) << sbuf << endl;
        storageOutputFile->flush();
      }
    }
  }
  incPlotFile.close();
  boundsFile.close();
  simOutFile.close();
  if (storageOutputFile) {
    storageOutputFile->close();
  }

  so.solver->finishLogging();
}

};  // namespace zmdp
