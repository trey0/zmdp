/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2007-04-08 22:46:57 $
   
 @file    BoundPairExec.cc
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
#include "BoundPairExec.h"
#include "MaxPlanesLowerBound.h"

using namespace std;
using namespace MatrixUtils;
using namespace sla;

namespace zmdp {

BoundPairExec::BoundPairExec(void) :
  bounds(NULL)
{}

// initializer to use if you already have data structures for the model
// and the bounds
void BoundPairExec::init(MDP* _mdp, BoundPair* _bounds)
{
  mdp = _mdp;
  bounds = _bounds;
  currentStateInitialized = false;
}

// alternate initializer that reads the model and bounds from files
void BoundPairExec::initReadFiles(const std::string& modelFileName,
				  const std::string& policyFileName,
				  const ZMDPConfig& config)
{
  bool useFastModelParser = config.getBool("useFastModelParser");
  std::string policyType = config.getString("policyType");
  timeval tv1, tv2;

  printf("BoundPairExec: reading pomdp model, useFastModelParser=%d\n",
	 useFastModelParser);
  gettimeofday(&tv1, NULL);
  Pomdp* pomdp = new Pomdp(modelFileName, &config);
  mdp = pomdp;
  gettimeofday(&tv2, NULL);
  printf("  (took %.3f seconds)\n",
	 (tv2.tv_sec - tv1.tv_sec) + 1e-6*(tv2.tv_usec - tv1.tv_usec));

  MaxPlanesLowerBound* lb = new MaxPlanesLowerBound(pomdp, &config);
  printf("BoundPairExec: reading policy of type '%s'\n", policyType.c_str());
  gettimeofday(&tv1, NULL);
  if (policyType == "maxPlanes") {
    lb->readFromFile(policyFileName);
  } else if (policyType == "cassandraAlpha") {
    lb->readFromCassandraAlphaFile(policyFileName);
  } else {
    fprintf(stderr, "ERROR: BoundPairExec: unknown policy type '%s'\n",
	    policyType.c_str());
    exit(EXIT_FAILURE);
  }
  gettimeofday(&tv2, NULL);
  printf("  (took %.3f seconds)\n",
	 (tv2.tv_sec - tv1.tv_sec) + 1e-6*(tv2.tv_usec - tv1.tv_usec));

  bounds = new BoundPair(/* maintainLowerBound = */ true,
			 /* maintainUpperBound = */ false,
			 /* useUpperBoundRunTimeActionSelection = */ false,
			 /* dualPointBounds = */ false);
  bounds->lowerBound = lb;
  bounds->initialize(mdp, &config);

  currentStateInitialized = false;
}

void BoundPairExec::setToInitialState(void)
{
  currentState = mdp->getInitialState();
  currentStateInitialized = true;
}

int BoundPairExec::chooseAction(void)
{
  return bounds->chooseAction(currentState);
}

void BoundPairExec::advanceToNextState(int a, int o)
{
  state_vector nextState;
  mdp->getNextState(nextState, currentState, a, o);
  currentState = nextState;
}

void BoundPairExec::setBelief(const belief_vector& b)
{
  currentState = b;
}
 
}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2007/03/23 02:11:50  trey
 * fixed init() to take MDP* argument instead of Pomdp*
 *
 * Revision 1.1  2007/03/23 00:26:11  trey
 * renamed MaxPlanesLowerBoundExec to BoundPairExec
 *
 * Revision 1.5  2007/03/23 00:01:04  trey
 * fixed to reflect migration from PomdpExec to MDPExec base class
 *
 * Revision 1.4  2006/11/08 16:40:15  trey
 * renamed useFastParser to useFastModelParser
 *
 * Revision 1.3  2006/10/24 02:36:12  trey
 * switched to use updated interface for MaxPlanesLowerBound
 *
 * Revision 1.2  2006/10/18 18:06:16  trey
 * now propagating config data structure to lower levels so config fields can be used to control more parts of the system
 *
 * Revision 1.1  2006/06/27 18:19:26  trey
 * initial check-in
 *
 *
 ***************************************************************************/
