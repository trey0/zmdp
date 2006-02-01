/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-02-01 18:23:37 $
   
 @file    RSDynamics.h
 @brief   No brief

 Copyright (c) 2005, Trey Smith. All rights reserved.

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

#ifndef INCRSDynamics_h
#define INCRSDynamics_h

#include "MatrixUtils.h"

namespace zmdp {

struct RSDynamics : public BeliefMDP {
  belief_vector b0;

  // helper functions
  void init(void);

  // main api
  const belief_vector& getInitialBelief(void) const;
  obs_prob_vector& getObsProbVector(obs_prob_vector& result, const belief_vector& b, int a) const;
  belief_vector& getNextBelief(belief_vector& result, const belief_vector& b, int a, int o) const;
  double getReward(const belief_vector& b, int a) const;

  // heuristics
  static double getInitUpperBound(const belief_vector& b);
  static double getInitLowerBound(const belief_vector& b);

  // conversion with non-factored representation
  static void compressBelief(belief_vector& result, const belief_vector& b);
  static void expandBelief(belief_vector& result, const belief_vector& b);
};

}; // namespace zmdp

#endif /* INCRSDynamics_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.1  2005/11/06 01:27:12  trey
 * initial check-in
 *
 *
 ***************************************************************************/
