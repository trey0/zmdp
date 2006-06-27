/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-06-27 18:20:18 $
   
 @file    PomdpExec.cc
 @brief   No brief

 Copyright (c) 2002-2006, Trey Smith. All rights reserved.

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

//#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

#include <iostream>
#include <fstream>

#include "zmdpCommonDefs.h"
#include "MatrixUtils.h"
#include "PomdpExec.h"

using namespace std;
using namespace MatrixUtils;
using namespace sla;

namespace zmdp {

PomdpExec::PomdpExec(void) :
  pomdp(NULL),
  currentBeliefInitialized(false)
{}

bool PomdpExec::getBeliefIsTerminal(void) const
{
  return pomdp->getIsTerminalState(currentBelief);
}

belief_vector& PomdpExec::getBelief(belief_vector& b) const
{
  b = currentBelief;
  return b;
}

int PomdpExec::getRandomObservation(int a) const
{
  obs_prob_vector opv;
  return chooseFromDistribution(pomdp->getObsProbVector(opv, currentBelief, a));
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/06/24 16:25:27  trey
 * initial check-in
 *
 *
 ***************************************************************************/
