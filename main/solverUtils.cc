/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.11 $  $Author: trey $  $Date: 2006-10-20 20:00:01 $

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
  {"frtdp",  S_FRTDP},
  {"hsvi",   S_HSVI},
  {"rtdp",   S_RTDP},
  {"lrtdp",  S_LRTDP},
  {"hdp",    S_HDP},
  {"script", S_SCRIPT},
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

static EnumEntry maintainValsG[] = {
  {"-", -1},
  {"0", 0},
  {"1", 1},
  {NULL, -1}
};

static EnumEntry actionSelValsG[] = {
  {"-",     -1},
  {"lower", 0},
  {"upper", 1},
  {NULL, -1}
};

int getEnum(const std::string& key, EnumEntry* table, const char* cmdName, const char *opt)
{
  EnumEntry* i;
  for (i = table; NULL != i->key; i++) {
    if (key == i->key) {
      return i->val;
    }
  }
  fprintf(stderr, "ERROR: invalid value %s for config field %s, expected one of ",
	  key.c_str(), opt);
  for (i = table; NULL != i->key; i++) {
    fprintf(stderr, "'%s' ", i->key);
  }
  fprintf(stderr, "\n");
  
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
  maintainLowerBound = getEnum(config.getString("maintainLowerBound"),
			       maintainValsG, cmdName, "maintainLowerBound");
  maintainUpperBound = config.getBool("maintainUpperBound");
  useUpperBoundRunTimeActionSelection =
    getEnum(config.getString("runTimeActionSelection"),
	    actionSelValsG, cmdName, "runTimeActionSelection");
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
      fprintf(stderr, "ERROR: couldn't infer problem type from model filename %s (use -t option, -h for help)\n",
	      probName);
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
    fprintf(stderr, "ERROR: '-v convex' can only be used with '-t pomdp' (-h for help)\n");
    exit(EXIT_FAILURE);
  }
}

void constructSolverObjects(SolverObjects& obj,
			    SolverParams& p,
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

  bool lowerBoundRequired;
  bool upperBoundRequired;

  switch (p.searchStrategy) {
  case S_FRTDP:
    obj.solver = new FRTDP();
    lowerBoundRequired = true;
    upperBoundRequired = true;
    break;
  case S_HSVI:
    obj.solver = new HSVI();
    lowerBoundRequired = true;
    upperBoundRequired = true;
    break;
  case S_RTDP:
    obj.solver = new RTDP();
    lowerBoundRequired = false;
    upperBoundRequired = true;
    break;
  case S_LRTDP:
    obj.solver = new LRTDP();
    lowerBoundRequired = false;
    upperBoundRequired = true;
   break;
  case S_HDP:
    obj.solver = new HDP();
    lowerBoundRequired = false;
    upperBoundRequired = true;
    break;
  case S_SCRIPT:
    obj.solver = new ScriptedUpdater();
    lowerBoundRequired = false;
    upperBoundRequired = false;
    break;
  default:
    assert(0); // never reach this point
  };

  switch (p.maintainLowerBound) {
  case -1:
    p.maintainLowerBound = lowerBoundRequired;
    break;
  case 0:
    if (lowerBoundRequired) {
      fprintf(stderr, "ERROR: maintainLowerBound=0 is incompatible with searchStrategy='%s' (-h for help)\n",
	      config.getString("searchStrategy").c_str());
      exit(EXIT_FAILURE);
    }
    break;
  case 1:
    // keep current value
    break;
  default:
    assert(0); // never reach this point
  }

  if (!p.maintainUpperBound && upperBoundRequired) {
    fprintf(stderr, "ERROR: maintainUpperBound=0 is incompatible with searchStrategy='%s' (-h for help)\n",
	    config.getString("searchStrategy").c_str());
    exit(EXIT_FAILURE);
  }

  if (! (p.maintainLowerBound || p.maintainUpperBound)) {
    fprintf(stderr, "ERROR: at least one bound must be maintained, lower or upper (-h for help)\n");
    exit(EXIT_FAILURE);
  }

  switch (p.useUpperBoundRunTimeActionSelection) {
  case -1:
    p.useUpperBoundRunTimeActionSelection = !p.maintainLowerBound;
    break;
  case 0:
    if (!p.maintainLowerBound) {
      fprintf(stderr, "ERROR: cannot specify runTimeActionSelection='lower' if lower bound is not maintained (-h for help)\n");
      exit(EXIT_FAILURE);
    }
    break;
  case 1:
    if (!p.maintainUpperBound) {
      fprintf(stderr, "ERROR: cannot specify runTimeActionSelection='upper' if upper bound is not maintained (-h for help)\n");
      exit(EXIT_FAILURE);
    }
    break;
  default:
    assert(0); // never reach this point
  }

  switch (p.valueFunctionRepresentation) {
  case V_POINT:
    PointBounds* pb;
    AbstractBound *initLowerBound, *initUpperBound;

    pb = new PointBounds(p.maintainLowerBound,
			 p.maintainUpperBound,
			 p.useUpperBoundRunTimeActionSelection);
    if (p.maintainLowerBound) {
      initLowerBound = obj.problem->newLowerBound(config);
    } else {
      initLowerBound = NULL;
    }
    if (p.maintainUpperBound) {
      if (p.useWeakUpperBoundHeuristic && T_POMDP == p.modelType) {
	initUpperBound = obj.problem->newUpperBound(config);
      } else {
	initUpperBound = new RelaxUBInitializer(obj.problem, config);
      }
    } else {
      initUpperBound = NULL;
    }
    pb->setBounds(initLowerBound, initUpperBound);
    obj.bounds = pb;
    break;
  case V_CONVEX:
    obj.bounds = new ConvexBounds(p.maintainLowerBound,
				  p.maintainUpperBound,
				  p.useUpperBoundRunTimeActionSelection);
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
 * Revision 1.10  2006/10/20 04:59:18  trey
 * made some config options more flexible
 *
 * Revision 1.9  2006/10/18 18:05:56  trey
 * now propagating config data structure to lower levels so config fields can be used to control more parts of the system
 *
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
