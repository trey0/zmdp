/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-02-14 19:34:33 $
   
 @file    RTDPCore.h
 @brief   No brief

 Copyright (c) 2006, Trey Smith. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * The software may not be sold or incorporated into a commercial
   product without specific prior written permission.
 * The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 ***************************************************************************/

#ifndef INCRTDPCore_h
#define INCRTDPCore_h

#include "MatrixUtils.h"
#include "Solver.h"
#include "AbstractBound.h"
#include "MDPCache.h"

#define OBS_IS_ZERO_EPS (1e-10)

namespace zmdp {

struct RTDPCore : public Solver {
  const MDP* problem;
  MDPNode* root;
  MDPHash* lookup;
  AbstractBound* initUpperBound;
  AbstractBound* initLowerBound;
  timeval boundsStartTime;
  timeval previousElapsedTime;
  int numStatesTouched;
  int numStatesExpanded;
  int numTrials;
  int numBackups;
  double lastPrintTime;
  std::ostream* boundsFile;
  bool initialized;

  RTDPCore(AbstractBound* _initUpperBound);

  void init(double targetPrecision);
  MDPNode* getNode(const state_vector& s);
  void expand(MDPNode& cn);
  int getMaxUBAction(MDPNode& cn) const;
  int getSimulatedOutcome(MDPNode& cn, int a) const;
  void update(MDPNode& cn);

  // different derived classes (RTDP variants) will implement these
  // in varying ways
  virtual bool getUseLowerBound(void) const = 0;
  virtual void updateInternal(MDPNode& cn) = 0;
  virtual bool doTrial(MDPNode& cn, double pTarget) = 0;

  // virtual functions from Solver that constitute the external api
  void planInit(const MDP* pomdp);
  bool planFixedTime(const state_vector& s,
		     double maxTimeSeconds,
		     double targetPrecision);
  int chooseAction(const state_vector& s);
  void setBoundsFile(std::ostream* boundsFile);
  ValueInterval getValueAt(const state_vector& s) const;
};

}; // namespace zmdp

#endif /* INCRTDPCore_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/02/13 20:20:32  trey
 * refactored some common code from RTDP and LRTDP
 *
 * Revision 1.1  2006/02/11 22:38:10  trey
 * moved much of the RTDP implementation into RTDPCore, where it can be shared by many RTDP variants
 *
 *
 ***************************************************************************/
