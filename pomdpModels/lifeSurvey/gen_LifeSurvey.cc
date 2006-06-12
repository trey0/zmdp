/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-06-12 18:11:39 $
  
 @file    gen_LifeSurvey.cc
 @brief   No brief

 Copyright (c) 2006, Trey Smith. All rights reserved.

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
#include <vector>

#include "zmdpCommonDefs.h"
#include "LSModelFile.h"

using namespace std;

/**********************************************************************
 * MACROS
 **********************************************************************/

#define LS_NUM_OBSERVATIONS (3)

#define LS_COST_BASE (1)

#define LS_COST_MOVE          (LS_COST_BASE)
#define LS_COST_SAMPLING_MOVE (5*LS_COST_BASE)
#define LS_COST_LOOKAHEAD     (5*LS_COST_BASE)

#define LS_PENALTY_ILLEGAL (100)

#define LS_REWARD_VISITED_REGION (5)
#define LS_REWARD_VISITED_LIFE   (20)
#define LS_REWARD_SAMPLED_LIFE   (50)

#define LS_UNREACHABLE (9)

/**********************************************************************
 * DATA STRUCTURES
 **********************************************************************/

enum LSDirectionEnum {
  LS_NE = 0,
  LS_E  = 1,
  LS_SE = 2
};

enum LSActionTypeEnum {
  LS_ACT_MOVE, /* move or sampling move */
  LS_ACT_LOOK,
  LS_ACT_EXIT
};

struct LSState {
  LSPos pos;
  int lastMoveDirection; /* (see LSDirectionEnum) */
  int usedLookaheadInThisCell; /* boolean */
  /* array of booleans, indexed by direction (see LSDirectionEnum) */
  int lifeInNeighborCell[3];
  /* vector of reward levels in range 0-3, indexed by region number */
  std::vector<int> rewardLevelInRegion;

  LSState(void) {}
  LSState(int si);
  int toInt(void) const;
  bool isTerminal(void) const {
    return (-1 == pos.x);
  }
  std::string toString(void) const {
    if (isTerminal()) {
      return "sterminal";
    } else {
      char rbuf[256], buf[256];
      assert(rewardLevelInRegion.size() < 250); /* avoid buffer overflow */
      FOR (i, rewardLevelInRegion.size()) {
	snprintf(&rbuf[i], 2, "%d", rewardLevelInRegion[i]);
      }
      snprintf(buf, sizeof(buf), "sx%02dy%dd%du%dn%d%d%dr%s",
	       pos.x, pos.y, lastMoveDirection, usedLookaheadInThisCell,
	       lifeInNeighborCell[0], lifeInNeighborCell[1], lifeInNeighborCell[2],
	       rbuf);
      return buf;
    }
  }
  static LSState getTerminalState(void) {
    LSState result;
    result.pos = LSPos(-1, -1);
    result.lastMoveDirection = -1;
    result.usedLookaheadInThisCell = -1;
    result.lifeInNeighborCell[0] = -1;
    result.lifeInNeighborCell[1] = -1;
    result.lifeInNeighborCell[2] = -1;
    return result;
  }
};

struct LSAction {
  int type; /* see LSActionTypeEnum */
  int useSample;
  int moveDirection;

  LSAction(void) {}
  LSAction(int ai) {
    assert(0 <= ai && ai <= 6);
    useSample = -1;
    moveDirection = -1;
    if (6 == ai) {
      type = LS_ACT_LOOK;
    } else if (7 == ai) {
      type = LS_ACT_EXIT;
    } else {
      type = LS_ACT_MOVE;
      useSample = ai / 3;
      moveDirection = ai % 3;
    }
  }
  int toInt(void) const {
    if (LS_ACT_LOOK == type) {
      return 6;
    } else if (LS_ACT_EXIT == type) {
      return 7;
    } else {
      return useSample * 3 + moveDirection;
    }
  }
  std::string toString(void) const {
    switch (toInt()) {
    case 0: return "ne";
    case 1: return "e";
    case 2: return "se";
    case 3: return "nes";
    case 4: return "es";
    case 5: return "ses";
    case 6: return "look";
    default:
      assert(0); // never reach this point
      return NULL;
    }
  }
};

struct LSObservation {
  int isNull; /* boolean */
  int lifeInNeighborConfidence[3];

  LSObservation(void) {}
  LSObservation(int oi) {
    if (8 == oi) {
      isNull = 1;
      lifeInNeighborConfidence[0] = -1;
      lifeInNeighborConfidence[1] = -1;
      lifeInNeighborConfidence[2] = -1;
    } else {
      isNull = 0;
      lifeInNeighborConfidence[2] = oi % LS_NUM_OBSERVATIONS;
      oi /= LS_NUM_OBSERVATIONS;
      lifeInNeighborConfidence[1] = oi % LS_NUM_OBSERVATIONS;
      oi /= LS_NUM_OBSERVATIONS;
      lifeInNeighborConfidence[0] = oi;
    }
  }
  int toInt(void) const {
    if (isNull) {
      return 0;
    } else {
      return lifeInNeighborConfidence[2]
	+ LS_NUM_OBSERVATIONS * (lifeInNeighborConfidence[1] +
				 LS_NUM_OBSERVATIONS * lifeInNeighborConfidence[0]);
    }
  }
  std::string toString(void) const {
    if (isNull) {
      return "onull";
    } else {
      char buf[256];
      snprintf(buf, sizeof(buf), "o%d%d%d",
	       lifeInNeighborConfidence[0],
	       lifeInNeighborConfidence[1],
	       lifeInNeighborConfidence[2]);
      return buf;
    }
  }
};

struct LSStateTable {
  std::map<std::string, int> lookup;
  std::vector<LSState> states;

  int getStateIndex(const LSState& s) {
    string ss = s.toString();
    typeof(lookup.begin()) sloc = lookup.find(ss);
    if (lookup.end() == sloc) {
      int si = states.size();
      lookup[ss] = si;
      states.push_back(s);
      return si;
    } else {
      return sloc->second;
    }
  }
  const LSState& getState(int si) const {
    return states[si];
  }
};

struct LSOutcome {
  double prob;
  int nextState;
  double reward;
};

struct LSObservationPair {
  double prob;
  int observation;
};

struct LSModel {
  LSModelFile mfile;
  std::vector<LSGrid> regionReachable;

  void init(const std::string& modelFileName) {
    mfile.readFromFile(modelFileName);

    // ensure terminal state gets index 0 (nice for debugging)
    LSState::getTerminalState().toInt();

    // FIX calculate regionReachable
  }

  LSPos getNeighbor(const LSPos& pos, int dir) const {
    switch (dir) {
    case LS_NE: return LSPos(pos.x+1, pos.y+1);
    case LS_E:  return LSPos(pos.x+1, pos.y);
    case LS_SE: return LSPos(pos.x,   pos.y-1);
    default:
      assert(0); /* never reach this point */
      return LSPos();
    }
  }

  double getReward(int oldRewardLevel, int newRewardLevel) const
  {
    static double rewards[] = { 0,
				LS_REWARD_VISITED_REGION,
				LS_REWARD_VISITED_LIFE,
				LS_REWARD_SAMPLED_LIFE };
    if (newRewardLevel > oldRewardLevel) {
      return rewards[newRewardLevel] - rewards[oldRewardLevel];
    } else {
      return 0.0;
    }
  }

  double getLifePrior(const LSPos& pos) const
  {
    int region = mfile.grid.getCellBounded(pos);
    if (LS_OBSTACLE == region) {
      return 0;
    } else {
      return mfile.regionPriors[region];
    }
  }

  void getOutcomeVector(std::vector<LSOutcome>& result,
			int si, int ai) const
  {
    LSState s(si);
    LSState nextState = s;
    LSOutcome outc;
    result.clear();

    if (s.isTerminal()) {
      /* terminal state -- all actions are absorbing with 0 cost */
      outc.prob = 1.0;
      outc.nextState = si;
      outc.reward = 0;
      result.push_back(outc);
      return;
    }

    LSAction a(ai);
    switch (a.type) {
    case LS_ACT_MOVE: { /* move or sampling move */
      /* update position-related information */
      nextState.pos = getNeighbor(s.pos, a.moveDirection);
      nextState.lastMoveDirection = a.moveDirection;
      nextState.usedLookaheadInThisCell = 0;
      
      int nextCellRegion = mfile.grid.getCellBounded(nextState.pos);
      if (/* move into an obstacle? */
	  (LS_OBSTACLE == nextCellRegion)
	  /* move turns too sharply? */
	  || (LS_NE == s.lastMoveDirection && LS_SE == a.moveDirection)
	  || (LS_SE == s.lastMoveDirection && LS_NE == a.moveDirection)) {
	/* illegal move -- no state change and incur a penalty */
	outc.prob = 1.0;
	outc.nextState = si;
	outc.reward = -LS_PENALTY_ILLEGAL;
	result.push_back(outc);
	return;
      }
      
      /* update reward level */
      double actionCost = a.useSample ? LS_COST_SAMPLING_MOVE : LS_COST_MOVE;
      int oldRewardLevel = s.rewardLevelInRegion[nextCellRegion];
      int newRewardLevel;
      if (s.lifeInNeighborCell[a.moveDirection]) {
	newRewardLevel = a.useSample ? 3 : 2;
      } else {
	newRewardLevel = 1;
      }
      double actionReward = getReward(oldRewardLevel, newRewardLevel);
      nextState.rewardLevelInRegion[nextCellRegion] = newRewardLevel;
      
      /* compress state by quashing useless reward level information
	 about unreachable regions */
      FOR (i, nextState.rewardLevelInRegion.size()) {
	if (LS_UNREACHABLE != nextState.rewardLevelInRegion[i]) {
	  if (1 != regionReachable[i].getCell(nextState.pos)) {
	    nextState.rewardLevelInRegion[i] = LS_UNREACHABLE;
	  }
	}
      }
      
      /* generate outcomes for different possible nextState.lifeInNeighborCell values */
      outc.reward = actionReward - actionCost;
      double np[3];
      FOR (j, 3) {
	np[j] = getLifePrior(getNeighbor(nextState.pos, j));
      }
      switch (a.moveDirection) {
      case LS_NE:
	/* new SE cell is old E cell */
	np[LS_SE] = s.lifeInNeighborCell[LS_E] ? 1.0 : 0.0;
	break;
      case LS_E:
	/* no overlap */
	break;
      case LS_SE:
	/* new NE cell is old E cell */
	np[LS_NE] = s.lifeInNeighborCell[LS_E] ? 1.0 : 0.0;
	break;
      default:
	assert(0); // never reach this point
      }
      FOR (i, 8) {
	outc.prob = 1.0;
	FOR (j, 3) {
	  int nset = (i>>j) & 1;
	  nextState.lifeInNeighborCell[j] = nset;
	  outc.prob *= nset ? np[j] : (1-np[j]);
	}
	if (outc.prob != 0.0) {
	  outc.nextState = nextState.toInt();
	  result.push_back(outc);
	}
      }
      return;
    } /* case LS_ACT_MOVE */

    case LS_ACT_LOOK:
      /* look action: mark look action as used, incur cost */
      nextState.usedLookaheadInThisCell = 1;

      outc.prob = 1.0;
      outc.nextState = nextState.toInt();
      outc.reward = -LS_COST_LOOKAHEAD;
      result.push_back(outc);
      return;

    case LS_ACT_EXIT:
      if (mfile.grid.getExitLegal(s.pos)) {
	/* legal exit: transition to terminal state, no cost */
	outc.prob = 1.0;
	outc.nextState = LSState::getTerminalState().toInt();
	outc.reward = 0;
	result.push_back(outc);
	return;
      } else {
	/* illegal exit: no change in state, incur penalty */
	outc.prob = 1.0;
	outc.nextState = si;
	outc.reward = -LS_PENALTY_ILLEGAL;
	result.push_back(outc);
	return;
      }

    default:
      assert(0); // never reach this point
      return;
    }
  }

  void getObservationVector(std::vector<LSObservationPair>& result,
			    const LSState& nextState,
			    int oi) const
  {
    // FIX fill me in
  }
};


/**********************************************************************
 * GLOBAL VARIABLES
 **********************************************************************/

LSStateTable tableG;

/**********************************************************************
 * FUNCTIONS
 **********************************************************************/

LSState::LSState(int si)
{
  *this = tableG.getState(si);
}

int LSState::toInt(void) const
{
  return tableG.getStateIndex(*this);
}

/**********************************************************************
 * MAIN
 **********************************************************************/

void usage(const char* argv0) {
  cerr <<
    "usage: " << argv0 << " OPTIONS <map.lifeSurvey>\n"
    "  -h or --help   Display this help\n";
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  static char shortOptions[] = "h";
  static struct option longOptions[]={
    {"help",          0,NULL,'h'},
    {NULL,0,0,0}
  };

  while (1) {
    char optchar = getopt_long(argc,argv,shortOptions,longOptions,NULL);
    if (optchar == -1) break;

    switch (optchar) {
    case 'h': // help
      usage(argv[0]);
      break;

    case '?': // unknown option
    case ':': // option with missing parameter
      // getopt() prints an informative error message
      cerr << endl;
      usage(argv[0]);
      break;
    default:
      abort(); // never reach this point
    }
  }
  if (argc-optind != 1) {
    cerr << "ERROR: wrong number of arguments (should be 1)" << endl << endl;
    usage(argv[0]);
  }

  const char* modelFileName = argv[optind++];

  LSModel m;
  m.init(modelFileName);
  
  // FIX fill me in
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
