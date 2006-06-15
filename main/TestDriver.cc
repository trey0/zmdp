/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-06-15 16:09:47 $

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

#include <iostream>
#include <fstream>

#include "zmdpCommonTime.h"
#include "TestDriver.h"
#include "zmdpCommonDefs.h"
#include "MatrixUtils.h"

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

#define NUM_SIM_ITERATIONS_TO_LOG (1)

void TestDriver::interleave(int numIterations,
			    AbstractSim* _sim,
			    Solver& solver,
			    int numSteps,
			    double minPrecision,
			    double minWait,
			    const string& scatterFileName,
			    const string& boundsFileNameFmt,
			    const string& simFileNameFmt)
{
  int action;
  belief_vector last_belief, diff;
  ofstream scatterFile, boundsFile, simFile;
  char boundsFileName[PATH_MAX], simFileName[PATH_MAX];
  double deltaTime, timeSoFar, episodeTimeSoFar;
  bool achievedPrecision, achievedTerminalState;

  sim = _sim;

  scatterFile.open( scatterFileName.c_str() );
  if (! scatterFile) {
    cerr << "ERROR: couldn't open " << scatterFileName << " for writing: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  system("mkdir -p plots");

  FOR (i, numIterations) {
    cout << "=-=-=-=-= interleave: trial " << (i+1) << " / " << numIterations << endl;

    // reset the planner
    solver.planInit(sim->getModel(), minPrecision);

    // set up a new bounds file and sim file for each iteration
    snprintf( boundsFileName, sizeof(boundsFileName), boundsFileNameFmt.c_str(), i );
    boundsFile.open( boundsFileName );
    if (! boundsFile) {
      cerr << "ERROR: couldn't open " << boundsFileName << " for writing: "
	   << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    solver.setBoundsFile(&boundsFile);
    
    snprintf( simFileName, sizeof(simFileName), simFileNameFmt.c_str(), i );
    simFile.open( simFileName );
    if (! simFile) {
      cerr << "ERROR: couldn't open " << simFileName << " for writing: "
	   << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    sim->simOutFile = &simFile;

    // restart the sim
    sim->restart();
    timeSoFar = 0;
    achievedTerminalState = false;

    // run in interleaved mode
    FOR (t, numSteps) {
      cout << "#-#-#-#-#-#-# interleave: trial " << (i+1) << " / " << numIterations
	   << ", time step " << (t+1) << " / " << numSteps << endl;
      // iterate the planner until it achieves the desired precision
      episodeTimeSoFar = 0.0;
      do {
	timeval plan_start = getTime();
	achievedPrecision =
	  solver.planFixedTime(sim->getModel()->getInitialState(),
			       /* maxTime = */ -1, /* minPrecision = */ minPrecision);
	deltaTime = timevalToSeconds(getTime() - plan_start);
	timeSoFar += deltaTime;
	episodeTimeSoFar += deltaTime;
      } while ( !(achievedPrecision && episodeTimeSoFar >= minWait) );

      // take one action
      action = solver.chooseAction(sim->getInformationState());
      sim->performAction(action);
      
      // check if we reached a terminal state
      if (sim->terminated) {
	achievedTerminalState = true;
	break;
      }
    }

    // record the total planning time and reward from this interleaved run
    scatterFile << timeSoFar << " "
		<< sim->rewardSoFar << " "
		<< achievedTerminalState << endl;

    boundsFile.close();
    simFile.close();
  }
  scatterFile.close();
}

void TestDriver::batchTestIncremental(int numIterations,
				      SolverObjects& so,
				      int numSteps,
				      double minPrecision,
				      double minOrder, double maxOrder,
				      double ticksPerOrder,
				      const string& incPlotFileName,
				      const string& boundsFileName,
				      const string& simFileName,
				      const char* outPolicyFileName)
{
  int action, last_action;
  belief_vector last_belief, diff;
  bool can_reuse_last_action;

  sim = so.sim;

  ofstream out( incPlotFileName.c_str() );
  if (! out) {
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

  so.solver->planInit(sim->getModel(), minPrecision);

  double timeSoFar = 1e-20;
  double logLastSimTime = -99;
  bool achieved_precision = false;
  bool achieved_terminal_state;
  while (!achieved_precision && timeSoFar < pow(10,maxOrder)) {
    timeval plan_start = getTime();
    achieved_precision =
      so.solver->planFixedTime(sim->getModel()->getInitialState(),
			       /* maxTime = */ -1, minPrecision);
    double deltaTime = timevalToSeconds(getTime() - plan_start);
    timeSoFar += deltaTime;

    sim->simOutFile = &simOutFile;

    if ((timeSoFar > pow(10,minOrder)
	 && log(timeSoFar) - logLastSimTime > ::log(10) / ticksPerOrder)
	// ensure we do a simulation after the last iteration
	|| timeSoFar >= pow(10,maxOrder)
	// or when the run ends because we achieved the desired precision
	|| achieved_precision) {
      logLastSimTime = ::log(timeSoFar);

      // write output policy at each evaluation epoch if that was requested
      if (NULL != outPolicyFileName) {
	// create a backup of the policy from the last epoch in case policy
	// writing is interrupted
	string cmd = string("mv ") + outPolicyFileName + " " + outPolicyFileName + ".bak >& /dev/null";
	system(cmd.c_str());

	so.bounds->writePolicy(outPolicyFileName);

	// delete the backup
	cmd = string("rm -f ") + outPolicyFileName + ".bak >& /dev/null";
	system(cmd.c_str());
      }

      // repeatedly simulate, reusing the initial solution
      simOutFile << "----- time " << timeSoFar << endl;
      rewardRecord.clear();
      cout << endl;
      int num_successes = 0;
      FOR (i, numIterations) {
	cout << "#-#-#-#-#-#-# batchTest " << (i+1) << " / " << numIterations;
	cout.flush();
	if (i >= NUM_SIM_ITERATIONS_TO_LOG) {
	  sim->simOutFile = NULL;
	}
	sim->restart();
	last_action = -1;
	achieved_terminal_state = false;
	FOR (j, numSteps) {
	  can_reuse_last_action = false;
	  if (-1 != last_action) {
	    diff = sim->getInformationState();
	    diff -= last_belief;
	    if (norm_inf(diff) < 1e-10) {
	      can_reuse_last_action = true;
	    }
	  }
	  if (can_reuse_last_action) {
	    action = last_action;
	  } else {
	    action = so.solver->chooseAction(sim->getInformationState());
	  }

	  last_action = action;
	  last_belief = sim->getInformationState();

	  sim->performAction(action);

	  if (sim->terminated) {
	    num_successes++;
	    achieved_terminal_state = true;
	    break;
	  }
	}
	cout << " (reward " << sim->rewardSoFar << ")" << endl;
	rewardRecord.push_back(sim->rewardSoFar);
	if (!achieved_terminal_state) {
	  cout << "(time ran out in simulation)" << endl;
	}
      }
      
      // collect statistics and write a line to the output file
      double avg, stdev;
      calc_avg_stdev_collection(rewardRecord.begin(), rewardRecord.end(), avg, stdev);
      
      double success_rate = ((double) num_successes) / numIterations;

#if 0
      ValueInterval val = so.solver.getValueAt(model->getInitialState());
      // for some reason, if i use sqrt() instead of ::sqrt(), it's ambiguous
      out << timeSoFar << " " << avg << " "
	  << (stdev/::sqrt(numIterations)*1.96) << " "
	  << val.l << " " << val.u << " " << success_rate << endl;
#else
      out << timeSoFar
	  << " " << avg
	  << " " << (stdev/::sqrt(numIterations)*1.96)
	  << " " << success_rate << endl;
#endif
     
      out.flush();
    }
  }
  out.close();
  boundsFile.close();
  simOutFile.close();
}
  
void TestDriver::printRewards(void) {
  cout << "rewards: ";
  FOR_EACH( reward, rewardRecord ) {
    cout << (*reward) << " ";
  }
  cout << endl;
  double avg, stdev;
  calc_avg_stdev_collection(rewardRecord.begin(), rewardRecord.end(), avg, stdev);
  cout << "reward avg stdev: " << avg << " " << stdev << endl;
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
