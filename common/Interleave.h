/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.4 $  $Author: trey $  $Date: 2005-10-27 22:28:54 $
 *  
 * @file    Interleave.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCInterleave_h
#define INCInterleave_h

#include <vector>

#include "Solver.h"
#include "PomdpSim.h"

namespace pomdp {

class Interleave {
public:
  PomdpSim* sim;
  std::vector<double> rewardRecord;
  
  Interleave(void) : sim(NULL) {}
  ~Interleave(void) {
    if (NULL != sim) {
      delete sim;
      sim = NULL;
    }
  }

  void interleave(int numIterations,
		  PomdpP pomdp,
		  Solver& solver,
		  int numSteps,
		  double minPrecision,
		  double minWait,
		  const std::string& outFileName,
		  const std::string& boundsFileNameFmt,
		  const std::string& simFileNameFmt);
  double getReward(void) { return sim->rewardSoFar; }
  void batchTestIncremental(int numIterations,
			    PomdpP pomdp, Solver& solver, int numSteps,
			    double minPrecision,
			    double minOrder, double maxOrder, double ticksPerOrder,
			    const std::string& outFileName,
			    const std::string& boundsFileName,
			    const std::string& simFileName);
  void printRewards(void);

};

}; // namespace pomdp

#endif // INCInterleave_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/10/27 22:11:43  trey
 * cleaned out cruft
 *
 * Revision 1.2  2005/10/21 20:07:00  trey
 * added namespace pomdp
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
