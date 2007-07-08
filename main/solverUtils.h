/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.18 $  $Author: trey $  $Date: 2007-07-08 20:59:30 $
   
 @file    solverUtils.h
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

#ifndef INCsolverUtils_h
#define INCsolverUtils_h

#include <iostream>

#include "zmdpConfig.h"

// search strategies
#include "FRTDP.h"
#include "HSVI.h"
#include "RTDP.h"
#include "LRTDP.h"
#include "HDP.h"
#include "ScriptedUpdater.h"

// problem types
#include "Pomdp.h"
#include "GenericDiscreteMDP.h"
#include "RaceTrack.h"
#include "CustomMDP.h"
#include "MDPSim.h"

// value function representations
#include "PointLowerBound.h"
#include "PointUpperBound.h"
#include "MaxPlanesLowerBound.h"
#include "SawtoothUpperBound.h"
#include "BoundPair.h"

// initialization code
#include "RelaxUBInitializer.h"

namespace zmdp {

struct EnumEntry {
  const char* key;
  int val;
};

enum StrategiesEnum {
  S_FRTDP,
  S_HSVI,
  S_RTDP,
  S_LRTDP,
  S_HDP,
  S_SCRIPT
};

enum ProbTypesEnum {
  T_POMDP,
  T_MDP,
  T_RACETRACK,
  T_CUSTOM
};

enum ValueReprsEnum {
  V_POINT,
  V_MAXPLANES,
  V_SAWTOOTH
};

struct SolverParams {
  const char* binaryName;
  const char* probName;
  bool usingBenchmarkFrontEnd;

  int searchStrategy;
  int modelType;
  int lowerBoundRepresentation;
  int upperBoundRepresentation;
  int maintainLowerBound;
  bool maintainUpperBound;
  const char* policyOutputFile;
  bool useFastModelParser;
  double terminateRegretBound;
  double terminateWallclockSeconds;
  int maxHorizon;
  bool useWeakUpperBoundHeuristic;
  int useUpperBoundRunTimeActionSelection;
  int evaluationTrialsPerEpoch;
  int evaluationMaxStepsPerTrial;
  double evaluationFirstEpochWallclockSeconds;
  double evaluationEpochsPerMagnitude;
  const char* evaluationOutputFile;
  const char* boundsOutputFile;
  const char* simulationTraceOutputFile;

  SolverParams(void);
  void setValues(const ZMDPConfig& config);
  void inferMissingValues(void);
};

struct SolverObjects {
  Solver* solver;
  BoundPair* bounds;
  MDP* problem;
  MDPSim* sim;
};

void constructSolverObjects(SolverObjects& obj,
			    SolverParams& p,
			    const ZMDPConfig& config);

}; // namespace zmdp

#endif // INCsolverUtils_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.17  2007/03/24 22:43:10  trey
 * now use MDPSim rather than abstract parent class AbstractSim
 *
 * Revision 1.16  2007/03/22 18:24:23  trey
 * now use MDPSim in all cases; PomdpSim no longer needed
 *
 * Revision 1.15  2006/11/09 20:49:26  trey
 * added modelType="mdp"
 *
 * Revision 1.14  2006/11/08 16:35:28  trey
 * renamed useFastPomdpParser to useFastModelParser; Pomdp constructor now takes different arguments
 *
 * Revision 1.13  2006/11/07 20:08:28  trey
 * added support for custom problem type
 *
 * Revision 1.12  2006/10/24 19:08:04  trey
 * bug fixes related to recent changes in available parameters
 *
 * Revision 1.11  2006/10/24 02:10:01  trey
 * major changes to permit more flexibility in matching different lower and upper bounds
 *
 * Revision 1.10  2006/10/20 04:59:18  trey
 * made some config options more flexible
 *
 * Revision 1.9  2006/10/18 18:05:56  trey
 * now propagating config data structure to lower levels so config fields can be used to control more parts of the system
 *
 * Revision 1.8  2006/10/16 05:48:19  trey
 * moved BenchmarkParams fields into SolverParams
 *
 * Revision 1.7  2006/10/15 23:45:30  trey
 * switched to new config mechanism
 *
 * Revision 1.6  2006/10/03 03:17:08  trey
 * added maxHorizon parameter
 *
 * Revision 1.5  2006/06/15 16:10:01  trey
 * restructured so zmdpBenchmark can output policies
 *
 * Revision 1.4  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.3  2006/04/27 23:18:48  trey
 * removed unnecessary #include of Interleave.h
 *
 * Revision 1.2  2006/04/27 23:07:54  trey
 * added bounds to SolverObjects data structure
 *
 * Revision 1.1  2006/04/27 20:20:21  trey
 * factored some interface code out of zmdpBenchmark.cc (moved to solverUtils) so it could be easily shared with zmdpSolve.cc
 *
 *
 ***************************************************************************/
