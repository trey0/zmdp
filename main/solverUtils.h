/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-04-27 20:20:21 $
   
 @file    solverUtils.h
 @brief   No brief

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

#ifndef INCsolverUtils_h
#define INCsolverUtils_h

#include <iostream>

#include "Interleave.h"

// search strategies
#include "RTDP.h"
#include "LRTDP.h"
#include "HDP.h"
#include "FRTDP.h"
#include "HSVI.h"

// problem types
#include "RaceTrack.h"
#include "Pomdp.h"
#include "MDPSim.h"
#include "PomdpSim.h"

// value function representations
#include "PointBounds.h"
#include "ConvexBounds.h"

// initialization code
#include "RelaxUBInitializer.h"

namespace zmdp {

struct EnumEntry {
  const char* key;
  int val;
};

enum StrategiesEnum {
  S_RTDP,
  S_LRTDP,
  S_HDP,
  S_HSVI,
  S_FRTDP
};

enum ProbTypesEnum {
  T_RACETRACK,
  T_POMDP
};

enum ValueReprsEnum {
  V_POINT,
  V_CONVEX
};

struct SolverParams {
  const char* cmdName;
  int strategy;
  int probType;
  int valueRepr;
  const char *probName;
  double targetPrecision;
  bool useFastParser;
  bool useHeuristic;
  bool forceLowerBound;
  bool forceUpperBoundActionSelection;

  SolverParams(void);
  void setStrategy(const char* strategyName);
  void setProbType(const char* probTypeName);
  void setValueRepr(const char* valueReprName);
  void inferMissingValues(void);
};

struct SolverObjects {
  Solver* solver;
  MDP* problem;
  AbstractSim* sim;
};

void constructSolverObjects(SolverObjects& obj, const SolverParams& p);

}; // namespace zmdp

#endif // INCsolverUtils_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
