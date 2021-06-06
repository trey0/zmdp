/********** tell emacs we use -*- c++ -*- style comments *******************
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
#include "HDP.h"
#include "HSVI.h"
#include "LRTDP.h"
#include "RTDP.h"
#include "ScriptedUpdater.h"

// problem types
#include "CustomMDP.h"
#include "GenericDiscreteMDP.h"
#include "MDPSim.h"
#include "Pomdp.h"
#include "RaceTrack.h"

// value function representations
#include "BoundPair.h"
#include "MaxPlanesLowerBound.h"
#include "PointLowerBound.h"
#include "PointUpperBound.h"
#include "SawtoothUpperBound.h"

// initialization code
#include "RelaxUBInitializer.h"

namespace zmdp {

struct EnumEntry {
  const char *key;
  int val;
};

enum StrategiesEnum { S_FRTDP, S_HSVI, S_RTDP, S_LRTDP, S_HDP, S_SCRIPT };

enum ProbTypesEnum { T_POMDP, T_MDP, T_RACETRACK, T_CUSTOM };

enum ValueReprsEnum { V_POINT, V_MAXPLANES, V_SAWTOOTH };

struct SolverParams {
  const char *binaryName;
  const char *probName;
  bool usingBenchmarkFrontEnd;

  int searchStrategy;
  int modelType;
  int lowerBoundRepresentation;
  int upperBoundRepresentation;
  int maintainLowerBound;
  bool maintainUpperBound;
  const char *policyOutputFile;
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
  const char *evaluationOutputFile;
  const char *boundsOutputFile;
  const char *simulationTraceOutputFile;

  SolverParams(void);
  void setValues(const ZMDPConfig &config);
  void inferMissingValues(void);
};

struct SolverObjects {
  Solver *solver;
  BoundPair *bounds;
  MDP *problem;
  MDPSim *sim;
};

void constructSolverObjects(SolverObjects &obj, SolverParams &p,
                            const ZMDPConfig &config);

}; // namespace zmdp

#endif // INCsolverUtils_h
