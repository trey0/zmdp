/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.27 $  $Author: trey $  $Date: 2007-07-08 20:59:30 $

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

extern void usage(const char* binaryName);

namespace zmdp {

/**********************************************************************
 * HELPER FUNCTIONS
 **********************************************************************/

static EnumEntry searchStrategyTableG[] = {
  {"frtdp",  S_FRTDP},
  {"hsvi",   S_HSVI},
  {"rtdp",   S_RTDP},
  {"lrtdp",  S_LRTDP},
  {"hdp",    S_HDP},
  {"script", S_SCRIPT},
  {NULL, -1}
};

static EnumEntry modelTypeTableG[] = {
  {"-",         -1},
  {"pomdp",     T_POMDP},
  {"mdp",       T_MDP},
  {"racetrack", T_RACETRACK},
  {"custom",    T_CUSTOM},
  {NULL, -1}
};

static EnumEntry lowerBoundRepresentationTableG[] = {
  {"-",      -1},
  {"point",  V_POINT},
  {"convex", V_MAXPLANES},
  {NULL, -1}
};

static EnumEntry upperBoundRepresentationTableG[] = {
  {"-",        -1},
  {"point",    V_POINT},
  {"sawtooth", V_SAWTOOTH},
  {NULL, -1}
};

static EnumEntry maintainLowerBoundTableG[] = {
  {"-", -1},
  {"0", 0},
  {"1", 1},
  {NULL, -1}
};

static EnumEntry runTimeActionSelectionTableG[] = {
  {"-",     -1},
  {"lower", 0},
  {"upper", 1},
  {NULL, -1}
};

int getEnum(const std::string& key, EnumEntry* table, const char *opt)
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
  fprintf(stderr, "(-h for help)\n");
  
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

SolverParams::SolverParams(void) :
  binaryName(NULL),
  probName(NULL),
  usingBenchmarkFrontEnd(false)
{}

#define SU_GET_ENUM(FIELD) \
  FIELD = getEnum(config.getString(#FIELD), FIELD##TableG, #FIELD);
#define SU_GET_BOOL(FIELD) \
  FIELD = config.getBool(#FIELD);
#define SU_GET_DOUBLE(FIELD) \
  FIELD = config.getDouble(#FIELD);
#define SU_GET_STRING(FIELD) \
  FIELD = config.getString(#FIELD).c_str();
#define SU_GET_INT(FIELD) \
  FIELD = config.getInt(#FIELD);

void SolverParams::setValues(const ZMDPConfig& config)
{
  binaryName = config.getString("binaryName").c_str();
  probName = config.getString("simulatorModel").c_str();

  // set global debug level, used by parts of the codebase that do not
  // depend on ZMDPConfig
  zmdpDebugLevelG = config.getInt("debugLevel");

  if (zmdpDebugLevelG >= 1) {
    cout << "[params begin]" << endl;
    config.writeToStream(cout);
    cout << "[params end]" << endl;
  }

  SU_GET_ENUM(searchStrategy);
  SU_GET_ENUM(modelType);
  SU_GET_ENUM(lowerBoundRepresentation);
  SU_GET_ENUM(upperBoundRepresentation);
  SU_GET_ENUM(maintainLowerBound);
  SU_GET_BOOL(maintainUpperBound);

  SU_GET_STRING(policyOutputFile);

  SU_GET_BOOL(useFastModelParser);
  SU_GET_DOUBLE(terminateRegretBound);

  SU_GET_DOUBLE(terminateWallclockSeconds);
  if (terminateWallclockSeconds <= 0.0) {
    terminateWallclockSeconds = 99e+20;
  }

  SU_GET_INT(maxHorizon);
  SU_GET_BOOL(useWeakUpperBoundHeuristic);
  SU_GET_ENUM(maintainLowerBound);
  useUpperBoundRunTimeActionSelection =
    getEnum(config.getString("runTimeActionSelection"),
	    runTimeActionSelectionTableG, "runTimeActionSelection");

  SU_GET_INT(evaluationTrialsPerEpoch);
  SU_GET_INT(evaluationMaxStepsPerTrial);
  SU_GET_DOUBLE(evaluationFirstEpochWallclockSeconds);
  SU_GET_DOUBLE(evaluationEpochsPerMagnitude);
  SU_GET_STRING(evaluationOutputFile);
  SU_GET_STRING(boundsOutputFile);
  SU_GET_STRING(simulationTraceOutputFile);

  // if the probName is NULL, we are not solving a planning problem
  // (e.g., we are running zmdpEvaluate).  skip filling in missing
  // solver parameters.
  if (NULL != probName) {
    inferMissingValues();
  }
}

void SolverParams::inferMissingValues(void)
{
  // fill in default and inferred values
  if (-1 == modelType) {
    if (endsWith(probName, ".pomdp")) {
      if (zmdpDebugLevelG >= 1) {
	printf("[params] inferred modelType='pomdp' from model filename extension\n");
      }
      modelType = T_POMDP;
    } else if (endsWith(probName, ".mdp")) {
      if (zmdpDebugLevelG >= 1) {
	printf("[params] inferred modelType='mdp' from model filename extension\n");
      }
      modelType = T_MDP;
    } else if (endsWith(probName, ".racetrack")) {
      if (zmdpDebugLevelG >= 1) {
	printf("[params] inferred modelType='racetrack' from model filename extension\n");
      }
      modelType = T_RACETRACK;
    } else if (0 == strcmp(probName, "custom")) {
      if (zmdpDebugLevelG >= 1) {
	printf("[params] inferred modelType='custom' from model filename\n");
      }
      modelType = T_CUSTOM;
    } else {
      fprintf(stderr, "ERROR: couldn't infer problem type from model filename %s (use -t option, -h for help)\n",
	      probName);
      exit(EXIT_FAILURE);
    }
  }
  if (-1 == lowerBoundRepresentation) {
    if (T_POMDP == modelType) {
      lowerBoundRepresentation = V_MAXPLANES;
      if (zmdpDebugLevelG >= 1) {
	printf("[params] selected lowerBoundRepresentation='maxPlanes' because modelType='pomdp'\n");
      }
    } else {
      lowerBoundRepresentation = V_POINT;
      if (zmdpDebugLevelG >= 1) {
	printf("[params] selected lowerBoundRepresentation='point' because model is an MDP\n");
      }
    }
  }
  if (-1 == upperBoundRepresentation) {
    if (T_POMDP == modelType) {
      upperBoundRepresentation = V_SAWTOOTH;
      if (zmdpDebugLevelG >= 1) {
	printf("[params] selected upperBoundRepresentation='sawtooth' because modelType='pomdp'\n");
      }
    } else {
      upperBoundRepresentation = V_POINT;
      if (zmdpDebugLevelG >= 1) {
	printf("[params] selected upperBoundRepresentation='point' because model is an MDP\n");
      }
    }
  }

  // error check
  if (V_MAXPLANES == lowerBoundRepresentation && T_POMDP != modelType) {
    fprintf(stderr, "ERROR: lowerBoundRepresentation='maxPlanes' requires modelType='pomdp' (-h for help)\n");
    exit(EXIT_FAILURE);
  }
  if (V_SAWTOOTH == upperBoundRepresentation && T_POMDP != modelType) {
    fprintf(stderr, "ERROR: upperBoundRepresentation='sawtooth' requires modelType='pomdp' (-h for help)\n");
    exit(EXIT_FAILURE);
  }

  if (NULL != policyOutputFile && 0 == strcmp(policyOutputFile, "none")) {
    policyOutputFile = NULL;
  }
  if (NULL != policyOutputFile && 0 == strcmp(policyOutputFile, "-")) {
    if (usingBenchmarkFrontEnd) {
      policyOutputFile = NULL;
      if (zmdpDebugLevelG >= 1) {
	printf("[params] selected policyOutputFile='none' because front end is zmdp benchmark\n");
      }
    } else {
      policyOutputFile = "out.policy";
      if (zmdpDebugLevelG >= 1) {
	printf("[params] selected policyOutputFile='%s' because front end is zmdp solve\n",
	       policyOutputFile);
      }
    }
  }
}

void constructSolverObjects(SolverObjects& obj,
			    SolverParams& p,
			    const ZMDPConfig& config)
{
  switch (p.modelType) {
  case T_POMDP:
    obj.problem = new Pomdp(p.probName, &config);
    break;
  case T_MDP:
    obj.problem = new GenericDiscreteMDP(p.probName, &config);
    break;
  case T_RACETRACK:
    obj.problem = new RaceTrack(p.probName);
    break;
  case T_CUSTOM:
    obj.problem = new CustomMDP(config);
    break;
  default:
    assert(0); // never reach this point
  }
  obj.sim = new MDPSim(obj.problem);

  if (T_POMDP == p.modelType && obj.problem->getDiscount() >= 1.0 && -1 == p.maxHorizon) {
    cerr << "ERROR: with modelType='pomdp', you must have either discount < 1\n"
	 << "  or maxHorizon defined (i.e. non-negative) in order to generate initial\n"
	 << "  bounds (-h for help)" << endl;
    exit(EXIT_FAILURE);
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

  if (zmdpDebugLevelG >= 1) {
    printf("[params] with requested searchStrategy, lowerBoundRequired=%d, upperBoundRequired=%d\n",
	   lowerBoundRequired, upperBoundRequired);
  }

  switch (p.maintainLowerBound) {
  case -1:
    p.maintainLowerBound = lowerBoundRequired;
    if (zmdpDebugLevelG >= 1) {
      printf("[params] selected maintainLowerBound=%d because lowerBoundRequired=%d\n",
	     p.maintainLowerBound, lowerBoundRequired);
    }
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
    if (zmdpDebugLevelG >= 1) {
      printf("[params] selected useUpperBoundRunTimeActionSelection=%d because maintainLowerBound=%d\n",
	     p.useUpperBoundRunTimeActionSelection, p.maintainLowerBound);
    }
    break;
  case 0:
    if (!p.maintainLowerBound) {
      fprintf(stderr, "ERROR: runTimeActionSelection='lower' incompatible with maintainLowerBound=0 (-h for help)\n");
      exit(EXIT_FAILURE);
    }
    break;
  case 1:
    if (!p.maintainUpperBound) {
      fprintf(stderr, "ERROR: runTimeActionSelection='upper' incompatible with maintainUpperBound=0 (-h for help)\n");
      exit(EXIT_FAILURE);
    }
    break;
  default:
    assert(0); // never reach this point
  }

  if (NULL != p.policyOutputFile) {
    if (! (p.maintainLowerBound && (p.lowerBoundRepresentation == V_MAXPLANES))) {
      cerr << "WARNING: selected options indicate an output policy should be written, but\n"
	   << "  policy output is currently only supported when modelType='pomdp',\n"
	   << "  lowerBoundRepresentation='maxPlanes', and lower bound is maintained;\n"
	   << "  disabling policy output on this run"
	   << endl;
      p.policyOutputFile = NULL;
    }
  }

  bool dualPointBounds =
    p.maintainLowerBound && p.maintainUpperBound &&
    (V_POINT == p.lowerBoundRepresentation) &&
    (V_POINT == p.upperBoundRepresentation);
    
  obj.bounds = new BoundPair(p.maintainLowerBound,
			     p.maintainUpperBound,
			     p.useUpperBoundRunTimeActionSelection,
			     dualPointBounds);
  
  PointLowerBound* plb;
  MaxPlanesLowerBound* mlb;
  if (p.maintainLowerBound) {
    switch (p.lowerBoundRepresentation) {
    case V_POINT:
      plb = new PointLowerBound(obj.problem, &config, obj.bounds);
      plb->initBound = obj.problem->newLowerBound(&config);
      obj.bounds->lowerBound = plb;
    break;
    case V_MAXPLANES:
      mlb = new MaxPlanesLowerBound(obj.problem, &config);
      mlb->core = obj.bounds;
      obj.bounds->lowerBound = mlb;
      break;
    default:
      assert(0); // never reach this point
    }
  }

  PointUpperBound* pub;
  SawtoothUpperBound* sub;
  if (p.maintainUpperBound) {
    switch (p.upperBoundRepresentation) {
    case V_POINT:
      pub = new PointUpperBound(obj.problem, &config, obj.bounds);
      if (T_RACETRACK == p.modelType && !p.useWeakUpperBoundHeuristic) {
	// stronger upper bound strategy for racetrack MDP
	pub->initBound = new RelaxUBInitializer(obj.problem, &config);
      } else {
	// default upper bound strategy, depends on the MDP type
	pub->initBound = obj.problem->newUpperBound(&config);
      }
      obj.bounds->upperBound = pub;
    break;
    case V_SAWTOOTH:
      sub = new SawtoothUpperBound(obj.problem, &config);
      sub->core = obj.bounds;
      obj.bounds->upperBound = sub;
      break;
    default:
      assert(0); // never reach this point
    }
  }
  ((RTDPCore*) obj.solver)->setBounds(obj.bounds);
}

}; // namespace zmdp

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.26  2007/03/25 21:39:32  trey
 * added conditions for disabling policy output to avoid crashing when the wrong options are specified
 *
 * Revision 1.25  2007/03/22 18:24:23  trey
 * now use MDPSim in all cases; PomdpSim no longer needed
 *
 * Revision 1.24  2007/01/15 17:25:24  trey
 * fixed logic to ensure that RelaxUpperBound is not used for POMDPs
 *
 * Revision 1.23  2007/01/14 00:53:51  trey
 * added ability to log storage space during a run
 *
 * Revision 1.22  2006/11/13 15:09:28  trey
 * clarified warning message
 *
 * Revision 1.21  2006/11/09 20:49:26  trey
 * added modelType="mdp"
 *
 * Revision 1.20  2006/11/08 16:35:28  trey
 * renamed useFastPomdpParser to useFastModelParser; Pomdp constructor now takes different arguments
 *
 * Revision 1.19  2006/11/07 20:08:29  trey
 * added support for custom problem type
 *
 * Revision 1.18  2006/10/30 20:26:56  trey
 * fixed annoying spelling error
 *
 * Revision 1.17  2006/10/30 20:22:44  trey
 * added lots of debug output when debugLevel=1
 *
 * Revision 1.16  2006/10/30 20:00:15  trey
 * USE_DEBUG_PRINT replaced with a run-time config parameter "debugLevel"
 *
 * Revision 1.15  2006/10/25 18:30:59  trey
 * added a more informative error message when params are set wrong for pomdp initial bounds to be calculated
 *
 * Revision 1.14  2006/10/25 02:07:06  trey
 * added guards in case policyOutputFile==NULL
 *
 * Revision 1.13  2006/10/24 19:08:04  trey
 * bug fixes related to recent changes in available parameters
 *
 * Revision 1.12  2006/10/24 02:10:01  trey
 * major changes to permit more flexibility in matching different lower and upper bounds
 *
 * Revision 1.11  2006/10/20 20:00:01  trey
 * added more flexibility as to which bounds are maintained with -v point (lower or upper or both)
 *
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
