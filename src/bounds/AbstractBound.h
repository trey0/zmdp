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

#ifndef INCAbstractBound_h
#define INCAbstractBound_h

#include <iostream>
#include <string>
#include <vector>

#include "MDPCache.h"

using namespace sla;

namespace zmdp {

#define ZMDP_S_NUM_ELTS (0)
#define ZMDP_S_NUM_ENTRIES (1)
#define ZMDP_S_NUM_ELTS_TABULAR (2)
#define ZMDP_S_NUM_ENTRIES_TABULAR (3)

struct AbstractBound {
  virtual ~AbstractBound(void) {}

  // performs any computation necessary to initialize the bound
  virtual void initialize(double targetPrecision) = 0;

  // returns the bound value at state s
  virtual double getValue(const state_vector &s, const MDPNode *cn) const = 0;

  // returns amount of storage space used by data structure.
  // whichMetric selects which metric to return (see ZMDP_S_* macros
  // above).  we don't require all derived classes to have a method for
  // tracking storage -- it's only implemented for the bounds
  // representations we really care about.
  virtual int getStorage(int whichMetric) const { return 0; }
};

}; // namespace zmdp

#endif // INCAbstractBound_h
