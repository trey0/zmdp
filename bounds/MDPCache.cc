/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2007-01-15 17:26:10 $
   
 @file    MDPCache.cc
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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <queue>

#include "MDPCache.h"
#include "MatrixUtils.h"
#include "AbstractBound.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

int getNodeCacheStorage(const MDPHash* lookup, int whichMetric)
{
  int eltCount = 0;
  int entryCount = 0;
  FOR_EACH (pr, *lookup) {
    if (!pr->second->isFringe()) {
      eltCount++;
      // the number of entries we count for each belief/value pair is
      // the number of entries in the belief plus one for the value
      entryCount += pr->second->s.filled() + 1;
    }
  }
  
  switch (whichMetric) {
  case ZMDP_S_NUM_ELTS_TABULAR:
    return eltCount;

  case ZMDP_S_NUM_ENTRIES_TABULAR:
    return entryCount;

  default:
    /* N/A */
    return 0;
  }
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2007/01/14 00:53:30  trey
 * added hooks for plotting storage space during a run
 *
 * Revision 1.2  2006/10/19 19:34:32  trey
 * removed MDPNodeHashLogger
 *
 * Revision 1.1  2006/10/17 19:16:39  trey
 * initial check-in
 *
 *
 ***************************************************************************/
