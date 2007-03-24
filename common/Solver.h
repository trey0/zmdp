/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.17 $  $Author: trey $  $Date: 2007-03-24 22:38:55 $
   
 @file    Solver.h
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith. All rights reserved.

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

#ifndef INCSolver_h
#define INCSolver_h

#include "MDPModel.h"
#include "zmdpConfig.h"

namespace zmdp {

class Solver {
public:
  virtual ~Solver(void) {}

  // sets up the problem
  virtual void planInit(MDP* problem,
			const ZMDPConfig* config) = 0;

  // plan for a fixed amount of time.  if maxTimeSeconds < 0,
  //   the amount of time is chosen by the solver to optimize
  //   time performance.  returns true if targetPrecision has been
  //   reached.
  virtual bool planFixedTime(const state_vector& currentState,
			     double maxTimeSeconds,
			     double targetPrecision) = 0;

  virtual int chooseAction(const state_vector& currentState) = 0;

  virtual void setBoundsFile(std::ostream* boundsFile) = 0;
  virtual ValueInterval getValueAt(const state_vector& currentState) const = 0;

  // sets the minimum safety value, for a solver that understands safety
  virtual void setMinSafety(double _minSafety) {}
  
  // wraps up logging actions when the run is finished
  virtual void finishLogging(void) {}
};

}; // namespace zmdp

#endif // INCSolver_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.16  2006/11/08 16:33:14  trey
 * renamed MDP.h to MDPModel.h to avoid confusion with mdp.h, stupid case insensitive Mac OS X
 *
 * Revision 1.15  2006/10/27 18:24:29  trey
 * replaced logBackups() virtual function with finishLogging(), which provides a more general hook where other search strategies can do their cleanup actions
 *
 * Revision 1.14  2006/10/24 02:08:49  trey
 * changed some functions from pass-by-ref to pass-by-pointer to better match the rest of the system
 *
 * Revision 1.13  2006/10/19 19:32:33  trey
 * added hook for backup logging
 *
 * Revision 1.12  2006/10/18 18:05:20  trey
 * now propagating config data structure to lower levels so config fields can be used to control more parts of the system
 *
 * Revision 1.11  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.10  2006/02/15 16:21:37  trey
 * added targetPrecision argument to planInit()
 *
 * Revision 1.9  2006/02/01 01:09:37  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.8  2006/01/28 22:01:10  trey
 * switched include PomdpSim.h -> MDP.h
 *
 * Revision 1.7  2006/01/28 03:07:05  trey
 * improved flexibility for use with mdps
 *
 * Revision 1.6  2005/11/28 20:45:47  trey
 * fixed warning about non-virtual destructor
 *
 * Revision 1.5  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.4  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.3  2005/10/21 20:09:11  trey
 * added namespace zmdp
 *
 * Revision 1.2  2005/01/21 18:07:02  trey
 * preparing for transition to sla matrix types
 *
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.1  2003/09/23 21:11:51  trey
 * initial check-in
 *
 * Revision 1.1  2001/08/27 17:49:16  trey
 * initial check-in
 *
 *
 ***************************************************************************/
