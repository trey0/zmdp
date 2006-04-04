/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-04-04 17:22:51 $
   
 @file    IncrementalBounds.cc
 @brief   Common code used by multiple RTDP variants found in this
          directory.

 Copyright (c) 2006, Trey Smith. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * The software may not be sold or incorporated into a commercial
   product without specific prior written permission.
 * The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 ***************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <queue>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTime.h"
#include "MatrixUtils.h"
#include "Pomdp.h"
#include "IncrementalBounds.h"

using namespace std;
using namespace sla;
using namespace MatrixUtils;

namespace zmdp {

// relies on correct Q values!
int IncrementalBounds::getMaxUBAction(MDPNode& cn) const
{
  double bestVal = -99e+20;
  int bestAction = -1;
  FOR (a, cn.getNumActions()) {
    const MDPQEntry& Qa = cn.Q[a];
    if (Qa.ubVal > bestVal) {
      bestVal = Qa.ubVal;
      bestAction = a;
    }
  }
  return bestAction;
}

int IncrementalBounds::getSimulatedOutcome(MDPNode& cn, int a) const
{
  double r = unit_rand();
  int result = 0;
  MDPQEntry& Qa = cn.Q[a];
  FOR (o, Qa.getNumOutcomes()) {
    MDPEdge* e = Qa.outcomes[o];
    if (NULL != e) {
      r -= e->obsProb;
      if (r <= 0) {
	result = o;
	break;
      }
    }
  }

  return result;
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
