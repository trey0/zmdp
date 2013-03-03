/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
 @file    BlindLBInitializer.h
 @brief   No brief

 Copyright (c) 2005, Trey Smith. All rights reserved.

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

#ifndef INCBlindLBInitializer_h
#define INCBlindLBInitializer_h

#include "MatrixUtils.h"
#include "AbstractBound.h"
#include "Pomdp.h"
#include "MaxPlanesLowerBound.h"

namespace zmdp {

struct BlindLBInitializer {
  const Pomdp* pomdp;
  MaxPlanesLowerBound* bound;

  BlindLBInitializer(const MDP* _pomdp, MaxPlanesLowerBound* _bound);
  void initialize(double targetPrecision);

protected:
  void initBlind(double targetPrecision);
  void initBlindWorstCase(alpha_vector& weakAlpha);
};

}; // namespace zmdp

#endif /* INCBlindLBInitializer_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/04/05 21:43:20  trey
 * collected and renamed several classes into pomdpBounds
 *
 * Revision 1.4  2006/02/14 19:33:55  trey
 * added targetPrecision argument for bounds initialization
 *
 * Revision 1.3  2006/02/01 01:09:38  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.2  2006/01/31 20:13:45  trey
 * changed when MDP* arguments are passed into bounds initialization
 *
 * Revision 1.1  2006/01/31 19:18:24  trey
 * initial check-in
 *
 *
 ***************************************************************************/
