/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2007-03-06 04:32:33 $
   
 @file    RockExplorePolicy.h
 @brief   The RockExplorePolicy problem is closely related to the RockSample problem
          in my paper "Heuristic Search Value Iteration for POMDPs" (UAI 2004).
          This is a model for the problem.

 Copyright (c) 2007, Trey Smith.

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

#ifndef INCRockExplorePolicy_h
#define INCRockExplorePolicy_h

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "RockExplore.h"
#include "PomdpExec.h"
#include "MaxPlanesLowerBoundExec.h"

namespace zmdp {

struct MDPValueFunction {
  // The value function, represented as a vector of values.
  std::vector<double> V;

  // Auxiliary storage space used during a sweep
  std::vector<double> Vp;

  // Initializes the value function to have zero value for all states.
  void init(void);

  // Performs a sweep of value iteration, updating all states.  Returns the
  // maximum residual between the functions before and after the sweep.
  double valueIterationSweep(void);

  // Performs value iteration until the maximum residual between
  // successive sweeps is smaller than eps. 
  void valueIterationToResidual(double eps);

  // Returns the value V(s).
  double getValue(int s) const;

  // Returns the expected value of a belief V(b) = sum_s P(s | b) V(s)
  double getValue(const RockExploreBelief& b) const;

  // Returns HV(s) = max_a [ R(s,a) + discount * sum_s' P(s' | s,a) V(s') ].
  double getUpdatedValue(int s) const;

  // Returns HV(b) = max_a [ R(b,a) + discount * sum_o P(b' | b,a,o) V(b') ]
  double getUpdatedValue(const RockExploreBelief& b) const;
};

// This "policy" selects actions by asking the user.
struct UserPolicy : public PomdpExecCore {
  void setToInitialBelief(void) { /* do nothing */ }
  int chooseAction(void);
  void advanceToNextBelief(int a, int o) { /* do nothing */ }
};

// This is a base policy for all the heuristic policies based on the MDP value
// function.
struct HeuristicPolicy : public PomdpExecCore {
  MDPValueFunction vfn;
  RockExploreBelief b;

  HeuristicPolicy(void);

  // Informs the policy that the system is at the initial belief.
  void setToInitialBelief(void);

  // Informs the policy that action a was applied and observation o was received.
  void advanceToNextBelief(int a, int o);
};

struct QMDPPolicy : public HeuristicPolicy {
  // Chooses an action according to the QMDP heuristic.
  int chooseAction(void);
};

struct VotingPolicy : public HeuristicPolicy {
  // Chooses an action according to the voting heuristic.
  int chooseAction(void);
};

struct MostLikelyPolicy : public HeuristicPolicy {
  // Chooses an action according to the most likely state heuristic.
  int chooseAction(void);
};

struct TwoStepPolicy : public HeuristicPolicy {
  // Chooses an action according to the two-step lookahead heuristic.
  int chooseAction(void);
};

// Accepts a numeric value input on console.
extern int getUserChoice(void);

// Queries the user for their desired policy type and returns a policy.
extern PomdpExecCore* getPolicy(void);

}; // namespace zmdp

#endif // INCRockExplorePolicy_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/

