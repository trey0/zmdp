/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-02-06 19:26:09 $
   
 @file    Pomdp.h
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith
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

#ifndef INCPomdp_h
#define INCPomdp_h

#include <iostream>
#include <string>
#include <vector>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"
#include "MDP.h"

using namespace sla;

namespace zmdp {

class Pomdp : public MDP {
public:
  int numStates, numObservations;

  // initialBelief(s)
  cvector initialBelief;
  // R(s,a)
  cmatrix R;
  // T[a](s,s'), Ttr[a](s',s), O[a](s',o)
  std::vector<cmatrix> T, Ttr, O;
  std::vector<bool> isPomdpTerminalState;

  Pomdp(void) {}
  Pomdp(const std::string& fileName, bool useFastParser = false);

  void readFromFile(const std::string& fileName,
		    bool useFastParser = false);

  // returns the initial belief
  const belief_vector& getInitialBelief(void) const;

  // sets result to be the vector of observation probabilities when from
  // belief b action a is selected
  obs_prob_vector& getObsProbVector(obs_prob_vector& result, const belief_vector& b,
				    int a) const;

  // sets result to be the next belief when from belief b action a is
  // selected and observation o is observed
  belief_vector& getNextBelief(belief_vector& result, const belief_vector& b,
			       int a, int o) const;

  // returns the expected immediate reward when from belief b action a is selected
  double getReward(const belief_vector& b, int a) const;

  AbstractBound* newLowerBound(void) const;
  AbstractBound* newUpperBound(void) const;

  // POMDP-as-belief-MDP aliases for functions implemented in MDP
  int getBeliefSize(void) const { return getNumStateDimensions(); }
  int getNumObservations(void) const { return numObservations; }
  void setBeliefSize(int beliefSize) { numStateDimensions = beliefSize; }
  void setNumObservations(int _numObservations) { numObservations = _numObservations; }

  // POMDP-as-belief-MDP implementations for virtual functions declared in MDP
  const state_vector& getInitialState(void) const { return getInitialBelief(); }
  bool getIsTerminalState(const state_vector& s) const { return false; }
  outcome_prob_vector& getOutcomeProbVector(outcome_prob_vector& result, const state_vector& b,
					    int a) const
    { return getObsProbVector(result,b,a); }
  state_vector& getNextState(state_vector& result, const state_vector& s,
			     int a, int o) const
    { return getNextBelief(result,s,a,o); }
  
protected:
  void readFromFileCassandra(const std::string& fileName);
  void readFromFileFast(const std::string& fileName);

  void debugDensity(void);
};

}; // namespace zmdp

#endif // INCPomdp_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.2  2006/01/31 20:12:44  trey
 * added newXXXBound() functions
 *
 * Revision 1.1  2006/01/31 18:31:51  trey
 * moved many files from common to pomdpCore
 *
 * Revision 1.7  2006/01/29 00:18:36  trey
 * added Pomdp() constructor that calls readFromFile()
 *
 * Revision 1.6  2006/01/28 03:03:23  trey
 * replaced BeliefMDP -> MDP, corresponding changes in API
 *
 * Revision 1.5  2005/11/03 17:45:30  trey
 * moved transition dynamics from HSVI implementation to Pomdp
 *
 * Revision 1.4  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.3  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.2  2005/10/27 22:29:12  trey
 * removed dependence on SmartRef header
 *
 * Revision 1.1  2005/10/27 21:38:16  trey
 * renamed PomdpM to Pomdp
 *
 * Revision 1.7  2005/10/21 20:08:41  trey
 * added namespace zmdp
 *
 * Revision 1.6  2005/03/10 22:53:32  trey
 * now initialize T matrix even when using sla
 *
 * Revision 1.5  2005/01/27 05:32:02  trey
 * switched to use Ttr instead of T under sla
 *
 * Revision 1.4  2005/01/26 04:10:48  trey
 * modified problem reading to work with sla
 *
 * Revision 1.3  2005/01/21 15:21:19  trey
 * added readFromFileFast
 *
 * Revision 1.2  2004/11/24 20:50:16  trey
 * switched PomdpP to be a pointer, not a SmartRef
 *
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.2  2004/11/09 21:31:59  trey
 * got pomdp source tree into a building state again
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.5  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.4  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.3  2003/09/07 02:28:07  trey
 * started to adapt for boost matrix library
 *
 * Revision 1.2  2003/07/16 16:07:36  trey
 * added isTerminalState
 *
 * Revision 1.1  2003/06/26 15:41:22  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/

