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

#ifndef INCFastInfUBInitializer_h
#define INCFastInfUBInitializer_h

#include "MatrixUtils.h"
#include "Pomdp.h"
#include "SawtoothUpperBound.h"

namespace zmdp {

struct FastInfUBInitializer {
  const Pomdp *pomdp;
  SawtoothUpperBound *bound;
  std::vector<alpha_vector> alphas;

  FastInfUBInitializer(const MDP *problem, SawtoothUpperBound *_bound);
  void initialize(double targetPrecision);

protected:
  void initMDP(double targetPrecision);
  void initFIB(double targetPrecision);
};

}; // namespace zmdp

#endif /* INCFastInfUBInitializer_h */
