/********** tell emacs we use -*- c++ -*- style comments *******************
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

#ifndef INCFullObsUBInitializer_h
#define INCFullObsUBInitializer_h

#include "Pomdp.h"

#define MDP_MAX_ITERS (1000000)

namespace zmdp {

class FullObsUBInitializer {
public:
  dvector alpha;
  const Pomdp *pomdp;

  void nextAlphaAction(dvector &result, int a);
  double valueIterationOneStep(void);
  void valueIteration(const Pomdp *_pomdp, double eps);
};

}; // namespace zmdp

#endif // INCFullObsUBInitializer_h
