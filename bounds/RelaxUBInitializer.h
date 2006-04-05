/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-04-05 21:33:58 $
   
 @file    RelaxBound.h
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

#ifndef INCRelaxBound_h
#define INCRelaxBound_h

#include "MatrixUtils.h"
#include "AbstractBound.h"
#include "MDPCache.h"

namespace zmdp {

struct RelaxBound : public AbstractBound {
  const MDP* problem;
  MDPNode* root;
  MDPHash* lookup;
  AbstractBound* initLowerBound;
  AbstractBound* initUpperBound;

  RelaxBound(const MDP* _problem);
  virtual ~RelaxBound(void) {}

  MDPNode* getNode(const state_vector& s);
  void setup(double targetPrecision);
  void expand(MDPNode& cn);
  void updateInternal(MDPNode& cn);
  void update(MDPNode& cn);
  int getMaxUBAction(MDPNode& cn, double* maxUBValP, double* secondBestUBValP) const;
  void trialRecurse(MDPNode& cn, double costSoFar, double altActionPrio, int depth);
  void doTrial(MDPNode& cn, double pTarget);

  // implementation of AbstractBound interface
  void initialize(double targetPrecision);
  double getValue(const state_vector& s) const;
};

}; // namespace zmdp

#endif /* INCRelaxBound_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/04/04 17:22:43  trey
 * moved RelaxBound from common to bounds
 *
 * Revision 1.4  2006/02/17 21:09:07  trey
 * fixed stack overflow on undiscounted problems (unfortunately, initialization is slower now)
 *
 * Revision 1.3  2006/02/14 19:30:28  trey
 * added targetPrecision argument to initialize()
 *
 * Revision 1.2  2006/02/09 21:56:27  trey
 * added minor efficiency enhancement to calculate maxUBAction during update
 *
 * Revision 1.1  2006/02/08 19:21:44  trey
 * initial check-in
 *
 *
 ***************************************************************************/
