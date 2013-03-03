/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2007-01-14 00:53:30 $
   
 @file    PointUpperBound.h
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

#ifndef INCPointUpperBound_h
#define INCPointUpperBound_h

/**********************************************************************
 * INCLUDES
 **********************************************************************/

#include <string>
#include <vector>
#include <list>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"
#include "zmdpConfig.h"
#include "MDPModel.h"
#include "IncrementalUpperBound.h"
#include "BoundPairCore.h"

/**********************************************************************
 * CLASSES
 **********************************************************************/

namespace zmdp {

struct PointUpperBound : public IncrementalUpperBound {
  const MDP* problem;
  const ZMDPConfig* config;
  BoundPairCore* core;
  AbstractBound* initBound;
  double targetPrecision;

  PointUpperBound(const MDP* _pomdp,
		  const ZMDPConfig* _config,
		  BoundPairCore* _core);
  ~PointUpperBound(void);

  void initialize(double _targetPrecision);
  double getValue(const state_vector& s,
		  const MDPNode* cn) const;
  double getNewUBValueQ(MDPNode& cn, int a);
  void initNodeBound(MDPNode& cn);
  void updateSimple(MDPNode& cn, int* maxUBActionP);
  void updateUseCache(MDPNode& cn, int* maxUBActionP);
  void update(MDPNode& cn, int* maxUBActionP);
  int getStorage(int whichMetric) const;
};

}; // namespace zmdp

#endif // INCPointUpperBound_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2007/01/13 00:42:02  trey
 * added upper bound caching like sawtooth
 *
 * Revision 1.2  2006/11/08 16:32:28  trey
 * renamed MDP.h to MDPModel.h to avoid confusion with mdp.h, stupid case insensitive Mac OS X
 *
 * Revision 1.1  2006/10/24 02:06:16  trey
 * initial check-in
 *
 *
 ***************************************************************************/
