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

#ifndef ZMDP_SRC_POMDPMODELS_LIFESURVEY_LSPATHANDREACTEXEC_H_
#define ZMDP_SRC_POMDPMODELS_LIFESURVEY_LSPATHANDREACTEXEC_H_

#include <stdio.h>

#include <string>
#include <vector>

#include "LifeSurvey.h"
#include "MDPExec.h"

namespace zmdp {

struct LSPathEntry {
  int lastMoveDirection;
  LSPos pos;
};

typedef std::vector<LSPathEntry> LSPath;

struct LSValueEntry {
  double unreachableRegionValue;
  std::vector<int> regionCounts;
};

typedef std::vector<LSValueEntry> LSValueVector;

struct LSPathAndReactExec : public MDPExecCore {
  LSModel m;
  LSPath plannedPath;
  LSState currentLSState;

  std::vector<LSValueVector> bestValueMap;
  int numPathsEvaluated;
  bool useBlindActionSelection;

  LSPathAndReactExec(void);

  void init(const std::string &lifeSurveyFileName, const ZMDPConfig *config);

  // implement PomdpExec virtual methods
  void setToInitialState(void);
  int chooseAction(void);
  void advanceToNextState(int a, int o);

  // helpers
  int getDistanceToNearestPathCell(const LSPos &pos) const;
  void generatePath(void);
  void getBestExtension(LSPath &bestPathMatchingPrefix, double &bestPathValue,
                        LSPath &prefix, const LSValueEntry &valueSoFar);
  bool getValueIsDominated(const LSPathEntry &pe, const LSValueEntry &val);
  bool dominates(const LSValueEntry &val1, const LSValueEntry &val2);
};

}  // namespace zmdp

#endif  // ZMDP_SRC_POMDPMODELS_LIFESURVEY_LSPATHANDREACTEXEC_H_
