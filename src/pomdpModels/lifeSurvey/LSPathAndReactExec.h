/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.5 $  $Author: trey $  $Date: 2007-04-22 22:42:11 $
   
 @file    LSPathAndReactExec.h
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

#ifndef INCLSPathAndReactExec_h
#define INCLSPathAndReactExec_h

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

  void init(const std::string& lifeSurveyFileName,
	    const ZMDPConfig* config);

  // implement PomdpExec virtual methods
  void setToInitialState(void);
  int chooseAction(void);
  void advanceToNextState(int a, int o);

  // helpers
  int getDistanceToNearestPathCell(const LSPos& pos) const;
  void generatePath(void);
  void getBestExtension(LSPath& bestPathMatchingPrefix,
			double& bestPathValue,
			LSPath& prefix,
			const LSValueEntry& valueSoFar);
  bool getValueIsDominated(const LSPathEntry& pe,
			   const LSValueEntry& val);
  bool dominates(const LSValueEntry& val1,
		 const LSValueEntry& val2);
};

} // namespace zmdp

#endif // INCLSPathAndReactExec_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2007/03/23 00:01:20  trey
 * fixed to reflect migration from PomdpExec to MDPExec base class
 *
 * Revision 1.3  2006/11/08 16:42:38  trey
 * changed Pomdp constructor arguments
 *
 * Revision 1.2  2006/07/03 14:30:06  trey
 * code no longer makes subtly invalid assumptions about when one path dominates another, so the output path is guaranteed to be optimal (luckily, seems to output the same path as before)
 *
 * Revision 1.1  2006/06/29 21:37:56  trey
 * initial check-in
 *
 *
 ***************************************************************************/

