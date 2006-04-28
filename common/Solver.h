/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.11 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
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

#include "MDP.h"

namespace zmdp {

class Solver {
public:
  virtual ~Solver(void) {}

  // sets up the problem
  virtual void planInit(const MDP* problem,
			double targetPrecision) = 0;

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
  
};

}; // namespace zmdp

#endif // INCSolver_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
