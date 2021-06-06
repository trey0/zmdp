/********** tell emacs we use -*- c++ -*- style comments *******************
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

#ifndef ZMDP_SRC_POMDPMODELS_LIFESURVEY_LIFESURVEY_H_
#define ZMDP_SRC_POMDPMODELS_LIFESURVEY_LIFESURVEY_H_

#include <map>
#include <vector>

#include "LSModelFile.h"

namespace zmdp {

/**********************************************************************
 * MACROS
 **********************************************************************/

#define LS_NUM_ACTIONS (7)

#define LS_COST_MOVE (1 * mfile.baseCost)
#define LS_COST_SAMPLING_MOVE (5 * mfile.baseCost)
#define LS_COST_LOOKAHEAD (mfile.lookaheadCost * mfile.baseCost)

#define LS_PENALTY_ILLEGAL (100)

#define LS_REWARD_VISITED_REGION (5)
#define LS_REWARD_VISITED_LIFE (20)
#define LS_REWARD_SAMPLED_LIFE (50)

#define LS_UNREACHABLE (9)

#define OBS_NULL (LS_NUM_OBSERVATIONS - 1)

/**********************************************************************
 * DATA STRUCTURES
 **********************************************************************/

enum LSDirectionEnum { LS_NE = 0, LS_E = 1, LS_SE = 2 };

enum LSActionTypeEnum {
  LS_ACT_MOVE, /* move or sampling move */
  LS_ACT_LOOK
};

struct LSState {
  LSPos pos;
  int lastMoveDirection;       /* (see LSDirectionEnum) */
  int usedLookaheadInThisCell; /* boolean */
  /* array of booleans, indexed by direction (see LSDirectionEnum) */
  int lifeInNeighborCell[3];
  /* vector of reward levels in range 0-3, indexed by region number */
  std::vector<int> rewardLevelInRegion;

  LSState(void) {}
  explicit LSState(int si);
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
  explicit LSAction(int ai);
  int toInt(void) const;
  std::string toString(void) const;
};

struct LSObservation {
  int isNull; /* boolean */
  int lifeInNeighborConfidence[3];

  LSObservation(void) {}
  explicit LSObservation(int oi);
  int toInt(void) const;
  std::string toString(void) const;
};

struct LSStateEntry {
  LSState s;
  bool touched;
  double maxIncomingReward;
  int prevState;
};

struct LSStateTable {
  std::map<std::string, int> lookup;
  std::vector<LSStateEntry> states;

  int getStateIndex(const LSState &s);
  LSStateEntry &getState(int si);
};

struct LSOutcome {
  double prob;
  int nextState;
};

struct LSObsOutcome {
  double prob;
  int obs;
};

struct LSModel {
  LSModelFile mfile;
  std::vector<LSGrid> regionReachable;
  std::vector<LSPos> *targetList;

  LSModel(void);
  void init(const std::string &modelFileName);
  void calculateRegionReachable(LSGrid &result, const LSGrid &g, int r);
  LSPos getNeighbor(const LSPos &pos, int dir) const;
  double getReward(int oldRewardLevel, int newRewardLevel) const;
  double getLifePrior(const LSPos &pos) const;
  void getOutcomes(std::vector<LSOutcome> &outcomes, double &reward,
                   const LSState &s, int ai) const;
  void getObservations(std::vector<LSObsOutcome> &result, int spi,
                       int ai) const;
  void getInitialStateDistribution(std::vector<LSOutcome> &initStates);
  void writeToFile(FILE *outFile, bool fullIdentifiers);
  void setTargetList(const std::string &targetListFileName);
  static void readTargetList(std::vector<LSPos> &result, const char *fname);
  static bool getInTargetList(const LSPos &pos,
                              const std::vector<LSPos> targetList);
};

/**********************************************************************
 * GLOBAL VARIABLES
 **********************************************************************/

extern LSStateTable tableG;

}  // namespace zmdp

#endif  // ZMDP_SRC_POMDPMODELS_LIFESURVEY_LIFESURVEY_H_
