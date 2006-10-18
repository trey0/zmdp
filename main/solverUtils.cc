/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.9 $  $Author: trey $  $Date: 2006-10-18 18:05:56 $

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

static EnumEntry modelTypesG[] = {
  {"-",         -1},
  {"racetrack", T_RACETRACK},
  {"pomdp",     T_POMDP},
  {NULL, -1}
};

static EnumEntry valueReprsG[] = {
  {"-",      -1},
  {"point",  V_POINT},
  {"convex", V_CONVEX},
  {NULL, -1}
};

int getEnum(const std::string& key, EnumEntry* table, const char* cmdName, const char *opt)
{
  EnumEntry* i = table;
  for (; NULL != i->key; i++) {
    if (key == i->key) {
      return i->val;
    }
  }
  fprintf(stderr, "ERROR: invalid value %s for config field %s\n\n", key.c_str(), opt);
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
  cmdName = NULL;
  probName = NULL;
}

void SolverParams::setValues(const ZMDPConfig& config)
{
  searchStrategy = getEnum(config.getString("searchStrategy"),
			   strategiesG, cmdName, "searchStrategy");
  modelType = getEnum(config.getString("modelType"),
		      modelTypesG, cmdName, "modelType");
  valueFunctionRepresentation = getEnum(config.getString("valueFunctionRepresentation"),
					valueReprsG, cmdName, "valueFunctionRepresentation");
  policyOutputFile = config.getString("policyOutputFile").c_str();
  if (0 == strcmp(policyOutputFile, "none")) {
    policyOutputFile = NULL;
  }
  useFastPomdpParser = config.getBool("useFastPomdpParser");
  terminateRegretBound = config.getDouble("terminateRegretBound");
  terminateWallclockSeconds = config.getDouble("terminateWallclockSeconds");
  if (terminateWallclockSeconds <= 0.0) {
    terminateWallclockSeconds = 99e+20;
  }
  maxHorizon = config.getInt("maxHorizon");
  useWeakUpperBoundHeuristic = config.getBool("useWeakUpperBoundHeuristic");
  forceMaintainLowerBound = config.getBool("forceMaintainLowerBound");
  forceUpperBoundRunTimeActionSelection = config.getBool("forceUpperBoundRunTimeActionSelection");
  evaluationTrialsPerEpoch = config.getInt("evaluationTrialsPerEpoch");
  evaluationMaxStepsPerTrial = config.getInt("evaluationMaxStepsPerTrial");
  evaluationFirstEpochWallclockSeconds = config.getDouble("evaluationFirstEpochWallclockSeconds");
  evaluationEpochsPerMagnitude = config.getDouble("evaluationEpochsPerMagnitude");
  evaluationOutputFile = config.getString("evaluationOutputFile").c_str();
  boundsOutputFile = config.getString("boundsOutputFile").c_str();
  simulationTraceOutputFile = config.getString("simulationTraceOutputFile").c_str();

  inferMissingValues();
}

void SolverParams::inferMissingValues(void)
{
  // fill in default and inferred values
  if (-1 == modelType) {
    if (endsWith(probName, ".racetrack")) {
      modelType = T_RACETRACK;
    } else if (endsWith(probName, ".pomdp")) {
      modelType = T_POMDP;
    } else {
      fprintf(stderr, "ERROR: couldn't infer problem type from model filename %s (use --type option)\n\n",
	      probName);
      usage(cmdName);
      exit(EXIT_FAILURE);
    }
  }
  if (-1 == valueFunctionRepresentation) {
    if (T_POMDP == modelType) {
      valueFunctionRepresentation = V_CONVEX;
    } else {
      valueFunctionRepresentation = V_POINT;
    }
  }

  // error check
  if (V_CONVEX == valueFunctionRepresentation && T_POMDP != modelType) {
    fprintf(stderr, "ERROR: '-v convex' can only be used with '-t pomdp'\n\n");
    usage(cmdName);
  }
}

void constructSolverObjects(SolverObjects& obj, const SolverParams& p,
			    const ZMDPConfig& config)
{
  switch (p.modelType) {
  case T_RACETRACK:
    obj.problem = new RaceTrack(p.probName);
    obj.sim = new MDPSim(obj.problem);
    break;
  case T_POMDP:
    obj.problem = new Pomdp(p.probName, p.useFastPomdpParser, p.maxHorizon);
    obj.sim = new PomdpSim((Pomdp*) obj.problem);
    break;
  default:
    assert(0); // never reach this point
  }

  switch (p.searchStrategy) {
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

  bool keepLowerBound = p.forceMaintainLowerBound
    || ((RTDPCore *) obj.solver)->getUseLowerBound();
  switch (p.valueFunctionRepresentation) {
  case V_POINT:
    PointBounds* pb;
    AbstractBound *initLowerBound, *initUpperBound;

    pb = new PointBounds();
    initLowerBound = keepLowerBound ? obj.problem->newLowerBound(config) : NULL;
    if (p.useWeakUpperBoundHeuristic && T_POMDP == p.modelType) {
      initUpperBound = obj.problem->newUpperBound(config);
    } else {
      initUpperBound = new RelaxUBInitializer(obj.problem, config);
    }
    pb->setBounds(initLowerBound, initUpperBound, p.forceUpperBoundRunTimeActionSelection);
    obj.bounds = pb;
    break;
  case V_CONVEX:
    obj.bounds = new ConvexBounds(keepLowerBound, p.forceUpperBoundRunTimeActionSelection);
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
 * Revision 1.8  2006/10/17 19:18:39  trey
 * centralized proper handling of negative terminateWallclockSeconds values
 *
 * Revision 1.7  2006/10/16 05:48:19  trey
 * moved BenchmarkParams fields into SolverParams
 *
 * Revision 1.6  2006/10/15 23:45:31  trey
 * switched to new config mechanism
 *
 * Revision 1.5  2006/10/03 03:17:08  trey
 * added maxHorizon parameter
 *
 * Revision 1.4  2006/06/15 16:10:01  trey
 * restructured so zmdpBenchmark can output policies
 *
 * Revision 1.3  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.2  2006/04/27 23:07:54  trey
 * added bounds to SolverObjects data structure
 *
 * Revision 1.1  2006/04/27 20:20:21  trey
 * factored some interface code out of zmdpBenchmark.cc (moved to solverUtils) so it could be easily shared with zmdpSolve.cc
 *
 *
 ***************************************************************************/
