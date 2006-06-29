/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-06-29 21:37:56 $
   
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
#include "PomdpExec.h"

namespace zmdp {

struct LSPathEntry {
  int lastMoveDirection;
  LSPos pos;
};

typedef std::vector<LSPathEntry> LSPath;

struct LSPathAndReactExec : public PomdpExec {
  LSModel m;
  LSPath plannedPath;
  LSState currentState;

  std::vector<double> bestValueMap;
  int numPathsEvaluated;

  LSPathAndReactExec(void);

  void init(const std::string& pomdpFileName,
	    bool useFastParser,
	    const std::string& lifeSurveyFileName);

  // implement PomdpExec virtual methods
  void setToInitialBelief(void);
  int chooseAction(void);
  void advanceToNextBelief(int a, int o);

  // helpers
  int getDistanceToNearestPathCell(const LSPos& pos) const;
  void generatePath(void);
  void getBestExtension(LSPath& bestPathMatchingPrefix,
			double& bestPathValue,
			LSPath& prefix,
			const std::vector<int>& cellsCoveredInRegion);
};

} // namespace zmdp

#endif // INCLSPathAndReactExec_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/

