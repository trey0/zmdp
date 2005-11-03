/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2005-11-03 17:44:22 $
   
 @file    BeliefMDP.h
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

#ifndef INCBeliefMDP_h
#define INCBeliefMDP_h

#include <iostream>
#include <string>
#include <vector>

#include "pomdpCommonDefs.h"
#include "pomdpCommonTypes.h"

using namespace sla;

namespace pomdp {

struct BeliefMDP {
  int beliefSize, numActions, numObservations;
  double discount;

  virtual ~BeliefMDP(void) {}

  int getBeliefSize(void) const { return beliefSize; }
  int getNumActions(void) const { return numActions; }
  int getNumObservations(void) const { return numObservations; }
  double getDiscount(void) const { return discount; }

  // returns the initial belief
  virtual const belief_vector& getInitialBelief(void) const = 0;
  // sets result to be the vector of observation probabilities when from belief b action a is selected
  virtual obs_prob_vector& getObsProbVector(obs_prob_vector& result, const belief_vector& b, int a)
       const = 0;
  // sets result to be the next belief when from belief b action a is selected and observation o is observed
  virtual belief_vector& getNextBelief(belief_vector& result, const belief_vector& b, int a,
				       int o) const = 0;
  // returns the expected immediate reward when from belief b action a is selected
  virtual double getReward(const belief_vector& b, int a) const = 0;
};

}; // namespace pomdp

#endif // INCBeliefMDP_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/

