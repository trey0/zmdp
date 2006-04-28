/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $

 @file    solverUtils.cc
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

#include <iostream>
#include <fstream>

#include "zmdpCommonTime.h"
#include "zmdpCommonDefs.h"
#include "MatrixUtils.h"
#include "solverUtils.h"

using namespace std;
using namespace MatrixUtils;

extern void usage(const char* cmdName);

namespace zmdp {

/**********************************************************************
 * HELPER FUNCTIONS
 **********************************************************************/

static EnumEntry strategiesG[] = {
  {"rtdp",  S_RTDP},
  {"lrtdp", S_LRTDP},
  {"hdp",   S_HDP},
  {"hsvi",  S_HSVI},
  {"frtdp", S_FRTDP},
  {NULL, -1}
};

static EnumEntry probTypesG[] = {
  {"racetrack", T_RACETRACK},
  {"pomdp",     T_POMDP},
  {NULL, -1}
};

static EnumEntry valueReprsG[] = {
  {"point",  V_POINT},
  {"convex", V_CONVEX},
  {NULL, -1}
};

int getEnum(const char* key, EnumEntry* table, const char* cmdName, const char *opt)
{
  EnumEntry* i = table;
  for (; NULL != i->key; i++) {
    if (0 == strcmp(i->key, key)) {
      return i->val;
    }
  }
  fprintf(stderr, "ERROR: invalid parameter %s for option %s\n\n", key, opt);
  usage(cmdName);
  exit(EXIT_FAILURE);
}

bool endsWith(const std::string& s,
	      const std::string& suffix)
{
  if (s.size() < suffix.size()) return false;
  return (s.substr(s.size() - suffix.size()) == suffix);
}

/**********************************************************************
 * EXPORTED API
 **********************************************************************/

SolverParams::SolverParams(void)
{
  strategy = S_FRTDP;
  probType = -1;
  valueRepr = -1;
  probName = NULL;
  targetPrecision = 1e-3;
  useFastParser = false;
  useHeuristic = true;
  forceLowerBound = false;
  forceUpperBoundActionSelection = false;
}

void SolverParams::setStrategy(const char* strategyName)
{
  strategy = getEnum(strategyName, strategiesG, cmdName, "--search");
}

void SolverParams::setProbType(const char* probTypeName)
{
  probType = getEnum(probTypeName, probTypesG, cmdName, "--type");
}

void SolverParams::setValueRepr(const char* valueReprName)
{
  valueRepr = getEnum(valueReprName, valueReprsG, cmdName, "--value");
}

void SolverParams::inferMissingValues(void)
{
  // fill in default and inferred values
  if (-1 == probType) {
    if (endsWith(probName, ".racetrack")) {
      probType = T_RACETRACK;
    } else if (endsWith(probName, ".pomdp")) {
      probType = T_POMDP;
    } else {
      fprintf(stderr, "ERROR: couldn't infer problem type from problem filename %s (use --type option)\n\n",
	      probName);
      usage(cmdName);
      exit(EXIT_FAILURE);
    }
  }
  if (-1 == valueRepr) {
    if (T_POMDP == probType) {
      valueRepr = V_CONVEX;
    } else {
      valueRepr = V_POINT;
    }
  }

  // error check
  if (V_CONVEX == valueRepr && T_POMDP != probType) {
    fprintf(stderr, "ERROR: '-v convex' can only be used with '-t pomdp'\n\n");
    usage(cmdName);
  }
}

void constructSolverObjects(SolverObjects& obj, const SolverParams& p)
{
  switch (p.probType) {
  case T_RACETRACK:
    obj.problem = new RaceTrack(p.probName);
    obj.sim = new MDPSim(obj.problem);
    break;
  case T_POMDP:
    obj.problem = new Pomdp(p.probName, p.useFastParser);
    obj.sim = new PomdpSim((Pomdp*) obj.problem);
    break;
  default:
    assert(0); // never reach this point
  }

  switch (p.strategy) {
  case S_RTDP:
    obj.solver = new RTDP();
    break;
  case S_LRTDP:
    obj.solver = new LRTDP();
   break;
  case S_HDP:
    obj.solver = new HDP();
    break;
  case S_FRTDP:
    obj.solver = new FRTDP();
    break;
  case S_HSVI:
    obj.solver = new HSVI();
    break;
  default:
    assert(0); // never reach this point
  };

  bool keepLowerBound = p.forceLowerBound
    || ((RTDPCore *) obj.solver)->getUseLowerBound();
  switch (p.valueRepr) {
  case V_POINT:
    PointBounds* pb;
    AbstractBound *initLowerBound, *initUpperBound;

    pb = new PointBounds();
    initLowerBound = keepLowerBound ? obj.problem->newLowerBound() : NULL;
    if (p.useHeuristic && T_POMDP != p.probType) {
      initUpperBound = new RelaxUBInitializer(obj.problem);
    } else {
      initUpperBound = obj.problem->newUpperBound();
    }
    pb->setBounds(initLowerBound, initUpperBound, p.forceUpperBoundActionSelection);
    obj.bounds = pb;
    break;
  case V_CONVEX:
    obj.bounds = new ConvexBounds(keepLowerBound, p.forceUpperBoundActionSelection);
    break;
  default:
    assert(0); // never reach this point
  };
  ((RTDPCore*) obj.solver)->setBounds(obj.bounds);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/04/27 23:07:54  trey
 * added bounds to SolverObjects data structure
 *
 * Revision 1.1  2006/04/27 20:20:21  trey
 * factored some interface code out of zmdpBenchmark.cc (moved to solverUtils) so it could be easily shared with zmdpSolve.cc
 *
 *
 ***************************************************************************/
