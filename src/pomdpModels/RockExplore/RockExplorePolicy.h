/********** tell emacs we use -*- c++ -*- style comments *******************
 @file    RockExplorePolicy.h
 @brief   The RockExplorePolicy problem is closely related to the RockSample
 problem in my paper "Heuristic Search Value Iteration for POMDPs" (UAI 2004).
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

#ifndef ZMDP_SRC_POMDPMODELS_ROCKEXPLORE_ROCKEXPLOREPOLICY_H_
#define ZMDP_SRC_POMDPMODELS_ROCKEXPLORE_ROCKEXPLOREPOLICY_H_

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "BoundPairExec.h"
#include "MDPExec.h"
#include "RockExplore.h"

namespace zmdp {

struct REValueFunction {
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

  // Returns Q(s,a).
  double getQ(int s, int a) const;

  // Returns arg max_a Q(s,a).
  int getMaxQAction(int s) const;

  // Returns HV(s) = max_a Q(s,a).
  double getUpdatedValue(int s) const;

  // Returns the value of a belief V(b) = sum_s P(s | b) V(s)
  double getValue(const REBelief &b) const;

  // Returns Q(b,a).
  double getQ(const REBelief &b, int a) const;

  // Returns arg max_a Q(b,a).
  int getMaxQAction(const REBelief &b) const;

  // Returns HV(b) = max_a Q(b,a).
  double getUpdatedValue(const REBelief &b) const;
};

#define NUM_POLICIES (5)

enum PolicyTypes {
  P_QMDP = 1,
  P_VOTING = 2,
  P_MOST_LIKELY = 3,
  P_DUAL_MODE = 4,
  P_ZMDP = 5
};

// This "policy" selects actions by asking the user.
struct UserPolicy : public MDPExecCore {
  void setToInitialState(void) { /* do nothing */
  }
  int chooseAction(void);
  void advanceToNextState(int a, int o) { /* do nothing */
  }
};

// This is a base policy for all the heuristic policies based on the MDP value
// function.
struct HeuristicPolicy : public MDPExecCore {
  REBelief b;

  // Informs the policy that the system is at the initial belief.
  void setToInitialState(void);

  // Informs the policy that action a was applied and observation o was
  // received.
  void advanceToNextState(int a, int o);
};

struct QMDPPolicy : public HeuristicPolicy {
  REValueFunction vfn;
  explicit QMDPPolicy(const REValueFunction &_vfn) : vfn(_vfn) {}

  // Chooses an action according to the QMDP heuristic.
  int chooseAction(void);
};

struct VotingPolicy : public HeuristicPolicy {
  REValueFunction vfn;
  explicit VotingPolicy(const REValueFunction &_vfn) : vfn(_vfn) {}

  // Chooses an action according to the voting heuristic.
  int chooseAction(void);
};

struct MostLikelyPolicy : public HeuristicPolicy {
  REValueFunction vfn;
  explicit MostLikelyPolicy(const REValueFunction &_vfn) : vfn(_vfn) {}

  // Chooses an action according to the most likely state heuristic.
  int chooseAction(void);
};

struct DualModePolicy : public HeuristicPolicy {
  REValueFunction vfn;
  explicit DualModePolicy(const REValueFunction &_vfn) : vfn(_vfn) {}

  // Chooses an action according to the dual-mode heuristic.
  int chooseAction(void);
};

// Accepts a numeric value input on console.
extern int getUserChoice(void);

// Queries the user and returns the id for their desired policy type.
extern int getUserPolicyType(void);

// Returns a policy of the specified type.
extern MDPExecCore *getPolicy(int policyType);

// Returns the string name of the specified policy type.
extern const char *getPolicyName(int policyType);

};  // namespace zmdp

#endif  // ZMDP_SRC_POMDPMODELS_ROCKEXPLORE_ROCKEXPLOREPOLICY_H_
