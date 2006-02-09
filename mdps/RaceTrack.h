/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-02-06 19:27:24 $
   
 @file    RaceTrack.h
 @brief   No brief

 Copyright (c) 2006, Trey Smith
 All rights reserved.

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

#ifndef INCRaceTrack_h
#define INCRaceTrack_h

#include <iostream>
#include <string>
#include <vector>

#include "MDP.h"
#include "AbstractBound.h"

namespace zmdp {

struct TrackMap;

class RaceTrack : public MDP {
public:
  double errorProbability;
  state_vector bogusInitialState, terminalState;
  TrackMap *tmap;

  RaceTrack(void);
  ~RaceTrack(void);
  RaceTrack(const std::string& specFileName);

  void readFromFile(const std::string& specFileName);

  // returns the initial state
  const state_vector& getInitialState(void) const;

  // returns true if state is terminal
  bool getIsTerminalState(const state_vector& s) const;

  // sets result to be the vector of outcome probabilities when from
  // state s action a is selected
  outcome_prob_vector& getOutcomeProbVector(outcome_prob_vector& result,
					    const state_vector& s, int a)
       const;

  // sets result to be the next state when from state s action a is
  // selected and outcome o results
  state_vector& getNextState(state_vector& result, const state_vector& s, int a,
			     int o) const;

  // returns the expected immediate reward when from state s action a is selected
  double getReward(const state_vector& s, int a) const;

  AbstractBound* newLowerBound(void) const;
  AbstractBound* newUpperBound(void) const;
};

}; // namespace zmdp

#endif // INCRaceTrack_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/02/01 18:03:14  trey
 * fixed compile-time errors, not quite done yet
 *
 * Revision 1.2  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.1  2006/01/31 18:12:30  trey
 * initial check-in in mdps directory
 *
 *
 ***************************************************************************/
