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

#ifndef INCValueFunction_h
#define INCValueFunction_h

/**********************************************************************
 * INCLUDES
 **********************************************************************/

// this causes problems if it is included after the Lapack headers, so
//  pre-emptively include it here.  not sure exactly what the problem is.
#include <iostream>

#include <ext/hash_map>
#include <string>
#include <vector>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"

/**********************************************************************
 * CLASSES
 **********************************************************************/

namespace zmdp {

class ValueFunction {
public:
  int numStates;

  ValueFunction(void) : numStates(-1) {}
  virtual ~ValueFunction(void) {}

  virtual ValueInterval getValueAt(const belief_vector &b) const = 0;
  bool consistentWith(const ValueFunction &rhs, int numSamples,
                      bool debug = false) const;
};

}; // namespace zmdp

#endif // INCValueFunction_h
