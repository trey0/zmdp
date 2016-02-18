/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2007-03-24 22:43:57 $
  
 @file    CustomMDP.cc
 @brief   No brief

 Copyright (c) 2006, Trey Smith.

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

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <getopt.h>
#include <errno.h>

#include <iostream>
#include <fstream>
#include <map>

#include "MatrixUtils.h"
#include "Solver.h"
#include "CustomMDP.h"

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

#define ACTION_MOVE_LEFT (0)
#define ACTION_MOVE_RIGHT (1)

CustomMDP::CustomMDP(const ZMDPConfig& config)
{
  // USER CUSTOMIZE

  // We set up the myInitState variable here so that it can be returned by
  // the function getInitialState() later.  It specifies the initial
  // state of the MDP.  In the example problem x=0 is the initial state,
  // so we set myInitState to be a length 1 vector with the single entry
  // 0.  Because myInitState has type state_vector, we need to use the
  // push_back(index,value) function to set an entry.
  myInitState.resize(1);
  myInitState.push_back(0, 0);

  // This example shows how to enable parameters of your model to be
  // specified at run time.  The 'customMDPNumStates' parameter is set
  // in the file src/main/zmdp.config.  You can override it at the
  // command line with e.g. '--customMDPNumStates 10'.
  //
  // If you want to add your own custom config parameters, you need to
  // (1) declare a default value for each parameter in the zmdp.config
  // file, and (2) add code to read the parameter values here.  Note
  // that the default values specified in zmdp.config are actually
  // embedded into the ZMDP binaries at compile time, so if you change
  // the config file you may want to recompile.  You can get more
  // information about how ZMDP handles config information by running
  // 'zmdp -h'.
  //
  // There are other functions you can call to extract other types of
  // parameters; e.g. getString(), getDouble(), getBool().  See
  // src/common/zmdpConfig.h for the detailed interface.
  myNumStates = config.getInt("customMDPNumStates");

  // -----------------------------------------------------------------
  // The member variables *above* this point are not used outside the
  // CustomMDP class.  We just set them up so they can be used in the
  // functions defined below.  To remind you of this, we added the 'my'
  // prefix to the variables.  If you modify CustomMDP, you can make
  // any changes you like to the 'my' variables, or even get rid of
  // them.

  // The variables set *below* this point are declared in the abstract
  // parent class MDP.  The solver code outside the CustomMDP class uses
  // these variables, so if you make any modifications to CustomMDP
  // you'll have to make sure they have the right values.

  // numStateDimensions specifies the dimensionality of state vectors.
  // In the example problem states are really just integer values of x,
  // so we will represent them with length 1 vectors.  Note this is the
  // *dimensionality* of the state space, not the number of states.
  numStateDimensions = 1;

  // In the example problem, there are two actions, move left and move right.
  numActions = 2;

  // We'll set up the problem to be undiscounted.
  discount = 1.0;
}

CustomMDP::~CustomMDP(void)
{
  // Add any cleanup you need here.
}

const state_vector& CustomMDP::getInitialState(void)
{
  // Normally you don't need to modify this function.

  // We already set up the myInitState variable in the constructor, so we
  // can just return that.  Notice that this function returns by
  // reference for efficiency, so it would be a mistake to generate a
  // state_vector on the stack in this function and return that.
  return myInitState;
}

bool CustomMDP::getIsTerminalState(const state_vector& s)
{
  // USER CUSTOMIZE

  // getIsTerminalState(s) returns true iff s is a terminal state.

  // In the example problem, x=n-1 is the only terminal state.  Notice
  // how we extract x from the state vector first.
  int x = (int) s(0);
  return (x == (myNumStates-1));
}

outcome_prob_vector& CustomMDP::getOutcomeProbVector(outcome_prob_vector& result,
						     const state_vector& s, int a)
{
  // USER CUSTOMIZE

  // getOutcomeProbVector(result,s,a) sets <result> to be a vector of outcome probabilities
  // given that in state s you apply action a.

  // This code returns outcome probabilities for the example problem.
  int x = (int) s(0);
  if (x == (myNumStates-1)) {
    // x=n-1 is a terminal state.  Therefore, the outcome must loop back
    // to x=n-1 with probability 1.  Note that in this function we are
    // not specifying what the outcome is, only giving a vector of
    // probabilities.
    result.resize(1);
    result(0) = 1;
  } else {
    switch (a) {
    case ACTION_MOVE_LEFT:
      if (x == 0) {
	// We need a special case for x=0 because we don't want the robot
	// to be able to move off the left-hand side of the map.  We will
	// loop back to x=0 with probability 1.
	result.resize(1);
	result(0) = 1;
      } else {
	// For other positions, there will be a 90% chance of success and 10% of failure.
	result.resize(2);
	result(0) = 0.9;
	result(1) = 0.1;
      }
      break;
    case ACTION_MOVE_RIGHT:
      // 90% chance of success and 10% of failure.
      result.resize(2);
      result(0) = 0.9;
      result(1) = 0.1;
      break;
    default:
      assert(0); // never reach this point
    }
  }

  // optional sanity check -- probabilities should sum to 1.0
  assert(fabs(sum(result) - 1.0) < 1e-10);

  return result;
}

state_vector& CustomMDP::getNextState(state_vector& result, const state_vector& s,
				      int a, int o)
{
  // USER CUSTOMIZE

  // getOutcomeProbVector(result,s,a) sets <result> to be the next state
  // given that in state s you apply action a and get outcome o.  Here,
  // the outcome number o must correspond to an index in the outcome
  // probability vector returned by getOutcomeProbVector().

  // This code returns the next state for the example problem.
  int oldX = (int) s(0);
  int newX;

  if (oldX == (myNumStates-1)) {
    // Special case.  x=n-1 is a terminal state, so the outcome must always
    // to loop back to x=n-1.
    newX = myNumStates-1;
  } else {
    // successDX specifies the change in x if the move succeeds.
    int successDX = (a == ACTION_MOVE_LEFT) ? -1 : 1;

    // Special case: 'move left' action always fails if x=0.
    if ((oldX == 0) && (a == ACTION_MOVE_LEFT)) {
      successDX = 0;
    }

    switch (o) {
    case 0:
      // o=0 corresponds to the high-probability 'success' outcome in the
      // getOutcomeProbVector() return value.
      newX = oldX + successDX;
      break;
    case 1:
      // o=1 corresponds to the low-probability 'slip' outcome in the
      // getOutcomeProbVector() return value.
      newX = oldX;
      break;
    default:
      assert(0); // never reach this point
    }
  }

  // Because result is a state_vector, we need to use the
  // push_back(index,value) function to set an entry.
  result.resize(1);
  result.push_back(0, newX);

  return result;
}

double CustomMDP::getReward(const state_vector& s, int a)
{
  // USER CUSTOMIZE

  // getReward(s,a) returns the reward when in state s you apply action
  // a.

  // In the example problem, all moves have reward -1 (i.e. cost 1), except
  // for the terminal state, which must be a zero-reward absorbing state.
  int x = (int) s(0);
  if (x == myNumStates-1) {
    return 0;
  } else {
    return -1;
  }
}

double CustomMDP::getInitialLowerBoundValue(const state_vector& s)
{
  // USER CUSTOMIZE

  // getInitialLowerBoundValue(s) returns a lower bound on the optimal
  // expected long-term reward starting from state s.

  // For the example problem, this means some negative value that is
  // guaranteed to be lower than the expected total negative reward
  // accumulated before getting to the goal.  We'll use common sense and
  // fill in a number that's ridiculously low given the size of the
  // problem and the action costs involved.  Note that the tighter this
  // bound is, the less time it will take to achieve a particular regret
  // bound in the solution.
  return (-100 * myNumStates);
}

double CustomMDP::getInitialUpperBoundValue(const state_vector& s)
{
  // USER CUSTOMIZE

  // getInitialLowerBoundValue(s) returns a lower bound on the optimal
  // expected long-term reward starting from state s.

  // For the example problem, since all rewards are negative, 0 is
  // obviously an upper bound on the total reward.
  return 0;
}

/**********************************************************************
 * YOU SHOULDN'T NEED TO MODIFY BELOW THIS POINT
 **********************************************************************/

struct CustomLowerBound : public AbstractBound {
  CustomMDP* x;

  CustomLowerBound(CustomMDP* _x) : x(_x) {}
  void initialize(double targetPrecision) {}
  double getValue(const state_vector& s, const MDPNode* cn) const {
    return x->getInitialLowerBoundValue(s);
  }
};

struct CustomUpperBound : public AbstractBound {
  CustomMDP* x;

  CustomUpperBound(CustomMDP* _x) : x(_x) {}
  void initialize(double targetPrecision) {}
  double getValue(const state_vector& s, const MDPNode* cn) const {
    return x->getInitialUpperBoundValue(s);
  }
};

AbstractBound* CustomMDP::newLowerBound(const ZMDPConfig* _config)
{
  return new CustomLowerBound(this);
}

AbstractBound* CustomMDP::newUpperBound(const ZMDPConfig* _config)
{
  return new CustomUpperBound(this);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/11/08 16:36:48  trey
 * renamed MDP.h to MDPModel.h to avoid confusion with mdp.h, stupid case insensitive Mac OS X
 *
 * Revision 1.1  2006/11/07 20:09:42  trey
 * initial check-in
 *
 *
 ***************************************************************************/
