/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
 @file    RelaxUBInitializer.h
 @brief   No brief

 Copyright (c) 2006, Trey Smith. All rights reserved.

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

#ifndef INCRelaxUBInitializer_h
#define INCRelaxUBInitializer_h

#include "MatrixUtils.h"
#include "AbstractBound.h"
#include "MDPCache.h"

namespace zmdp {

struct RelaxUBInitializer : public AbstractBound {
  const MDP* problem;
  MDPNode* root;
  MDPHash* lookup;
  AbstractBound* initLowerBound;
  AbstractBound* initUpperBound;

  RelaxUBInitializer(const MDP* _problem);
  virtual ~RelaxUBInitializer(void) {}

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

#endif /* INCRelaxUBInitializer_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/04/06 04:09:45  trey
 * finished renaming RelaxBound -> RelaxUBInitializer
 *
 * Revision 1.1  2006/04/05 21:33:58  trey
 * renamed RelaxBound -> RelaxUBInitializer
 *
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
