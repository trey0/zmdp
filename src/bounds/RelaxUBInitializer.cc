/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2005, Trey Smith. All rights reserved.

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

#include "RelaxUBInitializer.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include "MDPModel.h"
#include "MatrixUtils.h"
#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

#define OBS_IS_ZERO_EPS (1e-10)

namespace zmdp {

RelaxUBInitializer::RelaxUBInitializer(MDP *_problem, const ZMDPConfig *_config)
    : problem(_problem), root(NULL), config(_config) {}

void RelaxUBInitializer::setup(double targetPrecision) {
  initLowerBound = problem->newLowerBound(config);
  initLowerBound->initialize(targetPrecision);
  initUpperBound = problem->newUpperBound(config);
  initUpperBound->initialize(targetPrecision);

  lookup = new MDPHash();
  root = getNode(problem->getInitialState());
}

MDPNode *RelaxUBInitializer::getNode(const state_vector &s) {
  string hs = hashable(s);
  MDPHash::iterator pr = lookup->find(hs);
  if (lookup->end() == pr) {
    // create a new fringe node
    MDPNode &cn = *(new MDPNode);
    cn.s = s;
    cn.lbVal = initLowerBound->getValue(s, NULL);
    cn.ubVal = initUpperBound->getValue(s, NULL);
    (*lookup)[hs] = &cn;
    return &cn;
  } else {
    // return existing node
    return pr->second;
  }
}

void RelaxUBInitializer::expand(MDPNode &cn) {
  // set up successors for this fringe node (possibly creating new fringe nodes)
  outcome_prob_vector opv;
  state_vector sp;
  cn.Q.resize(problem->getNumActions());
  FOR(a, problem->getNumActions()) {
    MDPQEntry &Qa = cn.Q[a];
    Qa.immediateReward = problem->getReward(cn.s, a);
    problem->getOutcomeProbVector(opv, cn.s, a);
    Qa.outcomes.resize(opv.size());
    FOR(o, opv.size()) {
      double oprob = opv(o);
      if (oprob > OBS_IS_ZERO_EPS) {
        MDPEdge *e = new MDPEdge();
        Qa.outcomes[o] = e;
        e->obsProb = oprob;
        e->nextState = getNode(problem->getNextState(sp, cn.s, a, o));
      } else {
        Qa.outcomes[o] = NULL;
      }
    }
  }
}

void RelaxUBInitializer::updateInternal(MDPNode &cn) {
  cn.lbVal = -99e+20;
  cn.ubVal = -99e+20;
  FOR(a, cn.getNumActions()) {
    MDPQEntry &Qa = cn.Q[a];
    Qa.lbVal = -99e+20;
    Qa.ubVal = -99e+20;
    FOR(o, Qa.getNumOutcomes()) {
      MDPEdge *e = Qa.outcomes[o];
      if (NULL != e) {
        MDPNode &sn = *e->nextState;
        Qa.lbVal = std::max(Qa.lbVal, sn.lbVal);
        Qa.ubVal = std::max(Qa.ubVal, sn.ubVal);
      }
    }
    Qa.lbVal = Qa.immediateReward + problem->getDiscount() * Qa.lbVal;
    Qa.ubVal = Qa.immediateReward + problem->getDiscount() * Qa.ubVal;

    cn.lbVal = std::max(cn.lbVal, Qa.lbVal);
    cn.ubVal = std::max(cn.ubVal, Qa.ubVal);
  }
}

void RelaxUBInitializer::update(MDPNode &cn) {
  if (cn.isFringe()) {
    expand(cn);
  }
  updateInternal(cn);
}

int RelaxUBInitializer::getMaxUBAction(MDPNode &cn, double *maxUBValP,
                                       double *secondBestUBValP) const {
  double maxUBVal = -99e+20;
  double secondBestUBVal = -99e+20;
  int maxUBAction = -1;

  FOR(a, cn.getNumActions()) {
    MDPQEntry &Qa = cn.Q[a];

    if (Qa.ubVal > maxUBVal) {
      secondBestUBVal = maxUBVal;
      maxUBVal = Qa.ubVal;
      maxUBAction = a;
    } else if (Qa.ubVal > secondBestUBVal) {
      secondBestUBVal = Qa.ubVal;
    }
  }

  if (NULL != maxUBValP) *maxUBValP = maxUBVal;
  if (NULL != secondBestUBValP) *secondBestUBValP = secondBestUBVal;
  return maxUBAction;
}

void RelaxUBInitializer::trialRecurse(MDPNode &cn, double costSoFar,
                                      double altActionPrio, int depth) {
  // update to ensure cached values in cn.Q are correct
  update(cn);
  double maxUBVal, secondBestUBVal;
  int maxUBAction = getMaxUBAction(cn, &maxUBVal, &secondBestUBVal);

  // check for termination
  double actionPrio = costSoFar + cn.ubVal;
  if (altActionPrio > actionPrio) {
    if (zmdpDebugLevelG >= 1) {
      printf(
          "  RB trialRecurse: depth=%d [%g .. %g] costSoFar=%g "
          "altActionPrio=%g actionPrio=%g (terminating)\n",
          depth, cn.lbVal, cn.ubVal, costSoFar, altActionPrio, actionPrio);
    }
    return;
  }

  // select best possible outcome
  MDPQEntry &Qbest = cn.Q[maxUBAction];
  double bestVal = -99e+20;
  int bestOutcome = -1;
  FOR(o, Qbest.getNumOutcomes()) {
    MDPEdge *e = Qbest.outcomes[o];
    if (NULL != e) {
      MDPNode &sn = *e->nextState;
      if (sn.ubVal > bestVal) {
        bestVal = sn.ubVal;
        bestOutcome = o;
      }
    }
  }

  if (zmdpDebugLevelG >= 1) {
    printf(
        "  RB trialRecurse: depth=%d a=%d o=%d [%g .. %g] altActionPrio=%g "
        "actionPrio=%g \n",
        depth, maxUBAction, bestOutcome, cn.lbVal, cn.ubVal, altActionPrio,
        actionPrio);
    printf("  RB trialRecurse: s=%s\n", sparseRep(cn.s).c_str());
  }

  // recurse to successor
  double nextCostSoFar = costSoFar + cn.Q[maxUBAction].immediateReward;
  double nextAltActionPrio =
      std::max(altActionPrio, secondBestUBVal + costSoFar);
  trialRecurse(cn.getNextState(maxUBAction, bestOutcome), nextCostSoFar,
               nextAltActionPrio, depth + 1);

  update(cn);
}

void RelaxUBInitializer::doTrial(MDPNode &cn, double pTarget) {
  trialRecurse(cn,
               /* costSoFar = */ 0,
               /* altActionPrio = */ -99e+20,
               /* depth = */ 0);
}

void RelaxUBInitializer::initialize(double targetPrecision) {
  timeval startTime;
  if (zmdpDebugLevelG >= 1) {
    startTime = getTime();
  }

  setup(targetPrecision);

  int trialNum = 1;
  double width;
  while (1) {
    width = root->ubVal - root->lbVal;
    if (zmdpDebugLevelG >= 1) {
      printf("-*- RB initialize trial=%d width=%g target=%g\n", trialNum, width,
             targetPrecision);
    }
    if (width < targetPrecision) {
      if (zmdpDebugLevelG >= 1) {
        printf("RB initialize width=%g target=%g terminating\n", width,
               targetPrecision);
      }
      break;
    }
    doTrial(*root, width * problem->getDiscount());
    trialNum++;
  }

  if (zmdpDebugLevelG >= 1) {
    double elapsedTime = timevalToSeconds(getTime() - startTime);
    printf("--> RB initialization completed after %g seconds\n", elapsedTime);
  }
}

double RelaxUBInitializer::getValue(const state_vector &s,
                                    const MDPNode *cn) const {
  if (NULL == cn) {
    return initUpperBound->getValue(s, NULL);
  } else {
    return cn->ubVal;
  }
}

int RelaxUBInitializer::getStorage(int whichMetric) const {
  return getNodeCacheStorage(lookup, whichMetric);
}

};  // namespace zmdp
