/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.5 $  $Author: trey $  $Date: 2007-04-03 06:07:33 $
   
 @file    BoundPair.h
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

#ifndef INCBoundPair_h
#define INCBoundPair_h

#include <iostream>
#include <string>
#include <vector>

#include "BoundPairCore.h"
#include "IncrementalLowerBound.h"
#include "IncrementalUpperBound.h"

using namespace sla;

namespace zmdp {

struct BoundPair : public BoundPairCore {
  MDP* problem;
  const ZMDPConfig* config;
  IncrementalLowerBound* lowerBound;
  IncrementalUpperBound* upperBound;
  bool maintainLowerBound;
  bool maintainUpperBound;
  bool useUpperBoundRunTimeActionSelection;
  bool dualPointBounds;
  double targetPrecision;

  BoundPair(bool _maintainLowerBound,
	    bool _maintainUpperBound,
	    bool _useUpperBoundRunTimeActionSelection,
	    bool _dualPointBounds);

  void updateDualPointBounds(MDPNode& cn, int* maxUBActionP);

  void initialize(MDP* _problem,
		  const ZMDPConfig* _config);

  MDPNode* getRootNode(void);
  MDPNode* getNode(const state_vector& s);
  MDPNode* getNodeOrNull(const state_vector& s) const;
  void expand(MDPNode& cn);
  void update(MDPNode& cn, int* maxUBActionP);
  int chooseAction(const state_vector& s) const;
  ValueInterval getValueAt(const state_vector& s) const;
  ValueInterval getQValue(const state_vector& s, int a) const;
  void writePolicy(const std::string& outFileName, bool canModifyBounds);
};

}; // namespace zmdp

#endif // INCBoundPair_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2007/03/24 22:39:27  trey
 * removed const-ness of problem argument
 *
 * Revision 1.3  2006/11/07 20:06:37  trey
 * added getQValue() function
 *
 * Revision 1.2  2006/10/24 19:11:01  trey
 * added writePolicy() from ConvexBounds
 *
 * Revision 1.1  2006/10/24 02:05:48  trey
 * initial check-in
 *
 *
 ***************************************************************************/

