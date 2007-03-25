/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2007-03-25 07:08:16 $
  
 @file    CacheMDP.cc
 @brief   No brief

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
#include "CacheMDP.h"

using namespace std;
using namespace MatrixUtils;

namespace zmdp {

#define ACTION_MOVE_LEFT (0)
#define ACTION_MOVE_RIGHT (1)

CacheMDP::CacheMDP(MDP* _problem) :
  MDP(*_problem),
  problem(_problem)
{
  root = getNodeX(problem->getInitialState());
  initialSI.resize(1);
  initialSI.push_back(0, root->si);
}

CacheMDP::~CacheMDP(void)
{
  // do not delete 'problem', which is owned by the caller
  // do not delete 'root', which is owned by the node table

  // deallocate the states in the node table
  FOR_EACH (elt, nodeTable) {
    delete (*elt);
  }
}

const state_vector& CacheMDP::getInitialState(void)
{
  return initialSI;
}

bool CacheMDP::getIsTerminalState(const state_vector& s)
{
  return getNode(s)->isTerminal;
}

outcome_prob_vector& CacheMDP::getOutcomeProbVector(outcome_prob_vector& result,
						    const state_vector& s, int a)
{
  result = getQ(*getNode(s), a)->opv;
  return result;
}

state_vector& CacheMDP::getNextState(state_vector& result, const state_vector& s,
				     int a, int o)
{
  int spi = getQ(*getNode(s), a)->outcomes[o]->nextState->si;

#if 0
  int si = (int) (s(0));
  printf("getNextState si=%d a=%d o=%d spi=%d\n",
	 si, a, o, spi);
  static int cnt = 0;
  if (cnt++ > 1000) exit(0);
#endif

  result.resize(1);
  result.push_back(0, spi);
  return result;
}

double CacheMDP::getReward(const state_vector& s, int a)
{
  assert(s.size() == 1 && s.data.size() == 1);

  double reward = getQ(*getNode(s), a)->immediateReward;

#if 0
  int si = (int) (s(0));
  printf("getReward si=%d a=%d r=%.1lf\n", si, a, reward);
#endif

  return reward;
}

const state_vector& CacheMDP::translateState(state_vector& result, const state_vector& s)
{
  result = getNode(s)->s;
  return result;
}

CMDPNode* CacheMDP::getNode(const state_vector& s)
{
  assert(s.size() == 1 && s.data.size() == 1);
  int si = ((int) s(0));
  return nodeTable[si];
}

CMDPNode* CacheMDP::getNodeX(const state_vector& s)
{
  string hs = hashable(s);
  CMDPHash::iterator pr = lookup.find(hs);
  if (lookup.end() == pr) {
    // create a new fringe node
    int si = nodeTable.size();
    CMDPNode& cn = *(new CMDPNode);
    cn.s = s;
    cn.si = si;
    cn.isTerminal = problem->getIsTerminalState(s);
    cn.Q.resize(problem->getNumActions(), NULL);
    cn.userInt = -1;
    nodeTable.push_back(&cn);
    lookup[hs] = si;
    return &cn;
  } else {
    // return existing node
    return nodeTable[pr->second];
  }
}

CMDPQEntry* CacheMDP::getQ(CMDPNode& cn, int a)
{
  if (NULL == cn.Q[a]) {
    CMDPQEntry& Qa = *(new CMDPQEntry);
    Qa.immediateReward = problem->getReward(cn.s, a);
    problem->getOutcomeProbVector(Qa.opv, cn.s, a);
    Qa.outcomes.resize(Qa.opv.size(), NULL);
    FOR (o, Qa.opv.size()) {
      if (Qa.opv(o) > OBS_IS_ZERO_EPS) {
	CMDPEdge* e = new CMDPEdge;
	state_vector sp;
	problem->getNextState(sp, cn.s, a, o);
	e->nextState = getNodeX(sp);
	e->userInt = -1;
	e->userDouble = 0.0;
	Qa.outcomes[o] = e;
      } else {
	Qa.outcomes[o] = NULL;
      }
    }
    cn.Q[a] = &Qa;
    return &Qa;
  } else {
    return cn.Q[a];
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2007/03/24 22:44:15  trey
 * initial check-in
 *
 *
 ***************************************************************************/
