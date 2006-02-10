/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-02-10 19:33:54 $
   
 @file    RTDP.h
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

#ifndef INCRTDP_h
#define INCRTDP_h

#include "MatrixUtils.h"
#include "Solver.h"
#include "AbstractBound.h"
#include "MDPCache.h"

namespace zmdp {

struct RTDP : public Solver {
  const MDP* problem;
  MDPNode* root;
  MDPHash* lookup;
  AbstractBound* initUpperBound;
  timeval boundsStartTime;
  timeval previousElapsedTime;
  int numNodes;
  int numInternalNodes;
  int numBoundUpdates;
  int numWeightPropUpdates;
  int numTrials;
  std::ostream* boundsFile;
  bool initialized;

  RTDP(AbstractBound* _initUpperBound);
  virtual ~RTDP(void) {}

  void init(void);
  MDPNode* getNode(const state_vector& s);
  void expand(MDPNode& cn);
  void updateInternal(MDPNode& cn, int* maxUBActionP);
  void update(MDPNode& cn, int* maxUBActionP);
  void trialRecurse(MDPNode& cn, double pTarget, int depth);
  void doTrial(MDPNode& cn, double pTarget);

  // virtual functions from Solver that constitute the external api
  void planInit(const MDP* pomdp);
  bool planFixedTime(const state_vector& s,
		     double maxTimeSeconds,
		     double minPrecision);
  int chooseAction(const state_vector& s);
  void setBoundsFile(std::ostream* boundsFile);
  ValueInterval getValueAt(const state_vector& s) const;
};

}; // namespace zmdp

#endif /* INCRTDP_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/02/09 21:59:04  trey
 * initial check-in
 *
 *
 ***************************************************************************/
