/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.22 $  $Author: trey $  $Date: 2007-04-03 06:08:51 $

 @file    TestDriver.cc
 @brief   No brief

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

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>

#include <iostream>
#include <fstream>

#include "zmdpCommonTime.h"
#include "TestDriver.h"
#include "zmdpCommonDefs.h"
#include "MatrixUtils.h"
#include "BoundPairExec.h"
#include "PolicyEvaluator.h"

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

void TestDriver::batchTestIncremental(const ZMDPConfig& config,
				      int numIterations,
				      SolverObjects& so,
				      int numSteps,
				      double minPrecision,
				      double firstEpochWallclockSeconds,
				      double terminateWallclockSeconds,
				      double ticksPerOrder,
				      const string& incPlotFileName,
				      const string& boundsFileName,
				      const string& simFileName,
				      const char* outPolicyFileName)
{
  belief_vector last_belief, diff;

  sim = so.sim;

  ofstream incPlotFile( incPlotFileName.c_str() );
  if (! incPlotFile) {
    cerr << "ERROR: couldn't open " << incPlotFileName << " for writing: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  ofstream boundsFile( boundsFileName.c_str() );
  if (! boundsFile) {
    cerr << "ERROR: couldn't open " << boundsFileName << " for writing: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  so.solver->setBoundsFile(&boundsFile);
  
  ofstream simOutFile( simFileName.c_str() );
  if (! simOutFile) {
    cerr << "ERROR: couldn't open " << simFileName << " for writing: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  int simulationTracesToLogPerEpoch = config.getInt("simulationTracesToLogPerEpoch");
  if (simulationTracesToLogPerEpoch < 0) {
    simulationTracesToLogPerEpoch = INT_MAX;
  }

  ofstream* storageOutputFile = NULL;
  string storageOutputFileName = config.getString("storageOutputFile");
  if (storageOutputFileName != "none") {
    storageOutputFile = new ofstream(storageOutputFileName.c_str());
    if (! storageOutputFile) {
      cerr << "ERROR: couldn't open " << storageOutputFile << " for writing: "
	   << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
  }

  double terminateLowerBoundValue = config.getDouble("terminateLowerBoundValue");
  double terminateUpperBoundValue = config.getDouble("terminateUpperBoundValue");

  printf("initializing solver (includes calculating initial bounds)\n");
  so.solver->planInit(sim->getModel(), &config);

  MDPNode* root = NULL;

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
    if (so.bounds->maintainLowerBound && so.bounds->maintainUpperBound
	&& minPrecision > 0) {
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

    if ((timeSoFar > firstEpochWallclockSeconds
	 && log(timeSoFar) - logLastSimTime > ::log(10) / ticksPerOrder)
	// ensure we do a simulation after the last iteration
	|| timeSoFar >= terminateWallclockSeconds
	// or when the run ends because the solver signaled it is done
	|| solverFinished) {
      logLastSimTime = ::log(timeSoFar);

      // write output policy at each evaluation epoch if that was requested
      if (NULL != outPolicyFileName) {
	// create a backup of the policy from the last epoch in case policy
	// writing is interrupted
	string cmd = string("mv ") + outPolicyFileName + " " + outPolicyFileName + ".bak >& /dev/null";
	system(cmd.c_str());

	so.bounds->writePolicy(outPolicyFileName, /* canModifyBounds = */ false);

	// delete the backup
	cmd = string("rm -f ") + outPolicyFileName + ".bak >& /dev/null";
	system(cmd.c_str());
      }

      // simulate running the policy many times and collect the per-run total reward values
      dvector rewardSamples;
      double successRate;
      eval.getRewardSamples(rewardSamples, successRate, /* verbose = */ true);

#if 0
      // collect policy evaluation statistics and write a line to the log file
      double avg, stdev;
      calc_avg_stdev_collection(rewardSamples.data.begin(), rewardSamples.data.end(),
				avg, stdev);

      incPlotFile << timeSoFar
		  << " " << avg
		  << " " << (stdev/::sqrt(numIterations)*1.96)
		  << " " << successRate << endl;
#endif

      // calculate summary statistics, mean and 95% confidence interval for the mean
      double mean, quantile1, quantile2;
      calc_bootstrap_mean_quantile(rewardSamples,
				   0.05, // 95% confidence interval
				   mean, quantile1, quantile2);

      incPlotFile << timeSoFar
		  << " " << mean
		  << " " << quantile1
		  << " " << quantile2
		  << " " << successRate << endl;
     
      incPlotFile.flush();

      // record storage space used by the bounds representation
      if (storageOutputFile) {
	char sbuf[1024];

	AbstractBound* lb = so.bounds->lowerBound;
	int lbNumElts1 = 0, lbNumEntries1 = 0, lbNumElts2 = 0, lbNumEntries2 = 0;
	if (lb) {
	  lbNumElts1    = lb->getStorage(ZMDP_S_NUM_ELTS);
	  lbNumEntries1 = lb->getStorage(ZMDP_S_NUM_ENTRIES);
	  lbNumElts2    = lb->getStorage(ZMDP_S_NUM_ELTS_TABULAR);
	  lbNumEntries2 = lb->getStorage(ZMDP_S_NUM_ENTRIES_TABULAR);
	}

	AbstractBound* ub = so.bounds->upperBound;
	int ubNumElts1 = 0, ubNumEntries1 = 0, ubNumElts2 = 0, ubNumEntries2 = 0;
	if (ub) {
	  ubNumElts1    = ub->getStorage(ZMDP_S_NUM_ELTS);
	  ubNumEntries1 = ub->getStorage(ZMDP_S_NUM_ENTRIES);
	  ubNumElts2    = ub->getStorage(ZMDP_S_NUM_ELTS_TABULAR);
	  ubNumEntries2 = ub->getStorage(ZMDP_S_NUM_ENTRIES_TABULAR);
	}

	int totalEntries = lbNumEntries1 + lbNumEntries2 + ubNumEntries1 + ubNumEntries2;
	
	snprintf(sbuf, sizeof(sbuf),
		 "%10lf %10d %10d %10d %10d %10d %10d %10d %10d %10d",
		 timeSoFar, totalEntries,
		 lbNumElts1, lbNumEntries1,
		 lbNumElts2, lbNumEntries2,
		 ubNumElts1, ubNumEntries1,
		 ubNumElts2, ubNumEntries2);

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
  
}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.21  2007/03/25 21:38:59  trey
 * removed obsolete printRewards() function
 *
 * Revision 1.20  2007/03/25 17:39:01  trey
 * simplified how success rate is tracked
 *
 * Revision 1.19  2007/03/25 15:16:10  trey
 * no longer deal with sample weights in policy evaluation
 *
 * Revision 1.18  2007/03/24 22:42:52  trey
 * now use MDPSim rather than abstract parent class AbstractSim; fixed to conform to new PolicyEvaluator constructor interface
 *
 * Revision 1.17  2007/03/23 02:20:17  trey
 * removed interleave() function, unused for a long time; policy evaluation now uses PolicyEvaluator class
 *
 * Revision 1.16  2007/01/15 21:16:37  trey
 * fixed logic bug in terminateUpperBoundValue support
 *
 * Revision 1.15  2007/01/15 20:22:04  trey
 * added support for terminate{Lower,Upper}BoundValue config parameters
 *
 * Revision 1.14  2007/01/15 17:24:20  trey
 * avoid crashing in getStorage() call when only one-sided bounds are being kept
 *
 * Revision 1.13  2007/01/14 00:53:51  trey
 * added ability to log storage space during a run
 *
 * Revision 1.12  2006/10/27 18:25:40  trey
 * logBackups() replaced with more general finishLogging()
 *
 * Revision 1.11  2006/10/24 02:09:18  trey
 * changes to match updated Solver interface
 *
 * Revision 1.10  2006/10/19 19:33:57  trey
 * added support for backup logging
 *
 * Revision 1.9  2006/10/18 18:46:47  trey
 * fixed spurious warning
 *
 * Revision 1.8  2006/10/18 18:30:51  trey
 * NUM_SIM_ITERATIONS_TO_LOG changed to a run-time parameter simulationTracesToLogPerEpoch
 *
 * Revision 1.7  2006/10/18 18:30:13  trey
 * NUM_SIM_ITERATIONS_TO_LOG changed to a run-time parameter simulationTracesToLogPerEpoch
 *
 * Revision 1.6  2006/10/18 18:05:56  trey
 * now propagating config data structure to lower levels so config fields can be used to control more parts of the system
 *
 * Revision 1.5  2006/10/16 05:49:12  trey
 * replaced minOrder, maxOrder with more intuitive parameters
 *
 * Revision 1.4  2006/07/24 17:06:37  trey
 * added more debug output
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
 * Revision 1.13  2006/02/17 21:08:00  trey
 * moved planInit() call after log files are opened, avoids some header lines getting lost
 *
 * Revision 1.12  2006/02/15 16:21:16  trey
 * now ensure simulation is performed at the end of the run if it ends because the target precision was achieved
 *
 * Revision 1.11  2006/02/06 19:28:55  trey
 * now make a sim trace only for the first simulation run in each batch, saves much time and disk space
 *
 * Revision 1.10  2006/02/01 01:09:37  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.9  2006/01/28 03:02:14  trey
 * increased flexibiilty for use with mdps
 *
 * Revision 1.8  2005/11/03 20:23:22  trey
 * removed bounds information in inc.plot
 *
 * Revision 1.7  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.6  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.5  2005/10/27 22:11:43  trey
 * cleaned out cruft
 *
 * Revision 1.4  2005/10/21 20:06:32  trey
 * added shortcut for reusing actions if the belief has not changed
 *
 * Revision 1.4  2005/03/29 16:56:51  trey
 * added shortcut for reusing actions if the belief has not changed
 *
 * Revision 1.3  2005/03/28 18:10:50  trey
 * added debug statement during simulation
 *
 * Revision 1.2  2005/01/26 04:08:42  trey
 * switched to use sparseRep in debug print
 *
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.2  2004/11/09 21:31:59  trey
 * got pomdp source tree into a building state again
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.12  2003/09/22 16:57:57  trey
 * added tracking of success rate, and ensured we test simulation performance at the end of the run
 *
 * Revision 1.11  2003/09/20 02:26:09  trey
 * found a major problem in F_a_o_transpose, now fixed; added command-line options for experiments
 *
 * Revision 1.10  2003/09/18 21:00:29  trey
 * upper bound self-update seems to be working
 *
 * Revision 1.9  2003/09/17 20:54:24  trey
 * seeing good performance on tagAvoid (but some mods since run started...)
 *
 * Revision 1.8  2003/09/17 18:30:16  trey
 * seems to show best performance so far
 *
 * Revision 1.7  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.6  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.5  2003/09/06 21:51:53  trey
 * many changes
 *
 * Revision 1.4  2003/07/24 15:33:43  trey
 * working test configuration for self-update
 *
 * Revision 1.3  2003/07/22 19:59:37  trey
 * several minor fixes; still observing discrepancy between sim and solver lower bound
 *
 * Revision 1.2  2003/07/17 22:19:53  trey
 * fixed problems with timing early in the solution run, added upperBoundQ
 *
 * Revision 1.1  2003/07/16 16:09:36  trey
 * initial check-in
 *
 *
 ***************************************************************************/
