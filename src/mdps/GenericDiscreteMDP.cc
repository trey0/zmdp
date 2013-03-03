/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2007-03-24 22:44:33 $
  
 @file    Pomdp.cc
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith. All rights reserved.

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

#include <iostream>
#include <fstream>

#include "zmdpCommonDefs.h"
#include "MatrixUtils.h"
#include "slaMatrixUtils.h"
#include "FastParser.h"
#include "CassandraParser.h"
#include "GenericDiscreteMDP.h"

using namespace std;
using namespace MatrixUtils;

#define MDP_LONG_TERM_UNBOUNDED (99e+20)

namespace zmdp {

GenericDiscreteMDP::GenericDiscreteMDP(const std::string& fileName,
				       const ZMDPConfig* config) :
  boundsInitialized(false)
{
  bool useFastModelParser = config->getBool("useFastModelParser");
  if (useFastModelParser) {
    FastParser parser;
    parser.readGenericDiscreteMDPFromFile(*this, fileName);
  } else {
    CassandraParser parser;
    parser.readGenericDiscreteMDPFromFile(*this, fileName);
  }
  
  // in the generic discrete MDP, states are just integers, which
  // we represent using length 1 vectors
  numStateDimensions = 1;

  maxHorizon = config->getInt("maxHorizon");
}

const state_vector& GenericDiscreteMDP::getInitialState(void)
{
  return initialState;
}

bool GenericDiscreteMDP::getIsTerminalState(const state_vector& sv)
{
  int s = (int) sv(0);
  return isTerminalState[s];
}

outcome_prob_vector& GenericDiscreteMDP::getOutcomeProbVector(outcome_prob_vector& result,
							      const state_vector& sv,
							      int a)
{
  int s = (int) sv(0);

  // extract the non-zero entries of column s of Ttr[a] and
  // pack them into the dense result vector.
  result.resize(Ttr[a].filled_in_column(s));
  int o = 0;
  FOR_CM_MINOR(s, Ttr[a]) {
    result(o++) = CM_VAL(Ttr[a]);
  }

  return result;
}

state_vector& GenericDiscreteMDP::getNextState(state_vector& result,
					       const state_vector& sv,
					       int a, int o)
{
  // NOTE: getNextState() could be made more efficient if we could cache
  // information over multiple calls with the same s and a
  // values... unfortunately, the current interface design has
  // getNextState() as a 'const' method, so we can't cache anything.

  int s = (int) sv(0);

  // return the row in which the 'o'th non-zero entry in column s of
  // Ttr[a] appears.  this corresponds to the 'o'th entry in the dense
  // vector returned earlier by getOutcomeProbVector().
  int i = 0;
  FOR_CM_MINOR (s, Ttr[a]) {
    if (i++ == o) {
      result.resize(1);
      result.push_back(0, CM_ROW(s, Ttr[a]));
      return result;
    }
  }

  assert(0); // never reach this point
}

double GenericDiscreteMDP::getReward(const state_vector& sv, int a)
{
  int s = (int) sv(0);
  return R(s,a);
}

double GenericDiscreteMDP::getLongTermFactor(void)
{
  // calculate longTermFactor
  double longTermFactor = MDP_LONG_TERM_UNBOUNDED;
  if (getDiscount() < 1.0) {
    if (maxHorizon != -1) {
      longTermFactor = (1.0 - pow(getDiscount(), maxHorizon+1))
	/ (1.0 - getDiscount());
    } else {
      longTermFactor = 1.0 / (1.0 - getDiscount());
    }
  }
  if (maxHorizon != -1) {
    longTermFactor = std::min(longTermFactor, (double) maxHorizon);
  }
  if (MDP_LONG_TERM_UNBOUNDED == longTermFactor) {
    fprintf(stderr,
	    "ERROR: using modelType='mdp', but the model is undiscounted and no maxHorizon\n"
	    "       was specified; the solver cannot generate any initial bounds\n");
    exit(EXIT_FAILURE);
  }
  return longTermFactor;
}

struct GenericDiscreteLowerBound : public AbstractBound {
  GenericDiscreteMDP* x;
  double globalLowerBound;

  GenericDiscreteLowerBound(GenericDiscreteMDP* _x) : x(_x) {}
  void initialize(double targetPrecision) {
    // maxMinReward = max_a min_s R(s,a)
    double maxMinReward = -99e+20;
    double minStateReward;
    FOR (a, x->getNumActions()) {
      minStateReward = 99e+20;
      FOR (s, x->numStates) {
	minStateReward = std::min(minStateReward, x->R(s,a));
      }
      maxMinReward = std::max(maxMinReward, minStateReward);
    }
    globalLowerBound = x->getLongTermFactor() * maxMinReward;
  }
  double getValue(const state_vector& s, const MDPNode* cn) const {
    return globalLowerBound;
  }
};

struct GenericDiscreteUpperBound : public AbstractBound {
  GenericDiscreteMDP* x;
  double globalUpperBound;

  GenericDiscreteUpperBound(GenericDiscreteMDP* _x) : x(_x) {}
  void initialize(double targetPrecision) {
    // maxReward = max_a max_s R(s,a)
    double maxReward = -99e+20;
    FOR (a, x->getNumActions()) {
      FOR (s, x->numStates) {
	maxReward = std::max(maxReward, x->R(s,a));
      }
    }
    globalUpperBound = x->getLongTermFactor() * maxReward;
  }
  double getValue(const state_vector& s, const MDPNode* cn) const {
    return globalUpperBound;
  }
};

AbstractBound* GenericDiscreteMDP::newLowerBound(const ZMDPConfig* _config)
{
  return new GenericDiscreteLowerBound(this);
}

AbstractBound* GenericDiscreteMDP::newUpperBound(const ZMDPConfig* _config)
{
  return new GenericDiscreteUpperBound(this);
}


}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/11/09 20:46:12  trey
 * initial check-in
 *
 *
 ***************************************************************************/
