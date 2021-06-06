/********** tell emacs we use -*- c++ -*- style comments *******************
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

#include "AbstractBound.h"
#include "MatrixUtils.h"
#include "MaxPlanesLowerBound.h"
#include "Pomdp.h"

namespace zmdp {

struct BlindLBInitializer {
  const Pomdp *pomdp;
  MaxPlanesLowerBound *bound;

  BlindLBInitializer(const MDP *_pomdp, MaxPlanesLowerBound *_bound);
  void initialize(double targetPrecision);

protected:
  void initBlind(double targetPrecision);
  void initBlindWorstCase(alpha_vector &weakAlpha);
};

}; // namespace zmdp

#endif /* INCBlindLBInitializer_h */
