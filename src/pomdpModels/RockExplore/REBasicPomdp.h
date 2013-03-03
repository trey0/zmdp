/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2007-03-07 08:34:14 $
   
 @file    BasicPomdp.h
 @brief   The BasicPomdp problem is closely related to the RockSample problem
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

#ifndef INCBasicPomdp_h
#define INCBasicPomdp_h

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace zmdp {

// A 2D position.
struct REPos {
  int x, y;

  REPos(void) {}
  REPos(int _x, int _y) : x(_x), y(_y) {}
  bool operator==(const REPos& a) const {
    return (x == a.x) && (y == a.y);
  }
};

// A world state in the RockExplore problem.
struct REState {
  // If true, the system is in the unique 'terminal state' and the other
  // fields are ignored
  bool isTerminalState;

  // Position of the robot
  REPos robotPos;

  // Vector indicating whether each rock is good
  std::vector<bool> rockIsGood;

  REState(void) :
    isTerminalState(false),
    robotPos(REPos(-1,-1))
  {}
};

// An entry in a probability distribution over states.
struct REBeliefEntry {
  double prob;
  int index;

  REBeliefEntry(void) {}
  REBeliefEntry(double _prob, int _index) : prob(_prob), index(_index) {}
};

// A vector (sparsely) representing a probability distribution over states.
typedef std::vector<REBeliefEntry> REBelief;

// A vector representing a probability distribution over observations.
typedef std::vector<double> REObsProbs;

// A structure used for the return value of getActionResult().
struct REActionResult {
  double reward;
  REBelief outcomes;
};

// A structure used for the return value of getObsProbs().
struct REObsProbsResult {
  double expectedReward;
  REObsProbs obsProbs;
};

// Abstract class for POMDPs.
struct REBasicPomdp {
  /**********************************************************************
   * FUNCTIONS DESCRIBING THE BASIC MODEL -- SUBCLASSES IMPLEMENT THESE
   **********************************************************************/
  
  virtual int getNumStates(void) = 0;
  virtual int getNumActions(void) = 0;
  virtual int getNumObservations(void) = 0;
  virtual REBelief getInitialBelief(void) = 0;
  virtual REActionResult getActionResult(int si, int ai) = 0;
  virtual double getObsProb(int ai, int sp, int o) = 0;

  /**********************************************************************
   * I/O FUNCTIONS -- SUBCLASSES IMPLEMENT THESE
   **********************************************************************/
  
  virtual std::string getStateString(int si) = 0;
  virtual std::string getStateString(const REState& s) = 0;
  virtual std::string getActionString(int ai) = 0;
  virtual std::string getObservationString(int oi) = 0;
  virtual std::string getMap(int si, const REBelief& b) = 0;

  /**********************************************************************
   * UTILITY FUNCTIONS THAT RELY ON THE BASIC MODEL
   **********************************************************************/
  
  // Returns the distribution of possible observations when action ai is
  // applied and the system transitions to state sp.
  REObsProbs getObsProbs(int ai, int sp);
  
  // Returns the expected reward and observation probabilities when from
  // belief b action ai is applied.
  REObsProbsResult getBeliefResult(const REBelief& b, int ai);

  // Returns the updated belief when from belief b action ai is executed
  // and observation o is received.
  REBelief getUpdatedBelief(const REBelief& b, int ai, int o);

  // Uses the transition model to generate all reachable states and assign
  // them index values.  This is called during initialization; before it is
  // called getNumStates() is not valid.
  void generateReachableStates(void);

  // Outputs a Cassandra-format POMDP model to the given file.
  void writeCassandraModel(const std::string& outFile);

  // Returns the index for the state s. This includes assigning an index
  // to the state if it doesn't already have one.
  int getStateId(const REState& s);

  // Returns the most likely state according to the distribution b.
  static int getMostLikelyState(const REBelief& b);

  // Returns a stochastically selected state index from the distribution b.
  static int chooseStochasticOutcome(const REBelief& b);

  // Returns a stochastically selected observation from the distribution obsProbs.
  static int chooseStochasticOutcome(const REObsProbs& obsProbs);

  // Need this virtual destructor to avoid a compile-time warning.
  virtual ~REBasicPomdp(void) {}

  /**********************************************************************
   * VARIABLES SET DURING INITIALIZATION
   **********************************************************************/

  // Maps from the index of a state to the corresponding state struct
  std::vector<REState> states;

  // Maps from the string identifier for a state to the state index
  std::map<std::string, int> stateLookup;

};

extern REBasicPomdp* modelG;

}; // namespace zmdp

#endif // INCREBasicPomdp_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2007/03/07 08:12:27  trey
 * refactored things
 *
 *
 ***************************************************************************/

