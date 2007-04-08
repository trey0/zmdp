/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2007-04-08 22:48:04 $
  
 @file    CassandraModel.cc
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

#include "CassandraModel.h"

using namespace std;

namespace zmdp {

CassandraModel::CassandraModel(void) :
  numStates(-1),
  numObservations(-1)
{}

void CassandraModel::checkForTerminalStates(void)
{
  if (zmdpDebugLevelG >= 1) {
    printf("model initialization -- marking zero-reward absorbing states as terminal\n");
  }
  isTerminalState.resize(numStates, /* initialValue = */ true);
  FOR (s, numStates) {
    FOR (a, numActions) {
      if ((fabs(1.0 - T[a](s,s)) > OBS_IS_ZERO_EPS) || R(s,a) != 0.0) {
	isTerminalState[s] = false;
	break;
      }
    }
  }
}

void CassandraModel::debugDensity(void)
{
  double T_size = -1, T_filled = -1;
  double O_size = -1, O_filled = -1;

  // use doubles to avoid int overflow (e.g. T_size is sometimes larger than MAX_INT)
  T_size = ((double) T[0].size1()) * T[0].size2() * numActions;
  T_filled = 0;
  if (-1 != numObservations) {
    O_size = ((double) O[0].size1()) * O[0].size2() * numActions;
    O_filled = 0;
  }
  FOR (a, numActions) {
    T_filled += T[a].filled();
    if (-1 != numObservations) {
      O_filled += O[a].filled();
    }
  }

  if (-1 == numObservations) {
    cout << "T density = " << (T_filled / T_size)
	 << endl;
  } else {
    cout << "T density = " << (T_filled / T_size)
	 << ", O density = " << (O_filled / O_size)
	 << endl;
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/11/09 21:01:33  trey
 * added bogus initialization code to avoid compilation warnings
 *
 * Revision 1.2  2006/11/09 20:47:46  trey
 * added initialState field for MDPs, fixed a potential problem with accessing undefined observation matrices with MDPs
 *
 * Revision 1.1  2006/11/08 16:40:50  trey
 * initial check-in
 *
 *
 ***************************************************************************/
