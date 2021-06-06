/********** tell emacs we use -*- c++ -*- style comments *******************
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

#include "Pomdp.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <fstream>
#include <iostream>

#include "CassandraParser.h"
#include "FastParser.h"
#include "MatrixUtils.h"
#include "MaxPlanesLowerBound.h"
#include "SawtoothUpperBound.h"
#include "slaMatrixUtils.h"
#include "zmdpCommonDefs.h"

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

Pomdp::Pomdp(const std::string &fileName, const ZMDPConfig *config) {
  bool useFastModelParser = config->getBool("useFastModelParser");
  if (useFastModelParser) {
    FastParser parser;
    parser.readPomdpFromFile(*this, fileName);
  } else {
    CassandraParser parser;
    parser.readPomdpFromFile(*this, fileName);
  }

  maxHorizon = config->getInt("maxHorizon");

  // belief vectors are the 'state vectors' of the belief-MDP; the
  // dimensionality of these vectors is the number of states in
  // the POMDP
  numStateDimensions = numStates;
}

const belief_vector &Pomdp::getInitialBelief(void) const {
  return initialBelief;
}

obs_prob_vector &Pomdp::getObsProbVector(obs_prob_vector &result,
                                         const belief_vector &b, int a) const {
  dvector tmp;  // FIX: for efficiency, should tmp be a cvector?
  // --- overall: result = O_a' * T_a' * b
  // tmp = T_a' * b
  mult(tmp, Ttr[a], b);
  // result = O_a' * tmp
  mult(result, tmp, O[a]);

  return result;
}

belief_vector &Pomdp::getNextBelief(belief_vector &result,
                                    const belief_vector &b, int a,
                                    int o) const {
  belief_vector tmp;

  // result = O_a(:,o) .* (T_a * b)
  mult(tmp, Ttr[a], b);
  emult_column(result, O[a], o, tmp);

  // renormalize
  result *= (1.0 / sum(result));

  return result;
}

double Pomdp::getReward(const belief_vector &b, int a) {
  return inner_prod_column(R, a, b);
}

AbstractBound *Pomdp::newLowerBound(const ZMDPConfig *_config) {
  return new MaxPlanesLowerBound(this, _config);
}

AbstractBound *Pomdp::newUpperBound(const ZMDPConfig *_config) {
  return new SawtoothUpperBound(this, _config);
}

bool Pomdp::getIsTerminalState(const state_vector &s) {
  double nonTerminalSum = 0.0;
  FOR_CV(s) {
    if (!isTerminalState[CV_INDEX(s)]) {
      nonTerminalSum += CV_VAL(s);
    }
  }
  return (nonTerminalSum < 1e-10);
}

};  // namespace zmdp
