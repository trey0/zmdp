/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-06-12 21:09:34 $
  
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
#include <queue>

#include "zmdpCommonDefs.h"
#include "LSModelFile.h"

using namespace std;

/**********************************************************************
 * MACROS
 **********************************************************************/

#define LS_NUM_ACTIONS (8)

#define LS_COST_BASE (1)

#define LS_COST_MOVE          (LS_COST_BASE)
#define LS_COST_SAMPLING_MOVE (5*LS_COST_BASE)
#define LS_COST_LOOKAHEAD     (5*LS_COST_BASE)

#define LS_PENALTY_ILLEGAL (100)

#define LS_REWARD_VISITED_REGION (5)
#define LS_REWARD_VISITED_LIFE   (20)
#define LS_REWARD_SAMPLED_LIFE   (50)

#define LS_UNREACHABLE (9)

#define OBS_NULL (8)

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
  bool isTerminal(void) const;
  std::string toString(void) const;
  static LSState getTerminalState(void);
};

struct LSAction {
  int type; /* see LSActionTypeEnum */
  int useSample;
  int moveDirection;

  LSAction(void) {}
  LSAction(int ai);
  int toInt(void) const;
  std::string toString(void) const;

};

struct LSObservation {
  int isNull; /* boolean */
  int lifeInNeighborConfidence[3];

  LSObservation(void) {}
  LSObservation(int oi);
  int toInt(void) const;
  std::string toString(void) const;
};

struct LSStateEntry {
  LSState s;
  bool visited;
};

struct LSStateTable {
  std::map<std::string, int> lookup;
  std::vector<LSStateEntry> states;

  int getStateIndex(const LSState& s);
  const LSStateEntry& getState(int si) const;
};

struct LSOutcome {
  double prob;
  int nextState;
  double reward;
};

struct LSObsOutcome {
  double prob;
  int obs;
};

struct LSModel {
  LSModelFile mfile;
  std::vector<LSGrid> regionReachable;

  void init(const std::string& modelFileName);
  void calculateRegionReachable(LSGrid& result,
				const LSGrid& g,
				int r);
  LSPos getNeighbor(const LSPos& pos, int dir) const;
  double getReward(int oldRewardLevel, int newRewardLevel) const;
  double getLifePrior(const LSPos& pos) const;
  void getOutcomeVector(std::vector<LSOutcome>& result,
			const LSState& s, int ai) const;
  void getObservationVector(std::vector<LSObsOutcome>& result,
			    int spi, int ai) const;
  void writeToFile(FILE* outFile);
};


/**********************************************************************
 * GLOBAL VARIABLES
 **********************************************************************/

LSStateTable tableG;

/**********************************************************************
 * LSSTATE FUNCTIONS
 **********************************************************************/

LSState::LSState(int si)
{
  *this = tableG.getState(si).s;
}

int LSState::toInt(void) const
{
  return tableG.getStateIndex(*this);
}

bool LSState::isTerminal(void) const
{
  return (-999 == pos.x);
}

std::string LSState::toString(void) const
{
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

LSState LSState::getTerminalState(void)
{
  LSState result;
  result.pos = LSPos(-999, -999);
  result.lastMoveDirection = -1;
  result.usedLookaheadInThisCell = -1;
  result.lifeInNeighborCell[0] = -1;
  result.lifeInNeighborCell[1] = -1;
  result.lifeInNeighborCell[2] = -1;
  return result;
}

/**********************************************************************
 * LSACTION FUNCTIONS
 **********************************************************************/

LSAction::LSAction(int ai)
{
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

int LSAction::toInt(void) const
{
  if (LS_ACT_LOOK == type) {
    return 6;
  } else if (LS_ACT_EXIT == type) {
    return 7;
  } else {
    return useSample * 3 + moveDirection;
  }
}

std::string LSAction::toString(void) const
{
  switch (toInt()) {
  case 0: return "ane";
  case 1: return "ae";
  case 2: return "ase";
  case 3: return "anes";
  case 4: return "aes";
  case 5: return "ases";
  case 6: return "alook";
  case 7: return "aexit";
  default:
    assert(0); // never reach this point
    return NULL;
  }
}

/**********************************************************************
 * LSOBSERVATION FUNCTIONS
 **********************************************************************/

LSObservation::LSObservation(int oi)
{
  if (OBS_NULL == oi) {
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

int LSObservation::toInt(void) const
{
  if (isNull) {
    return 0;
  } else {
    return lifeInNeighborConfidence[2]
      + LS_NUM_OBSERVATIONS * (lifeInNeighborConfidence[1] +
			       LS_NUM_OBSERVATIONS * lifeInNeighborConfidence[0]);
  }
}

std::string LSObservation::toString(void) const
{
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

/**********************************************************************
 * LSSTATETABLE FUNCTIONS
 **********************************************************************/

int LSStateTable::getStateIndex(const LSState& s)
{
  string ss = s.toString();
  typeof(lookup.begin()) sloc = lookup.find(ss);
  if (lookup.end() == sloc) {
    int si = states.size();
    lookup[ss] = si;
    LSStateEntry e;
    e.s = s;
    e.visited = false;
    states.push_back(e);
    return si;
  } else {
    return sloc->second;
  }
}

const LSStateEntry& LSStateTable::getState(int si) const
{
  return states[si];
}

/**********************************************************************
 * LSMODEL FUNCTIONS
 **********************************************************************/

void LSModel::init(const std::string& modelFileName)
{
  mfile.readFromFile(modelFileName);
  
  // ensure terminal state gets index 0 (nice for debugging)
  LSState::getTerminalState().toInt();

  // calculate a regionReachable map for each region r
  regionReachable.resize(mfile.regionPriors.size());
  FOR (r, mfile.regionPriors.size()) {
    calculateRegionReachable(regionReachable[r], mfile.grid, r);
  }
}

void LSModel::calculateRegionReachable(LSGrid& result,
				       const LSGrid& g,
				       int r)
{
  /* set obstacle cells in result to match obstacle cells in g,
     and turn non-obstacle cells of g into a binary mask, where
     1 indicates that the cell is in region r */
  result = g;
  FOR (i, g.width * g.height) {
    unsigned char gi = g.data[i];
    if (LS_OBSTACLE != gi) {
      result.data[i] = (r == gi);
    }
  }
  
  /* now use dynamic programming to additionally mark cells that can
     reach region r */
  int numChanges;
  do {
    numChanges = 0;
    FOR (y, result.height) {
      for (int x=result.width-1; x >= 0; x--) {
	LSPos pos(x,y);
	if (0 == result.getCell(pos)
	    && ((1 == result.getCellBounded(getNeighbor(pos, LS_NE)))
		|| (1 == result.getCellBounded(getNeighbor(pos, LS_E)))
		|| (1 == result.getCellBounded(getNeighbor(pos, LS_SE))))) {
	  result.setCell(pos, 1);
	  numChanges++;
	}
      }
    }
  } while (numChanges > 0);
}

LSPos LSModel::getNeighbor(const LSPos& pos, int dir) const
{
  switch (dir) {
  case LS_NE: return LSPos(pos.x+1, pos.y+1);
  case LS_E:  return LSPos(pos.x+1, pos.y);
  case LS_SE: return LSPos(pos.x,   pos.y-1);
  default:
    assert(0); /* never reach this point */
    return LSPos();
  }
}

double LSModel::getReward(int oldRewardLevel, int newRewardLevel) const
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

double LSModel::getLifePrior(const LSPos& pos) const
{
  int region = mfile.grid.getCellBounded(pos);
  if (LS_OBSTACLE == region) {
    return 0;
    } else {
      return mfile.regionPriors[region];
    }
}

void LSModel::getOutcomeVector(std::vector<LSOutcome>& result,
			       const LSState& s, int ai) const
{
  int si = s.toInt();
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
    
    /* canonicalize state by quashing useless reward level information
       about unreachable regions */
    FOR (r, nextState.rewardLevelInRegion.size()) {
      if (LS_UNREACHABLE != nextState.rewardLevelInRegion[r]) {
	if (!regionReachable[r].getCell(nextState.pos)) {
	  nextState.rewardLevelInRegion[r] = LS_UNREACHABLE;
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
      int i0 = i;
      for (int j=2; j >= 0; j--) {
	int nset = i0 % 2;
	i0 /= 2;
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
    if (s.usedLookaheadInThisCell) {
      /* can only use lookahead action once in any given cell */
      outc.prob = 1.0;
      outc.nextState = si;
      outc.reward = LS_PENALTY_ILLEGAL;
      result.push_back(outc);
      return;
    }
    
    /* mark look action as used, incur cost */
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

void LSModel::getObservationVector(std::vector<LSObsOutcome>& result,
				   int spi, int ai) const
{
  LSAction a(ai);
  LSObsOutcome outc;
  result.clear();
  
  if (LS_ACT_LOOK != a.type) {
    outc.prob = 1.0;
    outc.obs = OBS_NULL;
    result.push_back(outc);
  } else {
    LSState sp(spi);
    FOR (i, LS_NUM_OBSERVATIONS*LS_NUM_OBSERVATIONS*LS_NUM_OBSERVATIONS) {
      outc.obs = i;
      outc.prob = 1.0;
      int i0 = i;
      for (int j=2; j >= 0; j--) {
	int obsj = i0 % LS_NUM_OBSERVATIONS;
	i0 /= LS_NUM_OBSERVATIONS;
	outc.prob *= (sp.lifeInNeighborCell[j]
		      ? mfile.obsDistributionLifePresent[obsj]
		      : mfile.obsDistributionLifeAbsent[obsj]);
      }
      if (outc.prob > 0.0) {
	result.push_back(outc);
      }
    }
  }
}

void LSModel::writeToFile(FILE* outFile)
{
  // generate possible initial states
  LSState preInit;
  preInit.pos = LSPos(mfile.startX-1, mfile.startY);
  preInit.lastMoveDirection = LS_E;
  preInit.usedLookaheadInThisCell = 0;
  preInit.rewardLevelInRegion.resize(mfile.regionPriors.size(), 0);

  LSAction a;
  a.type = LS_ACT_MOVE;
  a.useSample = 0;
  a.moveDirection = LS_E;

  std::vector<LSOutcome> initStates;
  getOutcomeVector(initStates, preInit, a.toInt());
  
  // generate all reachable states using breadth-first search
  std::queue<int> stateQueue;
  FOR_EACH (sp, initStates) {
    stateQueue.push(sp->nextState);
  }
  while (!stateQueue.empty()) {
    int si = stateQueue.back();
    stateQueue.pop();
    const LSState& s = tableG.getState(si).s;
    FOR (ai, LS_NUM_ACTIONS) {
      std::vector<LSOutcome> outcomes;
      getOutcomeVector(outcomes, s, ai);
      FOR_EACH (sp, outcomes) {
	if (!tableG.getState(sp->nextState).visited) {
	  stateQueue.push(sp->nextState);
	}
      }
    }
  }
  int numStates = tableG.states.size();

  // convert initStates to dense belief vector
  std::vector<double> initBelief(numStates, 0);
  FOR_EACH (sp, initStates) {
    initBelief[sp->nextState] = sp->prob;
  }

  // write preamble
  fprintf(outFile, "discount: 1.0\n");
  fprintf(outFile, "values: reward\n");

  fprintf(outFile, "states: ");
  FOR (si, numStates) {
    LSState s(si);
    fprintf(outFile, "%s ", s.toString().c_str());
  }
  fprintf(outFile, "\n");

  fprintf(outFile, "actions: ");
  FOR (ai, LS_NUM_ACTIONS) {
    LSAction a(ai);
    fprintf(outFile, "%s ", a.toString().c_str());
  }
  fprintf(outFile, "\n");

  fprintf(outFile, "observations: ");
  FOR (oi, LS_NUM_OBSERVATIONS) {
    LSObservation o(oi);
    fprintf(outFile, "%s ", o.toString().c_str());
  }
  fprintf(outFile, "\n");

  fprintf(outFile, "start: ");
  FOR_EACH (sp, initBelief) {
    fprintf(outFile, "%lf ", *sp);
  }
  fprintf(outFile, "\n");

  fprintf(outFile, "\n");

  // write main body
  // FIX fill me in
}

/**********************************************************************
 * MAIN
 **********************************************************************/

void usage(const char* argv0) {
  cerr <<
    "usage: " << argv0 << " OPTIONS <my.lifeSurvey> <my.pomdp>\n"
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
  if (argc-optind != 2) {
    cerr << "ERROR: wrong number of arguments (should be 1)" << endl << endl;
    usage(argv[0]);
  }

  const char* modelFileName = argv[optind++];
  const char* pomdpFileName = argv[optind++];

  LSModel m;
  m.init(modelFileName);

  FILE* pomdpFile = fopen(pomdpFileName, "w");
  if (NULL == pomdpFile) {
    fprintf(stderr, "ERROR: couldn't open '%s' for writing: %s\n",
	    pomdpFileName, strerror(errno));
    exit(EXIT_FAILURE);
  }
  m.writeToFile(pomdpFile);
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/06/12 18:11:39  trey
 * initial check-in
 *
 *
 ***************************************************************************/
