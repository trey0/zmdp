/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.3 $  $Author: trey $  $Date: 2005-03-28 18:10:50 $
 *  
 * @file    Interleave.cc
 * @brief   No brief
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

#include "commonTime.h"
#include "Interleave.h"
#include "commonDefs.h"
#include "MatrixUtils.h"

using namespace std;
using namespace MatrixUtils;

void Interleave::interleave(PomdpP pomdp, Solver& solver, int numSteps,
			    double minPrecision,
			    double initialPlanTimeSeconds,
			    double perStepPlanTimeSeconds)
{
  if (initialPlanTimeSeconds == -1) {
    // batchTest passes in initialPlanTimeSeconds as -1 to instruct us to reuse
    // existing solution in the solver
    sim->restart();
    initialPlanTimeSeconds = 0;
  } else {
    sim = new PomdpSim(pomdp);
    solver.planInit(pomdp);
  }
  timeval startRun = getTime();
  FOR (i, numSteps) {
    double elapsed = timevalToSeconds(getTime() - startRun);
    double maxTime = (i == 0) ? initialPlanTimeSeconds : perStepPlanTimeSeconds;
    if (maxTime == 0.0) {
      cout << "(" << (i+1) << ") ";
      cout.flush();
    } else {
      cout << endl
	   << "interleave: " << (i+1) << " / " << numSteps
	   << "----------------" << endl;
      cout << "interleave: elapsed time total=" << elapsed
	   << ", per step=" << (elapsed / (i+1)) << endl;
      cout << "interleave: b=";
      cout << sparseRep(sim->currentBelief) << endl;
    }
    int action = solver.planFixedTime(sim->currentBelief, maxTime, minPrecision);
    sim->performAction(action);
  }
  cout << "interleave: run complete, total reward was " << getReward() << endl;
}

void Interleave::batchTestIncremental(int numIterations,
				      PomdpP pomdp, Solver& solver, int numSteps,
				      double minPrecision,
				      double minOrder, double maxOrder, double ticksPerOrder,
				      const string& outFileName,
				      const string& boundsFileName,
				      const string& simFileName)
{
  // make the initial solution
  sim = new PomdpSim(pomdp);
  solver.planInit(pomdp);

  ofstream out( outFileName.c_str() );
  if (! out) {
    cerr << "ERROR: couldn't open " << outFileName << " for writing: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  ofstream boundsFile( boundsFileName.c_str() );
  if (! boundsFile) {
    cerr << "ERROR: couldn't open " << boundsFileName << " for writing: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  solver.setBoundsFile(&boundsFile);
  
  ofstream simOutFile( simFileName.c_str() );
  if (! simOutFile) {
    cerr << "ERROR: couldn't open " << simFileName << " for writing: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  sim->simOutFile = &simOutFile;

  double timeSoFar = 1e-20;
  double logLastSimTime = -99;
  bool achieved_precision = false;
  bool achieved_terminal_state;
  while (!achieved_precision && timeSoFar < pow(10,maxOrder)) {
    timeval plan_start = getTime();
    achieved_precision =
      solver.planFixedTime(pomdp->initialBelief, /* maxTime = */ -1, minPrecision);
    double deltaTime = timevalToSeconds(getTime() - plan_start);
    timeSoFar += deltaTime;

    if ((timeSoFar > pow(10,minOrder)
	 && log(timeSoFar) - logLastSimTime > ::log(10) / ticksPerOrder)
	// ensure we do a simulation after the last iteration
	|| timeSoFar >= pow(10,maxOrder)) {
      logLastSimTime = ::log(timeSoFar);

      // repeatedly simulate, reusing the initial solution
      simOutFile << "----- time " << timeSoFar << endl;
      rewardRecord.clear();
      cout << endl;
      int num_successes = 0;
      FOR (i, numIterations) {
	cout << "#-#-#-#-#-#-# batchTest " << (i+1) << " / " << numIterations;
	cout.flush();
	sim->restart();
	achieved_terminal_state = false;
	FOR (j, numSteps) {
	  int action = solver.chooseAction(sim->currentBelief);
	  sim->performAction(action);
#if 0
	  cout << "(" << (j+1) << ") ";
	  cout.flush();
#endif
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

      ValueInterval val = solver.getValueAt(pomdp->initialBelief);
      // for some reason, if i use sqrt() instead of ::sqrt(), it's ambiguous
      out << timeSoFar << " " << avg << " " << (stdev/::sqrt(numIterations)*1.96) << " "
	  << val.l << " " << val.u << " " << success_rate << endl;
      out.flush();
    }
  }
  out.close();
  boundsFile.close();
  simOutFile.close();
}
  
void Interleave::printRewards(void) {
  cout << "rewards: ";
  FOR_EACH( reward, rewardRecord ) {
    cout << (*reward) << " ";
  }
  cout << endl;
  double avg, stdev;
  calc_avg_stdev_collection(rewardRecord.begin(), rewardRecord.end(), avg, stdev);
  cout << "reward avg stdev: " << avg << " " << stdev << endl;
}

#if 0

void testInterleave(void) {
  string prob_name = "network-e";

  PomdpP p = new PomdpM;
  string probfile = "examples/" + prob_name + ".pomdp";
  cout << "reading problem from " << probfile << endl;
  p->readFromFile(probfile);

  FocusedPomdp solver;

  Interleave x;
  x.interleave(p, solver,
	       /* numSteps = */ 20,
	       /* minPrecision = */ 1e-10,
	       /* initial time = */ 5.0,
	       /* per step time = */ 5.0);
}

void testBatch(void) {
  //string prob_name = "hallway2";
  string prob_name = "network-u";

  PomdpP p = new PomdpM;
  string probfile = "examples/" + prob_name + ".pomdp";
  cout << "reading problem from " << probfile << endl;
  p->readFromFile(probfile);

  FocusedPomdp solver;

  Interleave x;
  x.batchTestReuse(/* numIterations = */ 100,
		   p, solver,
		   /* numSteps = */ 20,
		   /* minPrecision = */ 1e-10,
		   /* initialPlanTime = */ 60);
  x.printRewards();
}

void Interleave::batchTestReuse(int numIterations,
				PomdpP pomdp, Solver& solver, int numSteps,
				double minPrecision,
				double initialPlanTimeSeconds)
{
  // make the initial solution
  sim = new PomdpSim(pomdp);
  solver.planInit(pomdp);
  solver.planFixedTime(pomdp->initialBelief, initialPlanTimeSeconds, minPrecision);
  
  // repeatedly simulate, reusing the initial solution
  FOR (i, numIterations) {
    cout << "#-#-#-#-#-#-# batchTest " << (i+1) << " / " << numIterations << endl;
    sim->restart();
    FOR (j, numSteps) {
      int action = solver.planFixedTime(sim->currentBelief, /* maxTime = */ 0,
					minPrecision);
      sim->performAction(action);
      //cout << "(" << (j+1) << ") ";
      cout.flush();
    }
    cout << "(reward " << sim->rewardSoFar << ")" << endl;
    rewardRecord.push_back(sim->rewardSoFar);
  }
}

void Interleave::batchTest(int numIterations,
			   PomdpP pomdp, Solver& solver, int numSteps,
			   double minPrecision,
			   double initialPlanTimeSeconds,
			   double perStepPlanTimeSeconds)
{
  FOR (i, numIterations) {
    cout << "#-#-#-#-#-#-# batchTest" << (i+1) << " / " << numIterations << endl;
    interleave(pomdp, solver, numSteps, minPrecision, initialPlanTimeSeconds,
	       perStepPlanTimeSeconds);
    rewardRecord.push_back(sim->rewardSoFar);
  }
}

#endif

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
